// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from sensors.1 DL and shall not be
 * modified.
 ********************************************************************/

#ifndef SENSORS_1_MODEL_H
#define SENSORS_1_MODEL_H

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

/*
 * interface definition
 */
#define SENSORS_1_INTERFACE_NAME "sensors"
#define SENSORS_1_INTERFACE_VERSION 1

/*
 * Define temperature telemetry on sensors interface.
 */
#define SENSORS_1_TEMPERATURE_TELEMETRY_NAME "temperature"
#define SENSORS_1_SUBSCRIBE_TEMPERATURE_TELEMETRY ((az_ulib_capability_index)0)
#define SENSORS_1_SUBSCRIBE_TEMPERATURE_TELEMETRY_NAME "subscribe_temperature"
#define SENSORS_1_UNSUBSCRIBE_TEMPERATURE_TELEMETRY ((az_ulib_capability_index)1)
#define SENSORS_1_UNSUBSCRIBE_TEMPERATURE_TELEMETRY_NAME "unsubscribe_temperature"
#define SENSORS_1_TEMPERATURE_CELSIUS_NAME "t_c"
  typedef struct
  {
    int32_t t_c;
  } sensors_1_temperature_model_in;

/*
 * Define accelerometer telemetry on sensors interface.
 */
#define SENSORS_1_ACCELEROMETER_TELEMETRY_NAME "accelerometer"
#define SENSORS_1_SUBSCRIBE_ACCELEROMETER_TELEMETRY ((az_ulib_capability_index)2)
#define SENSORS_1_SUBSCRIBE_ACCELEROMETER_TELEMETRY_NAME "subscribe_accelerometer"
#define SENSORS_1_UNSUBSCRIBE_ACCELEROMETER_TELEMETRY ((az_ulib_capability_index)3)
#define SENSORS_1_UNSUBSCRIBE_ACCELEROMETER_TELEMETRY_NAME "unsubscribe_accelerometer"
#define SENSORS_1_ACCELEROMETER_X_NAME "x"
#define SENSORS_1_ACCELEROMETER_Y_NAME "y"
#define SENSORS_1_ACCELEROMETER_Z_NAME "z"
  typedef struct
  {
    int32_t x;
    int32_t y;
    int32_t z;
  } sensors_1_accelerometer_model_in;

/*
 * Define reading temperature interval property on sensors interface.
 */
#define SENSORS_1_TEMPERATURE_INTERVAL_PROPERTY ((az_ulib_capability_index)4)
#define SENSORS_1_TEMPERATURE_INTERVAL_PROPERTY_NAME "temperature_interval"
  typedef uint32_t sensors_1_temperature_interval_model;

/*
 * Define reading accelerometer interval property on sensors interface.
 */
#define SENSORS_1_ACCELEROMETER_INTERVAL_PROPERTY ((az_ulib_capability_index)5)
#define SENSORS_1_ACCELEROMETER_INTERVAL_PROPERTY_NAME "accelerometer_interval"
  typedef uint32_t sensors_1_accelerometer_interval_model;

#ifdef __cplusplus
}
#endif

#endif /* SENSORS_1_MODEL_H */
