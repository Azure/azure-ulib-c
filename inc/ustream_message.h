// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef USTREAM_MESSAGE_H
#define USTREAM_MESSAGE_H

#include "azure_macro_utils/macro_utils.h"

//Verb constants
static const char* const AZIOT_ULIB_VERB_POST = "POST";
static const char* const AZIOT_ULIB_VERB_GET = "GET";

//Option constants
static const char *const AZIOT_ULIB_OPTION_TIME = "x-ms-date";
static const char *const AZIOT_ULIB_OPTION_VERSION = "x-ms-version";
static const char *const AZIOT_ULIB_OPTION_CONTENT_LENGTH = "Content-Length";
static const char* const AZIOT_ULIB_OPTION_CONTENT_TYPE = "Content-Type";
static const char* const AZIOT_ULIB_OPTION_HOST = "Host";
static const char *const AZIOT_ULIB_OPTION_HTTP_PREFIX = "https://";
static const char* const AZIOT_ULIB_OPTION_HTTP_VERSION = "HTTP/1.1";

static const char* AZIOT_ULIB_MESSAGE_VERB_STRINGS[] = {"POST", "GET"};

MU_DEFINE_ENUM(
    AZIOT_ULIB_MESSAGE_VERB,
    AZIOT_ULIB_MESSAGE_VERB_POST = 0,
    AZIOT_ULIB_MESSAGE_VERB_GET
)

MU_DEFINE_ENUM(
    AZIOT_ULIB_MESSAGE_OPTION,
    AZIOT_ULIB_MESSAGE_OPTION_CONTENT_TYPE,
    AZIOT_ULIB_MESSAGE_OPTION_VERSION,
    AZIOT_ULIB_MESSAGE_OPTION_TIME
)

#endif // USTREAM_MESSAGE_H
