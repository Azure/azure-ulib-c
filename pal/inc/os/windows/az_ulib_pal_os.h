// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_PAL_OS_WINDOWS_H
#define AZ_ULIB_PAL_OS_WINDOWS_H

#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  @brief  Pointer to a platform specific lock handle.
   */
  typedef SRWLOCK az_ulib_pal_os_lock;

  /*
   *  @brief  Pointer to a platform specific thread handle.
   */
#define az_ulib_pal_thread_handle HANDLE

  /*
   *  @brief  Platform specific thread arguments type.
   */
#define az_ulib_pal_thread_args LPVOID

  /*
   *  @brief  Platform specific thread return type.
   */
#define az_ulib_pal_thread_ret DWORD WINAPI

  /*
   *  @brief  Platform specific thread function signature.
   *
   * @param[in]     args        The #az_ulib_pal_thread_args with the arguments to the thread.
   *
   * @return  The #az_ulib_pal_thread_ret with the result of the thread.
   */
#define az_ulib_pal_start_function_ptr LPTHREAD_START_ROUTINE

#ifdef __cplusplus
}
#endif

#endif /* AZ_ULIB_PAL_OS_WINDOWS_H */
