// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef MSBUILD_X86_ULIB_PORT_H
#define MSBUILD_X86_ULIB_PORT_H

#include "windows.h"

#define AZ_ULIB_PORT_ATOMIC_INC_W(count) InterlockedIncrement((volatile LONG*)(count))
#define AZ_ULIB_PORT_ATOMIC_DEC_W(count) InterlockedDecrement((volatile LONG*)(count))
#define AZ_ULIB_PORT_ATOMIC_EXCHANGE_W(target, value) \
  InterlockedExchange((volatile LONG*)(target), (LONG)(value))
#define AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(target, value) \
  InterlockedExchangePointer((volatile PVOID*)(target), (PVOID)(value))

#define AZ_ULIB_PORT_THROW_HARD_FAULT (*(char*)NULL = 0)

#define AZ_ULIB_PORT_GET_DATA_CONTEXT(data_address)
#define AZ_ULIB_PORT_SET_DATA_CONTEXT(data_address)

#endif /* MSBUILD_X86_ULIB_PORT_H */
