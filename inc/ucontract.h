// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file ucontract.h
 */

#ifndef AZULIB_UCONTRACT_H
#define AZULIB_UCONTRACT_H

#include "azure_macro_utils/macro_utils.h"
#include "ulib_config.h"
#include "ulib_port.h"
#include "ulog.h"

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
 *  Parameters to this macro shall be a comma separated list of AZULIB_UCONTRACT_...
 *  macros as listed below.
 *
 *  Each public function shall have one AZULIB_UCONTRACT() macro with the listed
 *  requirements inside.
 */
#define AZULIB_UCONTRACT(...) do { MU_FOR_EACH_1(EVALUATE_REQUIRE, __VA_ARGS__) } while((void)0,0)

/**
 * @brief   Macro to define assertion for internal functions
 *
 *  Parameters to this macro shall be a comma separated list of AZULIB_UCONTRACT_...
 *  macros as listed below.
 */
#ifdef NDEBUG
#define AZULIB_UASSERT(...)
#else
#define AZULIB_UASSERT(...) AZULIB_UCONTRACT(__VA_ARGS__)
#endif

/**
 * @brief   Contract macro to evaluation user expression.
 *
 * @param   expression  boolean expression to be evaluated
 * @param   result      return value if expression is false
 * @param   msg         message to log if expression is false
 */
#define AZULIB_UCONTRACT_REQUIRE(expression, result, msg) \
    do { \
        if(!(expression)) \
        { \
            AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, msg); \
            return result; \
        } \
    } while((void)0,0)

/**
 * @brief   Contract macro to evaluate if two values are equal.
 *
 * @param   val         value to check
 * @param   expected    value expected
 * @param   result      returned result if values are not equal
 */
#define AZULIB_UCONTRACT_REQUIRE_EQUALS(val, expected, result) \
    do { \
        if(val != expected) \
        { \
            AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_EQUALS_STRING, MU_TOSTRING(val), MU_TOSTRING(expected)); \
            return result; \
        } \
    } while((void)0,0)

/**
 * @brief   Contract macro to evaluate if two values are not equal.
 *
 * @param   val         value to check
 * @param   expected    value not expected
 * @param   result      returned result if values are equal.
 */
#define AZULIB_UCONTRACT_REQUIRE_NOT_EQUALS(val, expected, result) \
    do { \
        if(val == expected) \
        { \
            AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_NOT_EQUALS_STRING, MU_TOSTRING(val), MU_TOSTRING(expected)); \
            return result; \
        } \
    } while((void)0,0)

/**
 * @brief   Contract macro to evaluate if value is not <tt>NULL</tt>.
 *
 * @param   val         value to check
 * @param   result      returned result if value is <tt>NULL</tt>
 */
#define AZULIB_UCONTRACT_REQUIRE_NOT_NULL(val, result) \
    do { \
        if(val == NULL) \
        { \
            AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_NOT_NULL_STRING, MU_TOSTRING(val)); \
            return result; \
        } \
    } while((void)0,0)

/**
 * @brief   Contract macro to evaluate user expression.
 *
 * @warning Throws hard fault if user expression is false
 *
 * @param   expression  expression to check
 * @param   msg         message to log if expression is false
 */
#define AZULIB_UCONTRACT_REQUIRE_HARD_FAULT(expression, msg) \
    do { \
        if(!(expression)) \
        { \
            AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, msg); \
            AZULIB_ULIB_PORT_THROW_HARD_FAULT; \
        } \
    } while((void)0,0)

/**
 * @brief   Contract macro to evaluate if two values are equal.
 *
 * @warning Throws hard fault if values are not equal
 *
 * @param   val         value to check
 * @param   expected    value expected
 */
#define AZULIB_UCONTRACT_REQUIRE_EQUALS_HARD_FAULT(val, expected) \
    do { \
        if(val != expected) \
        { \
            AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_EQUALS_STRING, MU_TOSTRING(val), MU_TOSTRING(expected)); \
            AZULIB_ULIB_PORT_THROW_HARD_FAULT; \
        } \
    } while((void)0,0)

/**
 * @brief   Contract macro to evaluate if two values are not equal.
 *
 * @warning Throws hard fault if values are equal
 *
 * @param   val         value to check
 * @param   expected    value not expected
 */
#define AZULIB_UCONTRACT_REQUIRE_NOT_EQUALS_HARD_FAULT(val, expected) \
    do { \
        if(val == expected) \
        { \
            AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_NOT_EQUALS_STRING, MU_TOSTRING(val), MU_TOSTRING(expected)); \
            AZULIB_ULIB_PORT_THROW_HARD_FAULT; \
        } \
    } while((void)0,0)

/**
 * @brief   Contract macro to evaluate if value is not <tt>NULL</tt>.
 *
 * @warning Throws hard fault if value is <tt>NULL</tt>
 *
 * @param   val         value to check
 */
#define AZULIB_UCONTRACT_REQUIRE_NOT_NULL_HARD_FAULT(val) \
    do { \
        if(val == NULL) \
        { \
            AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_NOT_NULL_STRING, MU_TOSTRING(val)); \
            AZULIB_ULIB_PORT_THROW_HARD_FAULT; \
        } \
    } while((void)0,0)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZULIB_UCONTRACT_H */
