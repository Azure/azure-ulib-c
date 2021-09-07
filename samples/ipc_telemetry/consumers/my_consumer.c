// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "my_consumer.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "sensors_1_model.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#define SENSORS_1_PACKAGE_NAME "sensors"
static az_ulib_ipc_interface_handle _sensors_1 = { 0 };

static void temperature_callback(
    az_ulib_callback_context context,
    const sensors_1_temperature_model_in* const in)
{
  (void)context;
  (void)printf("Temperature:%" PRIi32 "c.\r\n", in->t_c);
}

static void accelerometer_callback(
    az_ulib_callback_context context,
    const sensors_1_accelerometer_model_in* const in)
{
  (void)context;
  (void)printf(
      "Accelerometer [x,y,z]:[%" PRIi32 ", %" PRIi32 ", %" PRIi32 "].\r\n", in->x, in->y, in->z);
}

void my_consumer_create(void)
{
  (void)printf("Create my consumer...\r\n");
  AZ_ULIB_TRY
  {
    (void)printf("Try get sensors interface.\r\n");
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_try_get_interface(
        AZ_SPAN_EMPTY,
        AZ_SPAN_FROM_STR(SENSORS_1_PACKAGE_NAME),
        AZ_ULIB_VERSION_DEFAULT,
        AZ_SPAN_FROM_STR(SENSORS_1_INTERFACE_NAME),
        SENSORS_1_INTERFACE_VERSION,
        &_sensors_1));

    (void)printf("Subscribe to temperature.\r\n");
    az_ulib_telemetry_subscribe_model temperature_in
        = { .context = NULL, .callback = (az_ulib_telemetry_callback)temperature_callback };
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_call(
        _sensors_1, SENSORS_1_SUBSCRIBE_TEMPERATURE_TELEMETRY, &temperature_in, NULL));

    (void)printf("Subscribe to accelerometer.\r\n");
    az_ulib_telemetry_subscribe_model accelerometer_in
        = { .context = NULL, .callback = (az_ulib_telemetry_callback)accelerometer_callback };
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_call(
        _sensors_1, SENSORS_1_SUBSCRIBE_ACCELEROMETER_TELEMETRY, &accelerometer_in, NULL));
  }
  AZ_ULIB_CATCH(...)
  {
    (void)printf("Create consumer failed with code %" PRIi32 "\r\n", AZ_ULIB_TRY_RESULT);
  }
}

void my_consumer_destroy(void)
{
  (void)printf("Destroy my consumer\r\n");
  AZ_ULIB_TRY
  {
    (void)printf("Unsubscribe from temperature.\r\n");
    az_ulib_telemetry_subscribe_model temperature_in
        = { .context = NULL, .callback = (az_ulib_telemetry_callback)temperature_callback };
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_call(
        _sensors_1, SENSORS_1_UNSUBSCRIBE_TEMPERATURE_TELEMETRY, &temperature_in, NULL));

    (void)printf("Unsubscribe from accelerometer.\r\n");
    az_ulib_telemetry_subscribe_model accelerometer_in
        = { .context = NULL, .callback = (az_ulib_telemetry_callback)accelerometer_callback };
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_call(
        _sensors_1, SENSORS_1_UNSUBSCRIBE_ACCELEROMETER_TELEMETRY, &accelerometer_in, NULL));

    (void)printf("Release sensors interface.\r\n");
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_release_interface(_sensors_1));
  }
  AZ_ULIB_CATCH(...)
  {
    (void)printf("Destroy consumer failed with code %" PRIi32 "\r\n", AZ_ULIB_TRY_RESULT);
  }
}
