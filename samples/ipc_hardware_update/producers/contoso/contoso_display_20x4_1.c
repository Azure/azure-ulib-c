// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_result.h"
#include "contoso_display_20x4_bsp.h"
#include "interfaces/contoso_display_20x4_1_interface.h"
#include <stdio.h>

void contoso_display_20x4_1_create(void)
{
  (void)printf("Create Contoso producer for display 20x4 v1 ...\r\n");
  az_result result;
  if ((result = publish_contoso_display_20x4_1_interface()) != AZ_OK)
  {
    (void)printf("Contoso published display 20x4 v1 interface failed with error %d\r\n", result);
  }
  else
  {
    (void)printf("Contoso published display 20x4 v1 interface with success\r\n");
  }

  contoso_display_20x4_bsp_create();
}

void contoso_display_20x4_1_destroy(void)
{
  (void)printf("Destroy Contoso producer for display 20x4 v1.\r\n");
  unpublish_contoso_display_20x4_1_interface();

  contoso_display_20x4_bsp_destroy();
}
