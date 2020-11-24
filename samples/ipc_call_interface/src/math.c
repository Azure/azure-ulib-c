// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_action_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "az_ulib_ucontract.h"
#include "az_ulib_ulog.h"
#include "math.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Concrete implementations of the math methods.
 */
static az_ulib_result sum_concrete(az_ulib_model_in model_in, az_ulib_model_out model_out) {
  AZ_ULIB_UCONTRACT(
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(model_in, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(model_out, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));

  sum_model_in* in = (sum_model_in*)model_in;
  sum_model_out* out = (sum_model_out*)model_out;

  *out = (uint64_t)in->a + (uint64_t)in->b;

  return AZ_ULIB_SUCCESS;
}

static az_ulib_result subtract_concrete(az_ulib_model_in model_in, az_ulib_model_out model_out) {
  AZ_ULIB_UCONTRACT(
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(model_in, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(model_out, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));

  subtract_model_in* in = (subtract_model_in*)model_in;
  subtract_model_out* out = (subtract_model_out*)model_out;

  *out = (uint64_t)in->a - (uint64_t)in->b;

  return AZ_ULIB_SUCCESS;
}

AZ_ULIB_DESCRIPTOR_CREATE(
    MATH_DESCRIPTOR,
    MATH_INTERFACE_NAME,
    MATH_INTERFACE_VERSION,
    AZ_ULIB_DESCRIPTOR_ADD_METHOD(MATH_INTERFACE_SUM_METHOD_NAME, sum_concrete),
    AZ_ULIB_DESCRIPTOR_ADD_METHOD(MATH_INTERFACE_SUBTRACT_METHOD_NAME, subtract_concrete));

az_ulib_result math_publish_interface(void) { return az_ulib_ipc_publish(&MATH_DESCRIPTOR, NULL); }

az_ulib_result math_unpublish_interface(void) {
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
  return az_ulib_ipc_unpublish(&MATH_DESCRIPTOR, AZ_ULIB_NO_WAIT);
#else
  return AZ_ULIB_SUCCESS;
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
}
