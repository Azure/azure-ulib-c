// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_result.h"
#include "wrappers/math_1_wrapper.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

static az_ulib_ipc_interface_handle _math_1;

void consumer_create(void)
{
  (void)printf("Create consumer...\r\n");
  az_result result;
  if ((result = math_1_create(&_math_1)) != AZ_OK)
  {
    (void)printf("Client get math.1 interface failed with code %d\r\n", result);
  }
  else
  {
    int32_t a = 10;
    int32_t b = 20;
    int64_t res = 0;

    if ((result = math_1_sum(_math_1, a, b, &res)) == AZ_OK)
    {
      (void)printf("math.1.sum calculates %d + %d = %" PRId64 "\r\n", a, b, res);
    }
    else
    {
      (void)printf("math.1.sum failed with error %d\r\n", result);
    }

    if ((result = math_1_subtract(_math_1, a, b, &res)) == AZ_OK)
    {
      (void)printf("math.1.subtract calculates %d - %d = %" PRId64 "\r\n", a, b, res);
    }
    else
    {
      (void)printf("math.1.subtract failed with error %d\r\n", result);
    }
  }
}

void consumer_destroy(void)
{
  (void)printf("Destroy consumer\r\n");
  math_1_destroy(_math_1);
}
