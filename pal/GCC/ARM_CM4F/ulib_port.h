// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_PAL_GCC_ARM_CM4F_ULIB_PORT_H_
#define AZURE_ULIB_C_PAL_GCC_ARM_CM4F_ULIB_PORT_H_

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

__attribute__( ( always_inline ) ) static inline uint32_t ULIB_PORT_ATOMIC_INC_W(volatile uint32_t* addr)
{
    register uint32_t result;
    register uint32_t modified = 0;

    __asm volatile(
        "1:     ldrex   %0, [%1]                \n" 
        "       add     %0, #1                  \n"
        "       strex   %2, %0, [%1]            \n"
        "       cmp     %2, #0                  \n"
        "       bne     1b                      "
        : "=&r" (result)
        : "r" (addr), "r" (modified)
        : "cc", "memory"
    );

    return result;
}

__attribute__( ( always_inline ) ) static inline uint32_t ULIB_PORT_ATOMIC_DEC_W(volatile uint32_t* addr)
{
    register uint32_t result;
    register uint32_t modified = 0;

    __asm volatile(
        "1:     ldrex   %0, [%1]                \n" 
        "       sub     %0, #1                  \n"
        "       strex   %2, %0, [%1]            \n"
        "       cmp     %2, #0                  \n"
        "       bne     1b                      "
        : "=&r" (result)
        : "r" (addr), "r" (modified)
        : "cc", "memory"
    );

    return result;
}

#define ULIB_PORT_THROW_HARD_FAULT          (*(char*)NULL = 0)

#ifdef __cplusplus
}
#endif

#endif /* AZURE_ULIB_C_PAL_GCC_ARM_CM4F_ULIB_PORT_H_ */