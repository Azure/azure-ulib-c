// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azulib_http_api.h"
#include "azulib_message_api.h"
#include "azulib_transport_api.h"
#include "ulib_result.h"
#include "ucontract.h"
#include "internal/http.h"

static ULIB_RESULT concrete_open_async(
    AZULIB_TRANSPORT_HANDLE transport_handle,
    AZULIB_MESSAGE_CHANNEL* channel,
    AZULIB_TRANSPORT_RECEIVE_CALLBACK receive_message_callback,
    AZULIB_TRANSPORT_RESULT_CALLBACK open_result_callback,
    AZULIB_TRANSPORT_CLIENT_CONTEXT context);

static ULIB_RESULT concrete_close_async(
    AZULIB_TRANSPORT_HANDLE transport_handle,
    AZULIB_MESSAGE_CHANNEL* channel,
    AZULIB_TRANSPORT_RESULT_CALLBACK close_result_callback,
    AZULIB_TRANSPORT_CLIENT_CONTEXT context);

static ULIB_RESULT concrete_send_async(
    AZULIB_TRANSPORT_HANDLE transport_handle,
    AZULIB_MESSAGE_CHANNEL* channel,
    AZULIB_MESSAGE* message,
    AZULIB_TRANSPORT_RESULT_CALLBACK send_result_callback,
    AZULIB_TRANSPORT_CLIENT_CONTEXT context);

static ULIB_RESULT concrete_subscribe_connection_state(
    AZULIB_TRANSPORT_HANDLE transport_handle,
    AZULIB_TRANSPORT_CONNECTION_CALLBACK connection_state_callback);

static const AZULIB_I_TRANSPORT_CHANNEL I_TRANSPORT_CHANNEL =
{
    concrete_open_async,
    concrete_close_async,
    concrete_send_async,
    concrete_subscribe_connection_state
};

ULIB_RESULT azulib_http_create(
    AZULIB_HTTP* http,
    AZULIB_TRANSPORT_ENDPOINT* endpoint,
    AZULIB_CREDENTIAL* credential)
{
#ifdef ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(http, ULIB_ILLEGAL_ARGUMENT_ERROR),
        UCONTRACT_REQUIRE_NOT_NULL(endpoint, ULIB_ILLEGAL_ARGUMENT_ERROR));
#endif // ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS

    _AZULIB_HTTP* cb = (_AZULIB_HTTP*)http;

    cb->endpoint = endpoint;
    cb->credential = credential;
    for (int i = 0; i < ULIB_CONFIG_HTTP_MAX_CLIENT; i++)
    {
        cb->clients[i] = NULL;
    }

    return ULIB_SUCCESS;
}

ULIB_RESULT azulib_http_destroy(
    AZULIB_HTTP* http)
{
#ifdef ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(http, ULIB_ILLEGAL_ARGUMENT_ERROR));
#endif // ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS

    ULIB_RESULT result;

    if (((_AZULIB_HTTP*)http)->clients != NULL)
    {
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, "there are clients attached to this transport");
        result = ULIB_BUSY_ERROR;
    }
    else
    {
        result = ULIB_SUCCESS;
    }

    return result;
}

ULIB_RESULT azulib_http_connect(
    AZULIB_HTTP* http)
{
#ifdef ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(http, ULIB_ILLEGAL_ARGUMENT_ERROR));
#endif // ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS

    ((_AZULIB_HTTP*)http)->logic_state = AZULIB_TRANSPORT_STATE_CONNECTED;

    return ULIB_SUCCESS;
}

ULIB_RESULT azulib_http_disconnect(
    AZULIB_HTTP* http)
{
#ifdef ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(http, ULIB_ILLEGAL_ARGUMENT_ERROR));
#endif // ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS

    ((_AZULIB_HTTP*)http)->logic_state = AZULIB_TRANSPORT_STATE_DISCONNECTED;

    return ULIB_SUCCESS;
}


ULIB_RESULT azulib_http_attach_client(
    AZULIB_HTTP* http,
    AZULIB_TRANSPORT_CLIENT_CONTEXT context,
    const AZULIB_I_TRANSPORT_CLIENT* client)
{
#ifdef ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(http, ULIB_ILLEGAL_ARGUMENT_ERROR),
        UCONTRACT_REQUIRE_NOT_NULL(client, ULIB_ILLEGAL_ARGUMENT_ERROR),
        UCONTRACT_REQUIRE_NOT_NULL(client->set_transport_interface, ULIB_ILLEGAL_ARGUMENT_ERROR));
