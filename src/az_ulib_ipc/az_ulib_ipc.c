// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <memory.h>
#include <stdint.h>

#include "_az_ulib_interfaces.h"
#include "az_ulib_base.h"
#include "az_ulib_capability_api.h"
#include "az_ulib_config.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_interface_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_ipc_function_table.h"
#include "az_ulib_pal_api.h"
#include "az_ulib_registry_api.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"

#include <azure/core/internal/az_precondition_internal.h>

/**
 * @brief   IPC query continuation token.
 *
 * Structure to represent the continuation token in the IPC query. This is an internal structure,
 * developers shall not try to parse it.
 */
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

/**
 * @brief   Data stored in the Registry to recover IPC state after a device reset.
 */
typedef struct
{
  /** Interface flags following the #_az_ulib_ipc_flags. */
  uint32_t flags;

} ipc_registry_data;

#define MAX_INTERFACE_KEY_SIZE \
  (AZ_ULIB_CONFIG_MAX_DM_INTERFACE_NAME_VERSION + AZ_ULIB_CONFIG_MAX_DM_PACKAGE_NAME_VERSION + 1)

/**
 * @brief   IPC single instance.
 *
 * IPC is a singleton component, and shall be initialized only once.
 *
 * Make it volatile to avoid any compilation optimization.
 */
static az_ulib_ipc_control_block* volatile _az_ipc_control_block = NULL;

/*
 * This function follow the rules define in az_ulib_ipc_try_get_interface().
 */
static _az_ulib_ipc_interface* lookup_interface(
    az_span package_name,
    az_ulib_version package_version,
    az_span interface_name,
    az_ulib_version interface_version)
{
  _az_ulib_ipc_interface* interface_handle = NULL;

  // Find the lowest version that fits the criteria.
  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++)
  {
    const volatile az_ulib_interface_descriptor* const descriptor
        = _az_ipc_control_block->_internal.interface_list[i].interface_descriptor;

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
                _az_ipc_control_block->_internal.interface_list[i].flags,
                AZ_ULIB_IPC_FLAGS_DEFAULT))
        {
          // Use this interface.
          interface_handle = &(_az_ipc_control_block->_internal.interface_list[i]);
          break;
        }
      }
      // Package version matches.
      else if (package_version == descriptor->_internal.pkg_version)
      {
        interface_handle = &(_az_ipc_control_block->_internal.interface_list[i]);
        break;
      }
    }
  }

  return interface_handle;
}

static _az_ulib_ipc_interface* find_interface_descriptor(
    const az_ulib_interface_descriptor* interface_descriptor)
{
  _az_ulib_ipc_interface* result = NULL;

  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++)
  {
    if (_az_ipc_control_block->_internal.interface_list[i].interface_descriptor
        == interface_descriptor)
    {
      result = &(_az_ipc_control_block->_internal.interface_list[i]);
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
    if (_az_ipc_control_block->_internal.interface_list[i].ref_count == 0)
    {
      result = &(_az_ipc_control_block->_internal.interface_list[i]);
      break;
    }
  }

  return result;
}

static az_result try_lock_interface(_az_ulib_ipc_interface* ipc_interface)
{
  az_result result;

  if ((AZ_ULIB_FLAGS_IS_SET(ipc_interface->flags, AZ_ULIB_IPC_FLAGS_ON_HOLD))
      || (ipc_interface->ref_count < 1))
  {
    result = AZ_ERROR_ITEM_NOT_FOUND;
  }
  else if (ipc_interface->ref_count > AZ_ULIB_CONFIG_MAX_IPC_INSTANCES)
  {
    result = AZ_ERROR_NOT_ENOUGH_SPACE;
  }
  else
  {
    ipc_interface->ref_count++;
    result = AZ_OK;
  }

  return result;
}

static az_result publish_ipc_owned_interfaces(void)
{
  AZ_ULIB_TRY
  {
    AZ_ULIB_THROW_IF_AZ_ERROR(_az_ulib_ipc_query_interface_publish());
    AZ_ULIB_THROW_IF_AZ_ERROR(_az_ulib_ipc_interface_manager_interface_publish());
  }
  AZ_ULIB_CATCH(...) {}
  return AZ_ULIB_TRY_RESULT;
}

static az_result unpublish_ipc_owned_interfaces(void)
{
  AZ_ULIB_TRY
  {
    AZ_ULIB_THROW_IF_AZ_ERROR(_az_ulib_ipc_query_interface_unpublish());
    AZ_ULIB_THROW_IF_AZ_ERROR(_az_ulib_ipc_interface_manager_interface_unpublish());
  }
  AZ_ULIB_CATCH(...) {}
  return AZ_ULIB_TRY_RESULT;
}

