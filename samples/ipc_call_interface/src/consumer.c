// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_result.h"
#include "math_client.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

/*
 *client code.
 */
void client_use_math(void) {
  math_handle handle;
  az_ulib_result result;
  if ((result = math_create(&handle)) != AZ_ULIB_SUCCESS) {
    (void)printf("Client get math interface failed with code %d\r\n", result);
  } else {
    int32_t a = 10;
    int32_t b = 20;
    int64_t res = 0;

    if ((result = math_sum(handle, a, b, &res)) == AZ_ULIB_SUCCESS) {
      (void)printf("math.sum calculates %d + %d = %" PRId64 "\r\n", a, b, res);
    } else {
      (void)printf("math.sum failed with error %d\r\n", result);
    }

    if ((result = math_subtract(handle, a, b, &res)) == AZ_ULIB_SUCCESS) {
      (void)printf("math.subtract calculates %d - %d = %" PRId64 "\r\n", a, b, res);
    } else {
      (void)printf("math.subtract failed with error %d\r\n", result);
    }

    math_destroy(handle);
  }
}
