// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file azulib_credential_api.h
 */

#ifndef AZURE_ULIB_C_INC_AZULIB_CREDENTIAL_API_H_
#define AZURE_ULIB_C_INC_AZULIB_CREDENTIAL_API_H_

#include "ulib_config.h"
#include "ulib_result.h"

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"
#include "internal/credential.h"

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif /* __cplusplus */

/**
 * @brief   Credential control block.
 */
typedef struct INTERNAL_CREDENTIAL_TAG AZULIB_CREDENTIAL;

/**
 * @brief   Create a credential component from certificate.
 */
static inline ULIB_RESULT azulib_credential_create_from_cert(
    AZULIB_CREDENTIAL* credential,
    const char* const cert)
{
#ifdef AZULIB_CREDENTIAL_VALIDATE_ARGUMENTS
    return internal_credential_create_from_cert(
        credential,
        cert);
#else
    return internal_credential_create_from_cert_no_test(
        credential,
        cert);
#endif // AZULIB_CREDENTIAL_VALIDATE_ARGUMENTS
}

/**
 * @brief   Create a credential component from connection string.
 */
static inline ULIB_RESULT azulib_credential_create_from_connection_string(
    AZULIB_CREDENTIAL* credential,
    const char* const connection_string)
{
#ifdef AZULIB_CREDENTIAL_VALIDATE_ARGUMENTS
    return internal_credential_create_from_connection_string(
        credential,
        connection_string);
#else
    return internal_credential_create_from_connection_string_no_test(
        credential,
        connection_string);
#endif // AZULIB_CREDENTIAL_VALIDATE_ARGUMENTS
}

/**
 * @brief   Destroy the credential component.
 */
static inline ULIB_RESULT azulib_credential_destroy(
    AZULIB_CREDENTIAL* credential)
{
#ifdef AZULIB_CREDENTIAL_VALIDATE_ARGUMENTS
    return internal_credential_destroy(
        credential);
#else
    return internal_credential_destroy_no_test(
        credential);
#endif // AZULIB_CREDENTIAL_VALIDATE_ARGUMENTS
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_AZULIB_CREDENTIAL_API_H_ */
