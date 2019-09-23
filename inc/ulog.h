// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file ulog.h
 * 
 * @brief Small footprint logging functionality
 */

#ifndef AZ_ULOG_H
#define AZ_ULOG_H

#include "azure_macro_utils/macro_utils.h"
#include "ulib_config.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern const char* const AZ_ULOG_REQUIRE_EQUALS_STRING;
extern const char* const AZ_ULOG_REQUIRE_NOT_EQUALS_STRING;
extern const char* const AZ_ULOG_REQUIRE_NOT_NULL_STRING;
extern const char* const AZ_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING;
extern const char* const AZ_ULOG_OUT_OF_MEMORY_STRING;
extern const char* const AZ_ULOG_REPORT_EXCEPTION_STRING;

/**
 * @brief   enum to select the log type
 */
MU_DEFINE_ENUM(
    AZ_ULOG_TYPE,
    AZ_ULOG_TYPE_ERROR   = 0,    /**<error log message */
    AZ_ULOG_TYPE_INFO    = 1     /**<info log message */
);

/**
 * @brief   AZ_ULOG_TYPE string values
 */
extern const char* const AZ_ULOG_TYPE_STRING[];

/**
 * @brief log function for ulib_config
 *
 * @param   type    AZ_ULOG_TYPE to signify error or info log
 * @param   format  Format prefix for the passed string in (...)
 * @param   ...     Passed string (with any % formatting parameters) to print to the log
 */
void az_ulog_print(AZ_ULOG_TYPE type, const char* const format, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZ_ULOG_H */
