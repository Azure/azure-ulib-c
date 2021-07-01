// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from sensors v1 DL and shall not be
 * modified.
 ********************************************************************/

#ifndef SENSORS_V1I1_INTERFACE_H
#define SENSORS_V1I1_INTERFACE_H

#include "az_ulib_result.h"

#ifdef __cplusplus
extern "C"
{
#else
#endif

#define SENSORS_1_PACKAGE_NAME "sensors"
#define SENSORS_1_PACKAGE_VERSION 1

  /*
   * Publish sensors interface.
   */
  az_result publish_sensors_v1i1_interface(void);

  /*
   * Unpublish sensors interface.
   */
  az_result unpublish_sensors_v1i1_interface(void);

#ifdef __cplusplus
}
#endif

#endif /* SENSORS_V1I1_INTERFACE_H */
