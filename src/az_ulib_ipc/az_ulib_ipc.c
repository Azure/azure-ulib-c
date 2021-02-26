// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stdint.h>

#include "az_ulib_base.h"
#include "az_ulib_capability_api.h"
#include "az_ulib_config.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_pal_os_api.h"
#include "az_ulib_port.h"
#include "az_ulib_result.h"
#include "az_ulib_ucontract.h"
#include "az_ulib_ulog.h"

#include <azure/core/internal/az_precondition_internal.h>

/*
 * IPC is a singleton component, and shall be initialized only once.
 *
 * Make it volatile to avoid any compilation optimization.
 */
static az_ulib_ipc* volatile _az_ipc_cb = NULL;

static _az_ulib_ipc_interface* get_interface(
    const char* const name,
    az_ulib_version version,
    az_ulib_version_match_criteria match_criteria)
{
  _az_ulib_ipc_interface* result = NULL;

  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++)
  {
    if ((_az_ipc_cb->_internal.interface_list[i].interface_descriptor != NULL)
        && (strcmp(_az_ipc_cb->_internal.interface_list[i].interface_descriptor->name, name) == 0)
        && az_ulib_version_match(
            _az_ipc_cb->_internal.interface_list[i].interface_descriptor->version,
            version,
            match_criteria))
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

  return AZ_OK;
}

