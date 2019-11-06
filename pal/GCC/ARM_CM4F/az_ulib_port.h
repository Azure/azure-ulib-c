// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license
// information.

#ifndef AZ_ULIB_GCC_ARM_CM4F_PORT_H
#define AZ_ULIB_GCC_ARM_CM4F_PORT_H

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

__attribute__((always_inline)) static inline uint32_t AZ_ULIB_PORT_ATOMIC_INC_W(
    volatile uint32_t* addr) {
  register uint32_t result;
  register uint32_t prev;
  register uint32_t modified = 0;

  __asm volatile("1:     ldrex   %0, [%2]                \n"
                 "       str     %0, %1                  \n"
                 "       add     %0, #1                  \n"
                 "       strex   %3, %0, [%2]            \n"
                 "       cmp     %3, #0                  \n"
                 "       bne     1b                      "
                 : "=&r"(result), "=&r"(prev)
                 : "r"(addr), "r"(modified)
                 : "cc", "memory");

  return prev;
}

__attribute__((always_inline)) static inline uint32_t AZ_ULIB_PORT_ATOMIC_DEC_W(
    volatile uint32_t* addr) {
  register uint32_t result;
  register uint32_t prev;
  register uint32_t modified = 0;

  __asm volatile("1:     ldrex   %0, [%2]                \n"
                 "       str     %0, %1                  \n"
                 "       sub     %0, #1                  \n"
                 "       strex   %3, %0, [%2]            \n"
                 "       cmp     %3, #0                  \n"
                 "       bne     1b                      "
                 : "=&r"(result), "=&r"(prev)
                 : "r"(addr), "r"(modified)
                 : "cc", "memory");

  return prev;
}

__attribute__((always_inline)) static inline uint32_t AZ_ULIB_PORT_ATOMIC_EXCHANGE_W(
    volatile uint32_t* addr,
    uint32_t val) {
  register uint32_t result;
  register uint32_t modified = 0;
  register uint32_t value = val;

  __asm volatile("1:     ldrex   %0, [%1]                \n"
                 "       strex   %2, %3, [%1]            \n"
                 "       cmp     %2, #0                  \n"
                 "       bne     1b                      "
                 : "=&r"(result)
                 : "r"(addr), "r"(modified), "r"(value)
                 : "cc", "memory");

  return result;
}

__attribute__((always_inline)) static inline void* AZ_ULIB_PORT_ATOMIC_EXCHANGE_PTR(
    volatile void** addr,
    void* val) {
  register void* result;
  register uint32_t modified = 0;
  register void* value = val;

  __asm volatile("1:     ldrex   %0, [%1]                \n"
                 "       strex   %2, %3, [%1]            \n"
                 "       cmp     %2, #0                  \n"
                 "       bne     1b                      "
                 : "=&r"(result)
                 : "r"(addr), "r"(modified), "r"(value)
                 : "cc", "memory");

  return result;
}

#define AZ_ULIB_PORT_THROW_HARD_FAULT (*(char*)NULL = 0)

#ifdef __cplusplus
}
#endif

#endif /* AZ_ULIB_GCC_ARM_CM4F_PORT_H */
