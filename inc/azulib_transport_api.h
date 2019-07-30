// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file azulib_transport_api.h
 */

#ifndef AZURE_ULIB_C_INC_AZULIB_TRANSPORT_API_H_
#define AZURE_ULIB_C_INC_AZULIB_TRANSPORT_API_H_

#include "ulib_config.h"
#include "ulib_result.h"
#include "azulib_message_api.h"

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif /* __cplusplus */

/**
 * @brief   Transport control block.
 */
typedef void* AZULIB_TRANSPORT_HANDLE;
typedef void* AZULIB_TRANSPORT_CLIENT_CONTEXT;

/**
 * @brief   Transport connection state.
 */
MU_DEFINE_ENUM(AZULIB_TRANSPORT_STATE,
    AZULIB_TRANSPORT_STATE_CONNECTED,
    AZULIB_TRANSPORT_STATE_DISCONNECTED);

/**
 * @brief   Transport connection callback signature.
 */
typedef void(*AZULIB_TRANSPORT_CONNECTION_CALLBACK)(
    AZULIB_TRANSPORT_HANDLE transport_handle, 
    ULIB_RESULT result, 
    AZULIB_TRANSPORT_STATE* state);

/**
 * @brief   Transport receive message callback signature.
 */
typedef void(*AZULIB_TRANSPORT_RECEIVE_CALLBACK)(
    AZULIB_TRANSPORT_HANDLE transport_handle, 
    ULIB_RESULT result, 
    AZULIB_MESSAGE* rx_message, 
    AZULIB_MESSAGE_RELEASE_CALLBACK release_message_callback);

/**
 * @brief   Transport result callback signature.
 */
typedef void(*AZULIB_TRANSPORT_RESULT_CALLBACK)(
    AZULIB_TRANSPORT_CLIENT_CONTEXT transport_handle,
    ULIB_RESULT result, 
    AZULIB_MESSAGE* rx_message, 
    AZULIB_MESSAGE_RELEASE_CALLBACK release_message_callback);

/**
 * @brief   Transport connection end point.
 */
typedef struct AZULIB_TRANSPORT_ENDPOINT_TAG
{
    const char* const url;
    uint32_t port;
} AZULIB_TRANSPORT_ENDPOINT;

/**
 * @brief   Transport client interface that creates a communication channel between service and client.
 */
typedef struct AZULIB_I_TRANSPORT_CHANNEL_TAG
{
    ULIB_RESULT(*open_async)(
        AZULIB_TRANSPORT_HANDLE transport_handle,
        AZULIB_MESSAGE_CHANNEL* channel, 
        AZULIB_TRANSPORT_RECEIVE_CALLBACK receive_message_callback, 
        AZULIB_TRANSPORT_RESULT_CALLBACK open_result_callback,
        AZULIB_TRANSPORT_CLIENT_CONTEXT context);
    
    ULIB_RESULT(*close_async)(
        AZULIB_TRANSPORT_HANDLE transport_handle,
        AZULIB_MESSAGE_CHANNEL* channel,
        AZULIB_TRANSPORT_RESULT_CALLBACK close_result_callback,
        AZULIB_TRANSPORT_CLIENT_CONTEXT context);
    
    ULIB_RESULT(*send_async)(
        AZULIB_TRANSPORT_HANDLE transport_handle,
        AZULIB_MESSAGE_CHANNEL* channel, 
        AZULIB_MESSAGE* message, 
        AZULIB_TRANSPORT_RESULT_CALLBACK send_result_callback,
        AZULIB_TRANSPORT_CLIENT_CONTEXT context);

    ULIB_RESULT(*subscribe_connection_state)(
        AZULIB_TRANSPORT_HANDLE transport_handle,
        AZULIB_TRANSPORT_CONNECTION_CALLBACK connection_state_callback);
} AZULIB_I_TRANSPORT_CHANNEL;

/**
 * @brife   This is the interface that any client should expose, and the transport can call.
 */
typedef struct AZULIB_I_TRANSPORT_CLIENT_TAG
{
    void(*set_transport_interface)(
        AZULIB_TRANSPORT_CLIENT_CONTEXT context, 
        const AZULIB_I_TRANSPORT_CHANNEL* transport_interface, 
        AZULIB_TRANSPORT_HANDLE transport);
} AZULIB_I_TRANSPORT_CLIENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_AZULIB_TRANSPORT_API_H_ */
