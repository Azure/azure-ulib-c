// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file azulib_option_api.h
 */

#ifndef AZURE_ULIB_C_INC_AZULIB_OPTION_API_H_
#define AZURE_ULIB_C_INC_AZULIB_OPTION_API_H_

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
 * @brief   Option control block.
 */
typedef struct AZULIB_OPTION_TAG 
{
    const char* key;
    const void* val;
} AZULIB_OPTION;

/**
 * @brief   Get value by name.
 */
MOCKABLE_FUNCTION(, void*, azulib_option_get, const char* const, key);

#define AZULIB_OPTION_ADD(x,y)    {x, y}

#define AZULIB_OPTION_LIST(list_name, ...) \
    static const AZULIB_OPTION MU_C1(list_name)[] = \
    { \
        MU_FOR_EACH_1(MU_DEFINE_ENUMERATION_CONSTANT, __VA_ARGS__) \
    };

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_AZULIB_OPTION_API_H_ */
