// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <memory.h>
#include <stdint.h>

#include "_az_ulib_ipc_query.h"
#include "az_ulib_base.h"
#include "az_ulib_capability_api.h"
#include "az_ulib_config.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_ipc_interface.h"
#include "az_ulib_pal_os_api.h"
#include "az_ulib_port.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"

#include <azure/core/internal/az_precondition_internal.h>

typedef union
{
  struct
  {
    uint8_t query_type;
    uint8_t reserved;
    uint16_t count;
  } fields;
  uint32_t val;
} ipc_continuation_token;

/*
 * IPC is a singleton component, and shall be initialized only once.
 *
 * Make it volatile to avoid any compilation optimization.
 */
static az_ulib_ipc* volatile _az_ipc_cb = NULL;

static _az_ulib_ipc_interface* get_interface(
    az_span name,
    az_ulib_version version,
    az_ulib_version_match_criteria match_criteria)
{
  _az_ulib_ipc_interface* result = NULL;

  // Find the lowest version that fits the criteria.
  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++)
  {
    if ((_az_ipc_cb->_internal.interface_list[i].interface_descriptor != NULL)
        && (az_span_is_content_equal(
            _az_ipc_cb->_internal.interface_list[i].interface_descriptor->_name, name))
        && az_ulib_version_match(
            _az_ipc_cb->_internal.interface_list[i].interface_descriptor->_version,
            version,
            match_criteria))
    {
      if (result == NULL)
      {
        result = &(_az_ipc_cb->_internal.interface_list[i]);
      }
      else
      {
        if (result->interface_descriptor->_version
            > _az_ipc_cb->_internal.interface_list[i].interface_descriptor->_version)
        {
          result = &(_az_ipc_cb->_internal.interface_list[i]);
        }
      }
    }
  }

  return result;
}

static _az_ulib_ipc_interface* find_interface_descriptor(
    const az_ulib_interface_descriptor* interface_descriptor)
{
  _az_ulib_ipc_interface* result = NULL;

  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++)
  {
    if (_az_ipc_cb->_internal.interface_list[i].interface_descriptor == interface_descriptor)
    {
      result = &(_az_ipc_cb->_internal.interface_list[i]);
      break;
    }
  }

  return result;
}

static _az_ulib_ipc_interface* get_first_free()
{
  _az_ulib_ipc_interface* result = NULL;

  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++)
  {
    if ((_az_ipc_cb->_internal.interface_list[i].interface_descriptor == NULL)
        && (_az_ipc_cb->_internal.interface_list[i].ref_count == 0))
    {
      result = &(_az_ipc_cb->_internal.interface_list[i]);
      break;
    }
  }

  return result;
}

static az_result get_instance(_az_ulib_ipc_interface* ipc_interface)
{
  az_result result;
  if (ipc_interface->ref_count >= AZ_ULIB_CONFIG_MAX_IPC_INSTANCES)
  {
    result = AZ_ERROR_NOT_ENOUGH_SPACE;
  }
  else
  {
    result = AZ_OK;
    (void)AZ_ULIB_PORT_ATOMIC_INC_W(&(ipc_interface->ref_count));
  }
  return result;
}

AZ_NODISCARD az_result az_ulib_ipc_init(az_ulib_ipc* ipc_handle)
{
  _az_PRECONDITION_IS_NULL(_az_ipc_cb);
  _az_PRECONDITION_NOT_NULL(ipc_handle);

  _az_ipc_cb = ipc_handle;

  az_pal_os_lock_init(&(_az_ipc_cb->_internal.lock));

  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++)
  {
    _az_ipc_cb->_internal.interface_list[i].ref_count = 0;
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
    _az_ipc_cb->_internal.interface_list[i].running_count = 0;
    _az_ipc_cb->_internal.interface_list[i].running_count_low_watermark = 0;
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
    _az_ipc_cb->_internal.interface_list[i].interface_descriptor = NULL;
  }

  return _az_ulib_ipc_query_interface_publish();
}

