// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from math v1 DL and shall not be
 * modified.
 ********************************************************************/

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include <stdint.h>

/*
 * interface definition
 */
#define MATH_1_INTERFACE_NAME "math"
#define MATH_1_INTERFACE_VERSION 1

#define MATH_1_INTERFACE_SUM_COMMAND (az_ulib_capability_index)0
#define MATH_1_INTERFACE_SUBTRACT_COMMAND (az_ulib_capability_index)1

/*
 * Define sum command on math interface.
 */
#define MATH_1_INTERFACE_SUM_COMMAND_NAME "sum"

typedef struct math_1_sum_model_in_tag
{
  int32_t a;
  int32_t b;
} math_1_sum_model_in;

typedef int64_t math_1_sum_model_out;

/*
 * Define subtraction command on math interface.
 */
#define MATH_1_INTERFACE_SUBTRACT_COMMAND_NAME "subtract"

typedef struct math_1_subtract_model_in_tag
{
  int32_t a;
  int32_t b;
} math_1_subtract_model_in;

typedef int64_t math_1_subtract_model_out;

/*
 * Publish math interface.
 */
az_result math_1_publish_interface(void);

/*
 * Unpublish math interface.
 */
az_result math_1_unpublish_interface(void);
