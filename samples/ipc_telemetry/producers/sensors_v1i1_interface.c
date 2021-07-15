// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from sensors v1 DL and shall not be
 * modified.
 ********************************************************************/

#include "sensors_v1i1_interface.h"
#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "sensors_1_model.h"
#include "sensors_v1i1.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static az_result sensors_1_subscribe_temperature_concrete(
    const az_ulib_telemetry_subscribe_model* const in,
    az_ulib_model_out* out)
{
  (void)out;
  return sensors_v1i1_subscribe_temperature(in->context, in->callback);
}

static az_result sensors_1_unsubscribe_temperature_concrete(
    const az_ulib_telemetry_subscribe_model* const in,
    az_ulib_model_out* out)
{
  (void)out;
  return sensors_v1i1_unsubscribe_temperature(in->context, in->callback);
}

static az_result sensors_1_subscribe_accelerometer_concrete(
    const az_ulib_telemetry_subscribe_model* const in,
    az_ulib_model_out* out)
{
  (void)out;
  return sensors_v1i1_subscribe_accelerometer(in->context, in->callback);
}

static az_result sensors_1_unsubscribe_accelerometer_concrete(
    const az_ulib_telemetry_subscribe_model* const in,
    az_ulib_model_out* out)
{
  (void)out;
  return sensors_v1i1_unsubscribe_accelerometer(in->context, in->callback);
}

static az_result sensors_1_get_temperature_interval_concrete(
    const sensors_1_temperature_interval_model* const in,
    sensors_1_temperature_interval_model* out)
{
  (void)in;
  *out = sensors_v1i1_get_temperature_interval();
  return AZ_OK;
}

static az_result sensors_1_set_temperature_interval_concrete(
    const sensors_1_temperature_interval_model* const in,
    sensors_1_temperature_interval_model* out)
{
  sensors_v1i1_set_temperature_interval(*in);
  if (out != NULL)
  {
    *out = sensors_v1i1_get_temperature_interval();
  }
  return AZ_OK;
}

static az_result sensors_1_get_accelerometer_interval_concrete(
    const sensors_1_accelerometer_interval_model* const in,
    sensors_1_accelerometer_interval_model* out)
{
  (void)in;
  *out = sensors_v1i1_get_accelerometer_interval();
  return AZ_OK;
}

static az_result sensors_1_set_accelerometer_interval_concrete(
    const sensors_1_accelerometer_interval_model* const in,
    sensors_1_accelerometer_interval_model* out)
{
  sensors_v1i1_set_accelerometer_interval(*in);
  if (out != NULL)
  {
    *out = sensors_v1i1_get_accelerometer_interval();
  }
  return AZ_OK;
}

static const az_ulib_capability_descriptor SENSORS_1_CAPABILITIES[]
    = { AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            SENSORS_1_SUBSCRIBE_TEMPERATURE_TELEMETRY_NAME,
            sensors_1_subscribe_temperature_concrete,
            NULL),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            SENSORS_1_UNSUBSCRIBE_TEMPERATURE_TELEMETRY_NAME,
            sensors_1_unsubscribe_temperature_concrete,
            NULL),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            SENSORS_1_SUBSCRIBE_ACCELEROMETER_TELEMETRY_NAME,
            sensors_1_subscribe_accelerometer_concrete,
            NULL),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            SENSORS_1_UNSUBSCRIBE_ACCELEROMETER_TELEMETRY_NAME,
            sensors_1_unsubscribe_accelerometer_concrete,
            NULL),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            SENSORS_1_GET_TEMPERATURE_INTERVAL_PROPERTY_NAME,
            sensors_1_get_temperature_interval_concrete,
            NULL),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            SENSORS_1_SET_TEMPERATURE_INTERVAL_PROPERTY_NAME,
            sensors_1_set_temperature_interval_concrete,
            NULL),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            SENSORS_1_GET_ACCELEROMETER_INTERVAL_PROPERTY_NAME,
            sensors_1_get_accelerometer_interval_concrete,
            NULL),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            SENSORS_1_SET_ACCELEROMETER_INTERVAL_PROPERTY_NAME,
            sensors_1_set_accelerometer_interval_concrete,
            NULL) };

static const az_ulib_interface_descriptor SENSORS_1_DESCRIPTOR = AZ_ULIB_DESCRIPTOR_CREATE(
    SENSORS_1_PACKAGE_NAME,
    SENSORS_1_PACKAGE_VERSION,
    SENSORS_1_INTERFACE_NAME,
    SENSORS_1_INTERFACE_VERSION,
    SENSORS_1_CAPABILITIES);

az_result publish_sensors_v1i1_interface(void)
{
  return az_ulib_ipc_publish(&SENSORS_1_DESCRIPTOR, NULL);
}

az_result unpublish_sensors_v1i1_interface(void)
{
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
  return az_ulib_ipc_unpublish(&SENSORS_1_DESCRIPTOR, AZ_ULIB_NO_WAIT);
#else
  return AZ_OK;
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
}
