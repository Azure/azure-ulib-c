// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "sensors_1.h"
#include "az_ulib_pal_api.h"
#include "az_ulib_result.h"
#include "sensors_1_capabilities.h"
#include "sensors_1_model.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define SENSORS_1_PACKAGE_NAME "sensors"
#define SENSORS_1_PACKAGE_VERSION 1

static const az_ulib_interface_descriptor SENSORS_1_DESCRIPTOR = AZ_ULIB_DESCRIPTOR_CREATE(
    SENSORS_1_PACKAGE_NAME,
    SENSORS_1_PACKAGE_VERSION,
    SENSORS_1_INTERFACE_NAME,
    SENSORS_1_INTERFACE_VERSION,
    SENSORS_1_CAPABILITIES);

typedef struct
{
  az_ulib_callback_context context;
  az_ulib_telemetry_callback callback;
} subscription;

typedef struct
{
  bool end_thread;
  az_ulib_pal_thread_handle temperature_thread;
  subscription temperature_subscription;
  uint32_t temperature_interval;
  az_ulib_pal_thread_handle accelerometer_thread;
  subscription accelerometer_subscription;
  uint32_t accelerometer_interval;
} sensor_1_cb;

static sensor_1_cb cb = { .end_thread = false,
                          .temperature_subscription = { 0 },
                          .temperature_interval = 1000,
                          .accelerometer_subscription = { 0 },
                          .accelerometer_interval = 200 };

static az_ulib_pal_thread_ret read_and_notify_temperature(az_ulib_pal_thread_args args)
{
  (void)args;
  while (!cb.end_thread)
  {
    (void)printf("Send temperature...\r\n");
    if (cb.temperature_subscription.callback != NULL)
    {
      sensors_1_temperature_model_in in = { .t_c = 20 };
      cb.temperature_subscription.callback(cb.temperature_subscription.context, &in);
    }
    az_pal_os_sleep(cb.temperature_interval);
  }
  return 0;
}

static az_ulib_pal_thread_ret read_and_notify_accelerometer(az_ulib_pal_thread_args args)
{
  (void)args;
  while (!cb.end_thread)
  {
    (void)printf("Send accelerometer...\r\n");
    if (cb.accelerometer_subscription.callback != NULL)
    {
      sensors_1_accelerometer_model_in in = { .x = 20, .y = 15, .z = 30 };
      cb.accelerometer_subscription.callback(cb.accelerometer_subscription.context, &in);
    }
    az_pal_os_sleep(cb.accelerometer_interval);
  }
  return 0;
}

static az_result sensors_1_subscribe_temperature_concrete(
    const az_ulib_telemetry_subscribe_model* const in,
    az_ulib_model_out* out)
{
  (void)out;

  if (cb.temperature_subscription.callback != NULL)
  {
    return AZ_ERROR_NOT_ENOUGH_SPACE;
  }

  cb.temperature_subscription.context = in->context;
  cb.temperature_subscription.callback = in->callback;

  return AZ_OK;
}

static az_result sensors_1_unsubscribe_temperature_concrete(
    const az_ulib_telemetry_subscribe_model* const in,
    az_ulib_model_out* out)
{
  (void)out;

  if ((cb.temperature_subscription.callback != in->callback)
      || (cb.temperature_subscription.context != in->context))
  {
    return AZ_ERROR_ITEM_NOT_FOUND;
  }

  cb.temperature_subscription.context = NULL;
  cb.temperature_subscription.callback = NULL;

  return AZ_OK;
}

static az_result sensors_1_subscribe_accelerometer_concrete(
    const az_ulib_telemetry_subscribe_model* const in,
    az_ulib_model_out* out)
{
  (void)out;

  if (cb.accelerometer_subscription.callback != NULL)
  {
    return AZ_ERROR_NOT_ENOUGH_SPACE;
  }

  cb.accelerometer_subscription.context = in->context;
  cb.accelerometer_subscription.callback = in->callback;

  return AZ_OK;
}

static az_result sensors_1_unsubscribe_accelerometer_concrete(
    const az_ulib_telemetry_subscribe_model* const in,
    az_ulib_model_out* out)
{
  (void)out;

  if ((cb.accelerometer_subscription.callback != in->callback)
      || (cb.accelerometer_subscription.context != in->context))
  {
    return AZ_ERROR_ITEM_NOT_FOUND;
  }

  cb.accelerometer_subscription.context = NULL;
  cb.accelerometer_subscription.callback = NULL;

  return AZ_OK;
}

static az_result sensors_1_temperature_interval_concrete(
    const sensors_1_temperature_interval_model* const in,
    sensors_1_temperature_interval_model* out)
{
  if (in != NULL)
  {
    cb.temperature_interval = *in;
  }

  if (out != NULL)
  {
    *out = cb.temperature_interval;
  }

  return AZ_OK;
}

static az_result sensors_1_accelerometer_interval_concrete(
    const sensors_1_accelerometer_interval_model* const in,
    sensors_1_accelerometer_interval_model* out)
{
  if (in != NULL)
  {
    cb.accelerometer_interval = *in;
  }

  if (out != NULL)
  {
    *out = cb.accelerometer_interval;
  }

  return AZ_OK;
}

void sensors_1_create(void)
{
  az_result result;

  (void)printf("Create package sensors.1...\r\n");

  if ((result = az_ulib_ipc_publish(&SENSORS_1_DESCRIPTOR)) != AZ_OK)
  {
    (void)printf("Publish interface sensors.1 failed with error %" PRIi32 "\r\n", result);
  }
  else
  {
    (void)printf("Interface sensors.1 published with success\r\n");
  }

  if ((result = az_pal_os_thread_create(read_and_notify_temperature, NULL, &cb.temperature_thread))
      != AZ_OK)
  {
    (void)printf("Notification thread for temperature failed with error %" PRIi32 "\r\n", result);
  }
  else
  {
    (void)printf("Notification thread for temperature started with success\r\n");
  }

  if ((result
       = az_pal_os_thread_create(read_and_notify_accelerometer, NULL, &cb.accelerometer_thread))
      != AZ_OK)
  {
    (void)printf("Notification thread for accelerometer failed with error %" PRIi32 "\r\n", result);
  }
  else
  {
    (void)printf("Notification thread for accelerometer started with success\r\n");
  }
}

void sensors_1_destroy(void)
{
  az_result result;

  cb.end_thread = true;
  az_pal_os_thread_join(cb.temperature_thread, NULL);
  az_pal_os_thread_join(cb.accelerometer_thread, NULL);

  if ((result = az_ulib_ipc_unpublish(&SENSORS_1_DESCRIPTOR, AZ_ULIB_WAIT_FOREVER)) != AZ_OK)
  {
    (void)printf("Unpublish interface sensors.1 failed with error %" PRIi32 "\r\n", result);
  }
  else
  {
    (void)printf("Destroy package sensors.1.\r\n");
  }
}
