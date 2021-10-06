// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from sensors.1 DL and shall not be
 * modified.
 *
 * All concrete functions defined in this header shall be implemented
 * and compiled together with the interface.
 ********************************************************************/

#ifndef SENSORS_1_CAPABILITIES_H
#define SENSORS_1_CAPABILITIES_H

#include "az_ulib_capability_api.h"
#include "az_ulib_result.h"
#include "sensors_1_model.h"

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

  static az_result sensors_1_subscribe_temperature_concrete(
      const az_ulib_telemetry_subscribe_model* const in,
      az_ulib_model_out* out);

  static az_result sensors_1_unsubscribe_temperature_concrete(
      const az_ulib_telemetry_subscribe_model* const in,
      az_ulib_model_out* out);

  static az_result sensors_1_subscribe_accelerometer_concrete(
      const az_ulib_telemetry_subscribe_model* const in,
      az_ulib_model_out* out);

  static az_result sensors_1_unsubscribe_accelerometer_concrete(
      const az_ulib_telemetry_subscribe_model* const in,
      az_ulib_model_out* out);

  static az_result sensors_1_temperature_interval_concrete(
      const sensors_1_temperature_interval_model* const in,
      sensors_1_temperature_interval_model* out);

  static az_result sensors_1_accelerometer_interval_concrete(
      const sensors_1_accelerometer_interval_model* const in,
      sensors_1_accelerometer_interval_model* out);

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
              SENSORS_1_TEMPERATURE_INTERVAL_PROPERTY_NAME,
              sensors_1_temperature_interval_concrete,
              NULL),
          AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
              SENSORS_1_ACCELEROMETER_INTERVAL_PROPERTY_NAME,
              sensors_1_accelerometer_interval_concrete,
              NULL) };

#ifdef __cplusplus
}
#endif

#endif /* SENSORS_1_CAPABILITIES_H */
