// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stdint.h>

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#include "az_ulib_pal_os_api.h"
#include "az_ulib_action_api.h"
#include "az_ulib_base.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "internal/az_ulib_ipc.h"
#include "az_ulib_ucontract.h"
#include "az_ulib_config.h"
#include "az_ulib_port.h"
#include "az_ulib_result.h"
#include "az_ulib_ulog.h"

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

static AZ_ULIB_RESULT get_instance(_az_ulib_ipc_interface* ipc_interface) {
  AZ_ULIB_RESULT result;
  if (ipc_interface->ref_count >= AZ_ULIB_CONFIG_MAX_IPC_INSTANCES) {
    result = AZ_ULIB_BUSY_ERROR;
  } else {
    result = AZ_ULIB_SUCCESS;
    AZ_ULIB_PORT_ATOMIC_INC_W(&(ipc_interface->ref_count));
  }
  return result;
}

AZ_ULIB_RESULT _az_ulib_ipc_init_no_contract(_az_ulib_ipc* handle) {
  /*az_ulib_ipc_init_succeed*/
  ipc = handle;

  az_pal_os_lock_init(&(ipc->lock));

  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++) {
    ipc->interface_list[i].ref_count = 0;
    ipc->interface_list[i].running_count = 0;
    ipc->interface_list[i].interface_descriptor = NULL;
  }

  return AZ_ULIB_SUCCESS;
}

