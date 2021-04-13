// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_CAPABILITY_API_H
#define AZ_ULIB_CAPABILITY_API_H

#include "az_ulib_base.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"

#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
#endif /* __cplusplus */

#include "azure/core/_az_cfg_prefix.h"

/**
 * @file    az_ulib_capability_api.h
 *
 * @brief   Capabilities are telemetry, property, and command that can be handled by an interface.
 */

/**
 * @brief Enumerator that defines the type of the capability.
 */
typedef enum az_ulib_capability_type_tag
{
  /**
   * Telemetry describes the data emitted by any interface, whether the data is a regular stream of
   * sensor readings or a computed stream of data, such as occupancy, or an occasional error or
   * information message.
   */
  AZ_ULIB_CAPABILITY_TYPE_TELEMETRY = 0x00,

  /**
   * A Property describes the read-only and read/write state of the data in an interface. For
   * example, a device serial number may be a read-only property, the desired temperature on a
   * thermostat may be a read-write property, and the name of a room may be a read-write property.
   */
  AZ_ULIB_CAPABILITY_TYPE_PROPERTY = 0x01,

  /**
   * A Command describes a function or operation that can be synchronously performed in an
   * interface.
   */
  AZ_ULIB_CAPABILITY_TYPE_COMMAND = 0x02,

  /**
   * A Command Async describes a function or operation that can be asynchronously performed in an
   * interface.
   */
  AZ_ULIB_CAPABILITY_TYPE_COMMAND_ASYNC = 0x03
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
typedef void* az_ulib_model_out;

/**
 * @brief       Handle that uniquely identifies the capability instance.
 */
typedef uint16_t az_ulib_capability_index;

/**
 * @brief       Callback prototype for capability result.
 */
typedef void (*az_ulib_capability_result_callback)(
    const az_ulib_capability_token capability_token,
    az_result result,
    az_ulib_model_out const model_out);

/**
 * @brief       Telemetry prototype.
 */
typedef void (*az_ulib_capability_telemetry)(az_ulib_model_out const model_out);

/**
 * @brief       Cancellation token prototype.
 */
typedef az_result (*az_ulib_capability_cancellation_callback)(
    const az_ulib_capability_token capability_token);

/**
 * @brief       IPC synchronous command signature.
 *
 * This type defines the signature for the synchronous commands that will be published in an IPC
 * interface. A synchronous command is the one which runs in the same call stack as the caller.
 * The data in the `model_in` will be used only during the execution of the command and may be
 * released as soon as the command returns.
 *
 * As a standard, the synchronous command shall return #az_result. If the command needs to
 * return anything else, the data shall be stored it on the `model_out`.
 *
 * Both `model_in` and `model_out` shall be defined as part of the interface definition.
 *
 * @param[in]   model_in    The `az_ulib_model_in` that contains the input arguments for the
 *                          command. It may be `NULL`, the IPC will not do any validation on it.
 *                          The command itself shall implement any needed validation.
 * @param[out]  model_out   The `az_ulib_model_out` that contains the memory to store the output
 *                          arguments from the command. It may be `NULL`, the IPC will not do any
 *                          validation on it. The command itself shall implement any needed
 *                          validation.
 *
 * @return The #az_result with the result of the command call. All possible results shall be
 * defined as part of the interface.
 */
typedef az_result (
    *az_ulib_capability_command)(az_ulib_model_in model_in, az_ulib_model_out model_out);

/**
 * @brief       Call a capability in the interface using strings in `az_span`.
 *
 * This type defines the signature for the synchronous commands that will be published in an IPC
 * interface. A synchronous command is the one which runs in the same call stack as the caller.
 * The data in the `model_in` will be used only during the execution of the command and may be
 * released as soon as the command returns.
 *
 * As a standard, the synchronous command shall return #az_result. If the command needs to
 * return anything else, the data shall be stored it on the `model_out`.
 *
 * Both `model_in` and `model_out` shall be defined as part of the interface definition, and
 * both shall be strings with JSON inside.
 *
 * @param[in]   model_in_span   The `az_span` that contains a JSON with the input arguments for
 *                              the command. It may be an empty JSON `{}`, the IPC will not do any
 *                              validation on it. The command itself shall implement the JSON
 *                              parser with any needed validation.
 * @param[out]  model_out_span  The `az_ulib_model_out` that contains the memory to store the
 *                              JSON with the output arguments from the command. It may be `NULL`,
 *                              the IPC will not do any validation on it. The command itself shall
 *                              implement the JSON writer with any needed validation.
 *
 * @return The #az_result with the result of the command call. All possible results shall be
 * defined as part of the interface.
 */
typedef az_result (
    *az_ulib_capability_command_span_wrapper)(az_span model_in_span, az_span* model_out_span);

/**
 * @brief       IPC asynchronous task signature.
 */
typedef az_result (*az_ulib_capability_command_async)(
    az_ulib_model_in model_in,
    az_ulib_model_out model_out,
    const az_ulib_capability_token capability_token,
    const az_ulib_capability_cancellation_callback cancel);

/**
 * @brief       IPC asynchronous task signature using strings in `az_span`.
 */
typedef az_result (*az_ulib_capability_command_async_span_wrapper)(
    az_span model_in_span,
    az_span* model_out_span,
    const az_ulib_capability_token capability_token,
    const az_ulib_capability_cancellation_callback cancel);

/**
 * @brief       IPC get signature.
 */
typedef az_result (*az_ulib_capability_get)(az_ulib_model_out model_out);

/**
 * @brief       IPC get signature using strings in `az_span`.
 */
typedef az_result (*az_ulib_capability_get_span_wrapper)(az_span* model_out_span);

/**
 * @brief       IPC set signature.
 */
typedef az_result (*az_ulib_capability_set)(az_ulib_model_in model_in);

/**
 * @brief       IPC set signature using strings in `az_span`.
 */
typedef az_result (*az_ulib_capability_set_span_wrapper)(az_span model_in_span);

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_CAPABILITY_API_H */
