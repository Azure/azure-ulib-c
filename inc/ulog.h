// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

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

typedef enum ULOG_TYPE_TAG
{
    ULOG_TYPE_ERROR = 0,
    ULOG_TYPE_INFO = 1
} ULOG_TYPE;

extern const char* const ULOG_TYPE_STRING[];

void ulog_print(ULOG_TYPE type, const char* const format, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_ULOG_H_ */
