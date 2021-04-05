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
   *  @struct az_ulib_pal_os_lock
   *
   *  @brief  pointer to a platform specific struct for a lock implementation
   */
  typedef TX_MUTEX az_ulib_pal_os_lock;

#ifdef __cplusplus
}
#endif

#endif /* AZ_ULIB_PAL_OS_THREADX_H */
