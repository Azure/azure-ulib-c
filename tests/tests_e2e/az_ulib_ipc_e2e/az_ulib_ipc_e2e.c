// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <inttypes.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_ipc_e2e.h"
#include "az_ulib_pal_os_api.h"
#include "az_ulib_result.h"
#include "az_ulib_test_thread.h"

#include "cmocka.h"

static uint32_t my_property = 0;

static az_result get_my_property(az_ulib_model_out model_out)
{
  uint32_t* new_val = (uint32_t*)model_out;

  *new_val = my_property;

  return AZ_OK;
}

static az_result set_my_property(az_ulib_model_in model_in)
{
  const uint32_t* const new_val = (const uint32_t* const)model_in;

  my_property = *new_val;

  return AZ_OK;
}

typedef struct my_command_model_in_tag
{
  uint8_t capability;
  uint32_t max_sum;
  const az_ulib_interface_descriptor* descriptor;
  az_ulib_ipc_interface_handle handle;
  az_ulib_capability_index command_index;
  az_result return_result;
} my_command_model_in;

typedef enum my_command_capability_tag
{
  MY_COMMAND_CAPABILITY_JUST_RETURN,
  MY_COMMAND_CAPABILITY_SUM,
  MY_COMMAND_CAPABILITY_UNPUBLISH,
  MY_COMMAND_CAPABILITY_RELEASE_INTERFACE,
  MY_COMMAND_CAPABILITY_DEINIT,
  MY_COMMAND_CAPABILITY_CALL_AGAIN,
  MY_COMMAND_CAPABILITY_RETURN_ERROR
} my_command_capability;

static volatile long g_is_running;
static volatile long g_lock_thread;
static volatile uint32_t g_sum_sleep;

static az_result my_command(az_ulib_model_in model_in, az_ulib_model_out model_out)
{
  const my_command_model_in* const in = (const my_command_model_in* const)model_in;
  az_result* result = (az_result*)model_out;
  my_command_model_in in_2;
  uint64_t sum = 0;

  (void)AZ_ULIB_PORT_ATOMIC_INC_W(&g_is_running);
  switch (in->capability)
  {
    case MY_COMMAND_CAPABILITY_JUST_RETURN:
      *result = in->return_result;
      break;
    case MY_COMMAND_CAPABILITY_SUM:
      while (g_lock_thread != 0)
      {
        az_pal_os_sleep(10);
      };
      for (uint32_t i = 0; i < in->max_sum; i++)
      {
        if (g_sum_sleep != 0)
        {
          az_pal_os_sleep(g_sum_sleep);
        }
        sum += i;
      }
      *result = in->return_result;
      break;
    case MY_COMMAND_CAPABILITY_UNPUBLISH:
      *result = az_ulib_ipc_unpublish(in->descriptor, AZ_ULIB_NO_WAIT);
      break;
    case MY_COMMAND_CAPABILITY_RELEASE_INTERFACE:
      *result = az_ulib_ipc_release_interface(in->handle);
      break;
    case MY_COMMAND_CAPABILITY_DEINIT:
      *result = az_ulib_ipc_deinit();
      break;
    case MY_COMMAND_CAPABILITY_CALL_AGAIN:
      in_2.capability = 0;
      in_2.return_result = AZ_OK;
      *result = az_ulib_ipc_call(in->handle, in->command_index, &in_2, model_out);
      break;
    default:
      *result = AZ_ERROR_ITEM_NOT_FOUND;
      break;
  }
  (void)AZ_ULIB_PORT_ATOMIC_DEC_W(&g_is_running);

  return AZ_OK;
}

static az_result my_command_async(
    az_ulib_model_in model_in,
    az_ulib_model_out model_out,
    const az_ulib_capability_token capability_token,
    const az_ulib_capability_cancellation_callback cancel)
{
  (void)model_in;
  (void)model_out;
  (void)capability_token;
  (void)cancel;

  return AZ_OK;
}

static az_result my_command_cancel(const az_ulib_capability_token capability_token)
{
  (void)capability_token;

  return AZ_OK;
}

typedef enum
{
  MY_INTERFACE_PROPERTY = 0,
  MY_INTERFACE_TELEMETRY = 1,
  MY_INTERFACE_TELEMETRY2 = 2,
  MY_INTERFACE_COMMAND = 3,
  MY_INTERFACE_COMMAND_ASYNC = 4
} my_interface_index;

static const az_ulib_capability_descriptor MY_INTERFACE_1_V123_CAPABILITIES[5] = {
  AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property, NULL, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
  AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
      "my_command_async",
      my_command_async,
      NULL,
      my_command_cancel)
};
static const az_ulib_interface_descriptor MY_INTERFACE_1_V123
    = AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE_1", 123, 5, MY_INTERFACE_1_V123_CAPABILITIES);

