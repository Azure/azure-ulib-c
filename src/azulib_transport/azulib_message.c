// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azulib_message_api.h"
#include "ucontract.h"
#include "ulib_result.h"
#include "internal/message.h"

ULIB_RESULT azulib_message_init(AZULIB_MESSAGE* message, const char* channel_name, uint8_t channel_qos, USTREAM* payload)
{
#ifdef ULIB_CONFIG_MESSAGE_VALIDATE_ARGUMENTS
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(message, ULIB_ILLEGAL_ARGUMENT_ERROR));
#endif // ULIB_CONFIG_MESSAGE_VALIDATE_ARGUMENTS

    ULIB_RESULT result;

    _AZULIB_MESSAGE* cb = (_AZULIB_MESSAGE*)message;

    if (strlen(channel_name) >= ULIB_CONFIG_MESSAGE_MAX_CHANNEL_NAME_SIZE)
    {
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_DOES_NOT_FIT_STRING, "channel_name", "message->channel.name");
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        memset(cb, 0, sizeof(_AZULIB_MESSAGE));

        strncpy(cb->channel.name, channel_name, ULIB_CONFIG_MESSAGE_MAX_CHANNEL_NAME_SIZE);
        cb->channel.qos = channel_qos;
        if(payload != NULL)
        {
            cb->payload = ustream_clone(payload, 0);
        }
        else
        {
            cb->payload = NULL;
        }

        result = ULIB_SUCCESS;
    }

    return result;
}

void azulib_message_deinit(AZULIB_MESSAGE* message)
{
#ifdef ULIB_CONFIG_MESSAGE_VALIDATE_ARGUMENTS
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(message, ));
#endif // ULIB_CONFIG_MESSAGE_VALIDATE_ARGUMENTS

    _AZULIB_MESSAGE* cb = (_AZULIB_MESSAGE*)message;

    if(cb->payload != NULL)
    {
        ustream_dispose(cb->payload);
    }
}

ULIB_RESULT azulib_message_get_channel(AZULIB_MESSAGE* message, AZULIB_MESSAGE_CHANNEL* channel)
{
#ifdef ULIB_CONFIG_MESSAGE_VALIDATE_ARGUMENTS
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(message, ULIB_ILLEGAL_ARGUMENT_ERROR),
        UCONTRACT_REQUIRE_NOT_NULL(channel, ULIB_ILLEGAL_ARGUMENT_ERROR));
#endif // ULIB_CONFIG_MESSAGE_VALIDATE_ARGUMENTS

    _AZULIB_MESSAGE* cb = (_AZULIB_MESSAGE*)message;

    channel->name = cb->channel.name;
    channel->qos = cb->channel.qos;

    return ULIB_SUCCESS;
}


ULIB_RESULT azulib_message_set_method(AZULIB_MESSAGE* message, AZULIB_MESSAGE_METHODS method)
{
#ifdef ULIB_CONFIG_MESSAGE_VALIDATE_ARGUMENTS
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(message, ULIB_ILLEGAL_ARGUMENT_ERROR));
#endif // ULIB_CONFIG_MESSAGE_VALIDATE_ARGUMENTS

    _AZULIB_MESSAGE* cb = (_AZULIB_MESSAGE*)message;

    cb->method = method;

    return ULIB_SUCCESS;
}

ULIB_RESULT azulib_message_get_method(AZULIB_MESSAGE* message, AZULIB_MESSAGE_METHODS* method)
{
#ifdef ULIB_CONFIG_MESSAGE_VALIDATE_ARGUMENTS
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(message, ULIB_ILLEGAL_ARGUMENT_ERROR),
        UCONTRACT_REQUIRE_NOT_NULL(method, ULIB_ILLEGAL_ARGUMENT_ERROR));
#endif // ULIB_CONFIG_MESSAGE_VALIDATE_ARGUMENTS

    _AZULIB_MESSAGE* cb = (_AZULIB_MESSAGE*)message;

    *method = cb->method;

    return ULIB_SUCCESS;
}

ULIB_RESULT azulib_message_get_payload(AZULIB_MESSAGE* message, USTREAM** payload)
{
#ifdef ULIB_CONFIG_MESSAGE_VALIDATE_ARGUMENTS
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(message, ULIB_ILLEGAL_ARGUMENT_ERROR),
        UCONTRACT_REQUIRE_NOT_NULL(payload, ULIB_ILLEGAL_ARGUMENT_ERROR));
#endif // ULIB_CONFIG_MESSAGE_VALIDATE_ARGUMENTS

    _AZULIB_MESSAGE* cb = (_AZULIB_MESSAGE*)message;

    if (cb->payload != NULL)
    {
        *payload = ustream_clone(cb->payload, 0);
    }
    else
    {
        *payload = NULL;
    }

    return ULIB_SUCCESS;
}

ULIB_RESULT azulib_message_set_property(AZULIB_MESSAGE* message, AZULIB_MESSAGE_PROPERTY* property)
{
    //TODO: implement it.
    return ULIB_SUCCESS;
}

