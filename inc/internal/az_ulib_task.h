// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef INTERNAL_AZ_ULIB_TASK_H
#define INTERNAL_AZ_ULIB_TASK_H

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#include "../az_ulib_action_api.h"
#include "../az_ulib_descriptor_api.h"
#include "../az_ulib_ipc_api.h"
#include "az_ulib_base.h"
#include "az_ulib_config.h"
#include "az_ulib_pal_os_api.h"
#include "az_ulib_result.h"

#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
extern "C" {
#endif

typedef struct _az_ulib_task_tag {
  az_ulib_ipc_interface_handle interface_handle;
  az_ulib_action_index method_index;
  void* model_in;
  void* model_out;
  az_ulib_result result;
  az_ulib_action_result_callback result_callback;
  az_ulib_action_context action_context;
} _az_ulib_task;

MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_task_init_no_contract,
    _az_ulib_task*,
    task_handle,
    az_ulib_ipc_interface_handle,
    interface_handle,
    az_ulib_action_index,
    method_index,
    const void* const,
    model_in,
    const void*,
    model_out,
    az_ulib_action_result_callback,
    result_callback,
    az_ulib_pal_os_queue,
    queue);
MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_task_init,
    _az_ulib_task*,
    task_handle,
    az_ulib_ipc_interface_handle,
    interface_handle,
    az_ulib_action_index,
    method_index,
    const void* const,
    model_in,
    const void*,
    model_out,
    az_ulib_action_result_callback,
    result_callback,
    az_ulib_pal_os_queue,
    queue);

MOCKABLE_FUNCTION(, az_ulib_result, _az_ulib_task_run_no_contract, _az_ulib_task*, task_handle);
MOCKABLE_FUNCTION(, az_ulib_result, _az_ulib_task_run, _az_ulib_task*, task_handle);

MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_task_get_result_no_contract,
    _az_ulib_task*,
    task_handle);
MOCKABLE_FUNCTION(, az_ulib_result, _az_ulib_task_get_result, _az_ulib_task*, task_handle);

MOCKABLE_FUNCTION(, az_ulib_result, _az_ulib_task_cancel_no_contract, _az_ulib_task*, task_handle);
MOCKABLE_FUNCTION(, az_ulib_result, _az_ulib_task_cancel, _az_ulib_task*, task_handle);

MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_task_run_and_wait_no_contract,
    _az_ulib_task*,
    task_handle,
    uint32_t,
    wait_option_ms);
MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_task_run_and_wait,
    _az_ulib_task*,
    task_handle,
    uint32_t,
    wait_option_ms);

MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_task_enqueue_no_contract,
    _az_ulib_task*,
    task_handle,
    az_ulib_pal_os_queue,
    queue,
    az_ulib_task_result_callback,
    result_callback);
MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_task_enqueue,
    _az_ulib_task*,
    task_handle,
    az_ulib_pal_os_queue,
    queue,
    az_ulib_task_result_callback,
    result_callback);

MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_task_dispacher,
    az_ulib_pal_os_queue,
    queue,
    uint32_t,
    wait_option_ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* INTERNAL_AZ_ULIB_TASK_H */
