// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "internal/credential.h"
#include "ulib_result.h"

ULIB_RESULT internal_credential_create_from_cert_no_test(
    struct INTERNAL_CREDENTIAL_TAG* credential,
    const char* const cert)
{
    return ULIB_SUCCESS;
}

ULIB_RESULT internal_credential_create_from_cert(
    struct INTERNAL_CREDENTIAL_TAG* credential,
    const char* const cert)
{
    return internal_credential_create_from_cert_no_test(credential, cert);
}

ULIB_RESULT internal_credential_create_from_connection_string_no_test(
    struct INTERNAL_CREDENTIAL_TAG* credential,
    const char* const connection_string)
{
    return ULIB_SUCCESS;
}

ULIB_RESULT internal_credential_create_from_connection_string(
    struct INTERNAL_CREDENTIAL_TAG* credential,
    const char* const connection_string)
{
    return internal_credential_create_from_connection_string_no_test(credential, connection_string);
}

ULIB_RESULT internal_credential_destroy_no_test(
    struct INTERNAL_CREDENTIAL_TAG* credential)
{
    return ULIB_SUCCESS;
}

ULIB_RESULT internal_credential_destroy(
    struct INTERNAL_CREDENTIAL_TAG* credential)
{
    return internal_credential_destroy_no_test(credential);
}