AZ_NODISCARD az_result az_ulib_ipc_deinit(void)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);

  az_result result;

  result = _az_ulib_ipc_query_interface_unpublish();

  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++)
  {
    if ((_az_ipc_cb->_internal.interface_list[i].interface_descriptor != NULL)
        || (_az_ipc_cb->_internal.interface_list[i].ref_count != 0)
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
        || (_az_ipc_cb->_internal.interface_list[i].running_count != 0)
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
    )
    {
      // Do our best to publish IPC query the interface again.
      (void)_az_ulib_ipc_query_interface_publish();
      result = AZ_ERROR_ULIB_BUSY;
      break;
    }
  }

  if (result == AZ_OK)
  {
    az_pal_os_lock_deinit(&(_az_ipc_cb->_internal.lock));
    _az_ipc_cb = NULL;
  }

  return result;
}

AZ_NODISCARD az_result az_ulib_ipc_publish(
    const az_ulib_interface_descriptor* const interface_descriptor,
    az_ulib_ipc_interface_handle* interface_handle)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_NOT_NULL(interface_descriptor);

  az_result result;
  _az_ulib_ipc_interface* new_interface;

  az_pal_os_lock_acquire(&(_az_ipc_cb->_internal.lock));
  {
    if (get_interface(
            interface_descriptor->_name, interface_descriptor->_version, AZ_ULIB_VERSION_EQUALS_TO)
        != NULL)
    {
      // IPC shall not accept interfaces with same name and version because it cannot decided each
      // one to retrieve when someone uses az_ulib_ipc_try_get_interface().
      result = AZ_ERROR_ULIB_ELEMENT_DUPLICATE;
    }
    else if ((new_interface = get_first_free()) == NULL)
    {
      result = AZ_ERROR_NOT_ENOUGH_SPACE;
    }
    else
    {
      (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(
          (const volatile void**)&(new_interface->interface_descriptor),
          (const void*)interface_descriptor);
      new_interface->ref_count = 0;
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
      new_interface->running_count = 0;
      new_interface->running_count_low_watermark = 0;
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
      if (interface_handle != NULL)
      {
        *interface_handle = new_interface;
      }
      result = AZ_OK;
    }
  }
  az_pal_os_lock_release(&(_az_ipc_cb->_internal.lock));

  return result;
}

