// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_ACTION_API_H
#define AZ_ULIB_ACTION_API_H

#include "az_ulib_base.h"
#include "az_ulib_result.h"

#ifdef __cplusplus
#include <stdint.h>
extern "C" {
#include <cstdint>
#endif /* __cplusplus */

/**
 * @file    az_ulib_action_api.h
 *
 * @brief   Actions are properties, methods and events that can be handled by an interface.
 */

/**
 * @brief Enumerator that defines the type of the action.
 */
typedef enum az_ulib_action_type_tag {
    AZ_ULIB_ACTION_TYPE_PROPERTY        = 0x00, /**<Read and write property */
    AZ_ULIB_ACTION_TYPE_METHOD          = 0x01, /**<Synchronous method that can be invoked by other modules
                                                in the system */
    AZ_ULIB_ACTION_TYPE_METHOD_ASYNC    = 0x02, /**<Asynchronous task that can be invoked by other
                                                modules in the system */
    AZ_ULIB_ACTION_TYPE_EVENT           = 0x03 /**<Event that other modules in the system can subscribe to
                                                be notified */
} az_ulib_action_type;

/**
 * @brief       Context that uniquely identifies the action.
 *
 * The action context is the way that the action's caller associates the call to its answer. It can
 * have any value that is meaningful for the caller.
 */
typedef void* az_ulib_action_context;

/**
 * @brief       Handle that uniquely identifies the action instance.
 */
typedef uint16_t az_ulib_action_index;

/**
 * @brief       Callback prototype for action result.
 */
typedef void (*az_ulib_action_result_callback)(
    const az_ulib_action_context action_context,
    az_ulib_result result,
    const void* const model_out);

/**
 * @brief       Event prototype.
 */
typedef void (*az_ulib_action_event)(const void* const model_out);

/**
 * @brief       Cancellation context prototype.
 */
typedef az_ulib_result (*az_ulib_action_cancellation_callback)(
    const az_ulib_action_context action_context);

/**
 * @brief       IPC synchronous method signature.
 *
 * This type defines the signature for the synchronous methods that will be called by 
 * az_ulib_ipc_call().
 *
 * @param[in]   model_in    The `const void* const` that contains the input arguments for the
 *                          method. It may be `NULL`, the IPC will not do any validation on it.
 *                          The method itself shall implement any needed validation.
 * @param[out]  model_out   The `const void*` that contains the memory to store the output
 *                          arguments from the method. It may be `NULL`, the IPC will not do any
 *                          validation on it. The method itself shall implement any needed
 *                          validation.
 *
 * @return The #az_ulib_result with the result of the method call. All possible results shall be
 * defined as part of the interface.
 */
typedef az_ulib_result (*az_ulib_action_method)(const void* const model_in, const void* model_out);

/**
 * @brief       IPC asynchronous task signature.
 *
 * This type defines the signature for the asynchronous methods that will be called by 
 * az_ulib_ipc_call_async() and az_ulib_ipc_call_async_and_wait().
 *
 * @param[in]   model_in          The `const void* const` that contains the input arguments for the
 *                                method. It may be `NULL`, the IPC will not do any validation
 *                                on it. The method itself shall implement any needed validation.
 * @param[out]  model_out         The `const void*` that contains the memory to store the output
 *                                arguments from the method. It may be `NULL`, the IPC will not do
 *                                any validation on it. The method itself shall implement any needed
 *                                validation.
 * @param[in]   result_callback   The #az_ulib_action_result_callback that points to the method
 *                                that IPC shall call when the asynchronous call ends its
 *                                execution. This callback is optional and may be `NULL`. If the
 *                                callback is `NULL`, the IPC will make this call *fire and forget*.
 * @param[in]   action_context      The #az_ulib_action_context that unique identify the current
 *                                asynchronous call to the caller. The IPC will use this context
 *                                when call the `result_callback`. The caller shall use this context
 *                                to cancel an asynchronous call as well. This context is a opaque
 *                                value for the IPC, no validation will be performed on it.
 *
 * @return The #az_ulib_result with the result of the method call. All possible results shall be
 * defined as part of the interface.
 */
typedef az_ulib_result (*az_ulib_action_method_async)(
    const void* const model_in,
    const void* model_out,
    az_ulib_action_result_callback callback,
    const az_ulib_action_context action_context);

/**
 * @brief       IPC get signature.
 */
typedef az_ulib_result (*az_ulib_action_get)(const void* model_out);

/**
 * @brief       IPC set signature.
 */
typedef az_ulib_result (*az_ulib_action_set)(const void* const model_in);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZ_ULIB_ACTION_API_H */