AZ_NODISCARD az_result az_ulib_ipc_init(az_ulib_ipc_control_block* ipc_control_block)
{
  _az_PRECONDITION_IS_NULL(_az_ipc_control_block);
  _az_PRECONDITION_NOT_NULL(ipc_control_block);

  // Accept the control block memory.
  _az_ipc_control_block = ipc_control_block;

  // Prepare lock mechanism.
  az_pal_os_lock_init(&(_az_ipc_control_block->_internal.lock));

  // Random magic number. Just to avoid start from 0.
  _az_ipc_control_block->_internal.publish_count = 1;

  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++)
  {
    // Make each interface spot available.
    _az_ipc_control_block->_internal.interface_list[i].ref_count = 0;
    _az_ipc_control_block->_internal.interface_list[i].flags = AZ_ULIB_IPC_FLAGS_NONE;
    _az_ipc_control_block->_internal.interface_list[i].interface_descriptor = NULL;
  }

  // Publish the interfaces exposed by the IPC.
  return publish_ipc_owned_interfaces();
}

static az_result concat_name_version(
    az_span destination,
    az_span name,
    uint32_t version,
    az_span* out)
{
  uint8_t* buf = az_span_ptr(destination);
  int32_t buf_size = az_span_size(destination);

  az_span_to_str((char*)buf, buf_size, name);
  buf[az_span_size(name)] = '.';
  az_span version_span = az_span_create(
      (uint8_t*)&buf[az_span_size(name) + 1], (buf_size - (az_span_size(name) + 1)));
  az_span rest = AZ_SPAN_EMPTY;
  az_result result = az_span_i32toa(version_span, (int32_t)version, &rest);

  if (result == AZ_OK)
  {
    *out = az_span_create(
        buf, az_span_size(name) + 1 + az_span_size(version_span) - az_span_size(rest));
  }

  return result;
}

