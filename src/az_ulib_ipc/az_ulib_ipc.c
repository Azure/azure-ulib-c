// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stdint.h>

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

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
#include "internal/az_ulib_ipc.h"

/*
 * IPC is a singleton component, and shall be initialized only once.
 */
static _az_ulib_ipc* ipc = NULL;

static _az_ulib_ipc_interface* get_interface(
    const char* const name,
    az_ulib_version version,
    az_ulib_version_match_criteria match_criteria) {
  _az_ulib_ipc_interface* result = NULL;

  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++) {
    if ((ipc->interface_list[i].interface_descriptor != NULL)
        && (strcmp(ipc->interface_list[i].interface_descriptor->name, name) == 0)
        && az_ulib_version_match(
            ipc->interface_list[i].interface_descriptor->version, version, match_criteria)) {
      result = &(ipc->interface_list[i]);
      break;
    }
  }

  return result;
}

static _az_ulib_ipc_interface* get_first_free() {
  _az_ulib_ipc_interface* result = NULL;

  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++) {
    if ((ipc->interface_list[i].interface_descriptor == NULL)
        && (ipc->interface_list[i].ref_count == 0)) {
      result = &(ipc->interface_list[i]);
      break;
    }
  }

  return result;
}

static az_ulib_result get_instance(_az_ulib_ipc_interface* ipc_interface) {
  az_ulib_result result;
  if (ipc_interface->ref_count >= AZ_ULIB_CONFIG_MAX_IPC_INSTANCES) {
    result = AZ_ULIB_BUSY_ERROR;
  } else {
    result = AZ_ULIB_SUCCESS;
    (void)AZ_ULIB_PORT_ATOMIC_INC_W(&(ipc_interface->ref_count));
  }
  return result;
}

az_ulib_result _az_ulib_ipc_init_no_contract(_az_ulib_ipc* handle) {
  /*az_ulib_ipc_init_succeed*/
  ipc = handle;

  az_pal_os_lock_init(&(ipc->lock));

  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++) {
    ipc->interface_list[i].ref_count = 0;
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
    ipc->interface_list[i].running_count = 0;
    ipc->interface_list[i].running_count_low_watermark = 0;
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
    ipc->interface_list[i].interface_descriptor = NULL;
  }

  return AZ_ULIB_SUCCESS;
}

az_ulib_result _az_ulib_ipc_init(_az_ulib_ipc* handle) {
  AZ_ULIB_UCONTRACT(
      /*az_ulib_ipc_init_double_initialization_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NULL(ipc, AZ_ULIB_ALREADY_INITIALIZED_ERROR),
      /*az_ulib_ipc_init_with_null_handle_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_ipc_init_no_contract(handle);
}

az_ulib_result _az_ulib_ipc_deinit_no_contract(void) {
  az_ulib_result result;

  result = AZ_ULIB_SUCCESS;
  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++) {
    if ((ipc->interface_list[i].interface_descriptor != NULL)
        || (ipc->interface_list[i].ref_count != 0)
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
        || (ipc->interface_list[i].running_count != 0)
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
    ) {
      /*az_ulib_ipc_deinit_with_published_interface_failed*/
      /*az_ulib_ipc_deinit_with_instace_failed*/
      result = AZ_ULIB_BUSY_ERROR;
      break;
    }
  }

  if (result == AZ_ULIB_SUCCESS) {
    /*az_ulib_ipc_deinit_succeed*/
    az_pal_os_lock_deinit(&(ipc->lock));
    ipc = NULL;
  }

  return result;
}

