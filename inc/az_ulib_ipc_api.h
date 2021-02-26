// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_IPC_H
#define AZ_ULIB_IPC_H

#include "az_ulib_base.h"
#include "az_ulib_capability_api.h"
#include "az_ulib_config.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_pal_os_api.h"
#include "az_ulib_port.h"
#include "az_ulib_result.h"

#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
#endif /* __cplusplus */

#include "azure/core/_az_cfg_prefix.h"

/**
 * @file    az_ulib_ipc_api.h
 *
 * @brief   The inter-process communication.
 *
 * The IPC is the component responsible to expose interfaces created by one component to all other
 * components in the system.
 */

typedef struct
{
  volatile const az_ulib_interface_descriptor* interface_descriptor;
  volatile long ref_count;
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
  volatile long running_count;
  volatile long running_count_low_watermark;
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
} _az_ulib_ipc_interface;

/**
 * @brief IPC handle.
 */
typedef struct az_ulib_ipc_tag
{
  struct
  {
    az_ulib_pal_os_lock lock;
    _az_ulib_ipc_interface interface_list[AZ_ULIB_CONFIG_MAX_IPC_INTERFACE];
  } _internal;
} az_ulib_ipc;

/**
 * @brief Interface handle.
 */
typedef void* az_ulib_ipc_interface_handle;

/**
 * @brief   Initialize the IPC system.
 *
 * This API initialize the IPC. It shall be called only once, at the beginning of the code
 * execution.
 *
 * @note    This API **is not** thread safe, the other IPC API shall only be called after the
 *          initialization process is completely done.
 *
 * @param[in]   ipc_handle      The #az_ulib_ipc* that points to a memory position where
 *                              the IPC shall create its control block.
 *
 * @pre     \p ipc_handle shall be different than `NULL`.
 * @pre     IPC shall not been initialized.
 *
 * @note    This API **is not** thread safe, no other IPC API may be called during the execution of
 *          this init.
 *
 * @return The #az_result with the result of the initialization.
 *  @retval #AZ_OK                              If the IPC initialize with success.
 */
AZ_NODISCARD az_result az_ulib_ipc_init(az_ulib_ipc* ipc_handle);

/**
 * @brief   De-initialize the IPC system.
 *
 * This API will release all resources associated with the IPC, but only if the IPC is completely
 * free, which means that deinit the IPC shall be preceded by:
 *
 * 1) Stop all threads that make calls to the published interfaces.
 * 2) Finalize or cancel all asynchronous calls, and ensure that their callbacks were called.
 * 3) Unsubscribe all telemetries.
 * 4) Unpublish all interfaces.
 *
 * If the system needs the IPC again, it may call az_ulib_ipc_init() again to reinitialize the IPC.
 *
 * @note    This API **is not** thread safe, no other IPC API may be called during the execution of
 *          this deinit, and no other IPC API shall be running during the execution of this API.
 *
 * @note    Deinit the IPC without follow these steps may result in error, segmentation fault or
 *          memory leak.
 *
 * @pre     IPC shall already been initialized.
 *
 * @return The #az_result with the result of the de-initialization.
 *  @retval #AZ_OK                              If the IPC de-initialize with success.
 *  @retval #AZ_ERROR_ULIB_BUSY                 If the IPC is not completely free.
 */
AZ_NODISCARD az_result az_ulib_ipc_deinit(void);

/**
 * @brief   Publish a new interface on the IPC.
 *
 * This API publishes a new interface in the IPC using the interface descriptor. The interface
 * descriptor shall be valid up to the point when the interface is unpublished with success.
 *
 * Optionally, this API may return the handle of the interface in the IPC. This handle will be
 * automatically released when the interface is unpublished.
 *
 * @note    **Try to release the handle returned by this API may result in
 *          #AZ_ERROR_ITEM_NOT_FOUND or a future segmentation fault.**
 *
 * @param[in]   interface_descriptor  The `const` #az_ulib_interface_descriptor* with the
 *                                    descriptor of the interface. It cannot be `NULL` and
 *                                    shall be valid up to the interface is unpublished with
 *                                    success.
 * @param[out]  interface_handle      A pointer to #az_ulib_ipc_interface_handle to return the
 *                                    handle of the published interface in the IPC. It may be
 *                                    `NULL`. If it is `NULL`, this API will not return the
 *                                    interface handle.
 *
 * @pre     IPC shall already been initialized.
 * @pre     \p interface_handle shall be different than `NULL`.
 *
 * @return The #az_result with the result of the interface publish.
 *  @retval #AZ_OK                              If the interface is published with success.
 *  @retval #AZ_ERROR_ULIB_ELEMENT_DUPLICATE    If the interface is already published.
 *  @retval #AZ_ERROR_NOT_ENOUGH_SPACE          If there is no more available space to store the
 *                                              new interface.
 */