static az_result concat_full_name(
    az_span destination,
    az_span package_name,
    az_ulib_version package_version,
    az_span interface_name,
    az_ulib_version interface_version,
    az_span* out)
{
  AZ_ULIB_TRY
  {
    uint8_t* buf = az_span_ptr(destination);
    int32_t buf_size = az_span_size(destination);
    az_span package_out = AZ_SPAN_EMPTY;

    /* Add package name and version. */
    AZ_ULIB_THROW_IF_AZ_ERROR(
        concat_name_version(destination, package_name, package_version, &package_out));
    AZ_ULIB_THROW_IF_ERROR((az_span_size(package_out) < buf_size - 2), AZ_ERROR_NOT_ENOUGH_SPACE);

    /* Use '.' to split package from interface. */
    buf[az_span_size(package_out)] = '.';

    /* Add interface name and version. */
    az_span interface_out = az_span_create(
        &buf[az_span_size(package_out) + 1], buf_size - (az_span_size(package_out) + 1));
    AZ_ULIB_THROW_IF_AZ_ERROR(
        concat_name_version(interface_out, interface_name, interface_version, &interface_out));

    *out = az_span_create(buf, az_span_size(package_out) + 1 + az_span_size(interface_out));
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
}

static az_result get_interface_information_in_registry(
    _az_ulib_ipc_interface* ipc_interface,
    ipc_registry_data* registry_data)
{
  AZ_ULIB_TRY
  {
    uint8_t interface_full_name[MAX_INTERFACE_KEY_SIZE];
    az_span interface_span = az_span_create(interface_full_name, sizeof(interface_full_name));
    AZ_ULIB_THROW_IF_AZ_ERROR(concat_full_name(
        interface_span,
        ipc_interface->interface_descriptor->_internal.pkg_name,
        ipc_interface->interface_descriptor->_internal.pkg_version,
        ipc_interface->interface_descriptor->_internal.intf_name,
        ipc_interface->interface_descriptor->_internal.intf_version,
        &interface_span));

    az_span old_registry_data_span = AZ_SPAN_EMPTY;

    AZ_ULIB_THROW_IF_AZ_ERROR(
        az_ulib_registry_try_get_value(interface_span, &old_registry_data_span));

    AZ_ULIB_THROW_IF_ERROR(
        (az_span_size(old_registry_data_span) == sizeof(ipc_registry_data)), AZ_ERROR_ULIB_SYSTEM);
    registry_data->flags = ((ipc_registry_data*)az_span_ptr(old_registry_data_span))->flags;
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
}

static az_result delete_interface_information_in_registry(_az_ulib_ipc_interface* ipc_interface)
{
  AZ_ULIB_TRY
  {
    uint8_t interface_full_name[MAX_INTERFACE_KEY_SIZE];
    az_span interface_span = az_span_create(interface_full_name, sizeof(interface_full_name));
    AZ_ULIB_THROW_IF_AZ_ERROR(concat_full_name(
        interface_span,
        ipc_interface->interface_descriptor->_internal.pkg_name,
        ipc_interface->interface_descriptor->_internal.pkg_version,
        ipc_interface->interface_descriptor->_internal.intf_name,
        ipc_interface->interface_descriptor->_internal.intf_version,
        &interface_span));

    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_registry_delete(interface_span));
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
}

static az_result update_interface_information_in_registry(_az_ulib_ipc_interface* ipc_interface)
{
  AZ_ULIB_TRY
  {
    uint8_t interface_full_name[MAX_INTERFACE_KEY_SIZE];
    az_span interface_span = az_span_create(interface_full_name, sizeof(interface_full_name));
    AZ_ULIB_THROW_IF_AZ_ERROR(concat_full_name(
        interface_span,
        ipc_interface->interface_descriptor->_internal.pkg_name,
        ipc_interface->interface_descriptor->_internal.pkg_version,
        ipc_interface->interface_descriptor->_internal.intf_name,
        ipc_interface->interface_descriptor->_internal.intf_version,
        &interface_span));

    ipc_registry_data registry_data = { 0 };
    az_span new_registry_data_span
        = az_span_create((uint8_t*)&registry_data, sizeof(ipc_registry_data));
    az_span old_registry_data_span = AZ_SPAN_EMPTY;

    if (az_ulib_registry_try_get_value(interface_span, &old_registry_data_span) == AZ_OK)
    {
      AZ_ULIB_THROW_IF_ERROR(
          (az_span_size(old_registry_data_span) == sizeof(ipc_registry_data)),
          AZ_ERROR_ULIB_SYSTEM);

      if (((uint32_t)(ipc_interface->flags) & AZ_ULIB_IPC_FLAGS_DEFAULT)
          != (((ipc_registry_data*)az_span_ptr(old_registry_data_span))->flags
              & AZ_ULIB_IPC_FLAGS_DEFAULT))
      {
        registry_data.flags = (ipc_interface->flags & AZ_ULIB_IPC_FLAGS_DEFAULT);
        AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_registry_delete(interface_span));
        AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_registry_add(interface_span, new_registry_data_span));
      }
    }
    else
    {
      registry_data.flags = (ipc_interface->flags & AZ_ULIB_IPC_FLAGS_DEFAULT);
      AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_registry_add(interface_span, new_registry_data_span));
    }
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
}

AZ_NODISCARD az_result az_ulib_ipc_deinit(void)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_control_block);

  az_result result;

  if ((_az_ipc_control_block->_internal.interface_list[0].ref_count != 1)
      || (_az_ipc_control_block->_internal.interface_list[1].ref_count != 1))
  {
    result = AZ_ERROR_ULIB_BUSY;
  }
  else
  {
    result = AZ_OK;
    for (size_t i = 2; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++)
    {
      if (_az_ipc_control_block->_internal.interface_list[i].interface_descriptor != NULL)
      {
        result = AZ_ERROR_ULIB_BUSY;
        break;
      }
    }
  }

  if (result == AZ_OK)
  {
    (void)unpublish_ipc_owned_interfaces();
    az_pal_os_lock_deinit(&(_az_ipc_control_block->_internal.lock));
    _az_ipc_control_block = NULL;
  }

  return result;
}

