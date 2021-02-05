// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/**
 * @file az_ulib_result.h
 *
 * @brief ulib return values
 */

#ifndef AZ_ULIB_RESULT_H
#define AZ_ULIB_RESULT_H

#include "azure/core/az_result.h"

#ifndef __cplusplus
#else
extern "C"
{
#endif

/**
 * @brief   Error bit for error values.
 */
#define AZ_ULIB_ERROR_FLAG 0X80

/**
 * @brief   Enumeration of ulib results.
 */
enum az_result_ulib
{

  // === uLib: Success result ===
  /** End of file (no more data to read). */
  AZ_ULIB_EOF = _az_RESULT_MAKE_SUCCESS(_az_FACILITY_ULIB, 1),

  /** Intermediate state waiting for the end of the job. */
  AZ_ULIB_PENDING = _az_RESULT_MAKE_SUCCESS(_az_FACILITY_ULIB, 2),

  // === uLib: Error result ===
  /** Security error. */
  AZ_ERROR_ULIB_SECURITY = _az_RESULT_MAKE_ERROR(_az_FACILITY_ULIB, 1),

  /** System error (unrecoverable!). */
  AZ_ERROR_ULIB_SYSTEM = _az_RESULT_MAKE_ERROR(_az_FACILITY_ULIB, 2),

  /** Busy error. */
  AZ_ERROR_ULIB_BUSY = _az_RESULT_MAKE_ERROR(_az_FACILITY_ULIB, 3),

  /** Precondition for executing the command was not met. */
  AZ_ERROR_ULIB_PRECONDITION = _az_RESULT_MAKE_ERROR(_az_FACILITY_ULIB, 4),

  /** New element already exists. */
  AZ_ERROR_ULIB_ELEMENT_DUPLICATE = _az_RESULT_MAKE_ERROR(_az_FACILITY_ULIB, 5),

  /** Disabled error. */
  AZ_ERROR_ULIB_DISABLED = _az_RESULT_MAKE_ERROR(_az_FACILITY_ULIB, 6),

  /** Required version is not available error. */
  AZ_ERROR_ULIB_INCOMPATIBLE_VERSION = _az_RESULT_MAKE_ERROR(_az_FACILITY_ULIB, 7),

  /** Use a component that was not properly initialized. */
  AZ_ERROR_ULIB_NOT_INITIALIZED = _az_RESULT_MAKE_ERROR(_az_FACILITY_ULIB, 8),

  /** A singleton component is already initialized. */
  AZ_ERROR_ULIB_ALREADY_INITIALIZED = _az_RESULT_MAKE_ERROR(_az_FACILITY_ULIB, 9),
};

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // AZ_ULIB_RESULT_H
