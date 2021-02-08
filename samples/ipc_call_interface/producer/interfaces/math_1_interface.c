// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from math v1 DL.
 *
 * Implement the code under the concrete functions.
 *
 ********************************************************************/

#include "math_1_interface.h"
#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "az_ulib_ucontract.h"
#include "az_ulib_ulog.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Concrete implementations of the math commands.
 */
static az_result math_1_sum_concrete(az_ulib_model_in model_in, az_ulib_model_out model_out)
{
  AZ_ULIB_UCONTRACT(
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(model_in, AZ_ERROR_ARG),
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(model_out, AZ_ERROR_ARG));

  /*
   * ==================
   * The user code starts here.
   */
  math_1_sum_model_in* in = (math_1_sum_model_in*)model_in;
  math_1_sum_model_out* out = (math_1_sum_model_out*)model_out;

  *out = (uint64_t)in->a + (uint64_t)in->b;
  /*
   * The user code ends here.
   * ==================
   */

  return AZ_OK;
}

static az_result math_1_subtract_concrete(az_ulib_model_in model_in, az_ulib_model_out model_out)
{
  AZ_ULIB_UCONTRACT(
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(model_in, AZ_ERROR_ARG),
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(model_out, AZ_ERROR_ARG));

  /*
   * ==================
   * The user code starts here.
   */
  math_1_subtract_model_in* in = (math_1_subtract_model_in*)model_in;
  math_1_subtract_model_out* out = (math_1_subtract_model_out*)model_out;

  *out = (uint64_t)in->a - (uint64_t)in->b;
  /*
   * The user code ends here.
   * ==================
   */

  return AZ_OK;
}

AZ_ULIB_DESCRIPTOR_CREATE(
    MATH_1_DESCRIPTOR,
    MATH_1_INTERFACE_NAME,
    MATH_1_INTERFACE_VERSION,
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND(MATH_1_INTERFACE_SUM_COMMAND_NAME, math_1_sum_concrete),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND(
        MATH_1_INTERFACE_SUBTRACT_COMMAND_NAME,
        math_1_subtract_concrete));

az_result math_1_publish_interface(void) { return az_ulib_ipc_publish(&MATH_1_DESCRIPTOR, NULL); }

az_result math_1_unpublish_interface(void)
{
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
  return az_ulib_ipc_unpublish(&MATH_1_DESCRIPTOR, AZ_ULIB_NO_WAIT);
#else
  return AZ_OK;
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
}