#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
AZ_NODISCARD az_result az_ulib_ipc_unpublish(
    const az_ulib_interface_descriptor* const interface_descriptor,
    uint32_t wait_option_ms)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_NOT_NULL(interface_descriptor);

  az_result result;
  _az_ulib_ipc_interface* release_interface;

  az_pal_os_lock_acquire(&(_az_ipc_cb->_internal.lock));
  {
    if ((release_interface = find_interface_descriptor(interface_descriptor)) == NULL)
    {
      result = AZ_ERROR_ITEM_NOT_FOUND;
    }
    else
    {
      // The order of the code here, including the ones that looks not necessary, are associated to
      // the interlock between this function and the az_ulib_ipc_call.

      // Block access to this interface. After this point, any new call to az_ulib_ipc_call that
      // didn't get the interface pointer yet will return AZ_ERROR_ITEM_NOT_FOUND.
      (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(
          (const volatile void**)(&(release_interface->interface_descriptor)), (const void*)NULL);

      // If the running_count is `0` is because no other process is inside of any of the functions
      // commands, and they may be removed from the memory. There will be the case that the other
      // process is already in the az_ulib_ipc_call, in the direction to call a command in this
      // interface, but the call will just return AZ_ERROR_ITEM_NOT_FOUND from there.
      uint32_t retry_interval;
      if (wait_option_ms == AZ_ULIB_WAIT_FOREVER)
      {
        retry_interval = 100;
      }
      else
      {
        retry_interval = wait_option_ms >> 3;
        if (retry_interval == 0)
        {
          retry_interval = 1;
        }
      }

      (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_W(
          &(release_interface->running_count_low_watermark), release_interface->running_count);
      uint32_t retry_total_time = 0;

      // A semaphore here would be more efficient, but it would force a synchronization between
      // az_ulib_ipc_call and az_ulib_ipc_unpublish that would add extra code on az_ulib_ipc_call,
      // making it heavier. On the other hand, there is no expectations about heavy usage of the
      // function az_ulib_ipc_unpublish, in many applications, it will not be used at all. So, we
      // decided to open an exception here and use a busy loop on the az_ulib_ipc_unpublish
      // instead of a semaphore.
      while ((retry_total_time < wait_option_ms)
             && (release_interface->running_count_low_watermark != 0))
      {

        az_pal_os_sleep(retry_interval);

        if (wait_option_ms != AZ_ULIB_WAIT_FOREVER)
        {
          retry_total_time += retry_interval;
        }
      }

      if (release_interface->running_count_low_watermark == 0)
      {
        result = AZ_OK;
      }
      else
      {
        // If caller doesn't want to wait anymore, recover the interface and return
        // AZ_ERROR_ULIB_BUSY.
        (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(
            (const volatile void**)(&(release_interface->interface_descriptor)),
            (const void*)interface_descriptor);
        result = AZ_ERROR_ULIB_BUSY;
      }
    }
  }
  az_pal_os_lock_release(&(_az_ipc_cb->_internal.lock));

  return result;
}
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH

AZ_NODISCARD az_result az_ulib_ipc_try_get_interface(
    az_span name,
    az_ulib_version version,
    az_ulib_version_match_criteria match_criteria,
    az_ulib_ipc_interface_handle* interface_handle)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_VALID_SPAN(name, 1, false);
  _az_PRECONDITION_NOT_NULL(interface_handle);

  az_result result;
  _az_ulib_ipc_interface* ipc_interface;

  az_pal_os_lock_acquire(&(_az_ipc_cb->_internal.lock));
  {
    if ((ipc_interface = get_interface(name, version, match_criteria)) == NULL)
    {
      result = AZ_ERROR_ITEM_NOT_FOUND;
    }
    else if ((result = get_instance(ipc_interface)) == AZ_OK)
    {
      *interface_handle = ipc_interface;
    }
  }
  az_pal_os_lock_release(&(_az_ipc_cb->_internal.lock));

  return result;
}

AZ_NODISCARD az_result az_ulib_ipc_try_get_capability(
    az_ulib_ipc_interface_handle interface_handle,
    az_span name,
    az_ulib_capability_index* capability_index)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_NOT_NULL(interface_handle);
  _az_PRECONDITION_VALID_SPAN(name, 1, false);
  _az_PRECONDITION_NOT_NULL(capability_index);

  az_result result;
  _az_ulib_ipc_interface* ipc_interface = (_az_ulib_ipc_interface*)interface_handle;

#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
  // The double test on the interface_descriptor is part of the interlock between az_ulib_ipc_call
  // and az_ulib_ipc_unpublish. It will allow a interface to be unpublished even if it has a high
  // volume of calls.
  if (ipc_interface->interface_descriptor != NULL)
  {
    (void)AZ_ULIB_PORT_ATOMIC_INC_W(&(ipc_interface->running_count));
    if (ipc_interface->interface_descriptor == NULL)
    {
      result = AZ_ERROR_ITEM_NOT_FOUND;
    }
    else
    {
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH

      az_ulib_capability_index index;
      for (index = 0; index < ipc_interface->interface_descriptor->_size; index++)
      {
        if (az_span_is_content_equal(
                name, ipc_interface->interface_descriptor->_capability_list[index]._name))
        {
          break;
        }
      }

      if (index < ipc_interface->interface_descriptor->_size)
      {
        *capability_index = index;
        result = AZ_OK;
      }
      else
      {
        result = AZ_ERROR_ITEM_NOT_FOUND;
      }

#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
    }

    long new_running_count = AZ_ULIB_PORT_ATOMIC_DEC_W(&(ipc_interface->running_count));
    if (new_running_count < ipc_interface->running_count_low_watermark)
    {
      (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_W(
          &(ipc_interface->running_count_low_watermark), new_running_count);
    }
  }
  else
  {
    result = AZ_ERROR_ITEM_NOT_FOUND;
  }
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
  return result;
}

AZ_NODISCARD az_result az_ulib_ipc_get_interface(
    az_ulib_ipc_interface_handle original_interface_handle,
    az_ulib_ipc_interface_handle* interface_handle)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_NOT_NULL(original_interface_handle);
  _az_PRECONDITION_NOT_NULL(interface_handle);

  az_result result;

  az_pal_os_lock_acquire(&(_az_ipc_cb->_internal.lock));
  {
    _az_ulib_ipc_interface* ipc_interface = original_interface_handle;
    if (ipc_interface->interface_descriptor == NULL)
    {
      result = AZ_ERROR_ITEM_NOT_FOUND;
    }
    else if ((result = get_instance(ipc_interface)) == AZ_OK)
    {
      *interface_handle = ipc_interface;
    }
  }
  az_pal_os_lock_release(&(_az_ipc_cb->_internal.lock));

  return result;
}

