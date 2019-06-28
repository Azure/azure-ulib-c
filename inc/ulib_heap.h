// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file ulib_heap.h
 */

#ifndef AZURE_ULIB_C_INC_ULIB_HEAP_H_
#define AZURE_ULIB_C_INC_ULIB_HEAP_H_

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif /* __cplusplus */

MOCKABLE_FUNCTION(, void*, ulib_malloc,
            size_t, size);
MOCKABLE_FUNCTION(, void, ulib_free,
            void*, ptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_ULIB_HEAP_H_ */
