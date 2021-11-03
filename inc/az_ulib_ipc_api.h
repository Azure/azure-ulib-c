// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/**
 * @file    az_ulib_ipc_api.h
 *
 * @brief   The inter-process communication.
 *
 * The IPC is the component responsible to expose interfaces created by one component to all other
 * components in the system.
 */

#ifndef AZ_ULIB_IPC_API_H
#define AZ_ULIB_IPC_API_H

#include "az_ulib_base.h"
#include "az_ulib_capability_api.h"
#include "az_ulib_config.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_interface_api.h"
#include "az_ulib_ipc_function_table.h"
#include "az_ulib_pal_api.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"

#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
#endif /* __cplusplus */

#include "azure/core/_az_cfg_prefix.h"

#define IPC_1_PACKAGE_NAME "ipc"
#define IPC_1_PACKAGE_VERSION 1

/**
 * @brief   Initialize the IPC system.
 *
 * This API initializes the IPC. It shall be called only once, at the beginning of the code
 * execution.
 *
 * @note    This API **is not** thread safe. The other IPC APIs shall only be called after the
 *          initialization process is complete.
 *
 * @param[in]   ipc_control_block   The #az_ulib_ipc_control_block* that points to a memory
 *                                  position where the IPC shall create its control block.
 *
 * @pre     \p ipc_control_block shall not be `NULL`.
 * @pre     IPC shall not been initialized.
 *
 * @return The #az_result with the result of the initialization.
 *  @retval #AZ_OK                              If the IPC initializes with success.
 */
AZ_NODISCARD az_result az_ulib_ipc_init(az_ulib_ipc_control_block* ipc_control_block);

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
 *          this deinit and no other IPC API shall be running during the execution of this API.
 *
 * @note    Deinit the IPC without follow these steps may result in error, segmentation fault or
 *          memory leak.
 *
 * @pre     IPC shall already be initialized.
 *
 * @return The #az_result with the result of the de-initialization.
 *  @retval #AZ_OK                              If the IPC de-initialize with success.
 *  @retval #AZ_ERROR_ULIB_BUSY                 If the IPC is not completely free.
 */
AZ_NODISCARD az_result az_ulib_ipc_deinit(void);

/**
 * @brief   Return the table of the ipc interface.
 *
 * @pre     IPC shall already be initialized.
 *
 * @return The #az_ulib_ipc_function_table with the pointer to the IPC table.
 */
const az_ulib_ipc_function_table* az_ulib_ipc_get_function_table(void);

/**
 * @brief   Publish a new interface on the IPC.
 *
 * This API publishes a new interface in the IPC using the interface descriptor. The interface
 * descriptor shall be valid up to the point when the interface is unpublished with success.
 *
 * If the interface to publish already exist in the device, the new interface cannot belong to the
 * same package name and version.
 *
 * If no other package has published an interface with the same name and version, and the same
 * package name provided in the descriptor, this function will make this interface the default one.
 *
 * Optionally, this API may return the handle of the interface in the IPC. This handle will be
 * automatically released when the interface is unpublished.
 *
 * @note    **Try to release the handle returned by this API may result in
 *          #AZ_ERROR_ITEM_NOT_FOUND or a future segmentation fault.**
 *
 * @param[in]   interface_descriptor  The `const` #az_ulib_interface_descriptor* with the
 *                                    descriptor of the interface. It cannot be `NULL` and
 *                                    shall be valid up to the point the interface is successfully
 *                                    unpublished.
 *
 * @pre     IPC shall already be initialized.
 *
 * @return The #az_result with the result of the interface publish.
 *  @retval #AZ_OK                              If the interface is published with success.
 *  @retval #AZ_ERROR_ARG                       If the interface of package version is ANY [0].
 *  @retval #AZ_ERROR_ULIB_ELEMENT_DUPLICATE    If the interface is already published.
 *  @retval #AZ_ERROR_NOT_ENOUGH_SPACE          If there is no more available space to store the
 *                                              new interface.
 */
