// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license
// information.

#ifndef AZ_ULIB_GCC_ARM_CM4F_PORT_H
#define AZ_ULIB_GCC_ARM_CM4F_PORT_H

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

  __attribute__((always_inline)) static inline long AZ_ULIB_PORT_ATOMIC_INC_W(volatile long* addr)
  {
    register long prev;
    register long modified;
    register long result;

    __asm volatile("1:     ldrex   %0, [%3]                \n"
                   "       mov     %1, %0                  \n"
                   "       adds    %0, #1                  \n"
                   "       strex   %2, %0, [%3]            \n"
                   "       cmp     %2, #0                  \n"
                   "       bne     1b                      "
                   : "=&r"(result), "=&r"(prev), "=&r"(modified)
                   : "r"(addr)
                   : "cc", "memory");

    return prev;
  }

  __attribute__((always_inline)) static inline long AZ_ULIB_PORT_ATOMIC_DEC_W(volatile long* addr)
  {
    register long prev;
    register long modified;
    register long result;

    __asm volatile("1:     ldrex   %0, [%3]                \n"
                   "       mov     %1, %0                  \n"
                   "       subs    %0, #1                  \n"
                   "       strex   %2, %0, [%3]            \n"
                   "       cmp     %2, #0                  \n"
                   "       bne     1b                      "
                   : "=&r"(result), "=&r"(prev), "=&r"(modified)
                   : "r"(addr)
                   : "cc", "memory");

    return prev;
  }

  __attribute__((always_inline)) static inline long AZ_ULIB_PORT_ATOMIC_EXCHANGE_W(
      volatile long* addr,
      long val)
  {
    register long result;
    register long modified;

    __asm volatile("1:     ldrex   %0, [%2]                \n"
                   "       strex   %1, %3, [%2]            \n"
                   "       cmp     %1, #0                  \n"
                   "       bne     1b                      "
                   : "=&r"(result), "=&r"(modified)
                   : "r"(addr), "r"(val)
                   : "cc", "memory");

    return result;
  }

  __attribute__((always_inline)) static inline void* AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(
      const volatile void** addr,
      const void* val)
  {
    register void* result;
    register long modified;

    __asm volatile("1:     ldrex   %0, [%2]                \n"
                   "       strex   %1, %3, [%2]            \n"
                   "       cmp     %1, #0                  \n"
                   "       bne     1b                      "
                   : "=&r"(result), "=&r"(modified)
                   : "r"(addr), "r"(val)
                   : "cc", "memory");

    return result;
  }

  __attribute__((always_inline)) static inline void AZ_ULIB_PORT_GET_DATA_CONTEXT(
      volatile void** data_address)
  {
    __asm inline("       str     r9, [%0]                " : "=&r"(data_address) : : "memory");
  }

  __attribute__((always_inline)) static inline void AZ_ULIB_PORT_SET_DATA_CONTEXT(
      volatile void* data_address)
  {
    __asm inline("       ldr     r9, [%0]                " : : "r"(&data_address) : "r9");
  }

#define AZ_ULIB_PORT_THROW_HARD_FAULT (*(char*)NULL = 0)

#ifdef __cplusplus
}
#endif

#endif /* AZ_ULIB_GCC_ARM_CM4F_PORT_H */