AZ_NODISCARD az_result az_ulib_ipc_publish(
    const az_ulib_interface_descriptor* interface_descriptor,
    az_ulib_ipc_interface_handle* interface_handle);

#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
/**
 * @brief   Unpublish an interface from the IPC.
 *
 * @note    You may remove this API defining a global key `AZ_ULIB_CONFIG_REMOVE_UNPUBLISH` on your
 * compilation enviroment. See more at #AZ_ULIB_CONFIG_IPC_UNPUBLISH.
 *
 * @param[in]   interface_descriptor  The `const` #az_ulib_interface_descriptor * with the
 *                                    descriptor of the interface. It cannot be `NULL`.
 * @param[in]   wait_option_ms        The `uint32_t` with the maximum number of milliseconds
 *                                    the function may wait to unpublish the interface if it
 *                                    is busy:
 *                                        - #AZ_ULIB_NO_WAIT (0x00000000)
 *                                        - #AZ_ULIB_WAIT_FOREVER (0xFFFFFFFF)
 *                                        - timeout value (0x00000001 through 0xFFFFFFFE)
 *
 * @pre     IPC shall already been initialized.
 * @pre     \p interface_descriptor shall be different than `NULL`.
 *
 * @return The #az_result with the result of the interface unpublish.
 *  @retval #AZ_OK                              If the interface is unpublished with success.
 *  @retval #AZ_ERROR_ITEM_NOT_FOUND            If the provided descriptor didn't match any
 *                                              published interface.
 *  @retval #AZ_ERROR_ULIB_BUSY                 If the interface is busy and cannot be unpublished
 *                                              now.
 */
AZ_NODISCARD az_result az_ulib_ipc_unpublish(
    const az_ulib_interface_descriptor* interface_descriptor,
    uint32_t wait_option_ms);
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH

/**
 * @brief   Try get an interface handle by the name from the IPC.
 *
 * This API tries to find an interface that fits the provided name. If there is an interface that
 * fits this criteria, this API will return its handle. If no published procedure fits it, this API
 * will return #AZ_ERROR_ITEM_NOT_FOUND.
 *
 * Get a interface handle will increment the number of references to this interface, which means
 * that the IPC will know how many components is using this interface. When a component does not
 * need this interface anymore, it shall release it by calling az_ulib_ipc_release_interface().
 *
 * @note    **Do not release an interface will cause memory leak.**
 *
 * @param[in]   name              The `const char* const` with the interface name. It shall be a
 *                                valid `/0` terminated string.
 * @param[in]   version           The #az_ulib_version with the desired version.
 * @param[in]   match_criteria    The #az_ulib_version_match_criteria with the match criteria for
 *                                the interface version.
 * @param[out]  interface_handle  The #az_ulib_ipc_interface_handle* with the memory to store
 *                                the interface handle. It cannot be `NULL`.
 *
 * @pre     IPC shall already been initialized.
 * @pre     \p name shall be different than `NULL`.
 * @pre     \p interface_handle shall be different than `NULL`.
 *
 * @return The #az_result with the result of the get handle.
 *  @retval #AZ_OK                              If the interface was found and the returned
 *                                              handle can be used.
 *  @retval #AZ_ERROR_ITEM_NOT_FOUND            If the provided name didn't match any published
 *                                              interface.
 *  @retval #AZ_ERROR_NOT_ENOUGH_SPACE          If the interface already provided the maximum
 *                                              number of instances.
 */