AZ_NODISCARD az_result
az_ulib_ipc_publish(const az_ulib_interface_descriptor* const interface_descriptor);

/**
 * @brief   Set a default package for a given interface in the device.
 *
 * This API sets an interface implementation as default in the device, so if 2 or more packages
 * implement the same interface, IPC will be able to define each one shall be returned, if the
 * caller didn't specify the desired package.
 *
 * If another package was already defined as default for the given interface, calling this API will
 * change the default from the previous package to the provided one.
 *
 * @param[in]   package_name      The `az_span` with the package name.
 * @param[in]   package_version   The #az_ulib_version with the package version.
 * @param[in]   interface_name    The `az_span` with the interface name.
 * @param[in]   interface_version The #az_ulib_version with the interface version.
 *
 * @pre     IPC shall already be initialized.
 * @pre     \p package_name shall not be #AZ_SPAN_EMPTY.
 * @pre     \p interface_name shall not be #AZ_SPAN_EMPTY.
 *
 * @return The #az_result with the result of the interface publish.
 *  @retval #AZ_OK                              If the interface has been set as default with
 *                                              success.
 *  @retval #AZ_ERROR_ITEM_NOT_FOUND            If the provided name didn't match any published
 *                                              interface.
 */
AZ_NODISCARD az_result az_ulib_ipc_set_default(
    az_span package_name,
    az_ulib_version package_version,
    az_span interface_name,
    az_ulib_version interface_version);

/**
 * @brief   Unpublish an interface from the IPC.
 *
 * @param[in]   interface_descriptor  The `const` #az_ulib_interface_descriptor * with the
 *                                    descriptor of the interface.
 * @param[in]   wait_option_ms        The `uint32_t` with the maximum number of milliseconds
 *                                    the function may wait to unpublish the interface if it
 *                                    is busy:
 *                                        - #AZ_ULIB_NO_WAIT (0x00000000)
 *                                        - #AZ_ULIB_WAIT_FOREVER (0xFFFFFFFF)
 *                                        - timeout value (0x00000001 through 0xFFFFFFFE)
 *
 * @pre     IPC shall already be initialized.
 * @pre     \p interface_descriptor shall not be `NULL`.
 *
 * @return The #az_result with the result of the interface unpublish.
 *  @retval #AZ_OK                              If the interface is unpublished with success.
 *  @retval #AZ_ERROR_ITEM_NOT_FOUND            If the provided descriptor didn't match any
 *                                              published interface.
 *  @retval #AZ_ERROR_ULIB_BUSY                 If the interface is busy and cannot be unpublished
 *                                              now.
 */
AZ_NODISCARD az_result az_ulib_ipc_unpublish(
    const az_ulib_interface_descriptor* const interface_descriptor,
    uint32_t wait_option_ms);

