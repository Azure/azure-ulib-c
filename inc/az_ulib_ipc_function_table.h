// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_IPC_FUNCTION_TABLE_H
#define AZ_ULIB_IPC_FUNCTION_TABLE_H

#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_interface_api.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"

#ifndef __cplusplus
#include <stddef.h>
#include <stdint.h>
#else
#include <cstddef>
#include <cstdint>
#endif /* __cplusplus */

#include "azure/core/_az_cfg_prefix.h"

/**
 * @brief Function table of IPC APIs.
 *
 * Set of pointers to the IPC APIs uses to expose the IPC APIs to Modules that cannot be statically
 * linked to the IPC APIs.
 */
typedef struct
{
  az_result (*publish)(const az_ulib_interface_descriptor* const interface_descriptor);

  az_result (*set_default)(
      az_span package_name,
      az_ulib_version package_version,
      az_span interface_name,
      az_ulib_version interface_version);

  az_result (*unpublish)(
      const az_ulib_interface_descriptor* const interface_descriptor,
      uint32_t wait_option_ms);

  az_result (*try_get_interface)(
      az_span device_name,
      az_span package_name,
      az_ulib_version package_version,
      az_span interface_name,
      az_ulib_version interface_version,
      az_ulib_ipc_interface_handle* interface_handle);

  az_result (*try_get_capability)(
      az_ulib_ipc_interface_handle interface_handle,
      az_span name,
      az_ulib_capability_index* capability_index);

  az_result (*release_interface)(az_ulib_ipc_interface_handle interface_handle);

  az_result (*call)(
      az_ulib_ipc_interface_handle interface_handle,
      az_ulib_capability_index capability_index,
      az_ulib_model_in model_in,
      az_ulib_model_out model_out);

  az_result (*call_with_str)(
      az_ulib_ipc_interface_handle interface_handle,
      az_ulib_capability_index capability_index,
      az_span model_in_span,
      az_span* model_out_span);

  az_result (*split_method_name)(
      az_span full_name,
      az_span* device_name,
      az_span* package_name,
      uint32_t* package_version,
      az_span* interface_name,
      uint32_t* interface_version,
      az_span* capability_name);

  az_result (*query)(az_span query, az_span* result, uint32_t* continuation_token);

  az_result (*query_next)(uint32_t* continuation_token, az_span* result);

} az_ulib_ipc_function_table;

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_IPC_FUNCTION_TABLE_H */
