// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>

#include "ulib_heap.h"

void* azulib_malloc(size_t size)
{
    return malloc(size);
}

void azulib_free(void* ptr)
{
    free(ptr);
}
