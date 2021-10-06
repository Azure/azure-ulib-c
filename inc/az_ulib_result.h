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

#include "azure/core/_az_cfg_prefix.h"

// This definition is needed because some samples are not using the latest azure_sdk_for_c library
// that contains this definition in the az_result.
#ifndef _az_FACILITY_ULIB
#define _az_FACILITY_ULIB 0x7
#endif //_az_FACILITY_ULIB

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

  /** Handle renewed with success, equivalent to AZ_OK, but with a new handle. */
  AZ_ULIB_RENEW = _az_RESULT_MAKE_SUCCESS(_az_FACILITY_ULIB, 3),

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

  /** The required operation is forbidden. */
  AZ_ERROR_ULIB_FORBIDDEN = _az_RESULT_MAKE_ERROR(_az_FACILITY_ULIB, 10),

  /** Timeout. */
  AZ_ERROR_ULIB_TIMEOUT = _az_RESULT_MAKE_ERROR(_az_FACILITY_ULIB, 11),
};

/**
 * @brief   Try catch variable.
 *
 * Variable that contains the result for the try catch structure, you can use it in the catch to
 * split between results.
 *
 * <i><b>Example</b></i>
 *
 * @code
 * AZ_ULIB_CATCH(...)
 * {
 *   if (AZ_ULIB_TRY_RESULT == AZ_ERROR_ITEM_NOT_FOUND)
 *     (void)printf("display.1 was uninstalled.\r\n");
 *   else
 *     (void)printf(
 *         "my consumer uses display.1.cls failed with error %d.\r\n", AZ_ULIB_TRY_RESULT);
 * }
 * @endcode
 */
#define AZ_ULIB_TRY_RESULT _az_try_catch_result

/**
 * @brief   Try command in a try catch structure.
 *
 * Implementation of try catch structure for C99.
 *
 * <i><b>Example</b></i>
 *
 * @code
 * AZ_ULIB_TRY
 * {
 *   (void)printf("Before throw.\r\n");
 *   AZ_THROW(AZ_ERROR_ITEM_NOT_FOUND);
 *   (void)printf("After throw. It will not be printed.\r\n");
 * }
 * AZ_ULIB_CATCH(...)
 * {
 *   (void)printf("In the catch.\r\n");
 * }
 * @endcode
 *
 * @note    **Do not use `break` or `continue` inside of the Try session.**
 */
#define AZ_ULIB_TRY                     \
  az_result AZ_ULIB_TRY_RESULT = AZ_OK; \
  for (int _az_i = 0; _az_i < 1; _az_i++)

/**
 * @brief   Evaluate if the result represents an error.
 */
#define AZ_ULIB_IS_AZ_ERROR(x) ((x & _az_ERROR_FLAG) == _az_ERROR_FLAG)

/**
 * @brief   Catch command in a try catch structure.
 */
#define AZ_ULIB_CATCH(x) if (AZ_ULIB_IS_AZ_ERROR(AZ_ULIB_TRY_RESULT))

/**
 * @brief   Throw an error in a try catch structure.
 */
#define AZ_ULIB_THROW(error)    \
  if (true)                     \
  {                             \
    AZ_ULIB_TRY_RESULT = error; \
    break;                      \
  }

/**
 * @brief   Throw an error if the condition is false in a try catch structure.
 */
#define AZ_ULIB_THROW_IF_ERROR(condition, error) \
  if (!(condition))                              \
  {                                              \
    AZ_ULIB_TRY_RESULT = error;                  \
    break;                                       \
  }

/**
 * @brief   Throw an error if the command failed in a try catch structure.
 *
 * Call the `command` and evaluate the az_result returned from it. If command return any error,
 * mostly something different than #AZ_OK, this macro will throw the error.
 */
#define AZ_ULIB_THROW_IF_AZ_ERROR(command)                 \
  if (AZ_ULIB_IS_AZ_ERROR((AZ_ULIB_TRY_RESULT = command))) \
  {                                                        \
    break;                                                 \
  }

#include "azure/core/_az_cfg_suffix.h"

#endif // AZ_ULIB_RESULT_H
