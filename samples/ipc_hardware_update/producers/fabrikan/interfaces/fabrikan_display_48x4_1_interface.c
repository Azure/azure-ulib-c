// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from display v1 DL.
 *
 * Implement the code under the concrete functions.
 *
 ********************************************************************/

#include "fabrikan_display_48x4_1_interface.h"
#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "display_1_interface.h"
#include "fabrikan_display_48x4_bsp.h"

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
  (void)model_in;
  (void)model_out;

  fabrikan_display_48x4_bsp_cls();

  /*
   * The user code ends here.
   * ==================
   */

  return AZ_OK;
}

static az_result print_concrete(az_ulib_model_in model_in, az_ulib_model_out model_out)
{
  /*
   * ==================
   * The user code starts here.
   */
  (void)model_out;
  const display_1_print_model_in* const in = (const display_1_print_model_in* const)model_in;

  fabrikan_display_48x4_bsp_goto(in->x, in->y);
  fabrikan_display_48x4_bsp_print(in->buffer, in->size);

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
  (void)model_in;
  (void)model_out;

  fabrikan_display_48x4_bsp_invalidate();

  /*
   * The user code ends here.
   * ==================
   */

  return AZ_OK;
}

static const az_ulib_capability_descriptor DISPLAY_1_CAPABILITIES[DISPLAY_1_CAPABILITY_SIZE] = {
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND(DISPLAY_1_INTERFACE_CLS_COMMAND_NAME, cls_concrete),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND(DISPLAY_1_INTERFACE_PRINT_COMMAND_NAME, print_concrete),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND(DISPLAY_1_INTERFACE_INVALIDATE_COMMAND_NAME, invalidate_concrete)
};

static const az_ulib_interface_descriptor DISPLAY_1_DESCRIPTOR = AZ_ULIB_DESCRIPTOR_CREATE(
    DISPLAY_1_INTERFACE_NAME,
    DISPLAY_1_INTERFACE_VERSION,
    DISPLAY_1_CAPABILITY_SIZE,
    NULL,
    DISPLAY_1_CAPABILITIES);

az_result publish_fabrikan_display_48x4_1_interface(void)
{
  return az_ulib_ipc_publish(&DISPLAY_1_DESCRIPTOR, NULL);
}

az_result unpublish_fabrikan_display_48x4_1_interface(void)
{
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
  return az_ulib_ipc_unpublish(&DISPLAY_1_DESCRIPTOR, AZ_ULIB_NO_WAIT);
#else
  return AZ_OK;
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
}
