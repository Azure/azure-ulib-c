// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_ACTION_API_H
#define AZ_ULIB_ACTION_API_H

#include "az_ulib_base.h"
#include "azure_macro_utils/macro_utils.h"
#include "az_ulib_result.h"
#include "umock_c/umock_c_prod.h"

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
MU_DEFINE_ENUM(
    az_ulib_action_type,
    AZ_ULIB_ACTION_TYPE_PROPERTY = 0x00, /**<Read and write property */
    AZ_ULIB_ACTION_TYPE_METHOD = 0x01, /**<Synchronous method that can be invoked by other modules
                                       in the system */
    AZ_ULIB_ACTION_TYPE_METHOD_ASYNC = 0x02, /**<Asynchronous task that can be invoked by other
                                            modules in the system */
    AZ_ULIB_ACTION_TYPE_EVENT = 0x03 /**<Event that other modules in the system can subscribe to
                                     be notified */
);

/**
 * @brief       Token that uniquely identifies the action.
 *
 * The action token is the way that the action's caller associates the call to its answer. It can
 * have any value that is meaningful for the caller.
 */
typedef void* az_ulib_action_token;

/**
 * @brief       Handle that uniquely identifies the action instance.
 */
typedef uint16_t az_ulib_action_index;

/**
 * @brief       Callback prototype for action result.
 */
typedef void (*az_ulib_action_result_callback)(
    const az_ulib_action_token action_token,
    AZ_ULIB_RESULT result,
    const void* const model_out);

/**
 * @brief       Event prototype.
 */
typedef void (*az_ulib_action_event)(const void* const model_out);

/**
 * @brief       Cancellation token prototype.
 */
typedef AZ_ULIB_RESULT (*az_ulib_action_cancellation_callback)(
    const az_ulib_action_token action_token);

/**
 * @brief       IPC synchronous method signature.
 *
 * This type defines the signature for the synchronous methods that will be published in an IPC
 * interface. A synchronous method is the one which runs in the same call stack as the caller. The
 * data in the `model_in` will be used only during the execution of the method and may be released
 * as soon as the method returns.
 *
 * As a standard, the synchronous method shall return #AZ_ULIB_RESULT. If the method needs to return
 * anything else, the data shall be stored it on the `model_out`.
 *
 * Both `model_in` and `model_out` shall be defined as part of the interface definition.
 *
 * @param[in]   model_in    The `const void* const` that contains the input arguments for the
 *                          method. It may be `NULL`, the IPC will not do any validation on it.
 *                          The method itself shall implement any needed validation.
 * @param[out]  model_out   The `const void*` that contains the memory to store the output
 *                          arguments from the method. It may be `NULL`, the IPC will not do any
 *                          validation on it. The method itself shall implement any needed
 *                          validation.
 *
 * @return The #AZ_ULIB_RESULT with the result of the method call. All possible results shall be
 * defined as part of the interface.
 */
typedef AZ_ULIB_RESULT (*az_ulib_action_method)(const void* const model_in, const void* model_out);

/**
 * @brief       IPC asynchronous task signature.
 */
typedef AZ_ULIB_RESULT (*az_ulib_action_method_async)(
    const void* const model_in,
    az_ulib_action_result_callback callback,
    const az_ulib_action_token action_token,
    az_ulib_action_cancellation_callback* cancel);

/**
 * @brief       IPC get signature.
 */
typedef AZ_ULIB_RESULT (*az_ulib_action_get)(const void* model_out);

/**
 * @brief       IPC set signature.
 */
typedef AZ_ULIB_RESULT (*az_ulib_action_set)(const void* const model_in);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZ_ULIB_ACTION_API_H */