az_ulib_result _az_ulib_ipc_deinit(void) {
  AZ_ULIB_UCONTRACT(
      /*az_ulib_ipc_deinit_with_ipc_not_initialized_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(ipc, AZ_ULIB_NOT_INITIALIZED_ERROR));
  return _az_ulib_ipc_deinit_no_contract();
}

az_ulib_result _az_ulib_ipc_publish_no_contract(
    const az_ulib_interface_descriptor* interface_descriptor,
    _az_ulib_ipc_interface_handle* interface_handle) {
  az_ulib_result result;
  _az_ulib_ipc_interface* new_interface;

  az_pal_os_lock_acquire(&(ipc->lock));
  {
    if (get_interface(
            interface_descriptor->name, interface_descriptor->version, AZ_ULIB_VERSION_EQUALS_TO)
        != NULL) {
      /*az_ulib_ipc_publish_with__descriptor_with_same_name_and_version_failed*/
      result = AZ_ULIB_ELEMENT_DUPLICATE_ERROR;
    } else if ((new_interface = get_first_free()) == NULL) {
      /*az_ulib_ipc_publish_out_of_memory_failed*/
      result = AZ_ULIB_OUT_OF_MEMORY_ERROR;
    } else {
      /*az_ulib_ipc_publish_succeed*/
      (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(
          &(new_interface->interface_descriptor), interface_descriptor);
      new_interface->ref_count = 0;
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
      new_interface->running_count = 0;
      new_interface->running_count_low_watermark = 0;
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
      if (interface_handle != NULL) {
        /*az_ulib_ipc_publish_return_handle_succeed*/
        *interface_handle = new_interface;
      }
      result = AZ_ULIB_SUCCESS;
    }
  }
  az_pal_os_lock_release(&(ipc->lock));

  return result;
}

