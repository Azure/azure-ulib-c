// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_pal_os_api.h"
#include "az_ulib_result.h"
#include "az_ulib_test_thread.h"
#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"

static TEST_MUTEX_HANDLE g_test_by_test;

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
  ASSERT_FAIL("umock_c reported error :%i", error_code);
}

static uint32_t my_property = 0;

static az_result get_my_property(const void* model_out)
{
  uint32_t* new_val = (uint32_t*)model_out;

  *new_val = my_property;

  return AZ_OK;
}

static az_result set_my_property(const void* const model_in)
{
  uint32_t* new_val = (uint32_t*)model_in;

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
static volatile long g_sum_sleep;

static az_result my_command(const void* const model_in, const void* model_out)
{
  my_command_model_in* in = (my_command_model_in*)model_in;
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
    const void* const model_in,
    const void* model_out,
    const az_ulib_capability_token capability_token,
    az_ulib_capability_cancellation_callback* cancel)
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

AZ_ULIB_DESCRIPTOR_CREATE(
    MY_INTERFACE_1_V123,
    "MY_INTERFACE_1",
    123,
    AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC("my_command_async", my_command_async, my_command_cancel));

AZ_ULIB_DESCRIPTOR_CREATE(
    MY_INTERFACE_1_V2,
    "MY_INTERFACE_1",
    2,
    AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC("my_command_async", my_command_async, my_command_cancel));

AZ_ULIB_DESCRIPTOR_CREATE(
    MY_INTERFACE_2_V123,
    "MY_INTERFACE_2",
    123,
    AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC("my_command_async", my_command_async, my_command_cancel));

AZ_ULIB_DESCRIPTOR_CREATE(
    MY_INTERFACE_3_V123,
    "MY_INTERFACE_3",
    123,
    AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC("my_command_async", my_command_async, my_command_cancel));

static az_ulib_ipc g_ipc;

void init_ipc_and_publish_interfaces(bool shall_initialize)
{
  if (shall_initialize)
  {
    ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_init(&g_ipc));
  }
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_1_V123, NULL));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_1_V2, NULL));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_2_V123, NULL));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_3_V123, NULL));
}

void unpublish_interfaces_and_deinit_ipc(void)
{
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
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
      (void)printf("get interface returned: %d\r\n", result);
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
            (void)printf("ipc call returned: %d\r\n", result);
          }
        }
        else if (out != AZ_OK)
        {
          result = local_result;
          (void)printf("command returned: %d\r\n", result);
        }
      }
    }

    az_result release_result;
    if ((release_result = az_ulib_ipc_release_interface(local_handle)) != AZ_OK)
    {
      (void)printf("release interface returned: %d\r\n", release_result);
      if (result == AZ_OK)
      {
        result = release_result;
      }
    }
  }

  return (int)result;
}

/**
 * Beginning of the E2E for interface module.
 */
BEGIN_TEST_SUITE(az_ulib_ipc_e2e)

TEST_SUITE_INITIALIZE(suite_init)
{
  g_test_by_test = TEST_MUTEX_CREATE();
  ASSERT_IS_NOT_NULL(g_test_by_test);

  ASSERT_ARE_EQUAL(int, 0, umock_c_init(on_umock_c_error));
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
  umock_c_deinit();

  TEST_MUTEX_DESTROY(g_test_by_test);
}

TEST_FUNCTION_INITIALIZE(test_command_initialize)
{
  if (TEST_MUTEX_ACQUIRE(g_test_by_test))
  {
    ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
  }

  g_sum_sleep = 0;
  g_lock_thread = 0;

  umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(test_command_cleanup) { TEST_MUTEX_RELEASE(g_test_by_test); }

TEST_FUNCTION(az_ulib_ipc_e2e_call_sync_command_succeed)
{
  /// arrange
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_SUM;
  in.max_sum = 10000;
  in.return_result = AZ_OK;
  az_result out = AZ_ULIB_PENDING;

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_EQUAL(int, AZ_OK, out);

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_e2e_unpublish_interface_in_the_call_failed)
{
  /// arrange
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_UNPUBLISH;
  in.descriptor = &MY_INTERFACE_1_V123;
  az_result out = AZ_ULIB_PENDING;

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ULIB_BUSY, out);

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_e2e_release_interface_in_the_call_succeed)
{
  /// arrange
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_RELEASE_INTERFACE;
  in.handle = interface_handle;
  az_result out = AZ_ULIB_PENDING;

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_EQUAL(int, AZ_OK, out);

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_e2e_deinit_ipc_in_the_call_failed)
{
  /// arrange
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_DEINIT;
  az_result out = AZ_ULIB_PENDING;

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ULIB_BUSY, out);

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_e2e_call_recursive_in_the_call_succeed)
{
  /// arrange
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_CALL_AGAIN;
  in.handle = interface_handle;
  in.command_index = MY_INTERFACE_COMMAND;
  az_result out = AZ_ULIB_PENDING;

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_EQUAL(int, AZ_OK, out);

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_e2e_unpublish_interface_before_call_succeed)
{
  /// arrange
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_UNPUBLISH;
  in.descriptor = &MY_INTERFACE_1_V123;
  az_result out = AZ_ULIB_PENDING;

  /// act
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ITEM_NOT_FOUND, result);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_PENDING, out);

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_deinit();
}

TEST_FUNCTION(az_ulib_ipc_e2e_release_after_unpublish_succeed)
{
  /// arrange
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_JUST_RETURN;
  in.return_result = AZ_OK;
  az_result out = AZ_ULIB_PENDING;

  /// act
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  az_result result = az_ulib_ipc_release_interface(interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_EQUAL(int, AZ_OK, out);

  /// cleanup
  az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_deinit();
}

TEST_FUNCTION(az_ulib_ipc_e2e_call_sync_command_in_multiple_threads_succeed)
{
  /// arrange
  g_thread_max_sum = 10;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  /// act
  THREAD_HANDLE thread_handle[MAX_THREAD];
  for (int i = 0; i < MAX_THREAD; i++)
  {
    (void)test_thread_create(&thread_handle[i], &call_sync_thread, interface_handle);
  }
  az_ulib_ipc_release_interface(interface_handle);

  /// assert
  for (int i = 0; i < MAX_THREAD; i++)
  {
    int res;
    test_thread_join(thread_handle[i], &res);
    ASSERT_ARE_EQUAL(int, AZ_OK, res);
  }

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_e2e_call_sync_command_in_multiple_threads_unpublish_timeout_failed)
{
  /// arrange
  g_thread_max_sum = 100;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
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
  az_ulib_ipc_release_interface(interface_handle);

  /// assert
  int res;
  test_thread_join(thread_handle, &res);
  ASSERT_ARE_EQUAL(int, AZ_OK, res);
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ULIB_BUSY, result);

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

TEST_FUNCTION(az_ulib_ipc_e2e_call_sync_command_in_multiple_threads_and_unpublish_succeed)
{
  /// arrange
  g_thread_max_sum = 30;
  g_sum_sleep = 10;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

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
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, 10000));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, 10000));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, 10000));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, 10000));
  az_ulib_ipc_release_interface(interface_handle);

  /// assert
  for (int i = 0; i < SMALL_NUMBER_THREAD; i++)
  {
    int res;
    test_thread_join(thread_handle[i], &res);
    ASSERT_ARE_EQUAL(int, AZ_ERROR_ITEM_NOT_FOUND, res);
  }

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

END_TEST_SUITE(az_ulib_ipc_e2e)
