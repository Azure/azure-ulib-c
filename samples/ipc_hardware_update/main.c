// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "contoso_display_20x4_1.h"
#include "fabrikan_display_48x4_1.h"
#include "my_consumer.h"
#include <inttypes.h>
#include <stdio.h>

static az_ulib_ipc ipc_handle;

/*
 * OS code.
 */
int main(void)
{
  az_result result;

  (void)printf("Start ipc_call_interface sample.\r\n\r\n");

  /*
   * Create the IPC. It shall be called at the very beginning of the application.
   * The IPC will prepare itself to receive interfaces.
   */
  if ((result = az_ulib_ipc_init(&ipc_handle)) != AZ_OK)
  {
    (void)printf("Initialize IPC failed with code %" PRIi32 ".\r\n", result);
  }
  else
  {
    /* Contoso publish display interface.
     * After this point anybody can call the display commands through IPC. */
    contoso_display_20x4_1_create();
    (void)printf("\r\n");

    /* Consumer will use the display interface. */
    my_consumer_create();
    (void)printf("\r\n");

    /* My consumer try to use display to add numbers. */
    my_consumer_do_display();
    my_consumer_do_display();
    my_consumer_do_display();
    (void)printf("\r\n");

    /* Unpublish display interface. After this point, any call to display will return
     * AZ_ERROR_ITEM_NOT_FOUND. */
    contoso_display_20x4_1_destroy();
    (void)printf("\r\n");

    /* My consumer try to use display to add numbers. */
    my_consumer_do_display(); // It will fail because the Contoso handle is not available anymore.
    my_consumer_do_display(); // It will fail because there is not display interface in IPC.
    (void)printf("\r\n");

    /* Fabrikan publish display interface.
     * After this point anybody can call the display commands through IPC. */
    fabrikan_display_48x4_1_create();
    (void)printf("\r\n");

    /* My consumer try to use display to add numbers. */
    my_consumer_do_display();
    my_consumer_do_display();
    my_consumer_do_display();
    (void)printf("\r\n");

    /* Unpublish display interface. After this point, any call to display will return
     * AZ_ERROR_ITEM_NOT_FOUND. */
    fabrikan_display_48x4_1_destroy();
    (void)printf("\r\n");

    /* Contoso publish display interface again.
     * After this point anybody can call the display commands through IPC. */
    contoso_display_20x4_1_create();
    (void)printf("\r\n");

    /* My consumer try to use display to add numbers. */
    my_consumer_do_display(); // It will fail because the handle is still from Fabrikan interface.
    my_consumer_do_display();
    my_consumer_do_display();
    my_consumer_do_display();
    (void)printf("\r\n");

    /* Unpublish display interface. After this point, any call to display will return
     * AZ_ERROR_ITEM_NOT_FOUND. */
    contoso_display_20x4_1_destroy();
    (void)printf("\r\n");

    /* Destroy consumer. */
    my_consumer_destroy();
    (void)printf("\r\n");
  }

  return 0;
}
