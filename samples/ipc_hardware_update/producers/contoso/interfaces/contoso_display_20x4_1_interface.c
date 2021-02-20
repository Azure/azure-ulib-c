// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from display v1 DL.
 *
 * Implement the code under the concrete functions.
 *
 ********************************************************************/

#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "az_ulib_ucontract.h"
#include "az_ulib_ulog.h"
#include "contoso_display_20x4_bsp.h"
#include "display_1_interface.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Concrete implementations of the display commands.
 */

static az_result cls_concrete(az_ulib_model_in model_in, az_ulib_model_out model_out)
{
  /*
   * ==================
   * The user code starts here.
   */

  contoso_display_20x4_bsp_cls();

  /*
   * The user code ends here.
   * ==================
   */

  return AZ_OK;
}

static az_result print_concrete(az_ulib_model_in model_in, az_ulib_model_out model_out)
{
  AZ_ULIB_UCONTRACT(AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(model_in, AZ_ERROR_ARG));

  /*
   * ==================
   * The user code starts here.
   */
  display_1_print_model_in* in = (display_1_print_model_in*)model_in;

  contoso_display_20x4_bsp_goto(in->x, in->y);
  contoso_display_20x4_bsp_print(in->buffer, in->size);

  /*
   * The user code ends here.
   * ==================
   */

  return AZ_OK;
}

static az_result invalidate_concrete(az_ulib_model_in model_in, az_ulib_model_out model_out)
{
  /*
   * ==================
   * The user code starts here.
   */

  contoso_display_20x4_bsp_invalidate();

  /*
   * The user code ends here.
   * ==================
   */

  return AZ_OK;
}

AZ_ULIB_DESCRIPTOR_CREATE(
    DISPLAY_1_DESCRIPTOR,
    DISPLAY_1_INTERFACE_NAME,
    DISPLAY_1_INTERFACE_VERSION,
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND(DISPLAY_1_INTERFACE_CLS_COMMAND_NAME, cls_concrete),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND(DISPLAY_1_INTERFACE_PRINT_COMMAND_NAME, print_concrete),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND(
        DISPLAY_1_INTERFACE_INVALIDATE_COMMAND_NAME,
        invalidate_concrete));

az_result publish_contoso_display_20x4_1_interface(void)
{
  return az_ulib_ipc_publish(&DISPLAY_1_DESCRIPTOR, NULL);
}

az_result unpublish_contoso_display_20x4_1_interface(void)
{
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
  return az_ulib_ipc_unpublish(&DISPLAY_1_DESCRIPTOR, AZ_ULIB_NO_WAIT);
#else
  return AZ_OK;
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
}
