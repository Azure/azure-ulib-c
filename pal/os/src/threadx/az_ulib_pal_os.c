// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <tx_api.h>

#include "az_ulib_pal_os.h"
#include "az_ulib_pal_os_api.h"
#include "az_ulib_result.h"

void az_pal_os_lock_init(az_ulib_pal_os_lock* lock) { tx_mutex_create(lock, NULL, TX_NO_INHERIT); }

void az_pal_os_lock_deinit(az_ulib_pal_os_lock* lock) { tx_mutex_delete(lock); }

void az_pal_os_lock_acquire(az_ulib_pal_os_lock* lock) { tx_mutex_get(lock, TX_WAIT_FOREVER); }

void az_pal_os_lock_release(az_ulib_pal_os_lock* lock) { tx_mutex_put(lock); }

void az_pal_os_sleep(uint32_t sleep_time_ms) { tx_thread_sleep(sleep_time_ms); }

az_result az_pal_os_thread_create(
    az_ulib_pal_start_function_ptr function_ptr,
    az_ulib_pal_thread_args args,
    az_ulib_pal_thread_handle* handle)
{
  (void)function_ptr;
  (void)args;
  (void)handle;
  return AZ_ERROR_NOT_IMPLEMENTED;
}

az_result az_pal_os_thread_join(az_ulib_pal_thread_handle handle, int* res)
{
  (void)handle;
  (void)res;
  return AZ_ERROR_NOT_IMPLEMENTED;
}