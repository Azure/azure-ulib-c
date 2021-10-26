// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <errno.h>
#include <pthread.h>
#include <time.h>

#ifdef TI_RTOS
#include <ti/sysbios/knl/Task.h>
#else
#include <unistd.h>
#endif

#include "az_ulib_pal_os.h"
#include "az_ulib_pal_os_api.h"
#include "az_ulib_result.h"

void az_pal_os_lock_init(az_ulib_pal_os_lock* lock)
{
  pthread_mutex_init((pthread_mutex_t*)lock, NULL);
}

void az_pal_os_lock_deinit(az_ulib_pal_os_lock* lock)
{
  pthread_mutex_destroy((pthread_mutex_t*)lock);
}

void az_pal_os_lock_acquire(az_ulib_pal_os_lock* lock)
{
  pthread_mutex_lock((pthread_mutex_t*)lock);
}

void az_pal_os_lock_release(az_ulib_pal_os_lock* lock)
{
  pthread_mutex_unlock((pthread_mutex_t*)lock);
}

void az_pal_os_sleep(uint32_t sleep_time_ms)
{
#ifdef TI_RTOS
  Task_sleep(sleep_time_ms);
#else
  time_t seconds = sleep_time_ms / 1000;
  long remainder_nanoseconds = (sleep_time_ms % 1000) * 1000000;
  struct timespec time_to_sleep = { seconds, remainder_nanoseconds };
  (void)nanosleep(&time_to_sleep, NULL);
#endif
}

az_result az_pal_os_thread_create(
    az_ulib_pal_start_function_ptr function_ptr,
    az_ulib_pal_thread_args args,
    az_ulib_pal_thread_handle* handle)
{
  switch (pthread_create(handle, NULL, function_ptr, args))
  {
    case 0:
      return AZ_OK;
    case EAGAIN:
      return AZ_ERROR_OUT_OF_MEMORY;
    default:
      return AZ_ERROR_ULIB_SYSTEM;
  }
}

az_result az_pal_os_thread_join(az_ulib_pal_thread_handle handle, int* res)
{
  void* threadResult;
  if (pthread_join(handle, &threadResult) != 0)
  {
    return AZ_ERROR_ULIB_SYSTEM;
  }

  if (res != NULL)
  {
    *res = (int)(intptr_t)threadResult;
  }

  return AZ_OK;
}
