// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file ucontract.h
 */

#ifndef AZIOT_UCONTRACT_H_
#define AZIOT_UCONTRACT_H_

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
 *  Parameters to this macro shall be a comma separated list of AZIOT_UCONTRACT_...
 *  macros as listed below.
 *
 *  Each public function shall have one AZIOT_UCONTRACT() macro with the listed
 *  requirements inside.
 */
#define AZIOT_UCONTRACT(...) do { MU_FOR_EACH_1(EVALUATE_REQUIRE, __VA_ARGS__) } while((void)0,0)

/**
 * @brief   Macro to define assertion for internal functions
 *
 *  Parameters to this macro shall be a comma separated list of AZIOT_UCONTRACT_...
 *  macros as listed below.
 */
#ifdef NDEBUG
#define AZIOT_UASSERT(...)
#else
#define AZIOT_UASSERT(...) AZIOT_UCONTRACT(__VA_ARGS__)
#endif

/**
 * @brief   Contract macro to evaluation user expression.
 *
 * @param   expression  boolean expression to be evaluated
 * @param   result      return value if expression is false
 * @param   msg         message to log if expression is false
 */
#define AZIOT_UCONTRACT_REQUIRE(expression, result, msg) \
    do { \
        if(!(expression)) \
        { \
            AZIOT_ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, msg); \
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
#define AZIOT_UCONTRACT_REQUIRE_EQUALS(val, expected, result) \
    do { \
        if(val != expected) \
        { \
            AZIOT_ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_EQUALS_STRING, MU_TOSTRING(val), MU_TOSTRING(expected)); \
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
#define AZIOT_UCONTRACT_REQUIRE_NOT_EQUALS(val, expected, result) \
    do { \
        if(val == expected) \
        { \
            AZIOT_ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_NOT_EQUALS_STRING, MU_TOSTRING(val), MU_TOSTRING(expected)); \
            return result; \
        } \
    } while((void)0,0)

/**
 * @brief   Contract macro to evaluate if value is not <tt>NULL</tt>.
 *
 * @param   val         value to check
 * @param   result      returned result if value is <tt>NULL</tt>
 */
#define AZIOT_UCONTRACT_REQUIRE_NOT_NULL(val, result) \
    do { \
        if(val == NULL) \
        { \
            AZIOT_ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_NOT_NULL_STRING, MU_TOSTRING(val)); \
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
#define AZIOT_UCONTRACT_REQUIRE_HARD_FAULT(expression, msg) \
    do { \
        if(!(expression)) \
        { \
            AZIOT_ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, msg); \
            AZIOT_ULIB_PORT_THROW_HARD_FAULT; \
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
#define AZIOT_UCONTRACT_REQUIRE_EQUALS_HARD_FAULT(val, expected) \
    do { \
        if(val != expected) \
        { \
            AZIOT_ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_EQUALS_STRING, MU_TOSTRING(val), MU_TOSTRING(expected)); \
            AZIOT_ULIB_PORT_THROW_HARD_FAULT; \
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
#define AZIOT_UCONTRACT_REQUIRE_NOT_EQUALS_HARD_FAULT(val, expected) \
    do { \
        if(val == expected) \
        { \
            AZIOT_ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_NOT_EQUALS_STRING, MU_TOSTRING(val), MU_TOSTRING(expected)); \
            AZIOT_ULIB_PORT_THROW_HARD_FAULT; \
        } \
    } while((void)0,0)

/**
 * @brief   Contract macro to evaluate if value is not <tt>NULL</tt>.
 *
 * @warning Throws hard fault if value is <tt>NULL</tt>
 *
 * @param   val         value to check
 */
#define AZIOT_UCONTRACT_REQUIRE_NOT_NULL_HARD_FAULT(val) \
    do { \
        if(val == NULL) \
        { \
            AZIOT_ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_NOT_NULL_STRING, MU_TOSTRING(val)); \
            AZIOT_ULIB_PORT_THROW_HARD_FAULT; \
        } \
    } while((void)0,0)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZIOT_UCONTRACT_H_ */
