// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file azulib_http_api.h
 */

#ifndef AZURE_ULIB_C_INC_AZULIB_HTTP_API_H_
#define AZURE_ULIB_C_INC_AZULIB_HTTP_API_H_

#include "ulib_config.h"
#include "ulib_result.h"
#include "azulib_credential_api.h"
#include "azulib_transport_api.h"
#include "internal/http.h"

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif /* __cplusplus */

/**
 * @brief   HTTP control block.
 */
typedef struct AZULIB_HTTP_TAG
{
    uint8_t _private[sizeof(_AZULIB_HTTP)];
} AZULIB_HTTP;

/**
 * @brief   Create a HTTP transport.
 */
MOCKABLE_FUNCTION(,
    ULIB_RESULT, azulib_http_create,
    AZULIB_HTTP*, http,
    AZULIB_TRANSPORT_ENDPOINT*, endpoint,
    AZULIB_CREDENTIAL*, credential);

/**
 * @brief   Destroy the HTTP transport.
 */
MOCKABLE_FUNCTION(,
    ULIB_RESULT, azulib_http_destroy,
    AZULIB_HTTP*, http);

/**
 * @brief   Connect the created HTTP transport.
 */
MOCKABLE_FUNCTION(,
    ULIB_RESULT, azulib_http_connect,
    AZULIB_HTTP*, http);

/**
 * @brief   Disconnect the HTTP transport.
 */
MOCKABLE_FUNCTION(,
    ULIB_RESULT, azulib_http_disconnect,
    AZULIB_HTTP*, http);

/**
 * @brief   Attach a HTTP transport to a client.
 */
MOCKABLE_FUNCTION(,
    ULIB_RESULT, azulib_http_attach_client,
    AZULIB_HTTP*, http,
    AZULIB_TRANSPORT_CLIENT_CONTEXT, context,
    const AZULIB_I_TRANSPORT_CLIENT*, client);

/**
 * @brief   Detach a HTTP transport to a client.
 */
MOCKABLE_FUNCTION(,
    ULIB_RESULT, azulib_http_detach_client,
    AZULIB_HTTP*, http,
    AZULIB_TRANSPORT_CLIENT_CONTEXT, context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_AZULIB_HTTP_API_H_ */