AZ_ULIB_RESULT _az_ulib_ipc_init(_az_ulib_ipc* handle) {
  AZ_UCONTRACT(
      /*az_ulib_ipc_init_double_initialization_failed*/
      AZ_UCONTRACT_REQUIRE_NULL(ipc, AZ_ULIB_ALREADY_INITIALIZED_ERROR),
      /*az_ulib_ipc_init_with_null_handle_failed*/
      AZ_UCONTRACT_REQUIRE_NOT_NULL(handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_ipc_init_no_contract(handle);
}

AZ_ULIB_RESULT _az_ulib_ipc_deinit_no_contract(void) {
  AZ_ULIB_RESULT result;

  result = AZ_ULIB_SUCCESS;
  for (size_t i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++) {
    if ((ipc->interface_list[i].interface_descriptor != NULL)
        || (ipc->interface_list[i].ref_count != 0) || (ipc->interface_list[i].running_count != 0)) {
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

AZ_ULIB_RESULT _az_ulib_ipc_deinit(void) {
  AZ_UCONTRACT(
      /*az_ulib_ipc_deinit_with_ipc_not_initialized_failed*/
      AZ_UCONTRACT_REQUIRE_NOT_NULL(ipc, AZ_ULIB_NOT_INITIALIZED_ERROR));
  return _az_ulib_ipc_deinit_no_contract();
}

AZ_ULIB_RESULT _az_ulib_ipc_publish_no_contract(
    const az_ulib_interface_descriptor* interface_descriptor) {
  AZ_ULIB_RESULT result;
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
      new_interface->interface_descriptor = interface_descriptor;
      new_interface->ref_count = 0;
      new_interface->running_count = 0;
      result = AZ_ULIB_SUCCESS;
    }
  }
  az_pal_os_lock_release(&(ipc->lock));

  return result;
}

AZ_ULIB_RESULT
_az_ulib_ipc_publish(const az_ulib_interface_descriptor* interface_descriptor) {
  AZ_UCONTRACT(
      /*az_ulib_ipc_publish_with_non_initialized_ipc_failed*/
      AZ_UCONTRACT_REQUIRE_NOT_NULL(ipc, AZ_ULIB_NOT_INITIALIZED_ERROR),
      /*az_ulib_ipc_publish_with_null_descriptor_failed*/
      AZ_UCONTRACT_REQUIRE_NOT_NULL(interface_descriptor, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_ipc_publish_no_contract(interface_descriptor);
}

AZ_ULIB_RESULT
_az_ulib_ipc_unpublish_no_contract(
    const az_ulib_interface_descriptor* interface_descriptor,
    uint32_t wait_option_ms) {
  AZ_ULIB_RESULT result;
  _az_ulib_ipc_interface* release_interface;

  az_pal_os_lock_acquire(&(ipc->lock));
  {
    if ((release_interface = get_interface(
             interface_descriptor->name, interface_descriptor->version, AZ_ULIB_VERSION_EQUALS_TO))
        == NULL) {
      /*az_ulib_ipc_unpublish_with_unknown_descriptor_failed*/
      result = AZ_ULIB_NO_SUCH_ELEMENT_ERROR;
    } else if (release_interface->running_count != 0) {
      /*az_ulib_ipc_unpublish_with_method_running_failed*/
      if (wait_option_ms == AZ_ULIB_NO_WAIT) {
        result = AZ_ULIB_BUSY_ERROR;
      } else {
        // TODO: implement a semaphore to handle it.
        // For now, only ignore the wait option and return buzy.
        result = AZ_ULIB_BUSY_ERROR;
      }
    } else {
      /*az_ulib_ipc_unpublish_succeed*/
      /*az_ulib_ipc_unpublish_randon_order_succeed*/
      /*az_ulib_ipc_unpublish_release_resource_succeed*/
      /*az_ulib_ipc_unpublish_with_valid_interface_instance_succeed*/
      release_interface->interface_descriptor = NULL;
      result = AZ_ULIB_SUCCESS;
    }
  }
  az_pal_os_lock_release(&(ipc->lock));

  return result;
}

AZ_ULIB_RESULT
_az_ulib_ipc_unpublish(
    const az_ulib_interface_descriptor* interface_descriptor,
    uint32_t wait_option_ms) {
  AZ_UCONTRACT(
      /*az_ulib_ipc_unpublish_with_non_initialized_ipc_failed*/
      AZ_UCONTRACT_REQUIRE_NOT_NULL(ipc, AZ_ULIB_NOT_INITIALIZED_ERROR),
      /*az_ulib_ipc_unpublish_with_null_descriptor_failed*/
      AZ_UCONTRACT_REQUIRE_NOT_NULL(interface_descriptor, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_ipc_unpublish_no_contract(interface_descriptor, wait_option_ms);
}

AZ_ULIB_RESULT
_az_ulib_ipc_try_get_interface_no_contract(
    const char* const name,
    az_ulib_version version,
    az_ulib_version_match_criteria match_criteria,
    _az_ulib_ipc_interface_handle* interface_handle) {
  AZ_ULIB_RESULT result;
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

AZ_ULIB_RESULT
_az_ulib_ipc_try_get_interface(
    const char* const name,
    az_ulib_version version,
    az_ulib_version_match_criteria match_criteria,
    _az_ulib_ipc_interface_handle* interface_handle) {
  AZ_UCONTRACT(
      /*az_ulib_ipc_try_get_interface_with_ipc_not_initialized_failed*/
      AZ_UCONTRACT_REQUIRE_NOT_NULL(ipc, AZ_ULIB_NOT_INITIALIZED_ERROR),
      /*az_ulib_ipc_try_get_interface_with_null_name_failed*/
      AZ_UCONTRACT_REQUIRE_NOT_NULL(name, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
      /*az_ulib_ipc_try_get_interface_with_null_handle_failed*/
      AZ_UCONTRACT_REQUIRE_NOT_NULL(interface_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_ipc_try_get_interface_no_contract(
      name, version, match_criteria, interface_handle);
}

AZ_ULIB_RESULT
_az_ulib_ipc_get_interface_no_contract(
    _az_ulib_ipc_interface_handle original_interface_handle,
    _az_ulib_ipc_interface_handle* interface_handle) {
  AZ_ULIB_RESULT result;

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

AZ_ULIB_RESULT
_az_ulib_ipc_get_interface(
    _az_ulib_ipc_interface_handle original_interface_handle,
    _az_ulib_ipc_interface_handle* interface_handle) {
  AZ_UCONTRACT(
      /*az_ulib_ipc_get_interface_with_ipc_not_initialized_failed*/
      AZ_UCONTRACT_REQUIRE_NOT_NULL(ipc, AZ_ULIB_NOT_INITIALIZED_ERROR),
      /*az_ulib_ipc_get_interface_with_null_original_interface_handle_failed*/
      AZ_UCONTRACT_REQUIRE_NOT_NULL(original_interface_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
      /*az_ulib_ipc_get_interface_with_null_interface_handle_failed*/
      AZ_UCONTRACT_REQUIRE_NOT_NULL(interface_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_ipc_get_interface_no_contract(original_interface_handle, interface_handle);
}

AZ_ULIB_RESULT
_az_ulib_ipc_release_interface_no_contract(_az_ulib_ipc_interface_handle interface_handle) {
  _az_ulib_ipc_interface* ipc_interface = (_az_ulib_ipc_interface*)interface_handle;
  AZ_ULIB_RESULT result;

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

AZ_ULIB_RESULT
_az_ulib_ipc_release_interface(_az_ulib_ipc_interface_handle interface_handle) {
  AZ_UCONTRACT(
      /*az_ulib_ipc_release_interface_with_ipc_not_initialized_failed*/
      AZ_UCONTRACT_REQUIRE_NOT_NULL(ipc, AZ_ULIB_NOT_INITIALIZED_ERROR),
      /*az_ulib_ipc_release_interface_with_null_interface_handle_failed*/
      AZ_UCONTRACT_REQUIRE_NOT_NULL(interface_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_ipc_release_interface_no_contract(interface_handle);
}

AZ_ULIB_RESULT
_az_ulib_ipc_call_no_contract(
    _az_ulib_ipc_interface_handle interface_handle,
    az_ulib_action_index method_index,
    const void* const model_in,
    const void* model_out) {
  AZ_ULIB_RESULT result;
  _az_ulib_ipc_interface* ipc_interface = (_az_ulib_ipc_interface*)interface_handle;

  AZ_ULIB_PORT_ATOMIC_INC_W(&(ipc_interface->running_count));
  register const az_ulib_interface_descriptor* descriptor = ipc_interface->interface_descriptor;
  if (descriptor == NULL) {
    /*az_ulib_ipc_call_unpublished_interface_failed*/
    result = AZ_ULIB_NO_SUCH_ELEMENT_ERROR;
  } else {
    /*az_ulib_ipc_call_calls_the_method_succeed*/
    result = descriptor->action_list[method_index].action_ptr_1.method(model_in, model_out);
  }
  AZ_ULIB_PORT_ATOMIC_DEC_W(&(ipc_interface->running_count));

  return result;
}

AZ_ULIB_RESULT _az_ulib_ipc_call(
    _az_ulib_ipc_interface_handle interface_handle,
    az_ulib_action_index method_index,
    const void* const model_in,
    const void* model_out) {
  AZ_UCONTRACT(
      /*az_ulib_ipc_call_with_ipc_not_initialized_failed*/
      AZ_UCONTRACT_REQUIRE_NOT_NULL(ipc, AZ_ULIB_NOT_INITIALIZED_ERROR),
      /*az_ulib_ipc_call_with_null_interface_handle_failed*/
      AZ_UCONTRACT_REQUIRE_NOT_NULL(interface_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_ipc_call_no_contract(interface_handle, method_index, model_in, model_out);
}
