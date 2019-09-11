// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file ulog.h
 * 
 * @brief Small footprint logging functionality
 */

#ifndef AZIOT_ULOG_H
#define AZIOT_ULOG_H

#include "ulib_config.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern const char* const AZIOT_ULOG_REQUIRE_EQUALS_STRING;
extern const char* const AZIOT_ULOG_REQUIRE_NOT_EQUALS_STRING;
extern const char* const AZIOT_ULOG_REQUIRE_NOT_NULL_STRING;
extern const char* const AZIOT_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING;
extern const char* const AZIOT_ULOG_OUT_OF_MEMORY_STRING;
extern const char* const AZIOT_ULOG_REPORT_EXCEPTION_STRING;

/**
 * @brief   enum to select the log type
 */
typedef enum AZIOT_ULOG_TYPE_TAG
{
    AZIOT_ULOG_TYPE_ERROR = 0,    /**<error log message */
    AZIOT_ULOG_TYPE_INFO = 1      /**<info log message */
} AZIOT_ULOG_TYPE;

/**
 * @brief   AZIOT_ULOG_TYPE string values
 */
extern const char* const AZIOT_ULOG_TYPE_STRING[];

/**
 * @brief log function for ulib_config
 *
 * @param   type    AZIOT_ULOG_TYPE to signify error or info log
 * @param   format  Format prefix for the passed string in (...)
 * @param   ...     Passed string (with any % formatting parameters) to print to the log
 */
void aziot_ulog_print(AZIOT_ULOG_TYPE type, const char* const format, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZIOT_ULOG_H */
