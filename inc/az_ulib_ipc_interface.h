// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_IPC_VTABLE_H
#define AZ_ULIB_IPC_VTABLE_H

#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
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

typedef struct
{
  az_result (*publish)(
      const az_ulib_interface_descriptor* const interface_descriptor,
      az_ulib_ipc_interface_handle* interface_handle);

  az_result (*unpublish)(
      const az_ulib_interface_descriptor* const interface_descriptor,
      uint32_t wait_option_ms);

  az_result (*try_get_interface)(
      az_span name,
      az_ulib_version version,
      az_ulib_version_match_criteria match_criteria,
      az_ulib_ipc_interface_handle* interface_handle);

  az_result (*try_get_capability)(
      az_ulib_ipc_interface_handle interface_handle,
      az_span name,
      az_ulib_capability_index* capability_index);

  az_result (*get_interface)(
      az_ulib_ipc_interface_handle original_interface_handle,
      az_ulib_ipc_interface_handle* interface_handle);

  az_result (*release_interface)(az_ulib_ipc_interface_handle interface_handle);

  az_result (*call)(
      az_ulib_ipc_interface_handle interface_handle,
      az_ulib_capability_index command_index,
      az_ulib_model_in model_in,
      az_ulib_model_out model_out);

  az_result (*call_w_str)(
      az_ulib_ipc_interface_handle interface_handle,
      az_ulib_capability_index command_index,
      az_span model_in_span,
      az_span* model_out_span);

  az_result (*query)(az_span query, az_span* result, uint32_t* continuation_token);

  az_result (*query_next)(uint32_t* continuation_token, az_span* result);

} az_ulib_ipc_vtable;

AZ_INLINE az_result azi_ulib_ipc_publish(
    const az_ulib_ipc_vtable* const vtable,
    const az_ulib_interface_descriptor* const interface_descriptor,
    az_ulib_ipc_interface_handle* interface_handle)
{
  return vtable->publish(interface_descriptor, interface_handle);
}

AZ_INLINE az_result azi_ulib_ipc_unpublish(
    const az_ulib_ipc_vtable* const vtable,
    const az_ulib_interface_descriptor* const interface_descriptor,
    uint32_t wait_option_ms)
{
  return vtable->unpublish(interface_descriptor, wait_option_ms);
}

AZ_INLINE az_result azi_ulib_ipc_try_get_interface(
    const az_ulib_ipc_vtable* const vtable,
    az_span name,
    az_ulib_version version,
    az_ulib_version_match_criteria match_criteria,
    az_ulib_ipc_interface_handle* interface_handle)
{
  return vtable->try_get_interface(name, version, match_criteria, interface_handle);
}

AZ_INLINE az_result azi_ulib_ipc_try_get_capability(
    const az_ulib_ipc_vtable* const vtable,
    az_ulib_ipc_interface_handle interface_handle,
    az_span name,
    az_ulib_capability_index* capability_index)
{
  return vtable->try_get_capability(interface_handle, name, capability_index);
}

AZ_INLINE az_result azi_ulib_ipc_get_interface(
    const az_ulib_ipc_vtable* const vtable,
    az_ulib_ipc_interface_handle original_interface_handle,
    az_ulib_ipc_interface_handle* interface_handle)
{
  return vtable->get_interface(original_interface_handle, interface_handle);
}

AZ_INLINE az_result azi_ulib_ipc_release_interface(
    const az_ulib_ipc_vtable* vtable,
    az_ulib_ipc_interface_handle interface_handle)
{
  return vtable->release_interface(interface_handle);
}

AZ_INLINE az_result azi_ulib_ipc_call(
    const az_ulib_ipc_vtable* const vtable,
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_capability_index command_index,
    az_ulib_model_in model_in,
    az_ulib_model_out model_out)
{
  return vtable->call(interface_handle, command_index, model_in, model_out);
}

AZ_INLINE az_result azi_ulib_ipc_call_w_str(
    const az_ulib_ipc_vtable* const vtable,
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_capability_index command_index,
    az_span model_in_span,
    az_span* model_out_span)
{
  return vtable->call_w_str(interface_handle, command_index, model_in_span, model_out_span);
}

AZ_INLINE az_result azi_ulib_ipc_query(
    const az_ulib_ipc_vtable* const vtable,
    az_span query,
    az_span* result,
    uint32_t* continuation_token)
{
  return vtable->query(query, result, continuation_token);
}

AZ_INLINE az_result azi_ulib_ipc_query_next(
    const az_ulib_ipc_vtable* const vtable,
    uint32_t* continuation_token,
    az_span* result)
{
  return vtable->query_next(continuation_token, result);
}

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_IPC_VTABLE_H */
