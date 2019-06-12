// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file ulog.h
 */

#ifndef AZURE_ULIB_C_INC_ULOG_H_
#define AZURE_ULIB_C_INC_ULOG_H_

#include "ulib_config.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern const char* const ULOG_REQUIRE_EQUALS_STRING;
extern const char* const ULOG_REQUIRE_NOT_EQUALS_STRING;
extern const char* const ULOG_REQUIRE_NOT_NULL_STRING;
extern const char* const ULOG_REQUIRE_TYPE_OF_USTREAM_STRING;
extern const char* const ULOG_OUT_OF_MEMORY_STRING;
extern const char* const ULOG_REPORT_EXCEPTION_STRING;

/**
 * @brief   enum to select the log type
 */
typedef enum ULOG_TYPE_TAG
{
    ULOG_TYPE_ERROR = 0,    /**<error log message */
    ULOG_TYPE_INFO = 1      /**<info log message */
} ULOG_TYPE;

/**
 * @brief   ULOG_TYPE string values
 */
extern const char* const ULOG_TYPE_STRING[];

/**
 * @brief log function for ulib_config
 *
 * @param   type    ULOG_TYPE to signify error or info log
 * @param   format  Format prefix for the passed string in (...)
 * @param   ...     Passed string (with any % formatting parameters) to print to the log
 */
void ulog_print(ULOG_TYPE type, const char* const format, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_ULOG_H_ */
