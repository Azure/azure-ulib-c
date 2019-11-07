// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <pthread.h>
#include <time.h>

#ifdef TI_RTOS
#include <ti/sysbios/knl/Task.h>
#else
#include <unistd.h>
#endif

#include "az_ulib_pal_os.h"
#include "az_ulib_pal_os_api.h"

void az_pal_os_lock_init(AZ_PAL_OS_LOCK* lock) { pthread_mutex_init((pthread_mutex_t*)lock, NULL); }

void az_pal_os_lock_deinit(AZ_PAL_OS_LOCK* lock) { pthread_mutex_destroy((pthread_mutex_t*)lock); }

void az_pal_os_lock_acquire(AZ_PAL_OS_LOCK* lock) { pthread_mutex_lock((pthread_mutex_t*)lock); }

void az_pal_os_lock_release(AZ_PAL_OS_LOCK* lock) { pthread_mutex_unlock((pthread_mutex_t*)lock); }

void az_pal_os_sleep(uint32_t sleep_time_ms) {
#ifdef TI_RTOS
  Task_sleep(sleep_time_ms);
#else
  time_t seconds = sleep_time_ms / 1000;
  long remainder_nanoseconds = (sleep_time_ms % 1000) * 1000000;
  struct timespec time_to_sleep = { seconds, remainder_nanoseconds };
  (void)nanosleep(&time_to_sleep, NULL);
#endif
}
