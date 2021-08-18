// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

// This file gets included into az_ulib_port.h as a means of extending the behavior of
// atomic increment, decrement, and test.
#ifndef AZ_ULIB_GCC_LINUX_PORT_H
#define AZ_ULIB_GCC_LINUX_PORT_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

  // This Linux-specific header offers 3 strategies:
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

  /*the following macros increment/decrement a ref count in an atomic way, depending on the
   * platform*/
  /*The following mechanisms are considered in this order
  AZURE_ULIB_C_ATOMIC_DONTCARE does not use atomic operations
  - will result in ++/-- used for increment/decrement.
  C11
  - will result in #include <stdatomic.h>
  - will use atomic_fetch_add/sub;
  - about the return value: "Atomically, the value pointed to by object immediately before the
    effects" gcc
  - will result in no include (for gcc these are intrinsics build in)
  - will use __sync_fetch_and_add/sub
  - about the return value: "... returns the value that had previously been in memory."
    (https://gcc.gnu.org/onlinedocs/gcc-4.4.3/gcc/Atomic-Builtins.html#Atomic-Builtins)
  */

#if defined(AZURE_ULIB_C_ATOMIC_DONTCARE)
#define AZ_ULIB_PORT_ATOMIC_INC_W(count) ++(*(count))
#define AZ_ULIB_PORT_ATOMIC_DEC_W(count) --(*(count))
  static inline long AZ_ULIB_PORT_ATOMIC_EXCHANGE_W(volatile long* addr, long val)
  {
    long prev = *addr;
    *addr = val;
    return prev;
  }
  static inline long AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(volatile void** addr, void* val)
  {
    void* prev = *addr;
    *addr = val;
    return prev;
  }

#elif defined(AZURE_ULIB_C_USE_STD_ATOMIC)
#ifndef __cplusplus
#include <stdatomic.h>
#else
#include <atomic>
#endif /* __cplusplus */
static inline long AZ_ULIB_PORT_ATOMIC_INC_W(volatile long* addr)
{
  return (long)atomic_fetch_add((atomic_int)addr, 1) + 1;
}
static inline long AZ_ULIB_PORT_ATOMIC_DEC_W(volatile long* addr)
{
  return (long)atomic_fetch_sub((atomic_int)addr, 1) - 1;
}
#define AZ_ULIB_PORT_ATOMIC_EXCHANGE_W(target, value) atomic_exchange((target), (value))
#define AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(target, value) atomic_exchange((target), (value))

#elif defined(AZURE_ULIB_C_USE_GNU_C_ATOMIC)
#define AZ_ULIB_PORT_ATOMIC_INC_W(count) __sync_add_and_fetch((count), 1)
#define AZ_ULIB_PORT_ATOMIC_DEC_W(count) __sync_sub_and_fetch((count), 1)
#define AZ_ULIB_PORT_ATOMIC_EXCHANGE_W(target, value) \
  __sync_val_compare_and_swap((target), *(target), (value))
#define AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(target, value) \
  __sync_val_compare_and_swap((target), *(target), (value))

#endif /*defined(AZURE_ULIB_C_USE_GNU_C_ATOMIC)*/

#define AZ_ULIB_PORT_THROW_HARD_FAULT (*(char*)NULL = 0)

#define AZ_ULIB_PORT_GET_DATA_CONTEXT(data_address)
#define AZ_ULIB_PORT_SET_DATA_CONTEXT(data_address)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // AZ_ULIB_GCC_LINUX_PORT_H