AZ_NODISCARD az_result az_ulib_ipc_set_default(
    az_span package_name,
    az_ulib_version package_version,
    az_span interface_name,
    az_ulib_version interface_version)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_control_block);
  _az_PRECONDITION_VALID_SPAN(package_name, 1, false);
  _az_PRECONDITION_VALID_SPAN(interface_name, 1, false);

  az_result result;
  _az_ulib_ipc_interface* new_default_interface;

  if ((package_version == AZ_ULIB_VERSION_DEFAULT)
      || (interface_version == AZ_ULIB_VERSION_DEFAULT))
  {
    // Do not allows any default in this function.
    result = AZ_ERROR_ITEM_NOT_FOUND;
  }
  else
  {
    az_pal_os_lock_acquire(&(_az_ipc_control_block->_internal.lock));
    {
      // Find the interface to be the new default.
      if ((new_default_interface
           = lookup_interface(package_name, package_version, interface_name, interface_version))
          == NULL)
      {
        result = AZ_ERROR_ITEM_NOT_FOUND;
      }
      else
      {
        result = AZ_OK;
        _az_ulib_ipc_interface* old_default_interface;
        // Try to find the old default.
        if ((old_default_interface = lookup_interface(
                 package_name, AZ_ULIB_VERSION_DEFAULT, interface_name, interface_version))
            != NULL)
        {
          if (old_default_interface == new_default_interface)
          {
            result = AZ_ERROR_ARG;
          }
          else
          {
            // Set as not default anymore.
            old_default_interface->flags &= !AZ_ULIB_IPC_FLAGS_DEFAULT;

            // Force all old handle to renew and get the new default.
            old_default_interface->hash = (_az_ipc_control_block->_internal.publish_count++);

            // Change default in registry.
            result = update_interface_information_in_registry(old_default_interface);
          }
        }

        if (result == AZ_OK)
        {
          // Set new default interface.
          new_default_interface->flags |= AZ_ULIB_IPC_FLAGS_DEFAULT;

          // Change default in registry.
          result = update_interface_information_in_registry(new_default_interface);
        }
      }
    }
    az_pal_os_lock_release(&(_az_ipc_control_block->_internal.lock));
  }

  return result;
}

AZ_NODISCARD az_result
az_ulib_ipc_publish(const az_ulib_interface_descriptor* const interface_descriptor)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_control_block);
  _az_PRECONDITION_NOT_NULL(interface_descriptor);

  az_result result;
  _az_ulib_ipc_interface* new_interface;

  if ((interface_descriptor->_internal.pkg_version == AZ_ULIB_VERSION_DEFAULT)
      || (interface_descriptor->_internal.intf_version == AZ_ULIB_VERSION_DEFAULT))
  {
    // IPC shall not allow publish an interface using the default version.
    result = AZ_ERROR_ARG;
  }
  else
  {
    az_pal_os_lock_acquire(&(_az_ipc_control_block->_internal.lock));
    {
      if (lookup_interface(
              interface_descriptor->_internal.pkg_name,
              interface_descriptor->_internal.pkg_version,
              interface_descriptor->_internal.intf_name,
              interface_descriptor->_internal.intf_version)
          != NULL)
      {
        // IPC shall not accept interfaces with same name and version because it cannot
        // decided each one to retrieve when az_ulib_ipc_try_get_interface() is called.
        result = AZ_ERROR_ULIB_ELEMENT_DUPLICATE;
      }
      else
      {
        if (((new_interface = get_first_free()) == NULL) || // interface with ref_count == 0.
            (az_span_size(interface_descriptor->_internal.intf_name)
             >= AZ_ULIB_CONFIG_MAX_DM_INTERFACE_NAME))
        {
          result = AZ_ERROR_NOT_ENOUGH_SPACE;
        }
        else
        {
          new_interface->interface_descriptor = interface_descriptor;
          new_interface->flags = AZ_ULIB_IPC_FLAGS_NONE;
          new_interface->hash = (_az_ipc_control_block->_internal.publish_count++);
          AZ_ULIB_PORT_GET_DATA_CONTEXT(&(new_interface->data_base_address));

          ipc_registry_data registry_data;
          if (get_interface_information_in_registry(new_interface, &registry_data) == AZ_OK)
          {
            if (AZ_ULIB_FLAGS_IS_SET(registry_data.flags, AZ_ULIB_IPC_FLAGS_DEFAULT))
            {
              result = az_ulib_ipc_set_default(
                  interface_descriptor->_internal.pkg_name,
                  interface_descriptor->_internal.pkg_version,
                  interface_descriptor->_internal.intf_name,
                  interface_descriptor->_internal.intf_version);
            }
          }
          else
          {
            // Look up for default.
            if (lookup_interface(
                    interface_descriptor->_internal.pkg_name,
                    AZ_ULIB_VERSION_DEFAULT,
                    interface_descriptor->_internal.intf_name,
                    interface_descriptor->_internal.intf_version)
                == NULL)
            {
              // No other package exposes this interface, so make it default.
              new_interface->flags = AZ_ULIB_IPC_FLAGS_DEFAULT;
              result = update_interface_information_in_registry(new_interface);
            }
          }

          // ref_count >= 1 means that this is a valid interface.
          new_interface->ref_count = 1;

          result = AZ_OK;
        }
      }
    }
    az_pal_os_lock_release(&(_az_ipc_control_block->_internal.lock));
  }

  return result;
}

