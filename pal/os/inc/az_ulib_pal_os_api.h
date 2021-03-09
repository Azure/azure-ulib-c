// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/** @file az_ulib_pal_os_api.h
 *    @brief      A minimalistic platform agnostic abstraction.
 */

#ifndef AZ_ULIB_PAL_OS_API_H
#define AZ_ULIB_PAL_OS_API_H

#include "az_ulib_pal_os.h"

#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
extern "C"
{
#endif /* __cplusplus */

/**
 * @brief   This API initialize and returns a valid lock handle.
 *
 * @param[in,out]   lock    The #az_ulib_pal_os_lock* that points to the lock handle.
 */
void az_pal_os_lock_init(az_ulib_pal_os_lock* lock);

/**
 * @brief   The lock instance is destroyed.
 *
 * @param[in]       lock    The #az_ulib_pal_os_lock* that points to a valid lock handle.
 */
void az_pal_os_lock_deinit(az_ulib_pal_os_lock* lock);

/**
 * @brief   Acquires a lock on the given lock handle. Uses platform specific mutex primitives in
 *          its implementation.
 *
 * @param[in]       lock    The #az_ulib_pal_os_lock* that points to a valid lock handle.
 */
void az_pal_os_lock_acquire(az_ulib_pal_os_lock* lock);

/**
 * @brief   Releases the lock on the given lock handle. Uses platform specific mutex primitives in
 *          its implementation.
 *
 * @param[in]       lock    The #az_ulib_pal_os_lock* that points to a valid lock handle.
 */
void az_pal_os_lock_release(az_ulib_pal_os_lock* lock);

/**
 * @brief   Sleep for some milliseconds.
 *
 * @param[in]       sleep_time_ms    The `uint32_t` with the number of milliseconds to sleep.
 */
void az_pal_os_sleep(uint32_t sleep_time_ms);

#ifdef __cplusplus
}
#endif

#endif /* AZ_ULIB_PAL_OS_API_H */
