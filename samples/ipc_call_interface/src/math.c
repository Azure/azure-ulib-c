// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_action_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "math_tlb.h"
#include "az_ulib_ucontract.h"
#include "az_ulib_result.h"
#include "az_ulib_ulog.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Concrete implementations of the math methods.
 */
static AZ_ULIB_RESULT sum_concrete(const void* const model_in, const void* model_out) {
  AZ_UCONTRACT(
      AZ_UCONTRACT_REQUIRE_NOT_NULL(model_in, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
      AZ_UCONTRACT_REQUIRE_NOT_NULL(model_out, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));

  sum_model_in* in = (sum_model_in*)model_in;
  sum_model_out* out = (sum_model_out*)model_out;

  *out = (uint64_t)in->a + (uint64_t)in->b;

  return AZ_ULIB_SUCCESS;
}

static AZ_ULIB_RESULT subtraction_concrete(const void* const model_in, const void* model_out) {
  AZ_UCONTRACT(
      AZ_UCONTRACT_REQUIRE_NOT_NULL(model_in, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
      AZ_UCONTRACT_REQUIRE_NOT_NULL(model_out, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));

  subtraction_model_in* in = (subtraction_model_in*)model_in;
  subtraction_model_out* out = (subtraction_model_out*)model_out;

  *out = (uint64_t)in->a - (uint64_t)in->b;

  return AZ_ULIB_SUCCESS;
}

AZ_ULIB_DESCRIPTOR_CREATE(
    MATH_DESCRIPTOR,
    MATH_INTERFACE_NAME,
    MATH_INTERFACE_VERSION,
    AZ_ULIB_DESCRIPTOR_ADD_METHOD(MATH_INTERFACE_SUM_METHOD_NAME, sum_concrete),
    AZ_ULIB_DESCRIPTOR_ADD_METHOD(MATH_INTERFACE_SUBTRACTION_METHOD_NAME, subtraction_concrete));

AZ_ULIB_RESULT math_publish_interface(void) { return az_ulib_ipc_publish(&MATH_DESCRIPTOR); }

AZ_ULIB_RESULT math_unpublish_interface(void) {
#ifdef AZ_ULIB_CONFIG_MAX_IPC_UNPUBLISH
  return az_ulib_ipc_unpublish(&MATH_DESCRIPTOR, AZ_ULIB_NO_WAIT);
#else
  return AZ_ULIB_SUCCESS;
#endif // AZ_ULIB_CONFIG_MAX_IPC_UNPUBLISH
}
