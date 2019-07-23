// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// This file gets included into ulib_port.h as a means of extending the behavior of
// atomic increment, decrement, and test.
#ifndef AZIOT_ULIB_GCC_IOS_PORT_H_
#define AZIOT_ULIB_GCC_IOS_PORT_H_


// This iOS-specific header offers 3 strategies:
//   AZURE_ULIB_C_ATOMIC_DONTCARE     -- no atomicity guarantee
//   AZURE_ULIB_C_USE_STD_ATOMIC      -- C11 atomicity
//   AZURE_ULIB_C_USE_GNU_C_ATOMIC    -- GNU-specific atomicity

#if defined(__GNUC__)
#define AZURE_ULIB_C_USE_GNU_C_ATOMIC 1
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ == 201112)
#define AZURE_ULIB_C_USE_STD_ATOMIC 1
#undef AZURE_ULIB_C_USE_GNU_C_ATOMIC
#endif

#if defined(AZURE_ULIB_C_ATOMIC_DONTCARE)
#define COUNT_TYPE uint32_t
#elif defined(AZURE_ULIB_C_USE_STD_ATOMIC)
#define COUNT_TYPE _Atomic uint32_t
#else  // AZURE_ULIB_C_USE_GNU_C_ATOMIC
#define COUNT_TYPE uint32_t
#endif // defined(AZURE_ULIB_C_ATOMIC_DONTCARE)


/*the following macros increment/decrement a ref count in an atomic way, depending on the platform*/
/*The following mechanisms are considered in this order
AZURE_ULIB_C_ATOMIC_DONTCARE does not use atomic operations
- will result in ++/-- used for increment/decrement.
C11
- will result in #include <stdatomic.h>
- will use atomic_fetch_add/sub;
- about the return value: "Atomically, the value pointed to by object immediately before the effects"
gcc
- will result in no include (for gcc these are intrinsics build in)
- will use __sync_fetch_and_add/sub
- about the return value: "... returns the value that had previously been in memory." (https://gcc.gnu.org/onlinedocs/gcc-4.4.3/gcc/Atomic-Builtins.html#Atomic-Builtins)
*/


#if defined(AZURE_ULIB_C_ATOMIC_DONTCARE)
#define AZIOT_ULIB_PORT_ATOMIC_INC_W(count) ++(*count)
#define AZIOT_ULIB_PORT_ATOMIC_DEC_W(count) --(*count)

#elif defined(AZURE_ULIB_C_USE_STD_ATOMIC)
#include <stdatomic.h>
#define AZIOT_ULIB_PORT_ATOMIC_INC_W(count) atomic_fetch_add((count), 1)
#define AZIOT_ULIB_PORT_ATOMIC_DEC_W(count) atomic_fetch_sub((count), 1)

#elif defined(AZURE_ULIB_C_USE_GNU_C_ATOMIC)
#define AZIOT_ULIB_PORT_ATOMIC_INC_W(count) __sync_add_and_fetch((count), 1)
#define AZIOT_ULIB_PORT_ATOMIC_DEC_W(count) __sync_sub_and_fetch((count), 1)

#endif /*defined(AZURE_ULIB_C_USE_GNU_C_ATOMIC)*/

#define AZIOT_ULIB_PORT_THROW_HARD_FAULT      (*(char*)NULL = 0)

#endif // AZIOT_ULIB_GCC_IOS_PORT_H_
