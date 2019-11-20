// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef INTERNAL_AZ_ULIB_IPC_H
#define INTERNAL_AZ_ULIB_IPC_H

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#include "../az_ulib_action_api.h"
#include "../az_ulib_descriptor_api.h"
#include "az_ulib_pal_os_api.h"
#include "az_ulib_action_api.h"
#include "az_ulib_base.h"
#include "az_ulib_config.h"
#include "az_ulib_port.h"
#include "az_ulib_result.h"

#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
extern "C" {
#endif

typedef void* _az_ulib_ipc_interface_handle;

typedef struct _az_ulib_ipc_interface_tag {
  volatile const az_ulib_interface_descriptor* interface_descriptor;
  volatile long ref_count;
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
  volatile long running_count;
  volatile long running_count_low_watermark;
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
} _az_ulib_ipc_interface;

typedef struct _az_ulib_ipc_tag {
  az_ulib_pal_os_lock lock;
  _az_ulib_ipc_interface interface_list[AZ_ULIB_CONFIG_MAX_IPC_INTERFACE];
} _az_ulib_ipc;

MOCKABLE_FUNCTION(, az_ulib_result, _az_ulib_ipc_init_no_contract, _az_ulib_ipc*, ipc_handle);
MOCKABLE_FUNCTION(, az_ulib_result, _az_ulib_ipc_init, _az_ulib_ipc*, ipc_handle);

MOCKABLE_FUNCTION(, az_ulib_result, _az_ulib_ipc_deinit_no_contract);
MOCKABLE_FUNCTION(, az_ulib_result, _az_ulib_ipc_deinit);

MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_ipc_publish_no_contract,
    const az_ulib_interface_descriptor*,
    interface_descriptor,
    _az_ulib_ipc_interface_handle*,
    interface_handle);
MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_ipc_publish,
    const az_ulib_interface_descriptor*,
    interface_descriptor,
    _az_ulib_ipc_interface_handle*,
    interface_handle);

#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_ipc_unpublish_no_contract,
    const az_ulib_interface_descriptor*,
    interface_descriptor,
    uint32_t,
    wait_option_ms);
MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_ipc_unpublish,
    const az_ulib_interface_descriptor*,
    interface_descriptor,
    uint32_t,
    wait_option_ms);
#endif //  AZ_ULIB_CONFIG_IPC_UNPUBLISH

MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_ipc_try_get_interface_no_contract,
    const char* const,
    name,
    az_ulib_version,
    version,
    az_ulib_version_match_criteria,
    match_criteria,
    _az_ulib_ipc_interface_handle*,
    interface_handle);
MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_ipc_try_get_interface,
    const char* const,
    name,
    az_ulib_version,
    version,
    az_ulib_version_match_criteria,
    match_criteria,
    _az_ulib_ipc_interface_handle*,
    interface_handle);

MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_ipc_get_interface_no_contract,
    _az_ulib_ipc_interface_handle,
    original_interface_handle,
    _az_ulib_ipc_interface_handle*,
    interface_handle);
MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_ipc_get_interface,
    _az_ulib_ipc_interface_handle,
    original_interface_handle,
    _az_ulib_ipc_interface_handle*,
    interface_handle);

MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_ipc_release_interface_no_contract,
    _az_ulib_ipc_interface_handle,
    interface_handle);
MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_ipc_release_interface,
    _az_ulib_ipc_interface_handle,
    interface_handle);

MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_ipc_call_no_contract,
    _az_ulib_ipc_interface_handle,
    interface_handle,
    az_ulib_action_index,
    method_index,
    const void* const,
    modelIn,
    const void*,
    modelOut);
MOCKABLE_FUNCTION(
    ,
    az_ulib_result,
    _az_ulib_ipc_call,
    _az_ulib_ipc_interface_handle,
    interface_handle,
    az_ulib_action_index,
    method_index,
    const void* const,
    modelIn,
    const void*,
    modelOut);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* INTERNAL_AZ_ULIB_IPC_H */
