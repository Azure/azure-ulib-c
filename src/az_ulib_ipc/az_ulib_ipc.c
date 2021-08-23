// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <memory.h>
#include <stdint.h>

#include "_az_ulib_interfaces.h"
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

/*
 * This function follow the rules define in az_ulib_ipc_try_get_interface().
 */
static az_result get_interface(
    az_span package_name,
    az_ulib_version package_version,
    az_span interface_name,
    az_ulib_version interface_version,
    _az_ulib_ipc_interface** handle)
{
  _az_ulib_ipc_interface* interface_handle = NULL;
  az_result result = AZ_ERROR_ITEM_NOT_FOUND;

  // Find the lowest version that fits the criteria.
  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++)
  {
    const volatile az_ulib_interface_descriptor* const descriptor
        = _az_ipc_cb->_internal.interface_list[i].interface_descriptor;

    // Is this interface valid and does it matches the criteria?
    if ((descriptor != NULL)
        // Does the interface name matches.
        && az_span_is_content_equal(descriptor->_internal.intf_name, interface_name)
        // Does the interface version matches.
        && (interface_version == descriptor->_internal.intf_version)
        // Does the package name matches.
        && az_span_is_content_equal(descriptor->_internal.pkg_name, package_name))
    {
      if (package_version == AZ_ULIB_VERSION_DEFAULT) // Use default package version.
      {
        // Is this the default version for this package.
        if (AZ_ULIB_FLAGS_IS_SET(
                _az_ipc_cb->_internal.interface_list[i].flags, AZ_ULIB_IPC_FLAGS_DEFAULT))
        {
          // Use this interface.
          result = AZ_OK;
          interface_handle = &(_az_ipc_cb->_internal.interface_list[i]);
          break;
        }
      }
      // Package version matches.
      else if (package_version == descriptor->_internal.pkg_version)
      {
        result = AZ_OK;
        interface_handle = &(_az_ipc_cb->_internal.interface_list[i]);
        break;
      }
    }
  }

  if ((handle != NULL) && (result == AZ_OK))
  {
    *handle = interface_handle;
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

static az_result publish_ipc_interfaces(void)
{
  AZ_ULIB_TRY
  {
    AZ_ULIB_THROW_IF_AZ_ERROR(_az_ulib_ipc_query_interface_publish());
    AZ_ULIB_THROW_IF_AZ_ERROR(_az_ulib_ipc_interface_manager_interface_publish());
  }
  AZ_ULIB_CATCH(...) {}
  return AZ_ULIB_TRY_RESULT;
}

static az_result unpublish_ipc_interfaces(void)
{
  AZ_ULIB_TRY
  {
    AZ_ULIB_THROW_IF_AZ_ERROR(_az_ulib_ipc_query_interface_unpublish());
    AZ_ULIB_THROW_IF_AZ_ERROR(_az_ulib_ipc_interface_manager_interface_unpublish());
  }
  AZ_ULIB_CATCH(...) {}
  return AZ_ULIB_TRY_RESULT;
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
    _az_ipc_cb->_internal.interface_list[i].flags = AZ_ULIB_IPC_FLAGS_NONE;
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
    _az_ipc_cb->_internal.interface_list[i].running_count = 0;
    _az_ipc_cb->_internal.interface_list[i].running_count_low_watermark = 0;
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
    _az_ipc_cb->_internal.interface_list[i].interface_descriptor = NULL;
  }

  return publish_ipc_interfaces();
}

