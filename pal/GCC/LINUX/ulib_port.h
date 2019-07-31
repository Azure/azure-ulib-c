// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// This file gets included into ulib_port.h as a means of extending the behavior of
// atomic increment, decrement, and test.
#ifndef AZULIB_GCC_LINUX_PORT_H
#define AZULIB_GCC_LINUX_PORT_H


// This Linux-specific header offers 3 strategies:
//   AZULIB_C_ATOMIC_DONTCARE     -- no atomicity guarantee
//   AZULIB_C_USE_STD_ATOMIC      -- C11 atomicity
//   AZULIB_C_USE_GNU_C_ATOMIC    -- GNU-specific atomicity

#if defined(__GNUC__)
#define AZULIB_C_USE_GNU_C_ATOMIC 1
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ == 201112)
#define AZULIB_C_USE_STD_ATOMIC 1
#undef AZULIB_C_USE_GNU_C_ATOMIC
#endif

#if defined(AZULIB_C_ATOMIC_DONTCARE)
#define COUNT_TYPE uint32_t
#elif defined(AZULIB_C_USE_STD_ATOMIC)
#define COUNT_TYPE _Atomic uint32_t
#else  // AZULIB_C_USE_GNU_C_ATOMIC
#define COUNT_TYPE uint32_t
#endif // defined(AZULIB_C_ATOMIC_DONTCARE)


/*the following macros increment/decrement a ref count in an atomic way, depending on the platform*/
/*The following mechanisms are considered in this order
AZULIB_C_ATOMIC_DONTCARE does not use atomic operations
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


#if defined(AZULIB_C_ATOMIC_DONTCARE)
#define AZULIB_PORT_ATOMIC_INC_W(count) ++(*count)
#define AZULIB_PORT_ATOMIC_DEC_W(count) --(*count)

#elif defined(AZULIB_C_USE_STD_ATOMIC)
#include <stdatomic.h>
#define AZULIB_PORT_ATOMIC_INC_W(count) atomic_fetch_add((count), 1)
#define AZULIB_PORT_ATOMIC_DEC_W(count) atomic_fetch_sub((count), 1)

#elif defined(AZULIB_C_USE_GNU_C_ATOMIC)
#define AZULIB_PORT_ATOMIC_INC_W(count) __sync_add_and_fetch((count), 1)
#define AZULIB_PORT_ATOMIC_DEC_W(count) __sync_sub_and_fetch((count), 1)

#endif /*defined(AZULIB_C_USE_GNU_C_ATOMIC)*/

#define AZULIB_PORT_THROW_HARD_FAULT      (*(char*)NULL = 0)

#endif // AZULIB_GCC_LINUX_PORT_H