az_ulib_result _az_ulib_ipc_publish(
    const az_ulib_interface_descriptor* interface_descriptor,
    _az_ulib_ipc_interface_handle* interface_handle) {
  AZ_ULIB_UCONTRACT(
      /*az_ulib_ipc_publish_with_non_initialized_ipc_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(ipc, AZ_ULIB_NOT_INITIALIZED_ERROR),
      /*az_ulib_ipc_publish_with_null_descriptor_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(interface_descriptor, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_ipc_publish_no_contract(interface_descriptor, interface_handle);
}

#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
az_ulib_result _az_ulib_ipc_unpublish_no_contract(
    const az_ulib_interface_descriptor* interface_descriptor,
    uint32_t wait_option_ms) {
  az_ulib_result result;
  _az_ulib_ipc_interface* release_interface;

  az_pal_os_lock_acquire(&(ipc->lock));
  {
    if ((release_interface = get_interface(
             interface_descriptor->name, interface_descriptor->version, AZ_ULIB_VERSION_EQUALS_TO))
        == NULL) {
      /*az_ulib_ipc_unpublish_with_unknown_descriptor_failed*/
      result = AZ_ULIB_NO_SUCH_ELEMENT_ERROR;
    } else {
      // The order of the code here, including the ones that looks not necessary, are associated to
      // the interlock between this function and the az_ulib_ipc_call.

      // Prepare to recover in case it was not possible to unpublish the interface.
      const az_ulib_interface_descriptor* recover_interface_descriptor
          = (const az_ulib_interface_descriptor*)release_interface->interface_descriptor;

      // Block access to this interface. After this point, any new call to az_ulib_ipc_call that
      // didn't get the interface pointer yet will return AZ_ULIB_NO_SUCH_ELEMENT_ERROR.
      (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(&(release_interface->interface_descriptor), NULL);

      // If the running_count is `0` is because no other process is inside of any of the functions
      // methods, and they may be removed from the memory. There will be the case that the other
      // process is already in the az_ulib_ipc_call, in the direction to call a method in this
      // interface, but the call will just return AZ_ULIB_NO_SUCH_ELEMENT_ERROR from there.
      /*az_ulib_ipc_unpublish_succeed*/
      uint32_t retry_interval;
      if (wait_option_ms == AZ_ULIB_WAIT_FOREVER) {
        retry_interval = 100;
      } else {
        retry_interval = wait_option_ms >> 3;
        if (retry_interval == 0) {
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
      /*az_ulib_ipc_unpublish_with_method_running_with_small_timeout_failed*/
      while ((retry_total_time < wait_option_ms)
             && (release_interface->running_count_low_watermark != 0)) {

        az_pal_os_sleep(retry_interval);

        if (wait_option_ms != AZ_ULIB_WAIT_FOREVER) {
          retry_total_time += retry_interval;
        }
      }

      if (release_interface->running_count_low_watermark == 0) {
        /*az_ulib_ipc_unpublish_random_order_succeed*/
        /*az_ulib_ipc_unpublish_release_resource_succeed*/
        /*az_ulib_ipc_unpublish_with_valid_interface_instance_succeed*/
        result = AZ_ULIB_SUCCESS;
      } else {
        /*az_ulib_ipc_unpublish_with_method_running_failed*/
        // If caller doesn't want to wait anymore, recover the interface and return
        // AZ_ULIB_BUSY_ERROR.
        (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(
            &(release_interface->interface_descriptor), recover_interface_descriptor);
        result = AZ_ULIB_BUSY_ERROR;
      }
    }
  }
  az_pal_os_lock_release(&(ipc->lock));

  return result;
}

az_ulib_result _az_ulib_ipc_unpublish(
    const az_ulib_interface_descriptor* interface_descriptor,
    uint32_t wait_option_ms) {
  AZ_ULIB_UCONTRACT(
      /*az_ulib_ipc_unpublish_with_non_initialized_ipc_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(ipc, AZ_ULIB_NOT_INITIALIZED_ERROR),
      /*az_ulib_ipc_unpublish_with_null_descriptor_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(interface_descriptor, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_ipc_unpublish_no_contract(interface_descriptor, wait_option_ms);
}
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH

az_ulib_result _az_ulib_ipc_try_get_interface_no_contract(
    const char* const name,
    az_ulib_version version,
    az_ulib_version_match_criteria match_criteria,
    _az_ulib_ipc_interface_handle* interface_handle) {
  az_ulib_result result;
  _az_ulib_ipc_interface* ipc_interface;

  az_pal_os_lock_acquire(&(ipc->lock));
  {
    if ((ipc_interface = get_interface(name, version, match_criteria)) == NULL) {
      /*az_ulib_ipc_try_get_interface_with_unknown_name_failed*/
      /*az_ulib_ipc_try_get_interface_with_unknown_version_failed*/
      result = AZ_ULIB_NO_SUCH_ELEMENT_ERROR;
      /*az_ulib_ipc_try_get_interface_with_max_interface_instances_failed*/
    } else if ((result = get_instance(ipc_interface)) == AZ_ULIB_SUCCESS) {
      /*az_ulib_ipc_try_get_interface_succeed*/
      *interface_handle = ipc_interface;
    }
  }
  az_pal_os_lock_release(&(ipc->lock));

  return result;
}

az_ulib_result _az_ulib_ipc_try_get_interface(
    const char* const name,
    az_ulib_version version,
    az_ulib_version_match_criteria match_criteria,
    _az_ulib_ipc_interface_handle* interface_handle) {
  AZ_ULIB_UCONTRACT(
      /*az_ulib_ipc_try_get_interface_with_ipc_not_initialized_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(ipc, AZ_ULIB_NOT_INITIALIZED_ERROR),
      /*az_ulib_ipc_try_get_interface_with_null_name_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(name, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
      /*az_ulib_ipc_try_get_interface_with_null_handle_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(interface_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_ipc_try_get_interface_no_contract(
      name, version, match_criteria, interface_handle);
}

az_ulib_result _az_ulib_ipc_get_interface_no_contract(
    _az_ulib_ipc_interface_handle original_interface_handle,
    _az_ulib_ipc_interface_handle* interface_handle) {
  az_ulib_result result;

  az_pal_os_lock_acquire(&(ipc->lock));
  {
    _az_ulib_ipc_interface* ipc_interface = original_interface_handle;
    if (ipc_interface->interface_descriptor == NULL) {
      /*az_ulib_ipc_get_interface_with_unpublished_interface_failed*/
      result = AZ_ULIB_NO_SUCH_ELEMENT_ERROR;
      /*az_ulib_ipc_get_interface_with_max_interface_instances_failed*/
    } else if ((result = get_instance(ipc_interface)) == AZ_ULIB_SUCCESS) {
      /*az_ulib_ipc_get_interface_succeed*/
      *interface_handle = ipc_interface;
    }
  }
  az_pal_os_lock_release(&(ipc->lock));

  return result;
}

az_ulib_result _az_ulib_ipc_get_interface(
    _az_ulib_ipc_interface_handle original_interface_handle,
    _az_ulib_ipc_interface_handle* interface_handle) {
  AZ_ULIB_UCONTRACT(
      /*az_ulib_ipc_get_interface_with_ipc_not_initialized_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(ipc, AZ_ULIB_NOT_INITIALIZED_ERROR),
      /*az_ulib_ipc_get_interface_with_null_original_interface_handle_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(original_interface_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
      /*az_ulib_ipc_get_interface_with_null_interface_handle_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(interface_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_ipc_get_interface_no_contract(original_interface_handle, interface_handle);
}

az_ulib_result _az_ulib_ipc_release_interface_no_contract(
    _az_ulib_ipc_interface_handle interface_handle) {
  _az_ulib_ipc_interface* ipc_interface = (_az_ulib_ipc_interface*)interface_handle;
  az_ulib_result result;

  az_pal_os_lock_acquire(&(ipc->lock));
  {
    if (ipc_interface->ref_count == 0) {
      /*az_ulib_ipc_release_interface_double_release_failed*/
      result = AZ_ULIB_PRECONDITION_ERROR;
    } else {
      /*az_ulib_ipc_release_interface_succeed*/
      result = AZ_ULIB_SUCCESS;
      ipc_interface->ref_count--;
    }
  }
  az_pal_os_lock_release(&(ipc->lock));

  return result;
}

