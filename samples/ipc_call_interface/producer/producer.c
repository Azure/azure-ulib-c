// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_result.h"
#include "interfaces/math_interface.h"
#include <stdio.h>

void producer_start(void)
{
  (void)printf("Start producer...\r\n");
  az_ulib_result result;
  if ((result = math_publish_interface()) != AZ_ULIB_SUCCESS)
  {
    (void)printf("Producer publish math interface failed with error %d\r\n", result);
  }
  else
  {
    (void)printf("Producer publish math interface with success\r\n");
  }
}

void producer_end(void)
{
  (void)printf("End producer\r\n");
  math_unpublish_interface();
}
