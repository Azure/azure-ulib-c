// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stdint.h>

#include "az_ulib_action_api.h"
#include "az_ulib_base.h"
#include "az_ulib_config.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_pal_os_api.h"
#include "az_ulib_port.h"
#include "az_ulib_result.h"
#include "az_ulib_ucontract.h"
#include "az_ulib_ulog.h"
#include "internal/az_ulib_task.h"

az_ulib_result _az_ulib_task_init_no_contract(
    _az_ulib_task* task_handle,
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_action_index method_index,
    const void* const model_in,
    const void* model_out,
    az_ulib_action_result_callback result_callback,
    az_ulib_pal_os_queue queue) {

  // TODO: Implement the method here.

  return AZ_ULIB_NOT_SUPPORTED_ERROR;
}

az_ulib_result _az_ulib_task_init(
    _az_ulib_task* task_handle,
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_action_index method_index,
    const void* const model_in,
    const void* model_out,
    az_ulib_action_result_callback result_callback,
    az_ulib_pal_os_queue queue) {
  AZ_ULIB_UCONTRACT(
      AZ_ULIB_UCONTRACT_REQUIRE_NULL(task_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(interface_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_task_init_no_contract(
      task_handle, interface_handle, method_index, model_in, model_out, result_callback, queue);
}

az_ulib_result _az_ulib_task_run_no_contract(_az_ulib_task* task_handle) {

  // TODO: Implement the method here.

  return AZ_ULIB_NOT_SUPPORTED_ERROR;
}

az_ulib_result _az_ulib_task_run(_az_ulib_task* task_handle) {
  AZ_ULIB_UCONTRACT(AZ_ULIB_UCONTRACT_REQUIRE_NULL(task_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_task_run_no_contract(task_handle);
}

az_ulib_result _az_ulib_task_get_result_no_contract(_az_ulib_task* task_handle) {

  // TODO: Implement the method here.

  return AZ_ULIB_NOT_SUPPORTED_ERROR;
}

az_ulib_result _az_ulib_task_get_result(_az_ulib_task* task_handle) {
  AZ_ULIB_UCONTRACT(AZ_ULIB_UCONTRACT_REQUIRE_NULL(task_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_task_get_result_no_contract(task_handle);
}

az_ulib_result _az_ulib_task_cancel_no_contract(_az_ulib_task* task_handle) {

  // TODO: Implement the method here.

  return AZ_ULIB_NOT_SUPPORTED_ERROR;
}

az_ulib_result _az_ulib_task_cancel(_az_ulib_task* task_handle) {
  AZ_ULIB_UCONTRACT(AZ_ULIB_UCONTRACT_REQUIRE_NULL(task_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_task_cancel_no_contract(task_handle);
}

az_ulib_result _az_ulib_task_run_and_wait_no_contract(
    _az_ulib_task* task_handle,
    uint32_t wait_option_ms) {

  // TODO: Implement the method here.

  return AZ_ULIB_NOT_SUPPORTED_ERROR;
}

az_ulib_result _az_ulib_task_run_and_wait(_az_ulib_task* task_handle, uint32_t wait_option_ms) {
  AZ_ULIB_UCONTRACT(AZ_ULIB_UCONTRACT_REQUIRE_NULL(task_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_task_run_and_wait_no_contract(task_handle, wait_option_ms);
}

az_ulib_result _az_ulib_task_enqueue_no_contract(
    _az_ulib_task* task_handle,
    az_ulib_pal_os_queue queue,
    az_ulib_task_result_callback result_callback) {

  // TODO: Implement the method here.

  return AZ_ULIB_NOT_SUPPORTED_ERROR;
}

az_ulib_result _az_ulib_task_enqueue(
    _az_ulib_task* task_handle,
    az_ulib_pal_os_queue queue,
    az_ulib_task_result_callback result_callback) {
  AZ_ULIB_UCONTRACT(AZ_ULIB_UCONTRACT_REQUIRE_NULL(task_handle, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
  return _az_ulib_task_enqueue_no_contract(task_handle, queue, result_callback);
}

az_ulib_result _az_ulib_task_dispacher(az_ulib_pal_os_queue queue, uint32_t wait_option_ms) {

  // TODO: Implement the method here.

  return AZ_ULIB_NOT_SUPPORTED_ERROR;
}