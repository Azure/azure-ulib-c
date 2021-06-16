// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "sensors_v1i1.h"
#include "az_ulib_pal_os_api.h"
#include "az_ulib_result.h"
#include "sensors_1_model.h"
#include "sensors_v1i1_interface.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct
{
  az_ulib_callback_context context;
  az_ulib_telemetry_callback callback;
} subscription;

static bool end_thread = false;

static az_ulib_pal_thread_handle temperature_thread;
static subscription temperature_subscription = { 0 };
static uint32_t temperature_interval = 1000;
uint32_t sensors_v1i1_get_temperature_interval(void) { return temperature_interval; }
void sensors_v1i1_set_temperature_interval(const uint32_t val) { temperature_interval = val; }

static az_ulib_pal_thread_handle accelerometer_thread;
static subscription accelerometer_subscription = { 0 };
static uint32_t accelerometer_interval = 200;
uint32_t sensors_v1i1_get_accelerometer_interval(void) { return accelerometer_interval; }
void sensors_v1i1_set_accelerometer_interval(const uint32_t val) { accelerometer_interval = val; }

static az_ulib_pal_thread_ret read_and_notify_temperature(az_ulib_pal_thread_args args)
{
  (void)args;
  while (!end_thread)
  {
    (void)printf("Send temperature...\r\n");
    if (temperature_subscription.callback != NULL)
    {
      sensors_1_temperature_model_in in = { .t_c = 20 };
      temperature_subscription.callback(temperature_subscription.context, &in);
    }
    az_pal_os_sleep(temperature_interval);
  }
  return 0;
}

static az_ulib_pal_thread_ret read_and_notify_accelerometer(az_ulib_pal_thread_args args)
{
  (void)args;
  while (!end_thread)
  {
    (void)printf("Send accelerometer...\r\n");
    if (accelerometer_subscription.callback != NULL)
    {
      sensors_1_accelerometer_model_in in = { .x = 20, .y = 15, .z = 30 };
      accelerometer_subscription.callback(accelerometer_subscription.context, &in);
    }
    az_pal_os_sleep(accelerometer_interval);
  }
  return 0;
}

void sensors_v1i1_create(void)
{
  az_result result;

  (void)printf("Create producer for sensors v1i1...\r\n");

  if ((result = publish_sensors_v1i1_interface()) != AZ_OK)
  {
    (void)printf("Publish interface sensors 1 failed with error %" PRIi32 "\r\n", result);
  }
  else
  {
    (void)printf("Interface sensors 1 published with success\r\n");
  }

  if ((result = az_pal_os_thread_create(read_and_notify_temperature, NULL, &temperature_thread))
      != AZ_OK)
  {
    (void)printf("Notification thread for temperature failed with error %" PRIi32 "\r\n", result);
  }
  else
  {
    (void)printf("Notification thread for temperature started with success\r\n");
  }

  if ((result = az_pal_os_thread_create(read_and_notify_accelerometer, NULL, &accelerometer_thread))
      != AZ_OK)
  {
    (void)printf("Notification thread for accelerometer failed with error %" PRIi32 "\r\n", result);
  }
  else
  {
    (void)printf("Notification thread for accelerometer started with success\r\n");
  }
}

void sensors_v1i1_destroy(void)
{
  (void)printf("Destroy producer for sensors 1.\r\n");

  end_thread = true;
  az_pal_os_thread_join(temperature_thread, NULL);
  az_pal_os_thread_join(accelerometer_thread, NULL);

  unpublish_sensors_v1i1_interface();
}

az_result sensors_v1i1_subscribe_temperature(
    az_ulib_callback_context context,
    az_ulib_telemetry_callback callback)
{
  if (temperature_subscription.callback != NULL)
  {
    return AZ_ERROR_NOT_ENOUGH_SPACE;
  }

  temperature_subscription.context = context;
  temperature_subscription.callback = callback;

  return AZ_OK;
}

az_result sensors_v1i1_unsubscribe_temperature(
    az_ulib_callback_context context,
    az_ulib_telemetry_callback callback)
{
  if ((temperature_subscription.callback != callback)
      || (temperature_subscription.context != context))
  {
    return AZ_ERROR_ITEM_NOT_FOUND;
  }

  temperature_subscription.context = NULL;
  temperature_subscription.callback = NULL;

  return AZ_OK;
}

az_result sensors_v1i1_subscribe_accelerometer(
    az_ulib_callback_context context,
    az_ulib_telemetry_callback callback)
{
  if (accelerometer_subscription.callback != NULL)
  {
    return AZ_ERROR_NOT_ENOUGH_SPACE;
  }

  accelerometer_subscription.context = context;
  accelerometer_subscription.callback = callback;

  return AZ_OK;
}

az_result sensors_v1i1_unsubscribe_accelerometer(
    az_ulib_callback_context context,
    az_ulib_telemetry_callback callback)
{
  if ((accelerometer_subscription.callback != callback)
      || (accelerometer_subscription.context != context))
  {
    return AZ_ERROR_ITEM_NOT_FOUND;
  }

  accelerometer_subscription.context = NULL;
  accelerometer_subscription.callback = NULL;

  return AZ_OK;
}
