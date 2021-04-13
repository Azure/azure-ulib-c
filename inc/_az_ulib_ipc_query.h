// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef _az_ULIB_IPC_QUERY_INTERFACE_H
#define _az_ULIB_IPC_QUERY_INTERFACE_H

#include "az_ulib_result.h"

#ifdef __cplusplus
extern "C"
{
#else
#endif

  /*
   * Publish IPC query interface.
   */
  az_result _az_ulib_ipc_query_interface_publish(void);

  /*
   * Unpublish IPC query interface.
   */
  az_result _az_ulib_ipc_query_interface_unpublish(void);

#ifdef __cplusplus
}
#endif

#endif /* _az_ULIB_IPC_QUERY_INTERFACE_H */