static const az_ulib_capability_descriptor MY_INTERFACE_1_V2_CAPABILITIES[5] = {
  AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property, NULL, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
  AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
      "my_command_async",
      my_command_async,
      NULL,
      my_command_cancel)
};
static const az_ulib_interface_descriptor MY_INTERFACE_1_V2
    = AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE_1", 2, 5, MY_INTERFACE_1_V2_CAPABILITIES);

static const az_ulib_capability_descriptor MY_INTERFACE_2_V123_CAPABILITIES[5] = {
  AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property, NULL, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
  AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
      "my_command_async",
      my_command_async,
      NULL,
      my_command_cancel)
};
static const az_ulib_interface_descriptor MY_INTERFACE_2_V123
    = AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE_2", 123, 5, MY_INTERFACE_2_V123_CAPABILITIES);

static const az_ulib_capability_descriptor MY_INTERFACE_3_V123_CAPABILITIES[5] = {
  AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property, NULL, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
  AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
      "my_command_async",
      my_command_async,
      NULL,
      my_command_cancel)
};
static const az_ulib_interface_descriptor MY_INTERFACE_3_V123
    = AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE_3", 123, 5, MY_INTERFACE_3_V123_CAPABILITIES);

static az_ulib_ipc g_ipc;

static void init_ipc_and_publish_interfaces(bool shall_initialize)
{
  if (shall_initialize)
  {
    assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  }
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_1_V123, NULL), AZ_OK);
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_1_V2, NULL), AZ_OK);
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_2_V123, NULL), AZ_OK);
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_3_V123, NULL), AZ_OK);
}