AZ_NODISCARD az_result az_ulib_ipc_deinit(void)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);

  az_result result;

  result = unpublish_ipc_interfaces();

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
      (void)publish_ipc_interfaces();
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

  if ((interface_descriptor->_internal.pkg_version == AZ_ULIB_VERSION_DEFAULT)
      || (interface_descriptor->_internal.intf_version == AZ_ULIB_VERSION_DEFAULT))
  {
    result = AZ_ERROR_ARG;
  }
  else
  {
    az_pal_os_lock_acquire(&(_az_ipc_cb->_internal.lock));
    {
      if (get_interface(
              interface_descriptor->_internal.pkg_name,
              interface_descriptor->_internal.pkg_version,
              interface_descriptor->_internal.intf_name,
              interface_descriptor->_internal.intf_version,
              NULL)
          == AZ_OK)
      {
        // IPC shall not accept interfaces with same name and version because it cannot decided
        // each one to retrieve when someone uses az_ulib_ipc_try_get_interface().
        result = AZ_ERROR_ULIB_ELEMENT_DUPLICATE;
      }
      else
      {
        if ((new_interface = get_first_free()) == NULL)
        {
          result = AZ_ERROR_NOT_ENOUGH_SPACE;
        }
        else
        {
          (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(
              (const volatile void**)&(new_interface->interface_descriptor),
              (const void*)interface_descriptor);
          new_interface->ref_count = 0;
          new_interface->flags = AZ_ULIB_IPC_FLAGS_NONE;
          AZ_ULIB_PORT_GET_DATA_CONTEXT(&(new_interface->data_base_address));
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
          new_interface->running_count = 0;
          new_interface->running_count_low_watermark = 0;
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH

          // Look up for default.
          if (get_interface(
                  interface_descriptor->_internal.pkg_name,
                  AZ_ULIB_VERSION_DEFAULT,
                  interface_descriptor->_internal.intf_name,
                  interface_descriptor->_internal.intf_version,
                  NULL)
              != AZ_OK)
          {
            new_interface->flags = AZ_ULIB_IPC_FLAGS_DEFAULT;
          }

          // If requested, returns the interface handle.
          if (interface_handle != NULL)
          {
            *interface_handle = new_interface;
          }
          result = AZ_OK;
        }
      }
    }
    az_pal_os_lock_release(&(_az_ipc_cb->_internal.lock));
  }

  return result;
}