AZ_NODISCARD az_result az_ulib_ipc_try_get_interface(
    const char* const name,
    az_ulib_version version,
    az_ulib_version_match_criteria match_criteria,
    az_ulib_ipc_interface_handle* interface_handle);

/**
 * @brief   Get an interface handle by an existent interface handle.
 *
 * This API will return an interface handle based on a provided interface handle. If the provided
 * interface handle was disabled, this API will return #AZ_ERROR_ULIB_DISABLED.
 *
 * Get a interface handle will increment the number of references to this interface, which means
 * that the IPC will know how many components is using this interface. When a component does not
 * need this interface anymore, it shall release it by calling az_ulib_ipc_release_interface().
 *
 * @note    **Do not release an interface will cause memory leak.**
 *
 * @param[in]   original_interface_handle The #az_ulib_ipc_interface_handle with the original
 *                                        interface handle. It cannot be `NULL`.
 * @param[out]  interface_handle          The #az_ulib_ipc_interface_handle* with the memory to
 *                                        store the interface handle. It cannot be `NULL`.
 *
 * @pre     IPC shall already been initialized.
 * @pre     \p original_interface_handle shall be different than `NULL`.
 * @pre     \p interface_handle shall be different than `NULL`.
 *
 * @return The #az_result with the result of the get handle.
 *  @retval #AZ_OK                              If the interface was found and the returned handle
 *                                              can be used.
 *  @retval #AZ_ERROR_ITEM_NOT_FOUND            If the provided handle didn't match any published
 *                                              interface.
 *  @retval #AZ_ERROR_NOT_ENOUGH_SPACE          If the interface already provided the maximum
 *                                              number of instances.
 */
AZ_NODISCARD az_result az_ulib_ipc_get_interface(
    az_ulib_ipc_interface_handle original_interface_handle,
    az_ulib_ipc_interface_handle* interface_handle);

/**
 * @brief   Release an interface handle from the IPC.
 *
 * This API releases an interface got by the az_ulib_ipc_try_get_interface() and
 * az_ulib_ipc_get_interface(). Release an interface is necessary to decrease the reference counter.
 * IPC will only free any memory related to the interface when all components that got this
 * interface releases it.
 *
 * The interface shall be released in two situations:
 *  * When it is not necessary anymore.
 *  * When any other IPC API returns #AZ_ERROR_ITEM_NOT_FOUND.
 *
 * @note    **Do not release an interface in one of this cases will leak memory.**
 *
 * @param[in]   interface_handle    The #az_ulib_ipc_interface_handle with the interface
 *                                  handle to release. It cannot be `NULL`.
 *
 * @pre     IPC shall already been initialized.
 * @pre     \p interface_handle shall be different than `NULL`.
 *
 * @return The #az_result with the result of the release.
 *  @retval #AZ_OK                              If the interface is released with success.
 *  @retval #AZ_ERROR_ULIB_PRECONDITION         If all required interfaces was already released.
 *                                              This is an unrecoverable error.
 */
AZ_NODISCARD az_result az_ulib_ipc_release_interface(az_ulib_ipc_interface_handle interface_handle);

/**
 * @brief   Synchronously Call a published procedure.
 *
 * @param[in]   interface_handle  The #az_ulib_ipc_interface_handle with the interface handle. It
 *                                cannot be `NULL`. Call
 *                                az_ulib_ipc_try_get_interface() to get the interface handle.
 * @param[in]   command_index      The #az_ulib_capability_index with the command handle.
 * @param[in]   model_in          The `const void *const` that points to the memory with the
 *                                input model content.
 * @param[out]  model_out         The `const void *` that points to the memory where the capability
 *                                should store the output model content.
 *
 * @pre     IPC shall already been initialized.
 * @pre     \p interface_handle shall be different than `NULL`.
 *
 * @return The #az_result with the result of the call.
 *  @retval #AZ_OK                              If the IPC get success calling the procedure.
 *  @retval #AZ_ERROR_ITEM_NOT_FOUND            If the target command was disabled.
 */
AZ_NODISCARD az_result az_ulib_ipc_call(
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_capability_index command_index,
    az_ulib_model_in model_in,
    az_ulib_model_out model_out);

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_IPC_H */
