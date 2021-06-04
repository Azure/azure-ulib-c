// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from display v1 DL.
 *
 * Implement the code under the concrete functions.
 *
 ********************************************************************/

#include "contoso_display_20x4_1_interface.h"
#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "contoso_display_20x4_bsp.h"
#include "display_1_model.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Concrete implementations of the display commands.
 */

static az_result cls_concrete(const display_1_cls_model_in* const in, display_1_cls_model_out* out)
{
  /*
   * ==================
   * The user code starts here.
   */
  (void)in;
  (void)out;

  contoso_display_20x4_bsp_cls();

  /*
   * The user code ends here.
   * ==================
   */

  return AZ_OK;
}

static az_result print_concrete(
    const display_1_print_model_in* const in,
    display_1_print_model_out* out)
{
  /*
   * ==================
   * The user code starts here.
   */
  (void)out;

  contoso_display_20x4_bsp_goto(in->x, in->y);
  contoso_display_20x4_bsp_print(in->buffer, in->size);

  /*
   * The user code ends here.
   * ==================
   */

  return AZ_OK;
}

static az_result invalidate_concrete(
    const display_1_invalidate_model_in* const in,
    display_1_invalidate_model_out* out)
{
  /*
   * ==================
   * The user code starts here.
   */
  (void)in;
  (void)out;

  contoso_display_20x4_bsp_invalidate();

  /*
   * The user code ends here.
   * ==================
   */

  return AZ_OK;
}

static az_result get_max_x_concrete(
    const display_1_max_x_model* const in,
    display_1_max_x_model* out)
{
  /*
   * ==================
   * The user code starts here.
   */
  (void)in;

  *out = contoso_display_20x4_bsp_get_max_x();

  /*
   * The user code ends here.
   * ==================
   */

  return AZ_OK;
}

static az_result get_max_y_concrete(
    const display_1_max_y_model* const in,
    display_1_max_y_model* out)
{
  /*
   * ==================
   * The user code starts here.
   */
  (void)in;

  *out = contoso_display_20x4_bsp_get_max_y();

  /*
   * The user code ends here.
   * ==================
   */

  return AZ_OK;
}

static const az_ulib_capability_descriptor DISPLAY_1_CAPABILITIES[] = {
  AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(DISPLAY_1_CLS_COMMAND_NAME, cls_concrete, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(DISPLAY_1_PRINT_COMMAND_NAME, print_concrete, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(DISPLAY_1_INVALIDATE_COMMAND_NAME, invalidate_concrete, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(DISPLAY_1_GET_MAX_X_PROPERTY_NAME, get_max_x_concrete, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(DISPLAY_1_GET_MAX_Y_PROPERTY_NAME, get_max_y_concrete, NULL)
};

static const az_ulib_interface_descriptor DISPLAY_1_DESCRIPTOR = AZ_ULIB_DESCRIPTOR_CREATE(
    DISPLAY_1_INTERFACE_NAME,
    DISPLAY_1_INTERFACE_VERSION,
    DISPLAY_1_CAPABILITIES);

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
