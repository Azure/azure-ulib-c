// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_INTERFACE_MANAGER_1_MODEL_H
#define AZ_ULIB_INTERFACE_MANAGER_1_MODEL_H

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#include "azure/core/_az_cfg_prefix.h"

/*
 * interface definition
 */
#define INTERFACE_MANAGER_1_INTERFACE_NAME "interface_manager"
#define INTERFACE_MANAGER_1_INTERFACE_VERSION 1

/*
 * Define set_default command on interface_manager interface.
 */
#define INTERFACE_MANAGER_1_SET_DEFAULT_COMMAND (az_ulib_capability_index)0
#define INTERFACE_MANAGER_1_SET_DEFAULT_COMMAND_NAME "set_default"
#define INTERFACE_MANAGER_1_SET_DEFAULT_INTERFACE_NAME "interface"
#define INTERFACE_MANAGER_1_SET_DEFAULT_PACKAGE_NAME_NAME "package_name"
#define INTERFACE_MANAGER_1_SET_DEFAULT_PACKAGE_VERSION_NAME "package_version"
#define INTERFACE_MANAGER_1_SET_DEFAULT_INTERFACE_NAME_NAME "interface_name"
#define INTERFACE_MANAGER_1_SET_DEFAULT_INTERFACE_VERSION_NAME "interface_version"
typedef struct
{
  az_span package_name;
  uint32_t package_version;
  az_span interface_name;
  uint32_t interface_version;
} interface_manager_1_set_default_model_in;

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_INTERFACE_MANAGER_1_MODEL_H */
