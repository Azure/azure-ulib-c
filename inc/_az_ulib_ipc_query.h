// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef _az_ULIB_IPC_QUERY_H
#define _az_ULIB_IPC_QUERY_H

#include "az_ulib_result.h"

#include "azure/core/_az_cfg_prefix.h"

/*
 * Publish IPC query interface.
 */
az_result _az_ulib_ipc_query_interface_publish(void);

/*
 * Unpublish IPC query interface.
 */
az_result _az_ulib_ipc_query_interface_unpublish(void);

#include "azure/core/_az_cfg_suffix.h"

#endif /* _az_ULIB_IPC_QUERY_H */
