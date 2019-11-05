// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <windows.h>

#include "az_ulib_pal_os.h"
#include "az_ulib_pal_os_api.h"

void az_pal_os_lock_init(AZ_PAL_OS_LOCK* lock) { InitializeSRWLock((SRWLOCK*)lock); }

void az_pal_os_lock_deinit(AZ_PAL_OS_LOCK* lock) { (void)lock; }

void az_pal_os_lock_acquire(AZ_PAL_OS_LOCK* lock) { AcquireSRWLockExclusive((SRWLOCK*)lock); }

void az_pal_os_lock_release(AZ_PAL_OS_LOCK* lock) { ReleaseSRWLockExclusive((SRWLOCK*)lock); }

void az_pal_os_sleep(uint32_t sleep_time_ms) { Sleep(sleep_time_ms); }
