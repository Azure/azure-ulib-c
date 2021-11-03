// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "_az_ulib_pal_flash_driver.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_ipc_function_table.h"
#include "az_ulib_registry_api.h"
#include "az_ulib_result.h"
#include <azure/core/internal/az_precondition_internal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief   Registry single instance.
 *
 * Registry shall be singleton, so we need a static variable to handle the
 * single instance. This variable is initialized in the az_ulib_registry_init() function.
 */
static const az_ulib_registry_control_block* _az_ulib_registry_cb = NULL;

/**
 * @brief   Registry lock to make APIs thread safe.
 */
static az_ulib_pal_os_lock registry_lock;

/**
 * @brief   Key value pair.
 *
 * Structure including two #az_span that each contains a pointer and a size for each key and
 * value to be stored in the flash. Used exclusively in #registry_node.
 */
typedef struct
{
  /** The #az_span that contains a pointer to a location in flash storing the key char string and
   * the string's size.*/
  az_span key;

  /** The #az_span that contains pointer to a location in flash storing the value char string and
   * the string's size.*/
  az_span value;
} registry_key_value_ptrs;

/**
 * @brief   Structure for the registry control node.
 *
 * Every entry into the registry of the device will have a registry control node that contains
 * relevant information to restore the state of the registry after device power cycle.
 */
typedef struct
{
  /** Two flags that shows the status of a node in the registry (ready, deleted). If both flags
   * are set, the node is deleted and cannot be used again. */
  uint64_t ready_flag;

  /** Flag that shows the status of a node in the registry. If this flag is 0xFFFFFFFFFFFFFFFF, the
   * data was not deleted anything else represents a deleted data. */
  uint64_t delete_flag;

  /** The #az_span that contains a pointer to a location in flash storing the key and value chars
   * and size.*/
  registry_key_value_ptrs key_value;

} registry_node;

#define AZ_ULIB_REGISTRY_FLAG_SIZE 8 // in bytes

#define NUMBER_OF_64BITS(x) (x >> 3) + (((x & 0x7) == 0) ? 0 : 1)
#define ROUND_UP_TO_64BITS(x) (((x & 0x7) == 0) ? x : ((x & (~0x7)) + 0x8))

/* registry_node status flags */
#define REGISTRY_FREE 0xFFFFFFFFFFFFFFFF
#define REGISTRY_READY 0x0000000000000000
#define REGISTRY_DELETED 0x0000000000000000

static inline az_result set_registry_node_ready_flag(registry_node* address)
{
  return _az_ulib_pal_flash_driver_write_64(&(address->ready_flag), REGISTRY_READY);
}

static inline az_result set_registry_node_delete_flag(registry_node* address)
{
  return _az_ulib_pal_flash_driver_write_64(&(address->delete_flag), REGISTRY_DELETED);
}

static bool is_empty_buf(uint8_t* test_buf, int32_t buf_size)
{
  while (buf_size > 3)
  {
    if (*(uint32_t*)(test_buf) != 0xFFFFFFFF)
    {
      return false;
    }
    test_buf += 4;
    buf_size -= 4;
  }
  while ((buf_size--) > 0)
  {
    // leftover, compare byte by byte
    if (*(test_buf++) != 0xFF)
    {
      return false;
    }
  }

  return true;
}

