// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_PAL_MSBUILD_X86_ULIB_PORT_H_
#define AZURE_ULIB_C_PAL_MSBUILD_X86_ULIB_PORT_H_

#include "windows.h"

#define ULIB_PORT_ATOMIC_INC_W(count)   InterlockedIncrement((volatile LONG*)(count))
#define ULIB_PORT_ATOMIC_DEC_W(count)   InterlockedDecrement((volatile LONG*)(count))

#define ULIB_PORT_THROW_HARD_FAULT      (*(char*)NULL = 0)

#endif /* AZURE_ULIB_C_PAL_MSBUILD_X86_ULIB_PORT_H_ */
