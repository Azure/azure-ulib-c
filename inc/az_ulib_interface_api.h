// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/**
 * @file    az_ulib_interface_api.h
 *
 * @brief   The data structures for the IPC interfaces.
 *
 */

#ifndef AZ_ULIB_INTERFACE_API_H
#define AZ_ULIB_INTERFACE_API_H

#include "az_ulib_base.h"
#include "az_ulib_capability_api.h"
#include "az_ulib_config.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_pal_api.h"
#include "az_ulib_result.h"

#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
#endif /* __cplusplus */

#include "azure/core/_az_cfg_prefix.h"

/**
 * @brief Set of interface flags.
 */
typedef enum
{
  /** None flag set. */
  AZ_ULIB_IPC_FLAGS_NONE = 0x00,

  /** Set that this package as the default for this interface. */
  AZ_ULIB_IPC_FLAGS_DEFAULT = 0x01,

  /** Put this interface on hold so it can be unpublished. */
  AZ_ULIB_IPC_FLAGS_ON_HOLD = 0x02
} _az_ulib_ipc_flags;

/**
 * @brief Internal IPC interface control block.
 */
typedef struct
{
  /** Pointer to the interface descriptor. */
  volatile const az_ulib_interface_descriptor* interface_descriptor;

  /** Number that uniquely identify this interface in the current power cycle of the device. */
  uint32_t hash;

  /** Set of interface flags. */
  volatile _az_ulib_ipc_flags flags;

  /** Track the number of references of this interface returned by the
   * az_ulib_ipc_try_get_interface(). */
  volatile long ref_count;

  /** Pointer to the interface base address (r9 in an ARM architecture with PIC). */
  volatile void* data_base_address;
} _az_ulib_ipc_interface;

/**
 * @brief Internal IPC control block.
 */
typedef struct
{
  struct
  {
    /** Lock to make IPC operations thread safe. */
    az_ulib_pal_os_lock lock;

    /** Reserved memory space to store the interfaces control block. */
    _az_ulib_ipc_interface interface_list[AZ_ULIB_CONFIG_MAX_IPC_INTERFACE];

    /** Counter to unique identify the interface in the device. It is incremented by one for each interface installation. */
    uint32_t publish_count;
  } _internal;
} az_ulib_ipc_control_block;

/**
 * @brief Interface handle.
 *
 * This is the interface handle returned in the az_ulib_ipc_try_get_interface().
 */
typedef struct
{
  struct
  {
    /** Pointer to the interface control block in the IPC. */
    _az_ulib_ipc_interface* ipc_interface;

    /** Hash that uniquely identifies the interface in the MCU.
     *
     * If the interface was unpublished and another interface got this control block,
     * the hash in the old references to this interface will fail to match the new hash,
     * so, the try_get will know that it shall renew those references.
     */
    uint32_t interface_hash;
  } _internal;
} az_ulib_ipc_interface_handle;

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_INTERFACE_API_H */
