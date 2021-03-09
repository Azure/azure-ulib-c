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

typedef enum az_ulib_version_match_criteria_tag
{
  AZ_ULIB_VERSION_ANY = 0b00000111, /**<Accept any version */
  AZ_ULIB_VERSION_GREATER_THAN = 0b00000100, /**<Accept version greater than the provided one */
  AZ_ULIB_VERSION_EQUALS_TO = 0b00000010, /**<Accept version equals to the provided one */
  AZ_ULIB_VERSION_LOWER_THAN = 0b00000001 /**<Accept version lower than the provided one */
} az_ulib_version_match_criteria;

static inline bool az_ulib_version_match(
    az_ulib_version current_version,
    az_ulib_version required_version,
    az_ulib_version_match_criteria match_criteria)
{
  return (
      (AZ_ULIB_FLAGS_IS_SET(match_criteria, AZ_ULIB_VERSION_EQUALS_TO)
       && (current_version == required_version))
      || (AZ_ULIB_FLAGS_IS_SET(match_criteria, AZ_ULIB_VERSION_GREATER_THAN)
          && (current_version > required_version))
      || (AZ_ULIB_FLAGS_IS_SET(match_criteria, AZ_ULIB_VERSION_LOWER_THAN)
          && (current_version < required_version)));
}

#include "azure/core/_az_cfg_suffix.h"

#endif // AZ_ULIB_BASE_H
