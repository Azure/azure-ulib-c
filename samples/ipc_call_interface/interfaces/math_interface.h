// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from math meta-data and shall not be 
 * modified.
 ********************************************************************/

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include <stdint.h>

/*
 * interface definition
 */
#define MATH_INTERFACE_NAME "math"
#define MATH_INTERFACE_VERSION 1

typedef az_ulib_ipc_interface_handle math_handle;

#define MATH_INTERFACE_SUM_METHOD (az_ulib_action_index)0
#define MATH_INTERFACE_SUBTRACT_METHOD (az_ulib_action_index)1

/*
 * Define sum method on math interface.
 */
#define MATH_INTERFACE_SUM_METHOD_NAME "sum"

typedef struct sum_model_in_tag {
  int32_t a;
  int32_t b;
} sum_model_in;

typedef int64_t sum_model_out;

/*
 * Define subtraction method on math interface.
 */
#define MATH_INTERFACE_SUBTRACT_METHOD_NAME "subtract"

typedef struct subtract_model_in_tag {
  int32_t a;
  int32_t b;
} subtract_model_in;

typedef int64_t subtract_model_out;

/*
 * Publish math interface.
 */
az_ulib_result math_publish_interface(void);

/*
 * Unpublish math interface.
 */
az_ulib_result math_unpublish_interface(void);
