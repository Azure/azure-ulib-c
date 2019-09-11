// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file ulib_heap.h
 * 
 * @brief ulib malloc and free functions available to change by developer
 */

#ifndef AZIOT_ULIB_HEAP_H
#define AZIOT_ULIB_HEAP_H

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif /* __cplusplus */

/**
 * @brief   User defined malloc function to be used in ulib.
 * 
 * The developer can use whatever malloc function they want. By default stdlib <tt>malloc</tt> is used.
 */
MOCKABLE_FUNCTION(, void*, aziot_ulib_malloc,
            size_t, size);

/**
 * @brief   User defined free function to be used in ulib.
 * 
 * The developer can use whatever free function they want. By default stdlib <tt>free</tt> is used.
 */
MOCKABLE_FUNCTION(, void, aziot_ulib_free,
            void*, ptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZIOT_ULIB_HEAP_H */