/**
 * @brief   Try get an interface handle by the name from the IPC.
 *
 * This API tries to find an interface that fits the provided name. If there is an interface that
 * fits this criteria, this API will return its handle.
 *
 * Get a interface handle will increment the number of references to this interface, which means
 * that the IPC will know how many components is using this interface. When a component does not
 * need this interface anymore, it shall release the handle by calling
 * az_ulib_ipc_release_interface().
 *
 * @note    **Not release an interface will cause memory leak and will block the interface to be
 *          unpublished and consequently updated.**
 *
 * An interface is identified by 3 characteristics.
 *  1. device:      Identifies **where** the interface is on a complex device that is composed by
 *                  multiple CPUs.
 *  2. package:     Identifies the **implementation** of the capabilities.
 *  3. interface:   Identifies the **signature** of the capabilities.
 *
 * This function returns the handle of the interface that best fits the requirements following the
 * follow rules. If the criteria doesn't match, this function will return #AZ_ERROR_ITEM_NOT_FOUND.
 *
 *  - interface_name is mandatory and shall match exactly.
 *  - interface_version is mandatory and shall match exactly.
 *  - package_name is mandatory and shall match exactly.
 *  - package_version
 *      - If package version is provided, this function will look up the interface only in the
 *          package_name.package_version.
 *      - If package version is #AZ_ULIB_VERSION_DEFAULT (0), this function will look up the
 *          interface in the default package that matches the package_name.
 * - device_name
 *      - If provided, this function will send the request for the Gateways that has the
 *          communication with the leaf devices. The Gateway will look up the interface
 *          in the appropriate device, if it exist. **This functionality is not supported yet.**
 *      - If #AZ_SPAN_EMPTY, this function will look up the interface only in the local
 *          device.
 *
 * The returned handle can be used as many times as the caller needs. After released, the handle
 * shall not be used anymore unless you call this API again. This API will try to reuse the released
 * handle by checking the hash. This check is faster than looking up the interface by name.
 *
 * Because the interface handle is a input/output parameter, it shall be initialized with `0`.
 * ```c
 * static az_ulib_ipc_interface_handle interface_handle = { 0 };
 * ```
 *
 * @param[in]     device_name       The `az_span` with the device name. It can be #AZ_SPAN_EMPTY.
 * @param[in]     package_name      The `az_span` with the package name.
 * @param[in]     package_version   The #az_ulib_version with the package version.
 * @param[in]     interface_name    The `az_span` with the interface name.
 * @param[in]     interface_version The #az_ulib_version with the interface version.
 * @param[in,out] interface_handle  The #az_ulib_ipc_interface_handle* with the memory to store
 *                                  the interface handle.
 *
 * @pre     IPC shall already be initialized.
 * @pre     \p package_name shall not be #AZ_SPAN_EMPTY.
 * @pre     \p interface_name shall not be #AZ_SPAN_EMPTY.
 * @pre     \p interface_version shall not be #AZ_ULIB_VERSION_DEFAULT (0).
 * @pre     \p interface_handle shall not be `NULL`.
 *
 * @return The #az_result with the result of the get handle.
 *  @retval #AZ_OK                              If the handle is valid and the returned handle
 *                                              can be used.
 *  @retval #AZ_ULIB_RENEW                      If the interface was found and the returned
 *                                              handle can be used. Same as AZ_OK, but with a new
 *                                              handle.
 *  @retval #AZ_ERROR_ITEM_NOT_FOUND            If the provided name didn't match any published
 *                                              interface.
 *  @retval #AZ_ERROR_NOT_ENOUGH_SPACE          If the interface already provided the maximum
 *                                              number of instances.
 *  @retval #AZ_ERROR_NOT_IMPLEMENTED           If the device name is not #AZ_SPAN_EMPTY. Complex
 *                                              device is not supported yet.
 */
AZ_NODISCARD az_result az_ulib_ipc_try_get_interface(
    az_span device_name,
    az_span package_name,
    az_ulib_version package_version,
    az_span interface_name,
    az_ulib_version interface_version,
    az_ulib_ipc_interface_handle* interface_handle);

/**
 * @brief   Try get a capability index in the interface by the name from the IPC.
 *
 * This API tries to find an capability that fits the provided name in the interface. It returns
 * the capability index.
 *
 * @param[in]   interface_handle    The #az_ulib_ipc_interface_handle with the interface handle.
 *                                  It cannot be `NULL`. Call az_ulib_ipc_try_get_interface() to
 *                                  get the interface handle.
 * @param[in]   name                The `az_span` with the interface name.
 * @param[out]  capability_index    The #az_ulib_capability_index* with the memory to store
 *                                  the capability index. It cannot be `NULL`.
 *
 * @pre     IPC shall already be initialized.
 * @pre     \p interface_handle shall not be `NULL`.
 * @pre     \p name shall not be `NULL`.
 * @pre     \p capability_index shall not be `NULL`.
 *
 * @return The #az_result with the result of the get handle.
 *  @retval #AZ_OK                              If the capability was found and the returned
 *                                              index can be used.
 *  @retval #AZ_ERROR_ITEM_NOT_FOUND            If the provided name didn't match any capability
 *                                              in the interface.
 */
