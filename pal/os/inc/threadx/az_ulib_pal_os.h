// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_PAL_OS_THREADX_H
#define AZ_ULIB_PAL_OS_THREADX_H

#include <tx_api.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  @brief  Pointer to a platform specific lock handle.
   */
  typedef TX_MUTEX az_ulib_pal_os_lock;

  /*
   *  @brief  Pointer to a platform specific thread handle.
   */
  typedef TX_THREAD az_ulib_pal_thread_handle;

  /*
   *  @brief  Platform specific thread arguments type.
   */
  typedef ULONG az_ulib_pal_thread_args;

  /*
   *  @brief  Platform specific thread return type.
   */
  typedef VOID az_ulib_pal_thread_ret;

  /*
   *  @brief  Platform specific thread function signature.
   *
   * @param[in]     args        The #az_ulib_pal_thread_args with the arguments to the thread.
   *
   * @return  The #az_ulib_pal_thread_ret with the result of the thread.
   */
  typedef az_ulib_pal_thread_ret (*az_ulib_pal_start_function_ptr)(az_ulib_pal_thread_args args);

#ifdef __cplusplus
}
#endif

#endif /* AZ_ULIB_PAL_OS_THREADX_H */
