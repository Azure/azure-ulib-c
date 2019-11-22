// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_IPC_H
#define AZ_ULIB_IPC_H

#include "az_ulib_action_api.h"
#include "az_ulib_base.h"
#include "az_ulib_config.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_pal_os_api.h"
#include "az_ulib_port.h"
#include "az_ulib_result.h"
#include "internal/az_ulib_ipc.h"

#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
extern "C" {
#endif /* __cplusplus */

/**
 * @file    az_ulib_ipc_api.h
 *
 * @brief   The inter-process communication.
 *
 * The IPC is the component responsible to expose interfaces created by one component to all other
 * components in the system.
 */

/**
 * @brief IPC handle.
 */
typedef struct az_ulib_ipc_tag {
  _az_ulib_ipc az_private;
} az_ulib_ipc;

/**
 * @brief Interface handle.
 */
typedef _az_ulib_ipc_interface_handle az_ulib_ipc_interface_handle;

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
 *                              the IPC shall create its control block. It cannot be `NULL`.
 *
 * @return The #az_ulib_result with the result of the initialization.
 *  @retval #AZ_ULIB_SUCCESS                    If the IPC initialize with success.
 *  @retval #AZ_ULIB_ILLEGAL_ARGUMENT_ERROR     If one of the arguments is invalid.
 *  @retval #AZ_ULIB_ALREADY_INITIALIZED_ERROR  If the IPC is already initialized.
 */
static inline az_ulib_result az_ulib_ipc_init(az_ulib_ipc* ipc_handle) {
#ifdef AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT
  return _az_ulib_ipc_init((_az_ulib_ipc*)ipc_handle);
#else
  return _az_ulib_ipc_init_no_contract((_az_ulib_ipc*)ipc_handle);
#endif /* AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT */
}

/**
 * @brief   De-initialize the IPC system.
 *
 * This API will release all resources associated with the IPC, but only if the IPC is completely
 * free, whitch means that deinit the IPC shall be preceded by:
 *
 * 1) Stop all threads that make calls to the published interfaces.
 * 2) Finalize or cancell all asynchronous calls, and ensure that their callbacks were called.
 * 3) Unsubscribe all events.
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
 * @return The #az_ulib_result with the result of the de-initialization.
 *  @retval #AZ_ULIB_SUCCESS                    If the IPC de-initialize with success.
 *  @retval #AZ_ULIB_PRECONDITION_ERROR         If the IPC was not initialized.
 *  @retval #AZ_ULIB_BUSY_ERROR                 If the IPC is not completely free.
 */
static inline az_ulib_result az_ulib_ipc_deinit(void) {
#ifdef AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT
  return _az_ulib_ipc_deinit();
#else
  return _az_ulib_ipc_deinit_no_contract();
#endif /* AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT */
}

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
 *          #AZ_ULIB_NO_SUCH_ELEMENT_ERROR or a future segmentation fault.**
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
 * @return The #az_ulib_result with the result of the interface publish.
 *  @retval #AZ_ULIB_SUCCESS                  If the interface is published with success.
 *  @retval #AZ_ULIB_ILLEGAL_ARGUMENT_ERROR   If one of the arguments is invalid.
 *  @retval #AZ_ULIB_OUT_OF_MEMORY_ERROR      If there is no more available space to store the new
 *                                            interface.
 */
static inline az_ulib_result az_ulib_ipc_publish(
    const az_ulib_interface_descriptor* interface_descriptor,
    az_ulib_ipc_interface_handle* interface_handle) {
#ifdef AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT
  return _az_ulib_ipc_publish(
      interface_descriptor, (_az_ulib_ipc_interface_handle*)interface_handle);
#else
  return _az_ulib_ipc_publish_no_contract(
      interface_descriptor, (_az_ulib_ipc_interface_handle*)interface_handle);
#endif /* AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT */
}

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
 * @return The #az_ulib_result with the result of the interface unpublish.
 *  @retval #AZ_ULIB_SUCCESS                  If the interface is unpublished with success.
 *  @retval #AZ_ULIB_ILLEGAL_ARGUMENT_ERROR   If one of the arguments is invalid.
 *  @retval #AZ_ULIB_NO_SUCH_ELEMENT_ERROR    If the provided descriptor didn't match any published
 *                                            interface.
 *  @retval #AZ_ULIB_BUSY_ERROR               If the interface is busy and cannot be unpublished
 *                                            now.
 */
static inline az_ulib_result az_ulib_ipc_unpublish(
    const az_ulib_interface_descriptor* interface_descriptor,
    uint32_t wait_option_ms) {
#ifdef AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT
  return _az_ulib_ipc_unpublish(interface_descriptor, wait_option_ms);
#else
  return _az_ulib_ipc_unpublish_no_contract(interface_descriptor, wait_option_ms);
#endif /* AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT */
}
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH

/**
 * @brief   Try get an interface handle by the name from the IPC.
 *
 * This API tries to find an interface that fits the provided name. If there is an interface that
 * fits this criteria, this API will return its handle. If no published procedure fits it, this API
 * will return #AZ_ULIB_NO_SUCH_ELEMENT_ERROR.
 *
 * Get an interface handle will increment the number of references to this interface, which means
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
 * @return The #az_ulib_result with the result of the get handle.
 *  @retval #AZ_ULIB_SUCCESS                      If the interface was found and the returned
 *                                                handle can be used.
 *  @retval #AZ_ULIB_ILLEGAL_ARGUMENT_ERROR       If one of the arguments is invalid.
 *  @retval #AZ_ULIB_NO_SUCH_ELEMENT_ERROR        If the provided name didn't match any published
 *                                                interface.
 *  @retval #AZ_ULIB_DISABLED_ERROR               If the required interface was disabled and cannot
 *                                                be used anymore.
 *  @retval #AZ_ULIB_INCOMPATIBLE_VERSION_ERROR   If the required version is not available.
 */
static inline az_ulib_result az_ulib_ipc_try_get_interface(
    const char* const name,
    az_ulib_version version,
    az_ulib_version_match_criteria match_criteria,
    az_ulib_ipc_interface_handle* interface_handle) {
#ifdef AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT
  return _az_ulib_ipc_try_get_interface(
      name, version, match_criteria, (_az_ulib_ipc_interface_handle*)interface_handle);
#else
  return _az_ulib_ipc_try_get_interface_no_contract(
      name, version, match_criteria, (_az_ulib_ipc_interface_handle*)interface_handle);
#endif /* AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT */
}

/**
 * @brief   Get an interface handle by an existent interface handle.
 *
 * This API will return an interface handle based on a provided interface handle. If the provided
 * interface handle was disabled, this API will return #AZ_ULIB_DISABLED_ERROR.
 *
 * Get an interface handle will increment the number of references to this interface, which means
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
 * @return The #az_ulib_result with the result of the get handle.
 *  @retval #AZ_ULIB_SUCCESS                  If the interface was found and the returned handle
 *                                            can be used.
 *  @retval #AZ_ULIB_ILLEGAL_ARGUMENT_ERROR   If one of the arguments is invalid.
 *  @retval #AZ_ULIB_DISABLED_ERROR           If the required interface was disabled and cannot be
 *                                            used anymore.
 */
static inline az_ulib_result az_ulib_ipc_get_interface(
    az_ulib_ipc_interface_handle original_interface_handle,
    az_ulib_ipc_interface_handle* interface_handle) {
#ifdef AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT
  return _az_ulib_ipc_get_interface(
      (_az_ulib_ipc_interface_handle)original_interface_handle,
      (_az_ulib_ipc_interface_handle*)interface_handle);
#else
  return _az_ulib_ipc_get_interface_no_contract(
      (_az_ulib_ipc_interface_handle)original_interface_handle,
      (_az_ulib_ipc_interface_handle*)interface_handle);
#endif /* AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT */
}

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
 *  * When any other IPC API returns #AZ_ULIB_NO_SUCH_ELEMENT_ERROR.
 *
 * @note    **Do not release an interface in one of this cases will leak memory.**
 *
 * @param[in]   interface_handle    The #az_ulib_ipc_interface_handle with the interface
 *                                  handle to release. It cannot be `NULL`.
 *
 * @return The #az_ulib_result with the result of the release.
 *  @retval #AZ_ULIB_SUCCESS                  If the interface is released with success.
 *  @retval #AZ_ULIB_ILLEGAL_ARGUMENT_ERROR   If one of the arguments is invalid.
 *  @retval #AZ_ULIB_NO_SUCH_ELEMENT_ERROR    If the provided descriptor didn't match any published
 *                                            interface.
 */
static inline az_ulib_result az_ulib_ipc_release_interface(
    az_ulib_ipc_interface_handle interface_handle) {
#ifdef AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT
  return _az_ulib_ipc_release_interface((_az_ulib_ipc_interface_handle)interface_handle);
#else
  return _az_ulib_ipc_release_interface_no_contract(
      (_az_ulib_ipc_interface_handle)interface_handle);
#endif /* AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT */
}

/**
 * @brief   Synchronously Call a published procedure.
 *
 * Calls a synchronous method published in an IPC interface. A synchronous method is the one which
 * runs in the same call stack as the caller. The data in the `model_in` will be used only during
 * the execution of the method and may be released as soon as the method returns.
 *
 * As a standard, the synchronous method shall return #az_ulib_result. If the method needs to return
 * anything else, the data shall be stored it on the `model_out`.
 *
 * Both `model_in` and `model_out` shall be defined as part of the interface definition.
 *
 * @param[in]   interface_handle  The #az_ulib_ipc_interface_handle with the interface handle. It
 *                                cannot be `NULL`. Call
 *                                az_ulib_ipc_try_get_interface() to get the interface handle.
 * @param[in]   method_index      The #az_ulib_action_index with the method handle.
 * @param[in]   model_in          The `const void *const` that points to the memory with the
 *                                input model content.
 * @param[out]  model_out         The `const void *` that points to the memory where the action
 *                                should store the output model content.
 * @return The #az_ulib_result with the result of the call.
 *  @retval #AZ_ULIB_SUCCESS                  If the IPC get success calling the procedure.
 *  @retval #AZ_ULIB_ILLEGAL_ARGUMENT_ERROR   If one of the arguments is invalid.
 *  @retval #AZ_ULIB_NO_SUCH_ELEMENT_ERROR    If the target method was unpublished. The
 *                                            `interface_handle` shall be released.
 *  @retval #AZ_ULIB_BUSY_ERROR               If the target method is cannot be executed at that
 *                                            moment.
 */
static inline az_ulib_result az_ulib_ipc_call(
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_action_index method_index,
    const void* const model_in,
    const void* model_out) {
#ifdef AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT
  return _az_ulib_ipc_call(
      (_az_ulib_ipc_interface_handle)interface_handle, method_index, model_in, model_out);
#else
  return _az_ulib_ipc_call_no_contract(
      (_az_ulib_ipc_interface_handle)interface_handle, method_index, model_in, model_out);
#endif /* AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT */
}

/**
 * @brief   Asynchronously Call a published procedure.
 *
 * Calls an asynchronous method published in an IPC interface. An asynchronous method is the one
 * which runs in background, in a thread or another hardware, and will call a callback when it
 * finish its execution providing the result of the operation. The IPC will **not copy** the
 * content of the `model_in`, instead, it will bypass the reference to the asynchronous method.
 * The decision between copy the data or not is part of the contract exposed by the component
 * which published the method.
 *
 * The published shall define the `model_in` retention policy. Three possible options are:
 *
 * 1. The asynchronous method will copy the content pointed by the `model_in` on its own area of
 *    memory. On this way, the caller may release the `model_in` memory as soon as the
 *    az_ulib_ipc_call_async() returns. This method is safer, but expensive.
 * 2. The asynchronous method will use the `model_in` from the provided memory for the whole
 *    asynchronous execution. It means, that the caller shall only release this memory when the IPC
 *    calls the callback with the conclusion of the asynchronous operation. This method will save
 *    resources, but may be riskier.
 * 3. The asynchronous method will require the `model_in` as a smart pointer (any variation of
 *    #az_ulib_ustream), and will clone it locally. With it, the caller may release its own copy of
 *    the `model_in` as soon as the az_ulib_ipc_call_async() returns. This method is less expensive
 *    than the first one (mainly for big `model_in`), and safer that the other 2 options.
 *
 * As a standard, the asynchronous method shall return #az_ulib_result. If the method needs to
 * return anything else, the data shall be stored it on the `model_out`, on the callback.
 *
 * Both `model_in` and `model_out` shall be defined as part of the interface definition.
 *
 * An asynchronous call may be cancel by the caller by calling az_ulib_ipc_cancel() API passing the
 * #az_ulib_action_context.
 *
 * The following diagram represents a asynchronous call through IPC. Observe that the IPC will not
 * take care of the background process or the future callback, it become a contract between the 
 * consumer and the publisher directly. 
 *
 * @code
 * +---------------+         +---------------+         +---------------+
 * |    Consumer   |         |      IPC      |         |   Publisher   |
 * +---------------+         +---------------+         +---------------+
 *         |                         |                         |
 *         +-az_ulib_ipc_call_async( |                         |
 *         |    interface_handle,    |                         |
 *         |    method_index,        |                         |
 *         |    model_in,            |                         |
 *         |    model_out,           |                         |
 *         |    complete_task_cb,    |                         |
 *         |    (context)task)------>|                         |
 *         |            +------------+                         |
 *         |            |interface_handle->running_count++     |
 *         |            |my_method_async <- interface_handle[method_index]
 *         |            +----------->|                         |
 *         |                         +-my_method_async(        |
 *         |                         |   model_in, model_out,  |
 *         |                         |   complete_task_cb,     |
 *         |                         |   (context)task)------->|
 *         |                         | +-----------------------+
 *         |                         | |Execute the synchronous portion of
 *         |                         | |the asynchronous method, starting
 *         |                         | |the background process.
 *         |                         | +---------------------->|
 *         |                         |                         +---+
 *         |                         |<---<AZ_ULIB_SUCCESS>----+   |b
 *         |                  +------+                         |   |a
 *         |                  | interface_handle->running_count--  |c
 *         |                  +----->|                         |   |k
 *         |<---<AZ_ULIB_SUCCESS>----+                         |   |g
 *         |                         |                         |   |r
 *         |                         |                         |   |o
 *         |                         |                         |   |u
 *         |                         |                         |   |n
 *         |                         |                         |   |d
 *                                                                 |
 *    Consumer may do other stuffs while wait for the callback.    |
 *                                                                 |
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *    Background ends an generates a hardware interruption (IRQ)   |
 *                                                             |<--+
 *         |            complete_task_cb((context)task,  ------+
 *         |<----------------- AZ_ULIB_SUCCESS, model_out)     |
 *     +---+                         |                         |
 *     | Use the information in the task.                      |
 *     | release(task->model_in)     |                         |
 *     | release(task->model_out)    |                         |
 *     | release(task)               |                         |
 *     +-->|                         |                         |
 *         +-------------------------------------------------->|
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *         |                         |                         |
 * @endcode
 *
 * @param[in]   interface_handle  The #az_ulib_ipc_interface_handle with the interface handle. It
 *                                cannot be `NULL`. Call
 *                                az_ulib_ipc_try_get_interface() to get the interface handle.
 * @param[in]   method_index      The #az_ulib_action_index with the method handle.
 * @param[in]   model_in          The `const void *const` that points to the memory with the
 *                                input model content.
 * @param[out]  model_out         The `const void *` that points to the memory where the action
 *                                should store the output model content.
 * @param[in]   result_callback   The #az_ulib_action_result_callback that points to the method
 *                                that IPC shall call when the asynchronous call ends its
 *                                execution. This callback is optional and may be `NULL`. If the
 *                                callback is `NULL`, the IPC will make this call *fire and forget*.
 * @param[in]   action_context    The #az_ulib_action_context that unique identify the current
 *                                asynchronous call to the caller. The IPC will use this context
 *                                when call the `result_callback`. The caller shall use this context
 *                                to cancel an asynchronous call as well. This context is an opaque
 *                                value for the IPC, no validation will be performed on it.
 * @return The #az_ulib_result with the result of the call.
 *  @retval #AZ_ULIB_SUCCESS                  If the IPC get success calling the procedure.
 *  @retval #AZ_ULIB_ILLEGAL_ARGUMENT_ERROR   If one of the arguments is invalid.
 *  @retval #AZ_ULIB_NO_SUCH_ELEMENT_ERROR    If the target method was unpublished. The
 *                                            `interface_handle` shall be released.
 *  @retval #AZ_ULIB_BUSY_ERROR               If the target method cannot be executed at that
 *                                            moment.
 */
static inline az_ulib_result az_ulib_ipc_call_async(
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_action_index method_index,
    const void* const model_in,
    const void* model_out,
    az_ulib_action_result_callback result_callback,
    az_ulib_action_context action_context) {
#ifdef AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT
  return _az_ulib_ipc_call_async(
      (_az_ulib_ipc_interface_handle)interface_handle,
      method_index,
      model_in,
      model_out,
      result_callback,
      action_context);
#else
  return _az_ulib_ipc_call_async_no_contract(
      (_az_ulib_ipc_interface_handle)interface_handle,
      method_index,
      model_in,
      model_out,
      result_callback,
      action_context);
#endif /* AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT */
}

/**
 * @brief   Cancel an asynchronously Call.
 *
 * @param[in]   action_context    The #az_ulib_action_context that unique identify the current
 *                                asynchronous call to the caller.
 * @return The #az_ulib_result with the result of the call.
 *  @retval #AZ_ULIB_SUCCESS                  If the IPC get success calling the procedure.
 *  @retval #AZ_ULIB_NO_SUCH_ELEMENT_ERROR    If the target method `action_context` didn't belong to
 *                                            any current asynchronous call.
 *  @retval #AZ_ULIB_BUSY_ERROR               If the target method cannot be cancelled at that
 *                                            moment.
 */
static inline az_ulib_result az_ulib_ipc_cancel(az_ulib_action_context action_context) {
#ifdef AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT
  return AZ_ULIB_NOT_SUPPORTED_ERROR;
#else
  return AZ_ULIB_NOT_SUPPORTED_ERROR;
#endif /* AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT */
}

/**
 * @brief   Asynchronously Call a published procedure and wait until end of execution.
 *
 * Calls an asynchronous method published in the IPC and use a semaphore to wait for the callback,
 * converting the asynchronous call in a synchronous (blocking) call.
 *
 * @param[in]   interface_handle  The #az_ulib_ipc_interface_handle with the interface handle. It
 *                                cannot be `NULL`. Call
 *                                az_ulib_ipc_try_get_interface() to get the interface handle.
 * @param[in]   method_index      The #az_ulib_action_index with the method handle.
 * @param[in]   model_in          The `const void *const` that points to the memory with the
 *                                input model content.
 * @param[out]  model_out         The `const void *` that points to the memory where the action
 *                                should store the output model content.
 * @param[in]   wait_option_ms    The `uint32_t` with the maximum number of milliseconds the
 *                                IPC may wait to the end of the execution before generate a
 *                                timeout error:
 *                                        - #AZ_ULIB_NO_WAIT (0x00000000)
 *                                        - #AZ_ULIB_WAIT_FOREVER (0xFFFFFFFF)
 *                                        - timeout value (0x00000001 through 0xFFFFFFFE)
 *
 * @return The #az_ulib_result with the result of the call.
 *  @retval #AZ_ULIB_SUCCESS                  If the IPC get success calling the procedure.
 *  @retval #AZ_ULIB_ILLEGAL_ARGUMENT_ERROR   If one of the arguments is invalid.
 *  @retval #AZ_ULIB_NO_SUCH_ELEMENT_ERROR    If the target method was unpublished. The
 *                                            `interface_handle` shall be released.
 *  @retval #AZ_ULIB_BUSY_ERROR               If the target method cannot be executed at that
 *                                            moment.
 *  @retval #AZ_ULIB_TIMEOUT_ERROR            If the method didn't return the callback before the
 *                                            provided `wait_option_ms` expires.
 */
static inline az_ulib_result az_ulib_ipc_call_async_and_wait(
    az_ulib_ipc_interface_handle interface_handle,
    az_ulib_action_index method_index,
    const void* const model_in,
    const void* model_out,
    uint32_t wait_option_ms) {
#ifdef AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT
  return AZ_ULIB_NOT_SUPPORTED_ERROR;
#else
  return AZ_ULIB_NOT_SUPPORTED_ERROR;
#endif /* AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT */
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZ_ULIB_IPC_H */
