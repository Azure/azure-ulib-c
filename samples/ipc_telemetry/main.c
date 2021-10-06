// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_ipc_api.h"
#include "az_ulib_pal_os_api.h"
#include "az_ulib_result.h"
#include "my_consumer.h"
#include "sensors_1.h"
#include <inttypes.h>
#include <stdio.h>

static az_ulib_ipc_control_block ipc_control_block;

/*
 * OS code.
 */
int main(void)
{
  az_result result;

  (void)printf("Start ipc_telemetry sample.\r\n\r\n");

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
    /* Publish sensors.1 interface.
     * After this point anybody can subscribe for telemetries in sensors.1.
     * To simulate sensors reading, sensor_v1i1 will spin a thread.
     */
    sensors_1_create();
    (void)printf("\r\n");

    /* Consumer will use the sensors.1 interface. */
    my_consumer_create();
    (void)printf("\r\n");

    /* Run the telemetry for 5 seconds. */
    az_pal_os_sleep(5000);

    /* Destroy consumer to unsubscribe from the telemetries in sensors.1 interface. */
    my_consumer_destroy();
    (void)printf("\r\n");

    /* Wait 2 seconds to check that there are no more telemetries. */
    az_pal_os_sleep(2000);

    /* Consumer subscribe to telemetries again. */
    my_consumer_create();
    (void)printf("\r\n");

    /* Run the telemetry for another 5 seconds. */
    az_pal_os_sleep(5000);

    /* Consumer will stop to use the sensors.1 interface. */
    my_consumer_destroy();
    (void)printf("\r\n");

    /* Unpublish sensors.1. After this point, any call to sensors.1 will return
     * AZ_ERROR_ITEM_NOT_FOUND and the telemetry will stop.*/
    sensors_1_destroy();
    (void)printf("\r\n");
  }

  return 0;
}