AZ_NODISCARD az_result az_ulib_ipc_release_interface(az_ulib_ipc_interface_handle interface_handle)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_NOT_NULL(interface_handle);

  _az_ulib_ipc_interface* ipc_interface = (_az_ulib_ipc_interface*)interface_handle;
  az_result result;

  az_pal_os_lock_acquire(&(_az_ipc_cb->_internal.lock));
  {
    if (ipc_interface->ref_count == 0)
    {
      result = AZ_ERROR_ULIB_PRECONDITION;
    }
    else
    {
      result = AZ_OK;
      ipc_interface->ref_count--;
    }
  }
  az_pal_os_lock_release(&(_az_ipc_cb->_internal.lock));

  return result;
}

AZ_NODISCARD az_result az_ulib_ipc_call(
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_capability_index command_index,
    az_ulib_model_in model_in,
    az_ulib_model_out model_out)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_NOT_NULL(interface_handle);

  az_result result;
  _az_ulib_ipc_interface* ipc_interface = (_az_ulib_ipc_interface*)interface_handle;

#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
  // The double test on the interface_descriptor is part of the interlock between az_ulib_ipc_call
  // and az_ulib_ipc_unpublish. It will allow a interface to be unpublished even if it has a high
  // volume of calls.
  if (ipc_interface->interface_descriptor != NULL)
  {
    (void)AZ_ULIB_PORT_ATOMIC_INC_W(&(ipc_interface->running_count));

    if (ipc_interface->interface_descriptor == NULL)
    {
      result = AZ_ERROR_ITEM_NOT_FOUND;
    }
    else
    {
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
      result = ipc_interface->interface_descriptor->_capability_list[command_index]
                   ._capability_ptr_1._command(model_in, model_out);
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
    }
    long new_running_count = AZ_ULIB_PORT_ATOMIC_DEC_W(&(ipc_interface->running_count));
    if (new_running_count < ipc_interface->running_count_low_watermark)
    {
      (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_W(
          &(ipc_interface->running_count_low_watermark), new_running_count);
    }
  }
  else
  {
    result = AZ_ERROR_ITEM_NOT_FOUND;
  }
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH

  return result;
}

AZ_NODISCARD az_result az_ulib_ipc_call_w_str(
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_capability_index command_index,
    az_span model_in_span,
    az_span* model_out_span)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);

  az_result result = AZ_OK;
  _az_ulib_ipc_interface* ipc_interface = (_az_ulib_ipc_interface*)interface_handle;

#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
  // The double test on the interface_descriptor is part of the interlock between az_ulib_ipc_call
  // and az_ulib_ipc_unpublish. It will allow a interface to be unpublished even if it has a high
  // volume of calls.
  if (ipc_interface->interface_descriptor != NULL)
  {
    (void)AZ_ULIB_PORT_ATOMIC_INC_W(&(ipc_interface->running_count));
    if (ipc_interface->interface_descriptor == NULL)
    {
      result = AZ_ERROR_ITEM_NOT_FOUND;
    }
    else
    {
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH

      if (ipc_interface->interface_descriptor->_capability_list[command_index]
              ._span_wrapper_ptr_1._command
          != NULL)
      {
        result = ipc_interface->interface_descriptor->_capability_list[command_index]
                     ._span_wrapper_ptr_1._command(model_in_span, model_out_span);
      }
      else
      {
        result = AZ_ERROR_NOT_SUPPORTED;
      }

#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
    }

    long new_running_count = AZ_ULIB_PORT_ATOMIC_DEC_W(&(ipc_interface->running_count));
    if (new_running_count < ipc_interface->running_count_low_watermark)
    {
      (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_W(
          &(ipc_interface->running_count_low_watermark), new_running_count);
    }
  }
  else
  {
    result = AZ_ERROR_ITEM_NOT_FOUND;
  }
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH

  return result;
}

