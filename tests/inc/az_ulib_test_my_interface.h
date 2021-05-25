// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_TEST_MY_INTERFACE_H
#define AZ_ULIB_TEST_MY_INTERFACE_H

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"

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
#define MY_INTERFACE_1_123_INTERFACE_NAME "MY_INTERFACE_1"
#define MY_INTERFACE_1_123_INTERFACE_VERSION 123
#define MY_INTERFACE_1_123_CAPABILITY_SIZE 5

#define MY_INTERFACE_1_2_INTERFACE_NAME "MY_INTERFACE_1"
#define MY_INTERFACE_1_2_INTERFACE_VERSION 2
#define MY_INTERFACE_1_2_CAPABILITY_SIZE 5

#define MY_INTERFACE_2_123_INTERFACE_NAME "MY_INTERFACE_2"
#define MY_INTERFACE_2_123_INTERFACE_VERSION 123
#define MY_INTERFACE_2_123_CAPABILITY_SIZE 5

#define MY_INTERFACE_3_123_INTERFACE_NAME "MY_INTERFACE_3"
#define MY_INTERFACE_3_123_INTERFACE_VERSION 123
#define MY_INTERFACE_3_123_CAPABILITY_SIZE 5

/*
 * Define model of my_property in my_interface.
 */
#define MY_INTERFACE_MY_PROPERTY (az_ulib_capability_index)0
#define MY_INTERFACE_MY_PROPERTY_NAME "my_property"
#define MY_INTERFACE_MY_PROPERTY_VAL_NAME "val"
  typedef int32_t my_property_model;

/*
 * Define model of my_telemetry in my_interface.
 */
#define MY_INTERFACE_MY_TELEMETRY (az_ulib_capability_index)1
#define MY_INTERFACE_MY_TELEMETRY_NAME "my_telemetry"
#define MY_INTERFACE_MY_TELEMETRY_VAL_NAME "val"
  typedef int32_t my_telemetry_model;

/*
 * Define model of my_telemetry2 in my_interface.
 */
#define MY_INTERFACE_MY_TELEMETRY2 (az_ulib_capability_index)2
#define MY_INTERFACE_MY_TELEMETRY2_NAME "my_telemetry2"
#define MY_INTERFACE_MY_TELEMETRY2_VAL_NAME "val"
  typedef int32_t my_telemetry2_model;

/*
 * Define model of my_command in my_interface.
 */
#define MY_INTERFACE_MY_COMMAND (az_ulib_capability_index)3
#define MY_INTERFACE_MY_COMMAND_NAME "my_command"
#define MY_INTERFACE_MY_COMMAND_CAPABILITY_NAME "capability"
#define MY_INTERFACE_MY_COMMAND_MAX_SUM_NAME "max_sum"
#define MY_INTERFACE_MY_COMMAND_DESCRIPTOR_NAME "descriptor"
#define MY_INTERFACE_MY_COMMAND_WAIT_POLICY_MS_NAME "wait_policy_ms"
#define MY_INTERFACE_MY_COMMAND_HANDLE_NAME "handle"
#define MY_INTERFACE_MY_COMMAND_COMMAND_INDEX_NAME "command_index"
#define MY_INTERFACE_MY_COMMAND_RETURN_RESULT_NAME "return_result"
#define MY_INTERFACE_MY_COMMAND_RESULT_NAME "result"
  typedef enum
  {
    MY_COMMAND_CAPABILITY_JUST_RETURN = 0,
    MY_COMMAND_CAPABILITY_SUM,
    MY_COMMAND_CAPABILITY_UNPUBLISH,
    MY_COMMAND_CAPABILITY_RELEASE_INTERFACE,
    MY_COMMAND_CAPABILITY_DEINIT,
    MY_COMMAND_CAPABILITY_CALL_AGAIN,
    MY_COMMAND_CAPABILITY_RETURN_ERROR
  } my_command_capability;

  typedef struct
  {
    uint8_t capability;
    uint32_t max_sum;
    const az_ulib_interface_descriptor* descriptor;
    uint32_t wait_policy_ms;
    az_ulib_ipc_interface_handle handle;
    az_ulib_capability_index command_index;
    az_result return_result;
  } my_command_model_in;

  typedef az_result my_command_model_out;

/*
 * Define model of my_command_async in my_interface.
 */
#define MY_INTERFACE_MY_COMMAND_ASYNC (az_ulib_capability_index)4
#define MY_INTERFACE_MY_COMMAND_ASYNC_NAME "my_command_async"
#define MY_INTERFACE_MY_COMMAND_ASYNC_CAPABILITY_NAME "capability"
#define MY_INTERFACE_MY_COMMAND_ASYNC_DESCRIPTOR_NAME "descriptor"
#define MY_INTERFACE_MY_COMMAND_ASYNC_WAIT_POLICY_MS_NAME "wait_policy_ms"
#define MY_INTERFACE_MY_COMMAND_ASYNC_HANDLE_NAME "handle"
#define MY_INTERFACE_MY_COMMAND_ASYNC_COMMAND_INDEX_NAME "command_index"
#define MY_INTERFACE_MY_COMMAND_ASYNC_RETURN_RESULT_NAME "return_result"
#define MY_INTERFACE_MY_COMMAND_ASYNC_RESULT_NAME "result"
  typedef enum
  {
    MY_COMMAND_ASYNC_CAPABILITY_JUST_RETURN = 0,
    MY_COMMAND_ASYNC_CAPABILITY_UNPUBLISH,
    MY_COMMAND_ASYNC_CAPABILITY_RELEASE_INTERFACE,
    MY_COMMAND_ASYNC_CAPABILITY_DEINIT,
    MY_COMMAND_ASYNC_CAPABILITY_CALL_AGAIN,
    MY_COMMAND_ASYNC_CAPABILITY_RETURN_ERROR
  } my_command_async_capability;

  typedef struct
  {
    uint8_t capability;
    const az_ulib_interface_descriptor* descriptor;
    uint32_t wait_policy_ms;
    az_ulib_ipc_interface_handle handle;
    az_ulib_capability_index command_index;
    az_result return_result;
  } my_command_async_model_in;

  typedef az_result my_command_async_model_out;

  /*
   * Publish interfaces.
   */
  extern const az_ulib_interface_descriptor MY_INTERFACE_1_V123;
  az_result az_ulib_test_my_interface_1_v123_publish(
      az_ulib_ipc_interface_handle* interface_handle);
  az_result az_ulib_test_my_interface_1_v123_unpublish(uint32_t wait_ms);
  az_result az_ulib_test_my_interface_1_v2_publish(az_ulib_ipc_interface_handle* interface_handle);
  az_result az_ulib_test_my_interface_1_v2_unpublish(uint32_t wait_ms);
  az_result az_ulib_test_my_interface_2_v123_publish(
      az_ulib_ipc_interface_handle* interface_handle);
  az_result az_ulib_test_my_interface_2_v123_unpublish(uint32_t wait_ms);
  az_result az_ulib_test_my_interface_3_v123_publish(
      az_ulib_ipc_interface_handle* interface_handle);
  az_result az_ulib_test_my_interface_3_v123_unpublish(uint32_t wait_ms);
  az_result az_ulib_test_my_interface_publish(int index);
  az_result az_ulib_test_my_interface_unpublish(int index);

  extern volatile long g_is_running;
  extern volatile long g_lock_thread;
  extern volatile uint32_t g_sum_sleep;

#ifdef __cplusplus
}
#endif

#endif /* AZ_ULIB_TEST_MY_INTERFACE_H */