AZ_NODISCARD az_result az_ulib_ipc_deinit(void)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);

  az_result result;

  result = AZ_OK;
  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++)
  {
    if ((_az_ipc_cb->_internal.interface_list[i].interface_descriptor != NULL)
        || (_az_ipc_cb->_internal.interface_list[i].ref_count != 0)
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
        || (_az_ipc_cb->_internal.interface_list[i].running_count != 0)
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
    )
    {
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
    const az_ulib_interface_descriptor* interface_descriptor,
    az_ulib_ipc_interface_handle* interface_handle)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_NOT_NULL(interface_descriptor);

  az_result result;
  _az_ulib_ipc_interface* new_interface;

  az_pal_os_lock_acquire(&(_az_ipc_cb->_internal.lock));
  {
    if (get_interface(
            interface_descriptor->name, interface_descriptor->version, AZ_ULIB_VERSION_EQUALS_TO)
        != NULL)
    {
      /*az_ulib_ipc_publish_with__descriptor_with_same_name_and_version_failed*/
      result = AZ_ERROR_ULIB_ELEMENT_DUPLICATE;
    }
    else if ((new_interface = get_first_free()) == NULL)
    {
      /*az_ulib_ipc_publish_out_of_memory_failed*/
      result = AZ_ERROR_NOT_ENOUGH_SPACE;
    }
    else
    {
      /*az_ulib_ipc_publish_succeed*/
      (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(
          &(new_interface->interface_descriptor), interface_descriptor);
      new_interface->ref_count = 0;
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
      new_interface->running_count = 0;
      new_interface->running_count_low_watermark = 0;
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
      if (interface_handle != NULL)
      {
        /*az_ulib_ipc_publish_return_handle_succeed*/
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
    const az_ulib_interface_descriptor* interface_descriptor,
    uint32_t wait_option_ms)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_NOT_NULL(interface_descriptor);

  az_result result;
  _az_ulib_ipc_interface* release_interface;

  az_pal_os_lock_acquire(&(_az_ipc_cb->_internal.lock));
  {
    if ((release_interface = get_interface(
             interface_descriptor->name, interface_descriptor->version, AZ_ULIB_VERSION_EQUALS_TO))
        == NULL)
    {
      /*az_ulib_ipc_unpublish_with_unknown_descriptor_failed*/
      result = AZ_ERROR_ITEM_NOT_FOUND;
    }
    else
    {
      // The order of the code here, including the ones that looks not necessary, are associated to
      // the interlock between this function and the az_ulib_ipc_call.

      // Prepare to recover in case it was not possible to unpublish the interface.
      const az_ulib_interface_descriptor* recover_interface_descriptor
          = (const az_ulib_interface_descriptor*)release_interface->interface_descriptor;

      // Block access to this interface. After this point, any new call to az_ulib_ipc_call that
      // didn't get the interface pointer yet will return AZ_ERROR_ITEM_NOT_FOUND.
      (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(&(release_interface->interface_descriptor), NULL);

      // If the running_count is `0` is because no other process is inside of any of the functions
      // commands, and they may be removed from the memory. There will be the case that the other
      // process is already in the az_ulib_ipc_call, in the direction to call a command in this
      // interface, but the call will just return AZ_ERROR_ITEM_NOT_FOUND from there.
      /*az_ulib_ipc_unpublish_succeed*/
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
      /*az_ulib_ipc_unpublish_with_command_running_with_small_timeout_failed*/
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
        /*az_ulib_ipc_unpublish_random_order_succeed*/
        /*az_ulib_ipc_unpublish_release_resource_succeed*/
        /*az_ulib_ipc_unpublish_with_valid_interface_instance_succeed*/
        result = AZ_OK;
      }
      else
      {
        /*az_ulib_ipc_unpublish_with_command_running_failed*/
        // If caller doesn't want to wait anymore, recover the interface and return
        // AZ_ERROR_ULIB_BUSY.
        (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(
            &(release_interface->interface_descriptor), recover_interface_descriptor);
        result = AZ_ERROR_ULIB_BUSY;
      }
    }
  }
  az_pal_os_lock_release(&(_az_ipc_cb->_internal.lock));

  return result;
}
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH

AZ_NODISCARD az_result az_ulib_ipc_try_get_interface(
    const char* const name,
    az_ulib_version version,
    az_ulib_version_match_criteria match_criteria,
    az_ulib_ipc_interface_handle* interface_handle)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_NOT_NULL(name);
  _az_PRECONDITION_NOT_NULL(interface_handle);

  az_result result;
  _az_ulib_ipc_interface* ipc_interface;

  az_pal_os_lock_acquire(&(_az_ipc_cb->_internal.lock));
  {
    if ((ipc_interface = get_interface(name, version, match_criteria)) == NULL)
    {
      /*az_ulib_ipc_try_get_interface_with_unknown_name_failed*/
      /*az_ulib_ipc_try_get_interface_with_unknown_version_failed*/
      result = AZ_ERROR_ITEM_NOT_FOUND;
      /*az_ulib_ipc_try_get_interface_with_max_interface_instances_failed*/
    }
    else if ((result = get_instance(ipc_interface)) == AZ_OK)
    {
      /*az_ulib_ipc_try_get_interface_succeed*/
      *interface_handle = ipc_interface;
    }
  }
  az_pal_os_lock_release(&(_az_ipc_cb->_internal.lock));

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
      /*az_ulib_ipc_get_interface_with_unpublished_interface_failed*/
      result = AZ_ERROR_ITEM_NOT_FOUND;
      /*az_ulib_ipc_get_interface_with_max_interface_instances_failed*/
    }
    else if ((result = get_instance(ipc_interface)) == AZ_OK)
    {
      /*az_ulib_ipc_get_interface_succeed*/
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
      /*az_ulib_ipc_release_interface_double_release_failed*/
      result = AZ_ERROR_ULIB_PRECONDITION;
    }
    else
    {
      /*az_ulib_ipc_release_interface_succeed*/
      result = AZ_OK;
      ipc_interface->ref_count--;
    }
  }
  az_pal_os_lock_release(&(_az_ipc_cb->_internal.lock));

  return result;
}

#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
AZ_NODISCARD az_result az_ulib_ipc_call(
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_capability_index command_index,
    const void* const model_in,
    const void* model_out)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_NOT_NULL(interface_handle);

  az_result result;
  _az_ulib_ipc_interface* ipc_interface = (_az_ulib_ipc_interface*)interface_handle;

  // The double test on the interface_descriptor is part of the interlock between az_ulib_ipc_call
  // and az_ulib_ipc_unpublish. It will allow a interface to be unpublished even if it has a high
  // volume of calls.
  if (ipc_interface->interface_descriptor != NULL)
  {
    (void)AZ_ULIB_PORT_ATOMIC_INC_W(&(ipc_interface->running_count));
    register const az_ulib_interface_descriptor* descriptor
        = (const az_ulib_interface_descriptor*)ipc_interface->interface_descriptor;

    if (descriptor == NULL)
    {
      /*az_ulib_ipc_call_unpublished_interface_failed*/
      result = AZ_ERROR_ITEM_NOT_FOUND;
    }
    else
    {
      /*az_ulib_ipc_call_calls_the_command_succeed*/
      result = descriptor->capability_list[command_index].capability_ptr_1.command(
          model_in, model_out);
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

  return result;
}
#else // AZ_ULIB_CONFIG_IPC_UNPUBLISH
AZ_NODISCARD az_result az_ulib_ipc_call(
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_capability_index command_index,
    const void* const model_in,
    const void* model_out)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_NOT_NULL(interface_handle);

  return ((_az_ulib_ipc_interface*)interface_handle)
      ->interface_descriptor->capability_list[command_index]
      .capability_ptr_1.command(model_in, model_out);
}
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
