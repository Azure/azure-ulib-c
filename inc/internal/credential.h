// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_INC_INTERNAL_CREDENTIAL_H_
#define AZURE_ULIB_C_INC_INTERNAL_CREDENTIAL_H_

#include "ulib_config.h"
#include "ulib_result.h"

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#else
#endif /* __cplusplus */

MU_DEFINE_ENUM(INTERNAL_CREDENTIAL_TYPE,
    INTERNAL_CREDENTIAL_TYPE_CERT,
    INTERNAL_CREDENTIAL_TYPE_CONNECTION_STRING);

struct INTERNAL_CREDENTIAL_TAG
{
    INTERNAL_CREDENTIAL_TYPE type;
    union value
    {
        const char* const cert;
        const char* const connection_string;
    };
};

MOCKABLE_FUNCTION(,
    ULIB_RESULT, internal_credential_create_from_cert_no_test,
    struct INTERNAL_CREDENTIAL_TAG*, credential,
    const char* const, cert);
MOCKABLE_FUNCTION(,
    ULIB_RESULT, internal_credential_create_from_cert,
    struct INTERNAL_CREDENTIAL_TAG*, credential,
    const char* const, cert);

MOCKABLE_FUNCTION(,
    ULIB_RESULT, internal_credential_create_from_connection_string_no_test,
    struct INTERNAL_CREDENTIAL_TAG*, credential,
    const char* const, connection_string);
MOCKABLE_FUNCTION(,
    ULIB_RESULT, internal_credential_create_from_connection_string,
    struct INTERNAL_CREDENTIAL_TAG*, credential,
    const char* const, connection_string);

MOCKABLE_FUNCTION(,
    ULIB_RESULT, internal_credential_destroy_no_test,
    struct INTERNAL_CREDENTIAL_TAG*, credential);
MOCKABLE_FUNCTION(,
    ULIB_RESULT, internal_credential_destroy,
    struct INTERNAL_CREDENTIAL_TAG*, credential);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_INTERNAL_CREDENTIAL_H_ */
