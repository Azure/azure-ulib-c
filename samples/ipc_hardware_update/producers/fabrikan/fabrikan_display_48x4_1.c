// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_result.h"
#include "fabrikan_display_48x4_bsp.h"
#include "interfaces/fabrikan_display_48x4_1_interface.h"
#include <stdio.h>

void fabrikan_display_48x4_1_create(void)
{
  (void)printf("Create Fabrikan producer for display 48x4 v1 ...\r\n");
  az_result result;
  if ((result = publish_fabrikan_display_48x4_1_interface()) != AZ_OK)
  {
    (void)printf("Fabrikan published display 48x4 v1 interface failed with error %d\r\n", result);
  }
  else
  {
    (void)printf("Fabrikan published display 48x4 v1 interface with success\r\n");
  }

  fabrikan_display_48x4_bsp_create();
}

void fabrikan_display_48x4_1_destroy(void)
{
  (void)printf("Destroy Fabrikan producer for display 48x4 v1.\r\n");
  unpublish_fabrikan_display_48x4_1_interface();

  fabrikan_display_48x4_bsp_destroy();
}
