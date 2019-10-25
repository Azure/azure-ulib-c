// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// See LICENSE file in the project root for full license information.

/**
 * @file az_ulib_ucontract.h
 * 
 * @brief Public API parameter validation
 */

#ifndef AZ_ULIB_UCONTRACT_H
#define AZ_ULIB_UCONTRACT_H

#include "azure_macro_utils/macro_utils.h"
#include "az_ulib_config.h"
#include "az_ulib_port.h"
#include "az_ulib_ulog.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/// @cond INTERNAL
#define EVALUATE_REQUIRE(x) x;
/// @endcond

/**
 * @brief   Macro to define contract for public function parameters.
 *
 *  Parameters to this macro shall be a comma separated list of AZ_UCONTRACT_...
 *  macros as listed below.
 *
 *  Each public function shall have one AZ_UCONTRACT() macro with the listed
 *  requirements inside.
 */
#define AZ_UCONTRACT(...) \
  do { \
    MU_FOR_EACH_1(EVALUATE_REQUIRE, __VA_ARGS__) \
  } while ((void)0, 0)

/**
 * @brief   Macro to define assertion for internal functions
 *
 *  Parameters to this macro shall be a comma separated list of AZ_UCONTRACT_...
 *  macros as listed below.
 */
#ifdef NDEBUG
#define AZ_UASSERT(...)
#else
#define AZ_UASSERT(...) AZ_UCONTRACT(__VA_ARGS__)
#endif

/**
 * @brief   Contract macro to evaluation developer expression.
 *
 * @param   expression  boolean expression to be evaluated
 * @param   result      return value if expression is false
 * @param   msg         message to log if expression is false
 */
#define AZ_UCONTRACT_REQUIRE(expression, result, msg) \
  do { \
    if (!(expression)) { \
      AZ_ULIB_CONFIG_LOG(AZ_ULOG_TYPE_ERROR, msg); \
      return result; \
    } \
  } while ((void)0, 0)

/**
 * @brief   Contract macro to evaluate if two values are equal.
 *
 * @param   val         value to check
 * @param   expected    value expected
 * @param   result      returned result if values are not equal
 */
#define AZ_UCONTRACT_REQUIRE_EQUALS(val, expected, result) \
  do { \
    if (val != expected) { \
      AZ_ULIB_CONFIG_LOG( \
          AZ_ULOG_TYPE_ERROR, \
          AZ_ULOG_REQUIRE_EQUALS_STRING, \
          MU_TOSTRING(val), \
          MU_TOSTRING(expected)); \
      return result; \
    } \
  } while ((void)0, 0)

/**
 * @brief   Contract macro to evaluate if two values are not equal.
 *
 * @param   val         value to check
 * @param   expected    value not expected
 * @param   result      returned result if values are equal.
 */
#define AZ_UCONTRACT_REQUIRE_NOT_EQUALS(val, expected, result) \
  do { \
    if (val == expected) { \
      AZ_ULIB_CONFIG_LOG( \
          AZ_ULOG_TYPE_ERROR, \
          AZ_ULOG_REQUIRE_NOT_EQUALS_STRING, \
          MU_TOSTRING(val), \
          MU_TOSTRING(expected)); \
      return result; \
    } \
  } while ((void)0, 0)

/**
 * @brief   Contract macro to evaluate if value is not `NULL`.
 *
 * @param   val         value to check
 * @param   result      returned result if value is `NULL`
 */
#define AZ_UCONTRACT_REQUIRE_NOT_NULL(val, result) \
  do { \
    if (val == NULL) { \
      AZ_ULIB_CONFIG_LOG(AZ_ULOG_TYPE_ERROR, AZ_ULOG_REQUIRE_NOT_NULL_STRING, MU_TOSTRING(val)); \
      return result; \
    } \
  } while ((void)0, 0)

/**
 * @brief   Contract macro to evaluate if value is `NULL`.
 *
 * @param   val         value to check
 * @param   result      returned result if value is not `NULL`
 */
#define AZ_UCONTRACT_REQUIRE_NULL(val, result) \
  do { \
    if (val != NULL) { \
      AZ_ULIB_CONFIG_LOG(AZ_ULOG_TYPE_ERROR, AZ_ULOG_REQUIRE_NULL_STRING, MU_TOSTRING(val)); \
      return result; \
    } \
  } while ((void)0, 0)

/**
 * @brief   Contract macro to evaluate user expression.
 *
 * @warning Throws hard fault if developer expression is false
 *
 * @param   expression  expression to check
 * @param   msg         message to log if expression is false
 */
#define AZ_UCONTRACT_REQUIRE_HARD_FAULT(expression, msg) \
  do { \
    if (!(expression)) { \
      AZ_ULIB_CONFIG_LOG(AZ_ULOG_TYPE_ERROR, msg); \
      AZ_ULIB_PORT_THROW_HARD_FAULT; \
    } \
  } while ((void)0, 0)

/**
 * @brief   Contract macro to evaluate if two values are equal.
 *
 * @warning Throws hard fault if values are not equal
 *
 * @param   val         value to check
 * @param   expected    value expected
 */
#define AZ_UCONTRACT_REQUIRE_EQUALS_HARD_FAULT(val, expected) \
  do { \
    if (val != expected) { \
      AZ_ULIB_CONFIG_LOG( \
          AZ_ULOG_TYPE_ERROR, \
          AZ_ULOG_REQUIRE_EQUALS_STRING, \
          MU_TOSTRING(val), \
          MU_TOSTRING(expected)); \
      AZ_ULIB_PORT_THROW_HARD_FAULT; \
    } \
  } while ((void)0, 0)

/**
 * @brief   Contract macro to evaluate if two values are not equal.
 *
 * @warning Throws hard fault if values are equal
 *
 * @param   val         value to check
 * @param   expected    value not expected
 */
#define AZ_UCONTRACT_REQUIRE_NOT_EQUALS_HARD_FAULT(val, expected) \
  do { \
    if (val == expected) { \
      AZ_ULIB_CONFIG_LOG( \
          AZ_ULOG_TYPE_ERROR, \
          AZ_ULOG_REQUIRE_NOT_EQUALS_STRING, \
          MU_TOSTRING(val), \
          MU_TOSTRING(expected)); \
      AZ_ULIB_PORT_THROW_HARD_FAULT; \
    } \
  } while ((void)0, 0)

/**
 * @brief   Contract macro to evaluate if value is not `NULL`.
 *
 * @warning Throws hard fault if value is `NULL`
 *
 * @param   val         value to check
 */
#define AZ_UCONTRACT_REQUIRE_NOT_NULL_HARD_FAULT(val) \
  do { \
    if (val == NULL) { \
      AZ_ULIB_CONFIG_LOG(AZ_ULOG_TYPE_ERROR, AZ_ULOG_REQUIRE_NOT_NULL_STRING, MU_TOSTRING(val)); \
      AZ_ULIB_PORT_THROW_HARD_FAULT; \
    } \
  } while ((void)0, 0)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZ_ULIB_UCONTRACT_H */
