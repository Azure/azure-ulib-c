// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file azulib_message_api.h
 */

#ifndef AZURE_ULIB_C_INC_AZULIB_MESSAGE_API_H_
#define AZURE_ULIB_C_INC_AZULIB_MESSAGE_API_H_

#include "ulib_config.h"
#include "ulib_result.h"
#include "ustream.h"
#include "internal/message.h"

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif /* __cplusplus */

/**
 * @brief   Message methods.
 */
typedef _AZULIB_MESSAGE_METHODS AZULIB_MESSAGE_METHODS;

/**
 * @brief   Release message signature.
 */
typedef _AZULIB_MESSAGE_RELEASE_CALLBACK AZULIB_MESSAGE_RELEASE_CALLBACK;

/**
 * @brief   Property node.
 */
typedef struct AZULIB_MESSAGE_PROPERTY_TAG
{
    const char* const key;
    const char* const value;
    struct AZULIB_MESSAGE_PROPERTY_TAG* next;
} AZULIB_MESSAGE_PROPERTY;

/**
 * @brief   Message channel.
 */
typedef struct AZULIB_MESSAGE_CHANNEL_TAG
{
    char* name;
    uint8_t qos;
} AZULIB_MESSAGE_CHANNEL;

/**
 * @brief   Message control block.
 */
typedef struct AZULIB_MESSAGE_TAG
{
    uint8_t _private[sizeof(_AZULIB_MESSAGE)];
} AZULIB_MESSAGE;

ULIB_RESULT azulib_message_init(AZULIB_MESSAGE* message, const char* channel_name, uint8_t channel_qos, USTREAM* payload);
void azulib_message_deinit(AZULIB_MESSAGE* message);

ULIB_RESULT azulib_message_get_channel(AZULIB_MESSAGE* message, AZULIB_MESSAGE_CHANNEL* channel);
ULIB_RESULT azulib_message_get_payload(AZULIB_MESSAGE* message, USTREAM** payload);

ULIB_RESULT azulib_message_set_method(AZULIB_MESSAGE* message, AZULIB_MESSAGE_METHODS method);
ULIB_RESULT azulib_message_get_method(AZULIB_MESSAGE* message, AZULIB_MESSAGE_METHODS* method);

ULIB_RESULT azulib_message_set_property(AZULIB_MESSAGE* message, AZULIB_MESSAGE_PROPERTY* property);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_AZULIB_MESSAGE_API_H_ */
