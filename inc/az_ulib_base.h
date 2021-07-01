// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/**
 * @file az_ulib_base.h
 *
 * @brief   Some base definitions for the ulib.
 */

#ifndef AZ_ULIB_BASE_H
#define AZ_ULIB_BASE_H

#ifndef __cplusplus
#include <stdbool.h>
#include <stdint.h>
#else
#include <cstdint>
#endif

#include "azure/core/_az_cfg_prefix.h"

/**
 * @brief   Does not wait if the resource is not available and returns the proper error code.
 */
#define AZ_ULIB_NO_WAIT 0x00000000

/**
 * @brief   Wait forever if the resource is not available.
 */
#define AZ_ULIB_WAIT_FOREVER 0xFFFFFFFF

#define AZ_ULIB_FLAGS_IS_SET(flag, bits) ((flag & bits) != 0)

/**
 * @brief   Version Handler
 */
typedef uint32_t az_ulib_version;

/**
 * @brief   Version 0 is reserved for ANY version.
 */
#define AZ_ULIB_VERSION_ANY 0

#include "azure/core/_az_cfg_suffix.h"

#endif // AZ_ULIB_BASE_H