AZ_NODISCARD az_result az_ulib_ipc_try_get_capability(
    az_ulib_ipc_interface_handle interface_handle,
    az_span name,
    az_ulib_capability_index* capability_index);

/**
 * @brief   Release an interface handle from the IPC.
 *
 * This API releases an interface got by the az_ulib_ipc_try_get_interface(). Release an interface
 * is necessary to decrease the reference counter. IPC will only free any memory related to the
 * interface when all components that got this interface releases it.
 *
 * @note    **Not release an interface will cause memory leak and will block the interface to be
 *          unpublished and consequently updated.**
 *
 * @note    **This API cannot be called in parallel with az_ulib_ipc_cal() or
 *          az_ulib_ipc_try_get_capability(). Call it in parallel may result in the CPU to
 *          run unknown code.**
 *
 * @param[in]   interface_handle    The #az_ulib_ipc_interface_handle with the interface
 *                                  handle to release.
 *
 * @pre     IPC shall already be initialized.
 * @pre     \p interface_handle shall not be `NULL`.
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
 * @param[in]   interface_handle  The #az_ulib_ipc_interface_handle with the interface handle. Call
 *                                az_ulib_ipc_try_get_interface() to get the interface handle.
 * @param[in]   capability_index  The #az_ulib_capability_index with the capability handle.
 * @param[in]   model_in          The `az_ulib_model_in` that points to the memory with the
 *                                input model content.
 * @param[out]  model_out         The `az_ulib_model_out` that points to the memory where the
 *                                capability should store the output model content.
 *
 * @pre     IPC shall already be initialized.
 * @pre     \p interface_handle shall not be `NULL`.
 *
 * @return The #az_result with the result forwarded from the capability call.
 */
AZ_NODISCARD az_result az_ulib_ipc_call(
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_capability_index capability_index,
    az_ulib_model_in model_in,
    az_ulib_model_out model_out);

/**
 * @brief   Synchronously Call a published procedure using string models.
 *
 * @param[in]   interface_handle    The #az_ulib_ipc_interface_handle with the interface handle.
 *                                  It cannot be `NULL`. Call az_ulib_ipc_try_get_interface() to
 *                                  get the interface handle.
 * @param[in]   capability_index    The #az_ulib_capability_index with the capability index. Call
 *                                  az_ulib_ipc_try_get_capability() to get the capability index.
 * @param[in]   model_in_span       The #az_span with the model in.
 * @param[out]  model_out_span      The pointer to #az_span where the capability should store the
 *                                  output content.
 *
 * @pre     IPC shall already be initialized.
 * @pre     \p interface_handle shall not be `NULL`.
 *
 * @return The #az_result with the result of the call.
 *  @retval #AZ_OK                              If the IPC get success calling the procedure.
 *  @retval #AZ_ERROR_ITEM_NOT_FOUND            If the target capability does not exist.
 *  @retval #AZ_ERROR_NOT_SUPPORTED             If the target capability does not support call with
 *                                              string.
 *  @retval Others                              Defined by the target function.
 */
AZ_NODISCARD az_result az_ulib_ipc_call_with_str(
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_capability_index capability_index,
    az_span model_in_span,
    az_span* model_out_span);

