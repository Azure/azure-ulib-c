// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "math.h"
#include <stdint.h>

static inline az_ulib_result math_create(math_handle handle) {
  return az_ulib_ipc_try_get_interface(
      MATH_INTERFACE_NAME, MATH_INTERFACE_VERSION, AZ_ULIB_VERSION_EQUALS_TO, handle);
}

static inline void math_destroy(math_handle handle) { az_ulib_ipc_release_interface(handle); }

/*
 * Inter-component Calling Wrapper for sum.
 */
static inline az_ulib_result math_sum(math_handle handle, int32_t a, int32_t b, int64_t* res) {
  sum_model_in sum_in;
  sum_model_out sum_out;

  // Marshalling
  sum_in.a = a;
  sum_in.b = b;

  sum_out = 0;

  // Call
  az_ulib_result result = az_ulib_ipc_call(handle, MATH_INTERFACE_SUM_METHOD, &sum_in, &sum_out);

  // Unmarshalling
  *res = sum_out;

  return result;
}

/*
 * Inter-component Calling Wrapper for subtraction.
 */
static inline az_ulib_result math_sub(math_handle handle, int32_t a, int32_t b, int64_t* res) {
  sub_model_in sub_in;
  sub_model_out sub_out;

  // Marshalling
  sub_in.a = a;
  sub_in.b = b;

  sub_out = 0;

  // Call
  az_ulib_result result = az_ulib_ipc_call(handle, MATH_INTERFACE_SUB_METHOD, &sub_in, &sub_out);

  // Unmarshalling
  *res = sub_out;

  return result;
}