#endif // ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS

    client->set_transport_interface(context, &I_TRANSPORT_CHANNEL, (AZULIB_TRANSPORT_HANDLE)http);
            
    return ULIB_SUCCESS;
}

ULIB_RESULT azulib_http_detach_client(
    AZULIB_HTTP* http,
    AZULIB_TRANSPORT_CLIENT_CONTEXT context)
{
#ifdef ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(http, ULIB_ILLEGAL_ARGUMENT_ERROR));
#endif // ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS

    _AZULIB_HTTP* cb = (_AZULIB_HTTP*)http;
    ULIB_RESULT result;
    uint8_t count_client;

    for(count_client = 0; count_client < ULIB_CONFIG_HTTP_MAX_CLIENT; count_client++)
    {
        if(cb->clients[count_client] == context)
        {
            break;
        }
    }

    if (count_client < ULIB_CONFIG_HTTP_MAX_CLIENT)
    {
        cb->clients[count_client] = NULL;
        result = ULIB_SUCCESS;
    }
    else
    {
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }

    return result;
}

static ULIB_RESULT concrete_open_async(
    AZULIB_TRANSPORT_HANDLE transport_handle,
    AZULIB_MESSAGE_CHANNEL* channel,
    AZULIB_TRANSPORT_RECEIVE_CALLBACK receive_message_callback,
    AZULIB_TRANSPORT_RESULT_CALLBACK open_result_callback,
    AZULIB_TRANSPORT_CLIENT_CONTEXT context)
{
#ifdef ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(transport_handle, ULIB_ILLEGAL_ARGUMENT_ERROR),
        UCONTRACT_REQUIRE_NOT_NULL(channel, ULIB_ILLEGAL_ARGUMENT_ERROR));
#endif // ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS

    ULIB_RESULT result;
    result = ULIB_SUCCESS;
    return result;
}

static ULIB_RESULT concrete_close_async(
    AZULIB_TRANSPORT_HANDLE transport_handle,
    AZULIB_MESSAGE_CHANNEL* channel,
    AZULIB_TRANSPORT_RESULT_CALLBACK close_result_callback,
    AZULIB_TRANSPORT_CLIENT_CONTEXT context)
{
#ifdef ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(transport_handle, ULIB_ILLEGAL_ARGUMENT_ERROR),
        UCONTRACT_REQUIRE_NOT_NULL(channel, ULIB_ILLEGAL_ARGUMENT_ERROR));
#endif // ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS

    ULIB_RESULT result;
    result = ULIB_SUCCESS;
    return result;
}

static ULIB_RESULT concrete_send_async(
    AZULIB_TRANSPORT_HANDLE transport_handle,
    AZULIB_MESSAGE_CHANNEL* channel,
    AZULIB_MESSAGE* message,
    AZULIB_TRANSPORT_RESULT_CALLBACK send_result_callback,
    AZULIB_TRANSPORT_CLIENT_CONTEXT context)
{
#ifdef ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(transport_handle, ULIB_ILLEGAL_ARGUMENT_ERROR),
        UCONTRACT_REQUIRE_NOT_NULL(channel, ULIB_ILLEGAL_ARGUMENT_ERROR),
        UCONTRACT_REQUIRE_NOT_NULL(message, ULIB_ILLEGAL_ARGUMENT_ERROR));
#endif // ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS

    ULIB_RESULT result;
    result = ULIB_SUCCESS;

    if(send_result_callback != NULL)
    {
        send_result_callback(context, ULIB_SUCCESS, NULL, NULL);
    }

    return result;
}

static ULIB_RESULT concrete_subscribe_connection_state(
    AZULIB_TRANSPORT_HANDLE transport_handle,
    AZULIB_TRANSPORT_CONNECTION_CALLBACK connection_state_callback)
{
#ifdef ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(transport_handle, ULIB_ILLEGAL_ARGUMENT_ERROR),
        UCONTRACT_REQUIRE_NOT_NULL(connection_state_callback, ULIB_ILLEGAL_ARGUMENT_ERROR));
#endif // ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS

    ULIB_RESULT result;
    result = ULIB_SUCCESS;
    return result;
}
