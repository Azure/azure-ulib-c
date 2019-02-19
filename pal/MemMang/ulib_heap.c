// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>

void* uLibMalloc(size_t size)
{
    return malloc(size);
}

void uLibFree(void* ptr)
{
    free(ptr);
}
