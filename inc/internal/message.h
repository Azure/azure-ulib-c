// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file azulib_http_api.h
 */

#ifndef AZURE_ULIB_C_INC_INTERNAL_MESSAGE_H_
#define AZURE_ULIB_C_INC_INTERNAL_MESSAGE_H_

#include "ulib_config.h"
#include "ulib_result.h"
#include "../ustream.h"

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif /* __cplusplus */

MU_DEFINE_ENUM(_AZULIB_MESSAGE_METHODS,
    AZULIB_MESSAGE_METHOD_POST,
    AZULIB_MESSAGE_METHOD_GET,
    AZULIB_MESSAGE_METHOD_PUT,
    AZULIB_MESSAGE_METHOD_PATCH,
    AZULIB_MESSAGE_METHOD_DELETE);

typedef void(*_AZULIB_MESSAGE_RELEASE_CALLBACK)(void* message);

typedef struct _AZULIB_MESSAGE_TAG
{
    _AZULIB_MESSAGE_METHODS method;

    struct 
    {
        uint8_t qos;
        char name[ULIB_CONFIG_MESSAGE_MAX_CHANNEL_NAME_SIZE];
    } channel;

    USTREAM* payload;

    char properties[ULIB_CONFIG_MESSAGE_MAX_PROPERTIES][ULIB_CONFIG_MESSAGE_MAX_PROPERTY_SIZE];
} _AZULIB_MESSAGE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_INTERNAL_MESSAGE_H_ */
