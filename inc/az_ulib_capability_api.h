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
 * @brief       IPC synchronous capability signature.
 *
 * This type defines the signature for the synchronous capabilities that will be published in an IPC
 * interface. A synchronous capability is the one which runs in the same call stack as the caller.
 * The data in the `model_in` will be used only during the execution of the capability and may be
 * released as soon as the capability call returns.
 *
 * As a standard, the synchronous capability call shall return #az_result. If the capability needs
 * to return anything else, the data shall be stored it on the `model_out`.
 *
 * Both `model_in` and `model_out` shall be defined as part of the interface definition.
 *
 * @param[in]   model_in    The `az_ulib_model_in` that contains the input arguments for the
 *                          capability. It may be `NULL`, the IPC will not validate it.
 *                          The capability itself shall implement any needed validation.
 * @param[out]  model_out   The `az_ulib_model_out` that contains the memory to store the output
 *                          arguments from the capability. It may be `NULL`, the IPC will not
 *                          validate it. The capability itself shall implement any needed
 *                          validation.
 *
 * @return The #az_result with the result of the capability call. All possible results shall be
 * defined as part of the interface.
 */
typedef az_result (*az_ulib_capability)(az_ulib_model_in model_in, az_ulib_model_out model_out);

/**
 * @brief       Call a capability in the interface using strings in `az_span`.
 *
 * This type defines the signature for the synchronous capabilities that will be published in an IPC
 * interface. A synchronous capability is the one which runs in the same call stack as the caller.
 * The data in the `model_in` will be used only during the execution of the capability and may be
 * released as soon as the capability call returns.
 *
 * As a standard, the synchronous capability shall return #az_result. If the capability needs to
 * return anything else, the data shall be stored in the `model_out`.
 *
 * Both `model_in` and `model_out` shall be defined as part of the interface definition, and
 * both shall be strings with JSON inside.
 *
 * @param[in]   model_in_span   The `az_span` that contains a JSON with the input arguments for
 *                              the capability. It may be an empty JSON `{}`, the IPC will not
 *                              validate it. The capability itself shall implement the JSON
 *                              parser with any needed validation.
 * @param[out]  model_out_span  The pointer to `az_span` that contains the memory to store the
 *                              JSON with the output arguments from the capability. It may be
 *                              `NULL`, the IPC will not validate it. The capability itself
 *                              shall implement the JSON writer with any needed validation.
 *
 * @return The #az_result with the result of the capability call. All possible results shall be
 * defined as part of the interface.
 */
typedef az_result (
    *az_ulib_capability_span_wrapper)(az_span model_in_span, az_span* model_out_span);

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_CAPABILITY_API_H */