az_ulib_result _az_ulib_ipc_release_interface(_az_ulib_ipc_interface_handle interface_handle) {
  AZ_ULIB_UCONTRACT(
      /*az_ulib_ipc_release_interface_with_ipc_not_initialized_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(ipc, AZ_ULIB_NOT_INITIALIZED_ERROR),
      /*az_ulib_ipc_release_interface_with_null_interface_handle_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(interface_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_ipc_release_interface_no_contract(interface_handle);
}

#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
az_ulib_result _az_ulib_ipc_call_no_contract(
    _az_ulib_ipc_interface_handle interface_handle,
    az_ulib_capability_index method_index,
    const void* const model_in,
    const void* model_out) {
  az_ulib_result result;
  _az_ulib_ipc_interface* ipc_interface = (_az_ulib_ipc_interface*)interface_handle;

  // The double test on the interface_descriptor is part of the interlock between az_ulib_ipc_call
  // and az_ulib_ipc_unpublish. It will allow a interface to be unpublished even if it has a high
  // volume of calls.
  if (ipc_interface->interface_descriptor != NULL) {
    (void)AZ_ULIB_PORT_ATOMIC_INC_W(&(ipc_interface->running_count));
    register const az_ulib_interface_descriptor* descriptor
        = (const az_ulib_interface_descriptor*)ipc_interface->interface_descriptor;

    if (descriptor == NULL) {
      /*az_ulib_ipc_call_unpublished_interface_failed*/
      result = AZ_ULIB_NO_SUCH_ELEMENT_ERROR;
    } else {
      /*az_ulib_ipc_call_calls_the_method_succeed*/
      result
          = descriptor->capability_list[method_index].capability_ptr_1.method(model_in, model_out);
    }
    long new_running_count = AZ_ULIB_PORT_ATOMIC_DEC_W(&(ipc_interface->running_count));
    if (new_running_count < ipc_interface->running_count_low_watermark) {
      (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_W(
          &(ipc_interface->running_count_low_watermark), new_running_count);
    }
  } else {
    result = AZ_ULIB_NO_SUCH_ELEMENT_ERROR;
  }

  return result;
}
#else // AZ_ULIB_CONFIG_IPC_UNPUBLISH
az_ulib_result _az_ulib_ipc_call_no_contract(
    _az_ulib_ipc_interface_handle interface_handle,
    az_ulib_capability_index method_index,
    const void* const model_in,
    const void* model_out) {
  return ((_az_ulib_ipc_interface*)interface_handle)
      ->interface_descriptor->capability_list[method_index]
      .capability_ptr_1.method(model_in, model_out);
}
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH

az_ulib_result _az_ulib_ipc_call(
    _az_ulib_ipc_interface_handle interface_handle,
    az_ulib_capability_index method_index,
    const void* const model_in,
    const void* model_out) {
  AZ_ULIB_UCONTRACT(
      /*az_ulib_ipc_call_with_ipc_not_initialized_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(ipc, AZ_ULIB_NOT_INITIALIZED_ERROR),
      /*az_ulib_ipc_call_with_null_interface_handle_failed*/
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(interface_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_ipc_call_no_contract(interface_handle, method_index, model_in, model_out);
}
