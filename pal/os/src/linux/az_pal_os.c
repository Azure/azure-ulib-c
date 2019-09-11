// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pthread.h>

#include "az_pal_os.h"
#include "az_pal_os_api.h"

void az_pal_os_lock_init(AZ_PAL_OS_LOCK* lock)
{
    pthread_mutex_init((pthread_mutex_t*)lock, NULL);
}

void az_pal_os_lock_deinit(AZ_PAL_OS_LOCK* lock)
{
    pthread_mutex_destroy((pthread_mutex_t*)lock);
}

void az_pal_os_lock_acquire(AZ_PAL_OS_LOCK* lock)
{
    pthread_mutex_lock((pthread_mutex_t*)lock);
}

void az_pal_os_lock_release(AZ_PAL_OS_LOCK* lock)
{
    pthread_mutex_unlock((pthread_mutex_t*)lock);
}