/**
 * @brief   Split the provided method full name.
 *
 * This function splits the provided method name between device, package, interface and capability
 * using the following positional rules
 *
 * 1. The expected method full name shall look like:
 *      <device_name>@<package_name>.<package_version>.<interface_name>.<interface_version>:<capability_name>
 * 2. The package_name, interface_name and interface_version are mandatory. They shall always
 *      match the published name and version.
 * 3. The names are case sensitive.
 * 4. If device_name or capability_name is not provided, this function shall return #AZ_SPAN_EMPTY.
 * 5. The package_version can be a number or a wildcard character `*`. This function shall return
 *      `package_version` = #AZ_ULIB_VERSION_DEFAULT for a wildcard character.
 *
 * @param[in]   full_name           The `az_span` with the method full name.
 * @param[out]  device_name         The pointer to `az_span` to return the device name.
 * @param[out]  package_name        The pointer to `az_span` to return the package name.
 * @param[out]  package_version     The pointer to `uint32_t` to return the package version.
 * @param[out]  interface_name      The pointer to `az_span` to return the interface name.
 * @param[out]  interface_version   The pointer to `uint32_t` to return the interface version.
 * @param[out]  capability_name     The pointer to `az_span` to return the capability name.
 *
 * @pre     IPC shall already be initialized.
 * @pre     \p full_name shall not be #AZ_SPAN_EMPTY.
 * @pre     \p device_name shall not be `NULL`.
 * @pre     \p package_name shall not be `NULL`.
 * @pre     \p package_version shall not be `NULL`.
 * @pre     \p interface_name shall not be `NULL`.
 * @pre     \p interface_version shall not be `NULL`.
 * @pre     \p capability_name shall not be `NULL`.
 *
 * @return The #az_result with the result of the call.
 *  @retval #AZ_OK                      If the method full name was split with success and can be
 *                                      used.
 *  @retval #AZ_ERROR_UNEXPECTED_CHAR   If the method full name didn't fit the above rules.
 */
AZ_NODISCARD az_result az_ulib_ipc_split_method_name(
    az_span full_name,
    az_span* device_name,
    az_span* package_name,
    uint32_t* package_version,
    az_span* interface_name,
    uint32_t* interface_version,
    az_span* capability_name);

/**
 * @brief   Query IPC information.
 *
 * Creates a query for IPC. The query retrieves information from the IPC, depending on the content
 * of the `query` argument. There are 2 valid query strings:
 *
 *  1) Empty string: query will return a list of all published interfaces.
 *  2) Interface name: query will return a list of all capabilities in the interface.
 *
 * The result of the query will be a list with the information separated by comma.
 *
 * @param[in]   query               The `az_span` with the query string.
 * @param[in]   result              The `az_span` with the buffer to return the query result.
 * @param[in]   continuation_token  The pointer to `uint32_t` to return the query continuation
 *                                  token. If the continuation token is 0, means that there is
 *                                  no more information to return in this query, if it is
 *                                  different than 0, means that a call to
 *                                  az_ulib_ipc_query_next() shall return more information.
 *
 * @pre     IPC shall already be initialized.
 * @pre     \p result shall be a valid az_span with at least 1 position.
 * @pre     \p continuation_token shall not be `NULL`.
 *
 * @return The #az_result with the result of the call.
 *  @retval #AZ_OK                      If the query call succeeded and the result and continuation
 *                                      have valid information.
 *  @retval #AZ_ULIB_EOF                If there is no more information to return in this query.
 *  @retval #AZ_ERROR_NOT_SUPPORTED     If the query is not supported.
 */
AZ_NODISCARD az_result
az_ulib_ipc_query(az_span query, az_span* result, uint32_t* continuation_token);

/**
 * @brief   Query next IPC information.
 *
 * Retrieves the next information from the IPC using the continuation token.
 *
 * @param[in]   continuation_token  The pointer to `uint32_t` with the current continuation
 *                                  token and where it will return the next continuation token.
 * @param[in]   result              The `az_span` with the buffer to return the query result.
 *
 * @pre     IPC shall already be initialized.
 * @pre     \p result shall be a valid az_span with at least 1 position.
 * @pre     \p continuation_token shall not be `NULL`.
 *
 * @return The #az_result with the result of the call.
 *  @retval #AZ_OK                      If the query next call succeeded and the result and
 *                                      continuation have valid information.
 *  @retval #AZ_ULIB_EOF                If there is no more information to return in this query.
 *  @retval #AZ_ERROR_NOT_SUPPORTED     If the continuation token is not supported.
 */
AZ_NODISCARD az_result az_ulib_ipc_query_next(uint32_t* continuation_token, az_span* result);

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_IPC_API_H */
