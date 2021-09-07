// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from display.1 DL and shall not be
 * modified.
 *
 * All concrete functions defined in this header shall be implemented
 * and compiled together with the interface.
 ********************************************************************/

#ifndef DISPLAY_1_CAPABILITIES_H
#define DISPLAY_1_CAPABILITIES_H

#include "az_ulib_capability_api.h"
#include "az_ulib_result.h"
#include "display_1_model.h"

#ifdef __cplusplus
extern "C"
{
#endif

  static az_result display_1_cls_concrete(
      const display_1_cls_model_in* const in,
      display_1_cls_model_out* out);

  static az_result display_1_print_concrete(
      const display_1_print_model_in* const in,
      display_1_print_model_out* out);

  static az_result display_1_invalidate_concrete(
      const display_1_invalidate_model_in* const in,
      display_1_invalidate_model_out* out);

  static az_result display_1_max_x_concrete(
      const display_1_max_x_model* const in,
      display_1_max_x_model* out);

  static az_result display_1_max_y_concrete(
      const display_1_max_y_model* const in,
      display_1_max_y_model* out);

  static const az_ulib_capability_descriptor DISPLAY_1_CAPABILITIES[] = {
    AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(DISPLAY_1_CLS_COMMAND_NAME, display_1_cls_concrete, NULL),
    AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(DISPLAY_1_PRINT_COMMAND_NAME, display_1_print_concrete, NULL),
    AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
        DISPLAY_1_INVALIDATE_COMMAND_NAME,
        display_1_invalidate_concrete,
        NULL),
    AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
        DISPLAY_1_MAX_X_PROPERTY_NAME,
        display_1_max_x_concrete,
        NULL),
    AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(DISPLAY_1_MAX_Y_PROPERTY_NAME, display_1_max_y_concrete, NULL)
  };

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_1_CAPABILITIES_H */
