// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_result.h"
#include "interfaces/math_1_interface.h"
#include <stdio.h>

void math_1_producer_1_create(void)
{
  (void)printf("Create producer v1 for math v1 ...\r\n");
  az_result result;
  if ((result = math_1_publish_interface()) != AZ_OK)
  {
    (void)printf("Producer v1 publish math v1 interface failed with error %d\r\n", result);
  }
  else
  {
    (void)printf("Producer v1 publish math v1 interface with success\r\n");
  }
}

void math_1_producer_1_destroy(void)
{
  (void)printf("Destroy producer v1 for math v1.\r\n");
  math_1_unpublish_interface();
}