AZ_NODISCARD az_result az_ulib_ipc_unpublish(
    const az_ulib_interface_descriptor* const interface_descriptor,
    uint32_t wait_option_ms)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_control_block);
  _az_PRECONDITION_NOT_NULL(interface_descriptor);

  az_result result;

  // We are using a retry loop to wait for the interface to be free. A semaphore would
  // probably be a better solution, however, our PAL does not support it yet. So, we will
  // implement it in a separated Feature [10869774].
  uint32_t retry_interval;
  uint32_t retry_total_time = 0;
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

  _az_ulib_ipc_interface* release_interface = find_interface_descriptor(interface_descriptor);

  if (release_interface == NULL)
  {
    result = AZ_ERROR_ITEM_NOT_FOUND;
  }
  else
  {
    do
    {
      az_pal_os_lock_acquire(&(_az_ipc_control_block->_internal.lock));
      {
        if (release_interface->interface_descriptor != interface_descriptor)
        {
          result = AZ_OK;
        }
        else
        {
          if (release_interface->ref_count == 1)
          {
            // Nobody is using this interface, just unpublish.
            result = delete_interface_information_in_registry(release_interface);
            if ((result == AZ_OK) || (result == AZ_ERROR_ITEM_NOT_FOUND))
            {
              release_interface->interface_descriptor = NULL;
              release_interface->ref_count = 0;
              result = AZ_OK;
            }
          }
          // Someone is using this interface.
          else if (retry_total_time < wait_option_ms) // Shall keep waiting.
          {
            if (wait_option_ms != AZ_ULIB_WAIT_FOREVER)
            {
              // It will not wait forever, so increment the waiting time.
              retry_total_time += retry_interval;
            }
            // Put this interface on hold, so try_get_interface will fail, it will give
            // this interface chance to be unpublished.
            release_interface->flags |= AZ_ULIB_IPC_FLAGS_ON_HOLD;
            result = AZ_ULIB_PENDING;
          }
          else
          {
            // Times up, free the interface and return Busy to the caller.
            release_interface->flags &= !AZ_ULIB_IPC_FLAGS_ON_HOLD;
            result = AZ_ERROR_ULIB_BUSY;
          }
        }
      }
      az_pal_os_lock_release(&(_az_ipc_control_block->_internal.lock));

      if (result == AZ_ULIB_PENDING)
      {
        // Give other threads chance to release this interface. It shall be outside of the
        // "lock".
        az_pal_os_sleep(retry_interval);
      }

    } while (result == AZ_ULIB_PENDING);
  }

  return result;
}

AZ_NODISCARD az_result az_ulib_ipc_try_get_interface(
    az_span device_name,
    az_span package_name,
    az_ulib_version package_version,
    az_span interface_name,
    az_ulib_version interface_version,
    az_ulib_ipc_interface_handle* interface_handle)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_control_block);
  _az_PRECONDITION_VALID_SPAN(package_name, 1, false);
  _az_PRECONDITION_VALID_SPAN(interface_name, 1, false);
  _az_PRECONDITION(interface_version != AZ_ULIB_VERSION_DEFAULT);
  _az_PRECONDITION_NOT_NULL(interface_handle);

  az_result result;

  if (az_span_size(device_name) != 0)
  {
    // We do not support daughter device yet.
    result = AZ_ERROR_NOT_IMPLEMENTED;
  }
  else
  {
    az_pal_os_lock_acquire(&(_az_ipc_control_block->_internal.lock));
    {
      _az_ulib_ipc_interface* ipc_interface = interface_handle->_internal.ipc_interface;

      if ((ipc_interface != NULL)
          && (interface_handle->_internal.interface_hash == ipc_interface->hash))
      {
        // The interface handle is still valid? Reuse it.
        result = try_lock_interface(ipc_interface);
      }
      // Current handle is not valid. Get interface from names.
      else
      {
        ipc_interface
            = lookup_interface(package_name, package_version, interface_name, interface_version);

        if (ipc_interface == NULL)
        {
          // Could not find the interface using name.
          result = AZ_ERROR_ITEM_NOT_FOUND;
        }
        else if ((result = try_lock_interface(ipc_interface)) == AZ_OK)
        {
          // Lock the interface and return the handle.
          interface_handle->_internal.ipc_interface = ipc_interface;
          interface_handle->_internal.interface_hash = ipc_interface->hash;
          result = AZ_ULIB_RENEW;
        }
      }
    }
    az_pal_os_lock_release(&(_az_ipc_control_block->_internal.lock));
  }

  return result;
}

