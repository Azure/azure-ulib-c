// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include "azulib_http_api.h"
#include "azulib_message_api.h"
#include "azulib_option_api.h"
#include "azulib_transport_api.h"
#include "ucontract.h"
#include "ulib_result.h"
#include "ulog.h"


/* SDK session. */

typedef AZULIB_TRANSPORT_RESULT_CALLBACK SDK_RESULT_CALLBACK;
typedef AZULIB_TRANSPORT_CLIENT_CONTEXT SDK_CLIENT_CONTEXT;

//AZULIB_OPTION_LIST(SDK_OPTION_LIST,
//    AZULIB_OPTION_ADD("my_option_key_1", "my_option_val_1"),
//    AZULIB_OPTION_ADD("my_option_key_2", "my_option_val_2"));

//static const AZULIB_OPTION _options[] =
//{
//    {"a", "b"},
//    {"c", "b"}
//};

typedef struct SDK_CLIENT_TAG
{
    AZULIB_TRANSPORT_HANDLE transport_handle;
    const AZULIB_I_TRANSPORT_CHANNEL* i_transport_channel;
} SDK_CLIENT;

static void sdk_on_receive_message(AZULIB_TRANSPORT_CLIENT_CONTEXT context, ULIB_RESULT result, AZULIB_MESSAGE* rx_message, AZULIB_MESSAGE_RELEASE_CALLBACK release_message)
{
    (void)context;
    (void)result;

    //TODO: use the received message.

    if(release_message != NULL)
    {
        release_message(rx_message);
    }
}

static void sdk_set_transport_interface(
    AZULIB_TRANSPORT_CLIENT_CONTEXT context, 
    const AZULIB_I_TRANSPORT_CHANNEL* transport_interface, 
    AZULIB_TRANSPORT_HANDLE transport_handle)
{
    ((SDK_CLIENT*)context)->i_transport_channel = transport_interface;
    ((SDK_CLIENT*)context)->transport_handle = transport_handle;

//    transport_interface->subscribe_receive(transport_handle, _topics, sdk_on_receive_message);
}

static const AZULIB_I_TRANSPORT_CLIENT i_transport_client =
{
    sdk_set_transport_interface
};

ULIB_RESULT sdk_create(SDK_CLIENT* client)
{
    client->i_transport_channel = NULL;
    client->transport_handle = NULL;
    return ULIB_SUCCESS;
}

const AZULIB_I_TRANSPORT_CLIENT* sdk_as_transport_client(SDK_CLIENT* client)
{
    return &i_transport_client;
}

ULIB_RESULT sdk_get_async(
    SDK_CLIENT* client, 
    AZULIB_MESSAGE* message, 
    SDK_RESULT_CALLBACK result_callback, 
    SDK_CLIENT_CONTEXT context)
{
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(client, ULIB_ILLEGAL_ARGUMENT_ERROR),
        UCONTRACT_REQUIRE_NOT_NULL(client->i_transport_channel, ULIB_NOT_INITIALIZED_ERROR));

    ULIB_RESULT result;

    AZULIB_MESSAGE_CHANNEL channel;

    if((result = azulib_message_get_channel(message, &channel)) != ULIB_SUCCESS)
    {
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, "Failed getting channel from message");
    }
    else if ((result = azulib_message_set_method(message, AZULIB_MESSAGE_METHOD_GET)) != ULIB_SUCCESS)
    {
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, "Failed setting method GET to message");
    }
    else if((result = client->i_transport_channel->send_async(
        client->transport_handle, 
        &channel, 
        message, 
        (AZULIB_TRANSPORT_RESULT_CALLBACK)result_callback,
        context)) != ULIB_SUCCESS)
    {
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, "Failed sending message to the transport");
    }

    return result;
}


/* User session. */

static SDK_CLIENT _sdk_client;
static AZULIB_HTTP _http;

typedef struct CONTOSO_TAG
{
    AZULIB_MESSAGE* tx_message;
} CONTOSO;

static CONTOSO _contoso;

static AZULIB_TRANSPORT_ENDPOINT ENDPOINT =
{
    "http://www.microsoft.com",
    80
};

void on_get_result(SDK_CLIENT_CONTEXT context, ULIB_RESULT result, AZULIB_MESSAGE* rx_message, AZULIB_MESSAGE_RELEASE_CALLBACK release_message)
{
    (void)result;

    CONTOSO* contoso = context;
    USTREAM* payload;

    if (rx_message != NULL)
    {
        if (azulib_message_get_payload(rx_message, &payload) == ULIB_SUCCESS)
        {
            (void)printf("message received/r/n");
        }

        if (release_message != NULL)
        {
            release_message(rx_message);
        }
    }

    azulib_message_deinit(contoso->tx_message);
    free(contoso->tx_message);
}

int main(void)
{
    (void)printf("Start HTTP-simple-get sample\r\n");
    ULIB_RESULT result;
    
    memset(&_contoso, 0, sizeof(CONTOSO));

    if ((result = sdk_create(&_sdk_client)) != ULIB_SUCCESS)
    {
        (void)printf("Failed [%d] creating client\r\n", result);
    }
    else if((result = azulib_http_create(&_http, &ENDPOINT, NULL)) != ULIB_SUCCESS)
    {
        (void)printf("Failed [%d] creating HTTP to %s:%d\r\n", result, ENDPOINT.url, ENDPOINT.port);
    }
    else
    {
        (void)printf("HTTP created to %s:%d\r\n", ENDPOINT.url, ENDPOINT.port);
        result = azulib_http_attach_client(
            &_http, 
            (AZULIB_TRANSPORT_CLIENT_CONTEXT)&_sdk_client, 
            sdk_as_transport_client(&_sdk_client));
        if(result != ULIB_SUCCESS)
        {
            (void)printf("Failed [%d] attaching client to HTTP\r\n", result);
        }
        else
        {
            (void)printf("Client attached to HTTP with success\r\n");
            if((result = azulib_http_connect(&_http)) != ULIB_SUCCESS)
            {
                (void)printf("Failed [%d] connecting to HTTP service\r\n", result);
            }
            else
            {
                _contoso.tx_message = (AZULIB_MESSAGE*)malloc(sizeof(AZULIB_MESSAGE));

                azulib_message_init(_contoso.tx_message, "", 0, NULL);
                
                sdk_get_async(&_sdk_client, _contoso.tx_message, on_get_result, &_contoso);
            }
        }
    }

    return 0;
}
