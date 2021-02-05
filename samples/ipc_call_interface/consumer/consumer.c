// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_result.h"
#include "wrappers/math_wrapper.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

static math_handle _handle;

void consumer_start(void)
{
  (void)printf("Start consumer...\r\n");
  az_result result;
  if ((result = math_create(&_handle)) != AZ_OK)
  {
    (void)printf("Client get math interface failed with code %d\r\n", result);
  }
  else
  {
    int32_t a = 10;
    int32_t b = 20;
    int64_t res = 0;

    if ((result = math_sum(_handle, a, b, &res)) == AZ_OK)
    {
      (void)printf("math.sum calculates %d + %d = %" PRId64 "\r\n", a, b, res);
    }
    else
    {
      (void)printf("math.sum failed with error %d\r\n", result);
    }

    if ((result = math_subtract(_handle, a, b, &res)) == AZ_OK)
    {
      (void)printf("math.subtract calculates %d - %d = %" PRId64 "\r\n", a, b, res);
    }
    else
    {
      (void)printf("math.subtract failed with error %d\r\n", result);
    }
  }
}

void consumer_end(void)
{
  (void)printf("End consumer\r\n");
  math_destroy(_handle);
}
