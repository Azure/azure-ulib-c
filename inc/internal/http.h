// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file azulib_http_api.h
 */

#ifndef AZURE_ULIB_C_INC_INTERNAL_HTTP_H_
#define AZURE_ULIB_C_INC_INTERNAL_HTTP_H_

#include "ulib_config.h"
#include "ulib_result.h"
#include "azulib_credential_api.h"
#include "azulib_transport_api.h"

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif /* __cplusplus */

typedef struct _AZULIB_HTTP_TAG
{
    AZULIB_TRANSPORT_ENDPOINT* endpoint;
    AZULIB_CREDENTIAL* credential;
    AZULIB_TRANSPORT_STATE logic_state;
    AZULIB_I_TRANSPORT_CLIENT* clients[ULIB_CONFIG_HTTP_MAX_CLIENT];
} _AZULIB_HTTP;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_INTERNAL_HTTP_H_ */