static az_result store_registry_node(registry_node node, registry_node** node_ptr)
{
  AZ_ULIB_TRY
  {
    registry_node* runner;

    /* Look for next available spot */
    for (runner = (registry_node*)_az_ulib_registry_cb->registry_info_start;
         runner < (registry_node*)_az_ulib_registry_cb->registry_info_end;
         runner++)
    {
      if (is_empty_buf((uint8_t*)(runner), sizeof(registry_node)))
      {
        break;
      }
    }

    /* Handle case if all nodes were used */
    AZ_ULIB_THROW_IF_ERROR(
        (runner < (registry_node*)_az_ulib_registry_cb->registry_info_end),
        AZ_ERROR_NOT_ENOUGH_SPACE);

    /* Store az_span (pointer + size) to key value pair into flash. */
    _az_ulib_pal_flash_driver_control_block key_value_cb;
    AZ_ULIB_THROW_IF_AZ_ERROR(
        _az_ulib_pal_flash_driver_open(&key_value_cb, (uint64_t*)&(runner->key_value)));
    AZ_ULIB_THROW_IF_AZ_ERROR(_az_ulib_pal_flash_driver_write(
        &key_value_cb, (uint8_t*)&(node.key_value), (uint32_t)sizeof(registry_key_value_ptrs)));
    AZ_ULIB_THROW_IF_AZ_ERROR(_az_ulib_pal_flash_driver_close(&key_value_cb, 0x00));

    /* Return pointer to this node for setting flags later */
    *node_ptr = runner;
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
}

/* Find pointer to the next available space in flash to store registry key/value. */
static uint64_t* get_start_registry_data_free_space()
{
  /* Find occupied node that has maximum address such that a new key-value pair can be stored. */
  for (uint64_t* runner = (uint64_t*)_az_ulib_registry_cb->registry_end - 1;
       runner >= (uint64_t*)_az_ulib_registry_cb->registry_start;
       runner--)
  {
    if (*runner != REGISTRY_FREE)
    {
      return runner + 1;
    }
  }

  return (uint64_t*)_az_ulib_registry_cb->registry_start;
}

static registry_node* find_node_in_registry(az_span key)
{
  /* Loop through registry for entry that matches the key */
  for (registry_node* runner = (registry_node*)_az_ulib_registry_cb->registry_info_start;
       runner < (registry_node*)_az_ulib_registry_cb->registry_info_end;
       runner++)
  {
    if (runner->delete_flag == REGISTRY_FREE)
    {
      if (runner->ready_flag == REGISTRY_READY)
      {
        if (az_span_is_content_equal(key, runner->key_value.key))
        {
          return runner;
        }
      }
      else if (runner->ready_flag == REGISTRY_FREE)
      {
        // Hit empty node entry, node, not found
        return NULL;
      }
    }
  }

  return NULL;
}

void az_ulib_registry_init(const az_ulib_registry_control_block* registry_cb)
{
  _az_PRECONDITION_NOT_NULL(registry_cb);
  _az_PRECONDITION_IS_NULL(_az_ulib_registry_cb);

  /* Initialize the registry control block. */
  _az_ulib_registry_cb = registry_cb;

  /* Initialize lock */
  az_pal_os_lock_init(&registry_lock);
}

void az_ulib_registry_deinit(void)
{
  _az_PRECONDITION_NOT_NULL(_az_ulib_registry_cb);

  /* Deinitialize lock */
  az_pal_os_lock_deinit(&registry_lock);

  /* Release the single instance. */
  _az_ulib_registry_cb = NULL;
}

AZ_NODISCARD az_result az_ulib_registry_delete(az_span key)
{
  /* Precondition check */
  _az_PRECONDITION_NOT_NULL(_az_ulib_registry_cb);
  _az_PRECONDITION_VALID_SPAN(key, 1, false);
  az_result result;

  az_pal_os_lock_acquire(&registry_lock);
  {
    registry_node* matched_node = find_node_in_registry(key);
    if (matched_node == NULL)
    {
      /* Item not found in registry */
      result = AZ_ERROR_ITEM_NOT_FOUND;
    }
    else
    {
      result = set_registry_node_delete_flag(matched_node);
    }
  }
  az_pal_os_lock_release(&registry_lock);
  return result;
}

AZ_NODISCARD az_result az_ulib_registry_try_get_value(az_span key, az_span* value)
{
  /* Precondition check */
  _az_PRECONDITION_NOT_NULL(_az_ulib_registry_cb);
  _az_PRECONDITION_VALID_SPAN(key, 1, false);
  _az_PRECONDITION_NOT_NULL(value);
  az_result result;

  az_pal_os_lock_acquire(&registry_lock);
  {
    registry_node* matched_node = find_node_in_registry(key);
    if (matched_node == NULL)
    {
      result = AZ_ERROR_ITEM_NOT_FOUND;
    }
    else
    {
      *value = matched_node->key_value.value;
      result = AZ_OK;
    }
  }
  az_pal_os_lock_release(&registry_lock);
  return result;
}

AZ_NODISCARD az_result az_ulib_registry_add(az_span key, az_span value)
{
  /* Precondition check */
  _az_PRECONDITION_NOT_NULL(_az_ulib_registry_cb);
  _az_PRECONDITION_VALID_SPAN(key, 1, false);
  _az_PRECONDITION_VALID_SPAN(value, 1, false);
  az_result result;

  az_pal_os_lock_acquire(&registry_lock);
  {
    AZ_ULIB_TRY
    {
      registry_node new_node;
      registry_node* new_node_ptr = NULL;
      uint64_t* key_dest_ptr;
      uint64_t* value_dest_ptr;

      /* Validate for duplicates before adding new entry */
      AZ_ULIB_THROW_IF_ERROR((find_node_in_registry(key) == NULL), AZ_ERROR_ULIB_ELEMENT_DUPLICATE);

      int32_t size_of_key_in_64_bits = NUMBER_OF_64BITS(az_span_size(key));
      int32_t size_of_value_in_64_bits = NUMBER_OF_64BITS(az_span_size(value));

      /* Find destination in flash buffer */
      key_dest_ptr = get_start_registry_data_free_space();
      value_dest_ptr = (uint64_t*)key_dest_ptr + size_of_key_in_64_bits;

      /* Handle out of space scenario */
      AZ_ULIB_THROW_IF_ERROR(
          ((value_dest_ptr + size_of_value_in_64_bits)
           <= (uint64_t*)(_az_ulib_registry_cb->registry_end)),
          AZ_ERROR_OUT_OF_MEMORY);

      /* Set free node information */
      new_node.key_value.key = az_span_create((uint8_t*)key_dest_ptr, az_span_size(key));
      new_node.key_value.value = az_span_create((uint8_t*)value_dest_ptr, az_span_size(value));

      /* Update registry node in flash */
      AZ_ULIB_THROW_IF_AZ_ERROR(store_registry_node(new_node, &new_node_ptr));

      /* Write key to flash */
      _az_ulib_pal_flash_driver_control_block key_flash_cb;
      AZ_ULIB_THROW_IF_AZ_ERROR(_az_ulib_pal_flash_driver_open(&key_flash_cb, key_dest_ptr));
      AZ_ULIB_THROW_IF_AZ_ERROR(_az_ulib_pal_flash_driver_write(
          &key_flash_cb, az_span_ptr(key), (uint32_t)az_span_size(key)));
      AZ_ULIB_THROW_IF_AZ_ERROR(_az_ulib_pal_flash_driver_close(&key_flash_cb, 0x00));

      /* Write value to flash */
      _az_ulib_pal_flash_driver_control_block value_flash_cb;
      AZ_ULIB_THROW_IF_AZ_ERROR(_az_ulib_pal_flash_driver_open(&value_flash_cb, value_dest_ptr));
      AZ_ULIB_THROW_IF_AZ_ERROR(_az_ulib_pal_flash_driver_write(
          &value_flash_cb, az_span_ptr(value), (uint32_t)az_span_size(value)));
      AZ_ULIB_THROW_IF_AZ_ERROR(_az_ulib_pal_flash_driver_close(&value_flash_cb, 0x00));

      /* After successful storage of registry node and actual key value pair, set flag in node to
      indicate the entry is now ready to use.  */
      AZ_ULIB_THROW_IF_AZ_ERROR(set_registry_node_ready_flag(new_node_ptr));
    }
    AZ_ULIB_CATCH(...) {}
    result = AZ_ULIB_TRY_RESULT;
  }
  az_pal_os_lock_release(&registry_lock);

  return result;
}

void az_ulib_registry_clean_all(void)
{
  _az_PRECONDITION_NOT_NULL(_az_ulib_registry_cb);

  az_pal_os_lock_acquire(&registry_lock);
  {
    _az_ulib_pal_flash_driver_erase(
      (uint64_t*)(_az_ulib_registry_cb->registry_info_start),
      (uint32_t)((uint8_t*)(_az_ulib_registry_cb->registry_info_end) - (uint8_t*)(_az_ulib_registry_cb->registry_info_start)));

    _az_ulib_pal_flash_driver_erase(
        (uint64_t*)(_az_ulib_registry_cb->registry_start),
        (uint32_t)((uint8_t*)(_az_ulib_registry_cb->registry_end) - (uint8_t*)(_az_ulib_registry_cb->registry_start)));
  }
  az_pal_os_lock_release(&registry_lock);
}

void az_ulib_registry_get_info(az_ulib_registry_info* info)
{
  _az_PRECONDITION_NOT_NULL(_az_ulib_registry_cb);
  _az_PRECONDITION_NOT_NULL(info);

  az_pal_os_lock_acquire(&registry_lock);
  {

    info->total_registry_info
      = (size_t)((size_t)((uint8_t*)_az_ulib_registry_cb->registry_info_end - (uint8_t*)_az_ulib_registry_cb->registry_info_start) / sizeof(registry_node));

    info->total_registry_data
        = (size_t)((uint8_t*)_az_ulib_registry_cb->registry_end - (uint8_t*)_az_ulib_registry_cb->registry_start);

    info->free_registry_data
        = (size_t)((uint8_t*)_az_ulib_registry_cb->registry_end - (uint8_t*)get_start_registry_data_free_space());

    info->free_registry_info = 0;
    info->in_use_registry_info = 0;
    info->in_use_registry_data = 0;
    for (registry_node* runner = (registry_node*)_az_ulib_registry_cb->registry_info_start;
         runner < (registry_node*)_az_ulib_registry_cb->registry_info_end;
         runner++)
    {
      if (is_empty_buf((uint8_t*)(runner), sizeof(registry_node)))
      {
        info->free_registry_info++;
      }
      else
      {
        if ((runner->delete_flag == REGISTRY_FREE) && (runner->ready_flag == REGISTRY_READY))
        {
          info->in_use_registry_info++;
          info->in_use_registry_data
              += (size_t)(((az_span_ptr(runner->key_value.value) + ROUND_UP_TO_64BITS(az_span_size(runner->key_value.value))))
                 - az_span_ptr(runner->key_value.key));
        }
      }
    }
  }
  az_pal_os_lock_release(&registry_lock);
}
