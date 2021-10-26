// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 *
 * @brief   The Registry Editor.
 *
 * The Registry Editor is capable of storing key value pairs into device flash.
 */

#ifndef AZ_ULIB_REGISTRY_API_H
#define AZ_ULIB_REGISTRY_API_H

#include "az_ulib_pal_api.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#else
#include <stddef.h>
#include <stdint.h>
#endif

#include "azure/core/_az_cfg_prefix.h"

/**
 * @brief   Registry control block.
 *
 *  Internal structure to control the registry.
 */
typedef struct
{
  /** Pointer for the memory to store the registry. */
  void* registry_start;

  /** Size of the buffer for the registry. */
  void* registry_end;

  /** Pointer for the memory to store the registry information. */
  void* registry_info_start;

  /** Size of the buffer for the registry info. */
  void* registry_info_end;

  /** Size of each page. */
  size_t page_size;

} az_ulib_registry_control_block;

/**
 * @brief   Registry information.
 *
 *  Structure to return registry information in the az_ulib_registry_get_info().
 */
typedef struct
{
  /** Total number of registry info that can be stored in the Registry. */
  size_t total_registry_info;

  /** Current number of registries stored in the Registry. */
  size_t in_use_registry_info;

  /** Number of free registry info positions. */
  size_t free_registry_info;

  /** Total memory reserved to store registry data. */
  size_t total_registry_data;

  /** Total memory currently used to store registry data. */
  size_t in_use_registry_data;

  /** Total free memory to store registry data. */
  size_t free_registry_data;
} az_ulib_registry_info;

/**
 * @brief   This function gets the #az_span value associated with the given #az_span key from the
 * registry.
 *
 * This function goes through the registry comparing keys until it finds one that matches the input.
 *
 * @param[in]   key                  The #az_span key to look for within the registry.
 * @param[out]   value                The #az_span value corresponding to the input key.
 *
 * @pre         \p key                            shall not be `#AZ_SPAN_EMPTY`.
 * @pre         \p value                          shall not be `NULL`.
 *
 * @return The #az_result with the result of the registry operations.
 *      @retval #AZ_OK                        If retrieving a value from the registry was
 *                                            successful.
 *      @retval #AZ_ERROR_ITEM_NOT_FOUND      If there are no values that correspond to the
 *                                            given key within the registry.
 */
AZ_NODISCARD az_result az_ulib_registry_try_get_value(az_span key, az_span* value);

/**
 * @brief   This function adds an #az_span key and an #az_span value into the device registry.
 *
 * This function goes through the registry ensuring there are no duplicate elements before
 * adding a new key value pair to the registry.
 *
 * @param[in]   key                  The #az_span key to add to the registry.
 * @param[in]   value                The #az_span value to add to the registry.
 *
 * @pre         \p key                            shall not be `#AZ_SPAN_EMPTY`.
 * @pre         \p value                          shall not be `#AZ_SPAN_EMPTY`.
 *
 * @return The #az_result with the result of the registry operations.
 *      @retval #AZ_OK                            If adding a value to the registry was successful.
 *      @retval #AZ_ERROR_ULIB_ELEMENT_DUPLICATE  If there is a key within the registry that is the
 *                                                same as the new key.
 *      @retval #AZ_ERROR_ULIB_SYSTEM             If the `az_ulib_registry_add` operation failed on
 *                                                the system level.
 *      @retval #AZ_ERROR_ULIB_BUSY               If the resources necesary for the
 *                                                `az_ulib_registry_add` operation are busy.
 *      @retval #AZ_ERROR_OUT_OF_MEMORY           If the flash space for `az_ulib_registry_add`
 *                                                is not enough for a new registry entry.
 */
AZ_NODISCARD az_result az_ulib_registry_add(az_span key, az_span value);

/**
 * @brief   This function removes an #az_span key and its corresponding #az_span value from the
 * device registry.
 *
 * This function goes through the registry and marks the node with the given key value pair as
 * deleted so that it can no longer be accessible.
 *
 * @param[in]   key                  The #az_span key to remove from the registry.
 *
 * @pre         \p key                            shall not be `#AZ_SPAN_EMPTY`.
 *
 * @return The #az_result with the result of the registry operations.
 *      @retval #AZ_OK                            If removing a value from the registry was
 *                                                successful.
 *      @retval #AZ_ERROR_ULIB_SYSTEM             If the `az_ulib_registry_delete` operation failed
 *                                                on the system level.
 *      @retval #AZ_ERROR_ULIB_BUSY               If the resources necessary for the
 *                                                `az_ulib_registry_delete` operation are busy.
 */
AZ_NODISCARD az_result az_ulib_registry_delete(az_span key);

/**
 * @brief   This function initializes the device registry.
 *
 * This function initializes components that the registry needs upon reboot. This function is not
 * thread safe and all other APIs shall only be invoked after the initialization ends.
 *
 * @param[in]   registry_cb         The pointer to #az_ulib_registry_control_block with the control
 *                                  block that contains the registry memory.
 */
void az_ulib_registry_init(const az_ulib_registry_control_block* registry_cb);

/**
 * @brief   This function deinitializes the device registry.
 *
 * This function deinitializes components that the registry used. The registry can be reinitialized.
 * This function is not thread safe and all other APIs shall release the resource before calling
 * the deinit() function.
 */
void az_ulib_registry_deinit(void);

/**
 * @brief   Erase all memory reserved for registry.
 *
 * This function delete all configurations stored in the registry.
 *
 * @note    **There is no rollback for this operation.**
 */
void az_ulib_registry_clean_all(void);

/**
 * @brief   Return the registry information.
 *
 * Return the current information about the registry memory utilization.
 */
void az_ulib_registry_get_info(az_ulib_registry_info* info);

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_REGISTRY_API_H */
