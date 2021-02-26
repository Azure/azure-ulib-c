// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/**
 * @file az_ulib_ulog.h
 *
 * @brief Small footprint logging functionality
 *
 * Using the az_ulib_ulog_print() api, you can pass enum types to differentiate logging purposes.
 * When the api prints to the console, it uses constant strings which prevents duplicate strings
 * from being saved in the bss.
 */

#ifndef AZ_ULIB_ULOG_H
#define AZ_ULIB_ULOG_H

#include "az_ulib_config.h"

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif /* __cplusplus */

#include "azure/core/_az_cfg_prefix.h"

extern const char* const AZ_ULIB_ULOG_REQUIRE_EQUALS_STRING;
extern const char* const AZ_ULIB_ULOG_REQUIRE_NOT_EQUALS_STRING;
extern const char* const AZ_ULIB_ULOG_REQUIRE_NOT_NULL_STRING;
extern const char* const AZ_ULIB_ULOG_REQUIRE_NULL_STRING;
extern const char* const AZ_ULIB_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING;
extern const char* const AZ_ULIB_ULOG_OUT_OF_MEMORY_STRING;
extern const char* const AZ_ULIB_ULOG_REPORT_EXCEPTION_STRING;

static const char* const AZ_ULIB_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING
    = "Passed ustream is not the correct type\r\n";

/**
 * @brief   enum to select the log type
 */
typedef enum az_ulib_ulog_type_tag
{
  AZ_ULIB_ULOG_TYPE_ERROR = 0, /**<error log message. */
  AZ_ULIB_ULOG_TYPE_INFO = 1 /**<info log message. */
} az_ulib_ulog_type;

/**
 * @brief   az_ulib_ulog_type string values
 */
extern const char* const AZ_ULIB_ULOG_TYPE_STRING[];

/**
 * @brief log function for ulib_config
 *
 * @param[in]   type    #az_ulib_ulog_type to signify error or info log.
 * @param[in]   format  Format prefix for the passed string in (...).
 * @param[in]   ...     Passed string (with any % formatting parameters) to print to the log.
 */
void az_ulib_ulog_print(az_ulib_ulog_type type, const char* const format, ...);

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_ULOG_H */
