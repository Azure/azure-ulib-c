// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_PAL_OS_WINDOWS_H
#define AZ_ULIB_PAL_OS_WINDOWS_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  @struct AZ_PAL_OS_LOCK
 *
 *  @brief  pointer to a platform specific struct for a lock implementation
 */
typedef SRWLOCK AZ_PAL_OS_LOCK;

#ifdef __cplusplus
}
#endif

#endif /* AZ_ULIB_PAL_OS_WINDOWS_H */