AZ_NODISCARD az_result az_ulib_ipc_set_default(
    az_span package_name,
    az_ulib_version package_version,
    az_span interface_name,
    az_ulib_version interface_version)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_VALID_SPAN(package_name, 1, false);
  _az_PRECONDITION_VALID_SPAN(interface_name, 1, false);

  az_result result;
  _az_ulib_ipc_interface* new_default_interface;

  if ((package_version == AZ_ULIB_VERSION_DEFAULT)
      || (interface_version == AZ_ULIB_VERSION_DEFAULT))
  {
    result = AZ_ERROR_ARG;
  }
  else
  {
    az_pal_os_lock_acquire(&(_az_ipc_cb->_internal.lock));
    {
      // Find the interface to be the new default.
      if (get_interface(
              package_name,
              package_version,
              interface_name,
              interface_version,
              &new_default_interface)
          != AZ_OK)
      {
        result = AZ_ERROR_ITEM_NOT_FOUND;
      }
      else
      {
        _az_ulib_ipc_interface* old_default_interface;
        // Try to find the old default.
        if (get_interface(
                package_name,
                AZ_ULIB_VERSION_DEFAULT,
                interface_name,
                interface_version,
                &old_default_interface)
            == AZ_OK)
        {
          // Set as not default anymore.
          old_default_interface->flags &= !AZ_ULIB_IPC_FLAGS_DEFAULT;
        }

        // Set new default interface.
        new_default_interface->flags |= AZ_ULIB_IPC_FLAGS_DEFAULT;
        result = AZ_OK;
      }
    }
    az_pal_os_lock_release(&(_az_ipc_cb->_internal.lock));
  }

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
      // The order of the code here, including the ones that looks not necessary, are associated
      // to the interlock between this function and the az_ulib_ipc_call.

      // Block access to this interface. After this point, any new call to az_ulib_ipc_call that
      // didn't get the interface pointer yet will return AZ_ERROR_ITEM_NOT_FOUND.
      (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(
          (const volatile void**)(&(release_interface->interface_descriptor)), (const void*)NULL);

      // If the running_count is `0` is because no other process is inside of any of the
      // capabilities call, and they may be removed from the memory. There will be the case that
      // the other process is already in the az_ulib_ipc_call, in the direction to call a
      // capability in this interface, but the call will just return AZ_ERROR_ITEM_NOT_FOUND from
      // there.
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
        release_interface->flags = AZ_ULIB_IPC_FLAGS_NONE;
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
    az_span device_name,
    az_span package_name,
    az_ulib_version package_version,
    az_span interface_name,
    az_ulib_version interface_version,
    az_ulib_ipc_interface_handle* interface_handle)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_VALID_SPAN(package_name, 1, false);
  _az_PRECONDITION_VALID_SPAN(interface_name, 1, false);
  _az_PRECONDITION(interface_version != AZ_ULIB_VERSION_DEFAULT);
  _az_PRECONDITION_NOT_NULL(interface_handle);

  if (az_span_size(device_name) != 0)
  {
    return AZ_ERROR_NOT_IMPLEMENTED;
  }

  az_result result;
  _az_ulib_ipc_interface* ipc_interface;

  az_pal_os_lock_acquire(&(_az_ipc_cb->_internal.lock));
  {
    if ((result = get_interface(
             package_name, package_version, interface_name, interface_version, &ipc_interface))
        == AZ_OK)
    {
      if ((result = get_instance(ipc_interface)) == AZ_OK)
      {
        *interface_handle = ipc_interface;
      }
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

  az_pal_os_lock_acquire(&(_az_ipc_cb->_internal.lock));
  {
    result = AZ_ERROR_ITEM_NOT_FOUND;
    if (ipc_interface->interface_descriptor != NULL)
    {
      for (az_ulib_capability_index index = 0;
           index < ipc_interface->interface_descriptor->_internal.size;
           index++)
      {
        if (az_span_is_content_equal(
                name,
                ipc_interface->interface_descriptor->_internal.capability_list[index]
                    ._internal.name))
        {
          *capability_index = index;
          result = AZ_OK;
          break;
        }
      }
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
    az_ulib_capability_index capability_index,
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
      AZ_ULIB_PORT_SET_DATA_CONTEXT(ipc_interface->data_base_address);
      result = ipc_interface->interface_descriptor->_internal.capability_list[capability_index]
                   ._internal.capability_ptr(model_in, model_out);
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

AZ_NODISCARD az_result az_ulib_ipc_call_with_str(
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_capability_index capability_index,
    az_span model_in_span,
    az_span* model_out_span)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_NOT_NULL(interface_handle);

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
      az_ulib_capability_span_wrapper capability_span_wrapper
          = ipc_interface->interface_descriptor->_internal.capability_list[capability_index]
                ._internal.capability_span_wrapper;

      if (capability_span_wrapper != NULL)
      {
        AZ_ULIB_PORT_SET_DATA_CONTEXT(ipc_interface->data_base_address);
        result = capability_span_wrapper(model_in_span, model_out_span);
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

AZ_NODISCARD az_result az_ulib_ipc_split_method_name(
    az_span full_name,
    az_span* device_name,
    az_span* package_name,
    uint32_t* package_version,
    az_span* interface_name,
    uint32_t* interface_version,
    az_span* capability_name)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_cb);
  _az_PRECONDITION_VALID_SPAN(full_name, 1, false);
  _az_PRECONDITION_NOT_NULL(device_name);
  _az_PRECONDITION_NOT_NULL(package_name);
  _az_PRECONDITION_NOT_NULL(package_version);
  _az_PRECONDITION_NOT_NULL(interface_name);
  _az_PRECONDITION_NOT_NULL(interface_version);
  _az_PRECONDITION_NOT_NULL(capability_name);

  AZ_ULIB_TRY
  {
    uint32_t package_version_temp;
    uint32_t interface_version_temp;
    az_span device_name_temp;
    az_span package_name_temp;
    az_span interface_name_temp;
    az_span capability_name_temp;
    int32_t split_pos;

    // Extract the device name, if provided.
    split_pos = az_span_find(full_name, AZ_SPAN_FROM_STR("@"));
    if (split_pos > 0)
    {
      device_name_temp = az_span_slice(full_name, 0, split_pos);
      full_name = az_span_slice_to_end(full_name, split_pos + 1);
    }
    else
    {
      device_name_temp = AZ_SPAN_EMPTY;
    }

    // Extract the capability name, if provided.
    split_pos = az_span_find(full_name, AZ_SPAN_FROM_STR(":"));
    if (split_pos > 0)
    {
      capability_name_temp = az_span_slice_to_end(full_name, split_pos + 1);
      full_name = az_span_slice(full_name, 0, split_pos);
    }
    else
    {
      capability_name_temp = AZ_SPAN_EMPTY;
    }

    // Extract the package name
    split_pos = az_span_find(full_name, AZ_SPAN_FROM_STR("."));
    AZ_ULIB_THROW_IF_ERROR((split_pos > 0), AZ_ERROR_UNEXPECTED_CHAR);
    package_name_temp = az_span_slice(full_name, 0, split_pos);
    full_name = az_span_slice_to_end(full_name, split_pos + 1);

    // Extract the package version
    split_pos = az_span_find(full_name, AZ_SPAN_FROM_STR("."));
    AZ_ULIB_THROW_IF_ERROR((split_pos > 0), AZ_ERROR_UNEXPECTED_CHAR);
    if (*az_span_ptr(full_name) == '*')
    {
      AZ_ULIB_THROW_IF_ERROR((split_pos == 1), AZ_ERROR_UNEXPECTED_CHAR);
      package_version_temp = AZ_ULIB_VERSION_DEFAULT;
    }
    else
    {
      AZ_ULIB_THROW_IF_AZ_ERROR(
          az_span_atou32(az_span_slice(full_name, 0, split_pos), &package_version_temp));
    }
    full_name = az_span_slice_to_end(full_name, split_pos + 1);

    // Extract the interface name
    split_pos = az_span_find(full_name, AZ_SPAN_FROM_STR("."));
    AZ_ULIB_THROW_IF_ERROR((split_pos > 0), AZ_ERROR_UNEXPECTED_CHAR);
    interface_name_temp = az_span_slice(full_name, 0, split_pos);
    full_name = az_span_slice_to_end(full_name, split_pos + 1);

    // Extract the interface version
    AZ_ULIB_THROW_IF_AZ_ERROR(az_span_atou32(full_name, &interface_version_temp));

    // Transfer the parser data to the output variables.
    if (az_span_size(device_name_temp) > 0)
    {
      *device_name = az_span_create(az_span_ptr(device_name_temp), az_span_size(device_name_temp));
    }
    else
    {
      *device_name = AZ_SPAN_EMPTY;
    }
    *package_name = az_span_create(az_span_ptr(package_name_temp), az_span_size(package_name_temp));
    *package_version = package_version_temp;
    *interface_name
        = az_span_create(az_span_ptr(interface_name_temp), az_span_size(interface_name_temp));
    *interface_version = interface_version_temp;
    if (az_span_size(capability_name_temp) > 0)
    {
      *capability_name
          = az_span_create(az_span_ptr(capability_name_temp), az_span_size(capability_name_temp));
    }
    else
    {
      *capability_name = AZ_SPAN_EMPTY;
    }
  }
  AZ_ULIB_CATCH(...) {}
  return AZ_ULIB_TRY_RESULT;
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
    const volatile az_ulib_interface_descriptor* const descriptor
        = _az_ipc_cb->_internal.interface_list[interface_index].interface_descriptor;
    if (descriptor != NULL)
    {
      int32_t next_size = az_span_size(descriptor->_internal.pkg_name)
          + az_span_size(descriptor->_internal.intf_name)
          + 6; // 6 = '"', '*', '.', '.', '.' and '"'

      char package_version_str[AZ_ULIB_STRINGIFIED_VERSION_SIZE];
      az_span package_version_span = AZ_SPAN_FROM_BUFFER(package_version_str);
      char interface_version_str[AZ_ULIB_STRINGIFIED_VERSION_SIZE];
      az_span interface_version_span = AZ_SPAN_FROM_BUFFER(interface_version_str);
      az_span remainder;

      if ((res
           = az_span_u32toa(package_version_span, descriptor->_internal.pkg_version, &remainder))
          == AZ_OK)
      {
        int32_t next_package_version_size
            = az_span_size(package_version_span) - az_span_size(remainder);
        next_size += next_package_version_size;

        if ((res = az_span_u32toa(
                 interface_version_span, descriptor->_internal.intf_version, &remainder))
            == AZ_OK)
        {
          int32_t next_interface_version_size
              = az_span_size(interface_version_span) - az_span_size(remainder);
          next_size += next_interface_version_size;

          if (pos == 0)
          {
            if (next_size > result_size)
            {
              res = AZ_ERROR_NOT_ENOUGH_SPACE;
              break;
            }
          }
          else
          {
            next_size++; // 1 = ','
            if (next_size > result_size - pos)
            {
              break;
            }
            else
            {
              result_str[pos++] = ',';
            }
          }

          res = AZ_OK;
          result_str[pos++] = '"';
          if (AZ_ULIB_FLAGS_IS_SET(
                  _az_ipc_cb->_internal.interface_list[interface_index].flags,
                  AZ_ULIB_IPC_FLAGS_DEFAULT))
          {
            result_str[pos++] = '*';
          }
          else
          {

            result_str[pos++] = ' ';
          }
          memcpy(
              &(result_str[pos]),
              az_span_ptr(descriptor->_internal.pkg_name),
              (size_t)az_span_size(descriptor->_internal.pkg_name));
          pos += az_span_size(descriptor->_internal.pkg_name);
          result_str[pos++] = '.';
          memcpy(
              &(result_str[pos]),
              az_span_ptr(package_version_span),
              (size_t)next_package_version_size);
          pos += next_package_version_size;
          result_str[pos++] = '.';
          memcpy(
              &(result_str[pos]),
              az_span_ptr(descriptor->_internal.intf_name),
              (size_t)az_span_size(descriptor->_internal.intf_name));
          pos += az_span_size(descriptor->_internal.intf_name);
          result_str[pos++] = '.';
          memcpy(
              &(result_str[pos]),
              az_span_ptr(interface_version_span),
              (size_t)next_interface_version_size);
          pos += next_interface_version_size;
          result_str[pos++] = '"';
        }
        else
        {
          break;
        }
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
      res = AZ_ERROR_NOT_SUPPORTED;
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
      res = AZ_ERROR_NOT_SUPPORTED;
    }
  }
  az_pal_os_lock_release(&(_az_ipc_cb->_internal.lock));

  return res;
}

static const az_ulib_ipc_table _table = { .publish = az_ulib_ipc_publish,
                                          .set_default = az_ulib_ipc_set_default,
                                          .unpublish = az_ulib_ipc_unpublish,
                                          .try_get_interface = az_ulib_ipc_try_get_interface,
                                          .try_get_capability = az_ulib_ipc_try_get_capability,
                                          .get_interface = az_ulib_ipc_get_interface,
                                          .release_interface = az_ulib_ipc_release_interface,
                                          .call = az_ulib_ipc_call,
                                          .call_with_str = az_ulib_ipc_call_with_str,
                                          .query = az_ulib_ipc_query,
                                          .query_next = az_ulib_ipc_query_next };

const az_ulib_ipc_table* az_ulib_ipc_get_table(void) { return &_table; }