static az_result report_interfaces(uint16_t start, az_span* result, uint16_t* next)
{
  char* result_str = (char*)az_span_ptr(*result);
  int32_t result_size = az_span_size(*result);
  int32_t pos = 0;
  uint16_t interface_index;

  az_result res = AZ_ULIB_EOF;
  for (interface_index = start; interface_index < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE;
       interface_index++)
  {
    if (_az_ipc_cb->_internal.interface_list[interface_index].interface_descriptor != NULL)
    {
      int32_t next_size = az_span_size(
          _az_ipc_cb->_internal.interface_list[interface_index].interface_descriptor->_name);
      char version_str[12];
      az_span version_span = AZ_SPAN_FROM_BUFFER(version_str);
      az_span reminder;
      if ((res = az_span_u32toa(
               version_span,
               _az_ipc_cb->_internal.interface_list[interface_index].interface_descriptor->_version,
               &reminder))
          == AZ_OK)
      {
        int32_t next_version_size = az_span_size(version_span) - az_span_size(reminder);
        if (pos == 0)
        {
          if (next_size + next_version_size + 1 > result_size) // 1 = '.'
          {
            res = AZ_ERROR_NOT_ENOUGH_SPACE;
            break;
          }
        }
        else
        {
          if (next_size + 2 + next_version_size > result_size - pos) // 3 = ',' and '.'
          {
            break;
          }
          else
          {
            result_str[pos++] = ',';
          }
        }

        res = AZ_OK;
        memcpy(
            &(result_str[pos]),
            az_span_ptr(
                _az_ipc_cb->_internal.interface_list[interface_index].interface_descriptor->_name),
            (size_t)next_size);
        pos += next_size;
        result_str[pos++] = '.';
        memcpy(&(result_str[pos]), az_span_ptr(version_span), (size_t)next_version_size);
        pos += next_version_size;
      }
      else
      {
        break;
      }
    }
  }

  if (res == AZ_OK)
  {
    *next = interface_index;
    *result = az_span_create((uint8_t*)result_str, pos);
  }

  return res;
}

AZ_NODISCARD az_result
az_ulib_ipc_query(az_span query, az_span* result, uint32_t* continuation_token)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_NOT_NULL(result);
  _az_PRECONDITION_VALID_SPAN(*result, 1, false);
  _az_PRECONDITION_NOT_NULL(continuation_token);
  az_result res;

  az_pal_os_lock_acquire(&(_az_ipc_cb->_internal.lock));
  {
    ipc_continuation_token* token = (ipc_continuation_token*)continuation_token;

    if (az_span_size(query) == 0)
    {
      if ((res = report_interfaces(0, result, &(token->fields.count))) == AZ_OK)
      {
        token->fields.query_type = 0xFF;
        token->fields.reserved = 0;
      }
    }
    else
    {
      res = AZ_ERROR_NOT_IMPLEMENTED;
    }
  }
  az_pal_os_lock_release(&(_az_ipc_cb->_internal.lock));

  return res;
}

AZ_NODISCARD az_result az_ulib_ipc_query_next(uint32_t* continuation_token, az_span* result)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_NOT_NULL(result);
  _az_PRECONDITION_VALID_SPAN(*result, 1, false);
  _az_PRECONDITION_NOT_NULL(continuation_token);
  az_result res;

  az_pal_os_lock_acquire(&(_az_ipc_cb->_internal.lock));
  {
    ipc_continuation_token* token = (ipc_continuation_token*)continuation_token;

    if (token->fields.query_type == 0xFF)
    {
      res = report_interfaces(token->fields.count, result, &(token->fields.count));
    }
    else
    {
      res = AZ_ERROR_NOT_IMPLEMENTED;
    }
  }
  az_pal_os_lock_release(&(_az_ipc_cb->_internal.lock));

  return res;
}

static const az_ulib_ipc_vtable _vtable = { az_ulib_ipc_publish,
                                            az_ulib_ipc_unpublish,
                                            az_ulib_ipc_try_get_interface,
                                            az_ulib_ipc_try_get_capability,
                                            az_ulib_ipc_get_interface,
                                            az_ulib_ipc_release_interface,
                                            az_ulib_ipc_call,
                                            az_ulib_ipc_call_w_str,
                                            az_ulib_ipc_query,
                                            az_ulib_ipc_query_next };

const az_ulib_ipc_vtable* az_ulib_ipc_get_vtable(void) { return &_vtable; }
