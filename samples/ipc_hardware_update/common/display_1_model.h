// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from display v1 DL and shall not be
 * modified.
 ********************************************************************/

#ifndef DISPLAY_1_MODEL_H
#define DISPLAY_1_MODEL_H

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

/*
 * interface definition
 */
#define DISPLAY_1_INTERFACE_NAME "display"
#define DISPLAY_1_INTERFACE_VERSION 1
#define DISPLAY_1_CAPABILITY_SIZE 3

/*
 * Define cls (clear screen) command on display interface.
 */
#define DISPLAY_1_CLS_COMMAND (az_ulib_capability_index)0
#define DISPLAY_1_CLS_COMMAND_NAME "cls"
  typedef void display_1_cls_model_in;
  typedef void display_1_cls_model_out;

/*
 * Define print command on display interface.
 */
#define DISPLAY_1_PRINT_COMMAND (az_ulib_capability_index)1
#define DISPLAY_1_PRINT_COMMAND_NAME "print"
#define DISPLAY_1_PRINT_X_NAME "x"
#define DISPLAY_1_PRINT_Y_NAME "y"
#define DISPLAY_1_PRINT_BUFFER_NAME "buffer"
#define DISPLAY_1_PRINT_SIZE_NAME "size"
  typedef struct display_1_print_model_in_tag
  {
    int32_t x;
    int32_t y;
    const char* buffer;
    size_t size;
  } display_1_print_model_in;
  typedef void display_1_print_model_out;

/*
 * Define invalidate (refresh display) command on display interface.
 */
#define DISPLAY_1_INVALIDATE_COMMAND (az_ulib_capability_index)2
#define DISPLAY_1_INVALIDATE_COMMAND_NAME "invalidate"
  typedef void display_1_invalidate_model_in;
  typedef void display_1_invalidate_model_out;

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_1_MODEL_H */
