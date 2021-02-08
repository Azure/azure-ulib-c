// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "consumer.h"
#include "math_1_producer_1.h"
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
    (void)printf("Initialize IPC failed with code %d.\r\n", result);
  }
  else
  {
    /*
     * Publish the math interface. After this point anybody can call the math commands
     * through IPC.
     */
    math_1_producer_1_create();

    /*
     * Consumer will use the math interface.
     */
    consumer_create();

    /*
     * Consumer will stop to use the math interface.
     */
    consumer_destroy();

    /*
     * Unpublish math interface. After this point, any call to math will return
     * AZ_ERROR_ITEM_NOT_FOUND.
     */
    math_1_producer_1_destroy();
  }

  return 0;
}
