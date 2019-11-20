// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include <stdint.h>

/*
 * interface definition
 */
#define MATH_INTERFACE_NAME "math"
#define MATH_INTERFACE_VERSION 1
typedef az_ulib_ipc_interface_handle math_handle;

typedef enum {
  MATH_INTERFACE_SUM_METHOD = 0,
  MATH_INTERFACE_SUBTRACTION_METHOD = 1
} math_interface_index;

static inline az_ulib_result math_create(math_handle handle) {
  return az_ulib_ipc_try_get_interface(
      MATH_INTERFACE_NAME, MATH_INTERFACE_VERSION, AZ_ULIB_VERSION_EQUALS_TO, handle);
}

static inline void math_destroy(math_handle handle) { az_ulib_ipc_release_interface(handle); }

/*
 * Define sum method on math interface.
 */
#define MATH_INTERFACE_SUM_METHOD_NAME "sum"

typedef struct sum_model_in_tag {
  int32_t a;
  int32_t b;
} sum_model_in;

typedef int64_t sum_model_out;

/*
 * Inter-component Calling Wrapper for sum.
 */
static inline az_ulib_result sum(math_handle handle, int32_t a, int32_t b, int64_t* res) {
  sum_model_in sum_in;
  sum_model_out sum_out;

  // Marshalling
  sum_in.a = a;
  sum_in.b = b;

  sum_out = 0;

  // Call
  az_ulib_result result = az_ulib_ipc_call(
      handle, (az_ulib_action_index)MATH_INTERFACE_SUM_METHOD, &sum_in, &sum_out);

  // Unmarshalling
  *res = sum_out;

  return result;
}

/*
 * Define subtraction method on math interface.
 */
#define MATH_INTERFACE_SUBTRACTION_METHOD_NAME "subtraction"

typedef struct subtraction_model_in_tag {
  int32_t a;
  int32_t b;
} subtraction_model_in;

typedef int64_t subtraction_model_out;

/*
 * Inter-component Calling Wrapper for subtraction.
 */
static inline az_ulib_result subtraction(math_handle handle, int32_t a, int32_t b, int64_t* res) {
  subtraction_model_in sub_in;
  subtraction_model_out sub_out;

  // Marshalling
  sub_in.a = a;
  sub_in.b = b;

  sub_out = 0;

  // Call
  az_ulib_result result = az_ulib_ipc_call(
      handle, (az_ulib_action_index)MATH_INTERFACE_SUBTRACTION_METHOD, &sub_in, &sub_out);

  // Unmarshalling
  *res = sub_out;

  return result;
}