static void unpublish_interfaces_and_deinit_ipc(void)
{
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

#define NUMBER_CALLS_IN_THREAD 1000
#define MAX_THREAD (AZ_ULIB_CONFIG_MAX_IPC_INSTANCES - 1)
#define SMALL_NUMBER_THREAD (AZ_ULIB_CONFIG_MAX_IPC_INSTANCES >> 1)

static uint32_t g_thread_max_sum;

static int call_sync_thread(void* arg)
{
  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_SUM;
  in.max_sum = g_thread_max_sum;
  in.return_result = AZ_OK;

  az_ulib_ipc_interface_handle local_handle;

  az_result result = az_ulib_ipc_get_interface((az_ulib_ipc_interface_handle)arg, &local_handle);

  if (result != AZ_OK)
  {
    if (result != AZ_ERROR_ITEM_NOT_FOUND)
    {
      (void)printf("get interface returned: %" PRIi32 "\r\n", result);
    }
  }
  else
  {
    for (int i = 0; i < NUMBER_CALLS_IN_THREAD; i++)
    {
      az_result out = AZ_ULIB_PENDING;
      az_result local_result
          = az_ulib_ipc_call((az_ulib_ipc_interface_handle)arg, MY_INTERFACE_COMMAND, &in, &out);
      if (result == AZ_OK)
      {
        if (local_result != AZ_OK)
        {
          result = local_result;
          if (result != AZ_ERROR_ITEM_NOT_FOUND)
          {
            (void)printf("ipc call returned: %" PRIi32 "\r\n", result);
          }
        }
        else if (out != AZ_OK)
        {
          result = local_result;
          (void)printf("command returned: %" PRIi32 "\r\n", result);
        }
      }
    }

    az_result release_result;
    if ((release_result = az_ulib_ipc_release_interface(local_handle)) != AZ_OK)
    {
      (void)printf("release interface returned: %" PRIi32 "\r\n", release_result);
      if (result == AZ_OK)
      {
        result = release_result;
      }
    }
  }

  return (int)result;
}

static int setup(void** state)
{
  (void)state;

  g_sum_sleep = 0;
  g_lock_thread = 0;

  return 0;
}

/**
 * Beginning of the E2E for interface module.
 */
static void az_ulib_ipc_e2e_call_sync_command_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_SUM;
  in.max_sum = 10000;
  in.return_result = AZ_OK;
  az_result out = AZ_ULIB_PENDING;

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(out, AZ_OK);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_e2e_unpublish_interface_in_the_call_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_UNPUBLISH;
  in.descriptor = &MY_INTERFACE_1_V123;
  az_result out = AZ_ULIB_PENDING;

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(out, AZ_ERROR_ULIB_BUSY);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_e2e_release_interface_in_the_call_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_RELEASE_INTERFACE;
  in.handle = interface_handle;
  az_result out = AZ_ULIB_PENDING;

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(out, AZ_OK);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_e2e_deinit_ipc_in_the_call_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_DEINIT;
  az_result out = AZ_ULIB_PENDING;

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(out, AZ_ERROR_ULIB_BUSY);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_e2e_call_recursive_in_the_call_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_CALL_AGAIN;
  in.handle = interface_handle;
  in.command_index = MY_INTERFACE_COMMAND;
  az_result out = AZ_ULIB_PENDING;

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(out, AZ_OK);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_e2e_unpublish_interface_before_call_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_UNPUBLISH;
  in.descriptor = &MY_INTERFACE_1_V123;
  az_result out = AZ_ULIB_PENDING;

  /// act
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(out, AZ_ULIB_PENDING);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

static void az_ulib_ipc_e2e_release_after_unpublish_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_JUST_RETURN;
  in.return_result = AZ_OK;
  az_result out = AZ_ULIB_PENDING;

  /// act
  assert_int_equal(az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  az_result result = az_ulib_ipc_release_interface(interface_handle);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(out, AZ_OK);

  /// cleanup
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

static void az_ulib_ipc_e2e_call_sync_command_in_multiple_threads_succeed(void** state)
{
  /// arrange
  (void)state;
  g_thread_max_sum = 10;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);

  /// act
  THREAD_HANDLE thread_handle[MAX_THREAD];
  for (int i = 0; i < MAX_THREAD; i++)
  {
    (void)test_thread_create(&thread_handle[i], &call_sync_thread, interface_handle);
  }
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);

  /// assert
  for (int i = 0; i < MAX_THREAD; i++)
  {
    int res;
    test_thread_join(thread_handle[i], &res);
    assert_int_equal(res, AZ_OK);
  }

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_e2e_call_sync_command_in_multiple_threads_unpublish_timeout_failed(
    void** state)
{
  /// arrange
  (void)state;
  g_thread_max_sum = 100;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  THREAD_HANDLE thread_handle;

  g_is_running = 0; // Assume that the command is not running in the thread.

  (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_W(
      &g_lock_thread, 1); // Lock the command that will run in the thread to do not finish until we
                          // complete the test.

  /// act
  // Create the thread to call the command.
  (void)test_thread_create(&thread_handle, &call_sync_thread, interface_handle);

  // Wait for the command start to work.
  while (g_is_running == 0)
  {
  };

  // Try to unpublish the interface during the time that one of its command is running.
  az_result result = az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, 3);

  // As soon as the unpublish failed, release the command to end its execution.
  (void)AZ_ULIB_PORT_ATOMIC_DEC_W(&g_lock_thread);
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);

  /// assert
  int res;
  test_thread_join(thread_handle, &res);
  assert_int_equal(res, AZ_OK);
  assert_int_equal(result, AZ_ERROR_ULIB_BUSY);

  /// cleanup
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

static void az_ulib_ipc_e2e_call_sync_command_in_multiple_threads_and_unpublish_succeed(
    void** state)
{
  /// arrange
  (void)state;
  g_thread_max_sum = 30;
  g_sum_sleep = 10;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);

  g_is_running = 0; // Assume that the command is not running in the thread.

  (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_W(
      &g_lock_thread, 0); // Lock the command that will run in the thread to do not finish until we
                          // complete the test.

  /// act
  THREAD_HANDLE thread_handle[SMALL_NUMBER_THREAD];
  for (int count_thread = 0; count_thread < SMALL_NUMBER_THREAD; count_thread++)
  {
    az_pal_os_sleep(100);
    (void)test_thread_create(&thread_handle[count_thread], &call_sync_thread, interface_handle);
  }
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, 10000), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, 10000), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, 10000), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, 10000), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);

  /// assert
  for (int i = 0; i < SMALL_NUMBER_THREAD; i++)
  {
    int res;
    test_thread_join(thread_handle[i], &res);
    assert_int_equal(res, AZ_ERROR_ITEM_NOT_FOUND);
  }

  /// cleanup
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

int az_ulib_ipc_e2e()
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test_setup(az_ulib_ipc_e2e_call_sync_command_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_e2e_unpublish_interface_in_the_call_failed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_e2e_release_interface_in_the_call_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_e2e_deinit_ipc_in_the_call_failed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_e2e_call_recursive_in_the_call_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_e2e_unpublish_interface_before_call_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_e2e_release_after_unpublish_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_e2e_call_sync_command_in_multiple_threads_succeed, setup),
    cmocka_unit_test_setup(
        az_ulib_ipc_e2e_call_sync_command_in_multiple_threads_unpublish_timeout_failed, setup),
    cmocka_unit_test_setup(
        az_ulib_ipc_e2e_call_sync_command_in_multiple_threads_and_unpublish_succeed, setup),
  };

  return cmocka_run_group_tests_name("az_ulib_ipc_e2e", tests, NULL, NULL);
}
