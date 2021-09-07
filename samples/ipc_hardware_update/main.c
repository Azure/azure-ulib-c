// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "contoso_display_200401.h"
#include "contoso_display_480401.h"
#include "my_consumer.h"
#include <inttypes.h>
#include <stdio.h>

static az_ulib_ipc_control_block ipc_control_block;
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
  if ((result = az_ulib_ipc_init(&ipc_control_block)) != AZ_OK)
  {
    (void)printf("Initialize IPC failed with code %" PRIi32 ".\r\n", result);
  }
  else
  {
    /* Contoso publish display interface.
     * After this point anybody can call the display commands through IPC. */
    contoso_display_200401_create();
    (void)printf("\r\n");

    /* Consumer will use the display interface in a new thread. */
    my_consumer_create();
    (void)printf("\r\n");

    /* Give some time to consumer to play with the display. */
    az_pal_os_sleep(5000);
    (void)printf("\r\n");

    /* Unpublish display interface. After this point, any call to display will return
     * AZ_ERROR_ITEM_NOT_FOUND. */
    contoso_display_200401_destroy();
    (void)printf("\r\n");

    /* My consumer try to use display. */
    az_pal_os_sleep(2000); // It will fail because the Contoso handle is not available anymore.
    (void)printf("\r\n");

    /* Contoso publish another display interface.
     * After this point anybody can call the display commands through IPC. */
    contoso_display_480401_create();
    (void)printf("\r\n");

    /* Give some more time to consumer to play with the display. */
    az_pal_os_sleep(5000);
    (void)printf("\r\n");

    /* Unpublish display interface. After this point, any call to display will return
     * AZ_ERROR_ITEM_NOT_FOUND. */
    contoso_display_480401_destroy();
    (void)printf("\r\n");

    /* Contoso publish display interface again.
     * After this point anybody can call the display commands through IPC. */
    contoso_display_200401_create();
    (void)printf("\r\n");

    /* Give some more time to consumer to play with the display. */
    az_pal_os_sleep(5000);
    (void)printf("\r\n");

    /* Unpublish display interface. After this point, any call to display will return
     * AZ_ERROR_ITEM_NOT_FOUND. */
    contoso_display_200401_destroy();
    (void)printf("\r\n");

    /* Destroy consumer. */
    my_consumer_destroy();
    (void)printf("\r\n");
  }

  return 0;
}
