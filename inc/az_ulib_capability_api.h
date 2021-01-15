// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_CAPABILITY_API_H
#define AZ_ULIB_CAPABILITY_API_H

#include "az_ulib_base.h"
#include "az_ulib_result.h"

#ifdef __cplusplus
#include <stdint.h>
extern "C" {
#include <cstdint>
#endif /* __cplusplus */

/**
 * @file    az_ulib_capability_api.h
 *
 * @brief   Capabilities are properties, methods and events that can be handled by an interface.
 */

/**
 * @brief Enumerator that defines the type of the capability.
 */
typedef enum az_ulib_capability_type_tag {
  AZ_ULIB_CAPABILITY_TYPE_PROPERTY = 0x00, /**<Read and write property */
  AZ_ULIB_CAPABILITY_TYPE_METHOD = 0x01, /**<Synchronous method that can be invoked by other modules
                                     in the system */
  AZ_ULIB_CAPABILITY_TYPE_METHOD_ASYNC = 0x02, /**<Asynchronous task that can be invoked by other
                                           modules in the system */
  AZ_ULIB_CAPABILITY_TYPE_EVENT = 0x03 /**<Event that other modules in the system can subscribe to
                                   be notified */
} az_ulib_capability_type;

/**
 * @brief       Token that uniquely identifies the capability.
 *
 * The capability token is the way that the capability's caller associates the call to its answer.
 * It can have any value that is meaningful for the caller.
 */
typedef void* az_ulib_capability_token;

/**
 * @brief       model_in type.
 */
typedef const void* const az_ulib_model_in;

/**
 * @brief       model_out type.
 */
typedef const void* az_ulib_model_out;

/**
 * @brief       Handle that uniquely identifies the capability instance.
 */
typedef uint16_t az_ulib_capability_index;

/**
 * @brief       Callback prototype for capability result.
 */
typedef void (*az_ulib_capability_result_callback)(
    const az_ulib_capability_token capability_token,
    az_ulib_result result,
    az_ulib_model_out const model_out);

/**
 * @brief       Event prototype.
 */
typedef void (*az_ulib_capability_event)(az_ulib_model_out const model_out);

/**
 * @brief       Cancellation token prototype.
 */
typedef az_ulib_result (*az_ulib_capability_cancellation_callback)(
    const az_ulib_capability_token capability_token);

/**
 * @brief       IPC synchronous method signature.
 *
 * This type defines the signature for the synchronous methods that will be published in an IPC
 * interface. A synchronous method is the one which runs in the same call stack as the caller. The
 * data in the `model_in` will be used only during the execution of the method and may be released
 * as soon as the method returns.
 *
 * As a standard, the synchronous method shall return #az_ulib_result. If the method needs to return
 * anything else, the data shall be stored it on the `model_out`.
 *
 * Both `model_in` and `model_out` shall be defined as part of the interface definition.
 *
 * @param[in]   model_in    The `az_ulib_model_in` that contains the input arguments for the
 *                          method. It may be `NULL`, the IPC will not do any validation on it.
 *                          The method itself shall implement any needed validation.
 * @param[out]  model_out   The `az_ulib_model_out` that contains the memory to store the output
 *                          arguments from the method. It may be `NULL`, the IPC will not do any
 *                          validation on it. The method itself shall implement any needed
 *                          validation.
 *
 * @return The #az_ulib_result with the result of the method call. All possible results shall be
 * defined as part of the interface.
 */
typedef az_ulib_result (
    *az_ulib_capability_method)(az_ulib_model_in model_in, az_ulib_model_out model_out);

/**
 * @brief       IPC asynchronous task signature.
 */
typedef az_ulib_result (*az_ulib_capability_method_async)(
    az_ulib_model_in model_in,
    az_ulib_capability_result_callback callback,
    const az_ulib_capability_token capability_token,
    az_ulib_capability_cancellation_callback* cancel);

/**
 * @brief       IPC get signature.
 */
typedef az_ulib_result (*az_ulib_capability_get)(az_ulib_model_out model_out);

/**
 * @brief       IPC set signature.
 */
typedef az_ulib_result (*az_ulib_capability_set)(az_ulib_model_in model_in);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZ_ULIB_CAPABILITY_API_H */