AZ_NODISCARD az_result az_ulib_ipc_try_get_capability(
    az_ulib_ipc_interface_handle interface_handle,
    az_span name,
    az_ulib_capability_index* capability_index)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_control_block);
  _az_PRECONDITION_VALID_SPAN(name, 1, false);
  _az_PRECONDITION_NOT_NULL(capability_index);

  const volatile az_ulib_interface_descriptor* descriptor
      = interface_handle._internal.ipc_interface->interface_descriptor;

  for (az_ulib_capability_index index = 0; index < descriptor->_internal.size; index++)
  {
    if (az_span_is_content_equal(name, descriptor->_internal.capability_list[index]._internal.name))
    {
      *capability_index = index;
      return AZ_OK;
    }
  }

  return AZ_ERROR_ITEM_NOT_FOUND;
}

AZ_NODISCARD az_result az_ulib_ipc_release_interface(az_ulib_ipc_interface_handle interface_handle)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_control_block);

  az_pal_os_lock_acquire(&(_az_ipc_control_block->_internal.lock));
  {
    interface_handle._internal.ipc_interface->ref_count--;
  }
  az_pal_os_lock_release(&(_az_ipc_control_block->_internal.lock));

  return AZ_OK;
}

AZ_NODISCARD az_result az_ulib_ipc_call(
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_capability_index capability_index,
    az_ulib_model_in model_in,
    az_ulib_model_out model_out)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_control_block);

  _az_ulib_ipc_interface* ipc_interface = interface_handle._internal.ipc_interface;

  AZ_ULIB_PORT_SET_DATA_CONTEXT(ipc_interface->data_base_address);
  return ipc_interface->interface_descriptor->_internal.capability_list[capability_index]
      ._internal.capability_ptr(model_in, model_out);
}

AZ_NODISCARD az_result az_ulib_ipc_call_with_str(
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_capability_index capability_index,
    az_span model_in_span,
    az_span* model_out_span)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_control_block);

  az_result result;

  _az_ulib_ipc_interface* ipc_interface = interface_handle._internal.ipc_interface;
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
  _az_PRECONDITION_NOT_NULL(_az_ipc_control_block);
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
        = _az_ipc_control_block->_internal.interface_list[interface_index].interface_descriptor;
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
                  _az_ipc_control_block->_internal.interface_list[interface_index].flags,
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
  _az_PRECONDITION_NOT_NULL(_az_ipc_control_block);
  _az_PRECONDITION_NOT_NULL(result);
  _az_PRECONDITION_VALID_SPAN(*result, 1, false);
  _az_PRECONDITION_NOT_NULL(continuation_token);
  az_result res;

  az_pal_os_lock_acquire(&(_az_ipc_control_block->_internal.lock));
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
  az_pal_os_lock_release(&(_az_ipc_control_block->_internal.lock));

  return res;
}

AZ_NODISCARD az_result az_ulib_ipc_query_next(uint32_t* continuation_token, az_span* result)
{
  _az_PRECONDITION_NOT_NULL(_az_ipc_control_block);
  _az_PRECONDITION_NOT_NULL(result);
  _az_PRECONDITION_VALID_SPAN(*result, 1, false);
  _az_PRECONDITION_NOT_NULL(continuation_token);
  az_result res;

  az_pal_os_lock_acquire(&(_az_ipc_control_block->_internal.lock));
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
  az_pal_os_lock_release(&(_az_ipc_control_block->_internal.lock));

  return res;
}

static const az_ulib_ipc_function_table _table
    = { .publish = az_ulib_ipc_publish,
        .set_default = az_ulib_ipc_set_default,
        .unpublish = az_ulib_ipc_unpublish,
        .try_get_interface = az_ulib_ipc_try_get_interface,
        .try_get_capability = az_ulib_ipc_try_get_capability,
        .release_interface = az_ulib_ipc_release_interface,
        .call = az_ulib_ipc_call,
        .call_with_str = az_ulib_ipc_call_with_str,
        .split_method_name = az_ulib_ipc_split_method_name,
        .query = az_ulib_ipc_query,
        .query_next = az_ulib_ipc_query_next };

const az_ulib_ipc_function_table* az_ulib_ipc_get_function_table(void) { return &_table; }
