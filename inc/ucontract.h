// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_INC_UCONTRACT_H_
#define AZURE_ULIB_C_INC_UCONTRACT_H_

#include "macro_utils.h"
#include "ulib_config.h"
#include "ulib_port.h"
#include "ulog.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Each public function shall have only one session of CONTRACT and must be
 * the first line of the function.
 */
#define EVALUATE_REQUIRE(x) x;
#define UCONTRACT(...) do { FOR_EACH_1(EVALUATE_REQUIRE, __VA_ARGS__) } while((void)0,0)

/*
 * Each internal function may have only one session of ASSERT and must be
 * the first line of the function.
 */
#ifdef NDEBUG
#define UASSERT(...)
#else
#define UASSERT(...) UCONTRACT(__VA_ARGS__)
#endif

#define UCONTRACT_REQUIRE(expression, result, msg) \
    do { \
        if(!(expression)) \
        { \
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, msg); \
            return result; \
        } \
    } while((void)0,0)

#define UCONTRACT_REQUIRE_EQUALS(val, expected, result) \
    do { \
        if(val != expected) \
        { \
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_EQUALS_STRING, TOSTRING(val), TOSTRING(expected)); \
            return result; \
        } \
    } while((void)0,0)

#define UCONTRACT_REQUIRE_NOT_EQUALS(val, expected, result) \
    do { \
        if(val == expected) \
        { \
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_EQUALS_STRING, TOSTRING(val), TOSTRING(expected)); \
            return result; \
        } \
    } while((void)0,0)

#define UCONTRACT_REQUIRE_NOT_NULL(val, result) \
    do { \
        if(val == NULL) \
        { \
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, TOSTRING(val)); \
            return result; \
        } \
    } while((void)0,0)

#define UCONTRACT_REQUIRE_HARD_FAULT(expression, msg) \
    do { \
        if(!(expression)) \
        { \
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, msg); \
            ULIB_PORT_THROW_HARD_FAULT; \
        } \
    } while((void)0,0)

#define UCONTRACT_REQUIRE_EQUALS_HARD_FAULT(val, expected) \
    do { \
        if(val != expected) \
        { \
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_EQUALS_STRING, TOSTRING(val), TOSTRING(expected)); \
            ULIB_PORT_THROW_HARD_FAULT; \
        } \
    } while((void)0,0)

#define UCONTRACT_REQUIRE_NOT_EQUALS_HARD_FAULT(val, expected) \
    do { \
        if(val == expected) \
        { \
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_EQUALS_STRING, TOSTRING(val), TOSTRING(expected)); \
            ULIB_PORT_THROW_HARD_FAULT; \
        } \
    } while((void)0,0)

#define UCONTRACT_REQUIRE_NOT_NULL_HARD_FAULT(val) \
    do { \
        if(val == NULL) \
        { \
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, TOSTRING(val)); \
            ULIB_PORT_THROW_HARD_FAULT; \
        } \
    } while((void)0,0)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_UCONTRACT_H_ */
