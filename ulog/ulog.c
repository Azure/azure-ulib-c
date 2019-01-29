// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ulog.h"

const char* const ULOG_REQUIRE_EQUALS_STRING = "%s requires equals %s\r\n";
const char* const ULOG_REQUIRE_NOT_EQUALS_STRING = "%s requires not equals %s\r\n";
const char* const ULOG_REQUIRE_NOT_NULL_STRING = "%s cannot be null\r\n";
const char* const ULOG_REQUIRE_TYPE_OF_USTREAMBUFFER_STRING = "uStreamBuffer is not the correct type\r\n";
const char* const ULOG_OUT_OF_MEMORY_STRING = "Not enough memory to create the %s\r\n";
const char* const ULOG_REPORT_EXCEPTION_STRING = "%s got exception [%d]\r\n";

const char* const ULOG_TYPE_STRING[] = 
{ 
    "[ERROR]", 
    "[INFO]" 
};
