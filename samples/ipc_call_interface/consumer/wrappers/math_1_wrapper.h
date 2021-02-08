// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from math v1 DL and shall not be
 * modified.
 ********************************************************************/

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include <stdint.h>

/*
 * interface definition
 */
#define MATH_1_INTERFACE_NAME "math"
#define MATH_1_INTERFACE_VERSION 1

#define MATH_1_INTERFACE_SUM_COMMAND (az_ulib_capability_index)0
#define MATH_1_INTERFACE_SUBTRACT_COMMAND (az_ulib_capability_index)1

/*
 * Define sum command on math interface.
 */
#define MATH_1_INTERFACE_SUM_COMMAND_NAME "sum"

typedef struct math_1_sum_model_in_tag
{
  int32_t a;
  int32_t b;
} math_1_sum_model_in;

typedef int64_t math_1_sum_model_out;

/*
 * Define subtraction command on math interface.
 */
#define MATH_1_INTERFACE_SUBTRACT_COMMAND_NAME "subtract"

typedef struct math_1_subtract_model_in_tag
{
  int32_t a;
  int32_t b;
} math_1_subtract_model_in;

typedef int64_t math_1_subtract_model_out;

/*
 * math class constructor.
 */
static inline az_result math_1_create(az_ulib_ipc_interface_handle handle)
{
  return az_ulib_ipc_try_get_interface(
      MATH_1_INTERFACE_NAME, MATH_1_INTERFACE_VERSION, AZ_ULIB_VERSION_EQUALS_TO, handle);
}

/*
 * math class destructor.
 */
static inline void math_1_destroy(az_ulib_ipc_interface_handle handle)
{
  az_ulib_ipc_release_interface(handle);
}

/*
 * Azure Callable Wrapper for math sum.
 */
static inline az_result math_1_sum(
    az_ulib_ipc_interface_handle handle,
    int32_t a,
    int32_t b,
    int64_t* res)
{
  // Marshalling
  math_1_sum_model_in in = { a, b };

  // Call
  return az_ulib_ipc_call(handle, MATH_1_INTERFACE_SUM_COMMAND, &in, res);
}

/*
 * Azure Callable Wrapper for math subtract.
 */
static inline az_result math_1_subtract(
    az_ulib_ipc_interface_handle handle,
    int32_t a,
    int32_t b,
    int64_t* res)
{
  // Marshalling
  math_1_subtract_model_in in = { a, b };

  // Call
  return az_ulib_ipc_call(handle, MATH_1_INTERFACE_SUBTRACT_COMMAND, &in, res);
}
