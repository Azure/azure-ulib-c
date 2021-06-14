// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <windows.h>

#include "az_ulib_pal_os.h"
#include "az_ulib_pal_os_api.h"
#include "az_ulib_result.h"

void az_pal_os_lock_init(az_ulib_pal_os_lock* lock) { InitializeSRWLock((SRWLOCK*)lock); }

void az_pal_os_lock_deinit(az_ulib_pal_os_lock* lock) { (void)lock; }

void az_pal_os_lock_acquire(az_ulib_pal_os_lock* lock) { AcquireSRWLockExclusive((SRWLOCK*)lock); }

void az_pal_os_lock_release(az_ulib_pal_os_lock* lock) { ReleaseSRWLockExclusive((SRWLOCK*)lock); }

void az_pal_os_sleep(uint32_t sleep_time_ms) { Sleep(sleep_time_ms); }

az_result az_pal_os_thread_create(
    az_ulib_pal_start_function_ptr function_ptr,
    az_ulib_pal_thread_args args,
    az_ulib_pal_thread_handle* handle)
{
  *handle = CreateThread(NULL, 0, function_ptr, args, 0, NULL);
  if (*handle == NULL)
  {
    return (GetLastError() == ERROR_OUTOFMEMORY) ? AZ_ERROR_OUT_OF_MEMORY : AZ_ERROR_ULIB_SYSTEM;
  }
  return AZ_OK;
}

az_result az_pal_os_thread_join(az_ulib_pal_thread_handle handle, int* res)
{
  az_result result;
  DWORD returnCode = WaitForSingleObject(handle, INFINITE);

  if (returnCode != WAIT_OBJECT_0)
  {
    result = AZ_ERROR_ULIB_SYSTEM;
  }
  else
  {
    result = AZ_OK;
    if (res != NULL)
    {
      DWORD exit_code;
      if (!GetExitCodeThread(
              handle,
              &exit_code)) // If thread end is signaled we need to get the Thread Exit Code;
      {
        result = AZ_ERROR_ULIB_SYSTEM;
      }
      else
      {
        *res = (int)exit_code;
      }
    }
  }
  CloseHandle(handle);
  return result;
}
