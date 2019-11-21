// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <windows.h>

#include "az_ulib_pal_os.h"
#include "az_ulib_pal_os_api.h"

void az_pal_os_lock_init(az_ulib_pal_os_lock* lock) { InitializeSRWLock((SRWLOCK*)lock); }

void az_pal_os_lock_deinit(az_ulib_pal_os_lock* lock) { (void)lock; }

void az_pal_os_lock_acquire(az_ulib_pal_os_lock* lock) { AcquireSRWLockExclusive((SRWLOCK*)lock); }

void az_pal_os_lock_release(az_ulib_pal_os_lock* lock) { ReleaseSRWLockExclusive((SRWLOCK*)lock); }

void az_pal_os_sleep(uint32_t sleep_time_ms) { Sleep(sleep_time_ms); }
