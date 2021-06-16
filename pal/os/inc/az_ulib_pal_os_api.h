// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/** @file az_ulib_pal_os_api.h
 *    @brief      A minimalistic platform agnostic abstraction.
 */

#ifndef AZ_ULIB_PAL_OS_API_H
#define AZ_ULIB_PAL_OS_API_H

#include "az_ulib_pal_os.h"
#include "az_ulib_result.h"

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

/**
 * @brief   Create a thread.
 *
 * @param[in]       function_ptr    The function with prototype `az_ulib_pal_start_function_ptr`
 *                                  that the thread shall run.
 * @param[in]       args            The `az_ulib_pal_thread_args` with the arguments to
 *                                  to the thread.
 * @param[out]      handle          The `az_ulib_pal_thread_handle` to return the thread handle.
 *
 * @return The #az_result with the thread creation result.
 *  @retval #AZ_OK                  If the thread was created with success.
 *  @retval #AZ_ERROR_OUT_OF_MEMORY If there is not enough memory to create the new thread.
 *  @retval #AZ_ERROR_ULIB_SYSTEM   If the create API in the OS return error.
 */
az_result az_pal_os_thread_create(
    az_ulib_pal_start_function_ptr function_ptr,
    az_ulib_pal_thread_args args,
    az_ulib_pal_thread_handle* handle);

/**
 * @brief   Join a thread.
 *
 * @param[out]      handle          The `az_ulib_pal_thread_handle` with the thread handle.
 * @param[out]      res             The `int` to return the thread join result.
 *
 * @return The #az_result with the join to a thread result.
 *  @retval #AZ_OK                  If the thread join was done with success.
 *  @retval #AZ_ERROR_ULIB_SYSTEM   If the join API in the OS return error.
 */
az_result az_pal_os_thread_join(az_ulib_pal_thread_handle handle, int* res);

#ifdef __cplusplus
}
#endif

#endif /* AZ_ULIB_PAL_OS_API_H */
