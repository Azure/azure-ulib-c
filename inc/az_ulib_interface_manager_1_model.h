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
 * Interface definition.
 */

/** @brief  Readable name of this `interface_manager` interface. */
#define INTERFACE_MANAGER_1_INTERFACE_NAME "interface_manager"

/** @brief  Version of this `interface_manager` interface. */
#define INTERFACE_MANAGER_1_INTERFACE_VERSION 1

/*
 * Definition of `set_default` command in the `interface_manager` interface.
 */

/** @brief  Position of `set_default` capability in the interface table. */
#define INTERFACE_MANAGER_1_SET_DEFAULT_COMMAND (az_ulib_capability_index)0

/** @brief  External name for `set_default` capability. */
#define INTERFACE_MANAGER_1_SET_DEFAULT_COMMAND_NAME "set_default"

/** @brief  External name for interface full name argument in `set_default`.
 *
 * The interface full name shall be composed by.
 *  <package_name>.<package_version>.<interface_name>.<interface_version>
 *
 * If the interface full name is provided no other argument shall be provided. Provide further
 * information may cause misbehavior.
 */
#define INTERFACE_MANAGER_1_SET_DEFAULT_INTERFACE_NAME "interface"

/** @brief  External name for package name argument in `set_default`. */
#define INTERFACE_MANAGER_1_SET_DEFAULT_PACKAGE_NAME_NAME "package_name"

/** @brief  External name for package version argument in `set_default`. */
#define INTERFACE_MANAGER_1_SET_DEFAULT_PACKAGE_VERSION_NAME "package_version"

/** @brief  External name for interface name argument in `set_default`. */
#define INTERFACE_MANAGER_1_SET_DEFAULT_INTERFACE_NAME_NAME "interface_name"

/** @brief  External name for interface version argument in `set_default`. */
#define INTERFACE_MANAGER_1_SET_DEFAULT_INTERFACE_VERSION_NAME "interface_version"

/**
 * @brief  Input arguments for `set_default` capability.
 *
 * Binary format of the `set_default` input data.
 */
typedef struct
{
  /** @brief  The `az_span` with the package name. */
  az_span package_name;

  /** @brief  The `uint32_t` with the package version. */
  uint32_t package_version;

  /** @brief  The `az_span` with the interface name. */
  az_span interface_name;

  /** @brief  The `uint32_t` with the interface version. */
  uint32_t interface_version;

} interface_manager_1_set_default_model_in;

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_INTERFACE_MANAGER_1_MODEL_H */
