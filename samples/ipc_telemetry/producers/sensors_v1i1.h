// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef SENSORS_V1I1_H
#define SENSORS_V1I1_H

#include "az_ulib_capability_api.h"
#include "az_ulib_result.h"

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

  void sensors_v1i1_create(void);
  void sensors_v1i1_destroy(void);

  uint32_t sensors_v1i1_get_temperature_interval(void);
  void sensors_v1i1_set_temperature_interval(const uint32_t val);

  az_result sensors_v1i1_subscribe_temperature(
      az_ulib_callback_context context,
      az_ulib_telemetry_callback callback);
  az_result sensors_v1i1_unsubscribe_temperature(
      az_ulib_callback_context context,
      az_ulib_telemetry_callback callback);

  uint32_t sensors_v1i1_get_accelerometer_interval(void);
  void sensors_v1i1_set_accelerometer_interval(const uint32_t val);

  az_result sensors_v1i1_subscribe_accelerometer(
      az_ulib_callback_context context,
      az_ulib_telemetry_callback callback);
  az_result sensors_v1i1_unsubscribe_accelerometer(
      az_ulib_callback_context context,
      az_ulib_telemetry_callback callback);

#ifdef __cplusplus
}
#endif

#endif /* SENSORS_V1I1_H */
