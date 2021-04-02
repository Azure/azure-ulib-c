// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <tx_api.h>

#include "az_ulib_pal_os.h"
#include "az_ulib_pal_os_api.h"

void az_pal_os_lock_init(az_ulib_pal_os_lock* lock)
{
  tx_mutex_create(lock, NULL, TX_NO_INHERIT);
}

void az_pal_os_lock_deinit(az_ulib_pal_os_lock* lock)
{
  tx_mutex_delete(lock);
}

void az_pal_os_lock_acquire(az_ulib_pal_os_lock* lock)
{
  tx_mutex_get(lock, TX_WAIT_FOREVER);
}

void az_pal_os_lock_release(az_ulib_pal_os_lock* lock)
{
  tx_mutex_put(lock);
}

void az_pal_os_sleep(uint32_t sleep_time_ms)
{
  tx_thread_sleep(sleep_time_ms);
}
