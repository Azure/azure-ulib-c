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
#include "az_ulib_query_1_model.h"
#include "az_ulib_result.h"
#include "az_ulib_test_my_interface.h"
#include "az_ulib_test_thread.h"

#include "cmocka.h"

#define IPC_QUERY_1_INTERFACE_NAME "ipc_" QUERY_1_INTERFACE_NAME

static az_ulib_ipc g_ipc;

static void init_ipc_and_publish_interfaces(bool shall_initialize)
{
  if (shall_initialize)
  {
    assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  }
  assert_int_equal(az_ulib_test_my_interface_1_v123_publish(NULL), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_1_v2_publish(NULL), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_2_v123_publish(NULL), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_3_v123_publish(NULL), AZ_OK);
}

static void unpublish_interfaces_and_deinit_ipc(void)
{
  assert_int_equal(az_ulib_test_my_interface_1_v123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_2_v123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_1_v2_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_3_v123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
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
          = az_ulib_ipc_call((az_ulib_ipc_interface_handle)arg, MY_INTERFACE_MY_COMMAND, &in, &out);
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
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_MY_COMMAND, &in, &out);

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
  in.wait_policy_ms = AZ_ULIB_NO_WAIT;
  in.descriptor = &MY_INTERFACE_1_V123;
  az_result out = AZ_ULIB_PENDING;

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_MY_COMMAND, &in, &out);

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
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_MY_COMMAND, &in, &out);

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
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_MY_COMMAND, &in, &out);

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
  in.command_index = MY_INTERFACE_MY_COMMAND;
  az_result out = AZ_ULIB_PENDING;

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_MY_COMMAND, &in, &out);

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
  in.wait_policy_ms = AZ_ULIB_NO_WAIT;
  in.descriptor = &MY_INTERFACE_1_V123;
  az_result out = AZ_ULIB_PENDING;

  /// act
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_MY_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(out, AZ_ULIB_PENDING);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_2_v123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_1_v2_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_3_v123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
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
  assert_int_equal(az_ulib_ipc_call(interface_handle, MY_INTERFACE_MY_COMMAND, &in, &out), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  az_result result = az_ulib_ipc_release_interface(interface_handle);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(out, AZ_OK);

  /// cleanup
  assert_int_equal(az_ulib_test_my_interface_2_v123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_1_v2_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_3_v123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
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
  unpublish_interfaces_and_deinit_ipc();
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
  assert_int_equal(az_ulib_test_my_interface_1_v123_unpublish(10000), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_2_v123_unpublish(10000), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_1_v2_unpublish(10000), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_3_v123_unpublish(10000), AZ_OK);
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

static void az_ulib_ipc_query_query_all_interfaces_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle query_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_FROM_STR(IPC_QUERY_1_INTERFACE_NAME),
          QUERY_1_INTERFACE_VERSION,
          AZ_ULIB_VERSION_EQUALS_TO,
          &query_handle),
      AZ_OK);

  query_1_query_model_in in = { .query = AZ_SPAN_FROM_STR("") };
  uint8_t buf[100];
  az_span query_result = AZ_SPAN_FROM_BUFFER(buf);
  query_1_query_model_out out = { .result = &query_result, .continuation_token = 0 };

  /// act
  az_result result = az_ulib_ipc_call(query_handle, QUERY_1_QUERY_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_true(az_span_is_content_equal(
      *out.result,
      AZ_SPAN_FROM_STR("\"ipc_query.1\",\"MY_INTERFACE_1.123\",\"MY_INTERFACE_1.2\",\"MY_INTERFACE_"
                       "2.123\",\"MY_INTERFACE_3.123\"")));
  assert_int_equal(out.continuation_token, 0x000a00ff);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(query_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_query_query_w_str_all_interfaces_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle query_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_FROM_STR(IPC_QUERY_1_INTERFACE_NAME),
          QUERY_1_INTERFACE_VERSION,
          AZ_ULIB_VERSION_EQUALS_TO,
          &query_handle),
      AZ_OK);

  az_span in = AZ_SPAN_LITERAL_FROM_STR("{}");
  uint8_t buf[300];
  az_span out = AZ_SPAN_FROM_BUFFER(buf);

  /// act
  az_result result = az_ulib_ipc_call_w_str(query_handle, QUERY_1_QUERY_COMMAND, in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_true(az_span_is_content_equal(
      out,
      AZ_SPAN_FROM_STR(
          "{\"result\":[\"ipc_query.1\",\"MY_INTERFACE_1.123\",\"MY_INTERFACE_1.2\",\"MY_INTERFACE_"
          "2.123\",\"MY_INTERFACE_3.123\"],\"continuation_token\":655615}")));

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(query_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_query_query_next_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle query_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_FROM_STR(IPC_QUERY_1_INTERFACE_NAME),
          QUERY_1_INTERFACE_VERSION,
          AZ_ULIB_VERSION_EQUALS_TO,
          &query_handle),
      AZ_OK);

  query_1_query_model_in query_in = { .query = AZ_SPAN_FROM_STR("") };
  uint8_t buf[50];

  /// act
  /// assert
  az_span query_result = AZ_SPAN_FROM_BUFFER(buf);
  query_1_query_model_out query_out = { .result = &query_result, .continuation_token = 0 };
  assert_int_equal(
      az_ulib_ipc_call(query_handle, QUERY_1_QUERY_COMMAND, &query_in, &query_out), AZ_OK);
  assert_true(az_span_is_content_equal(
      *query_out.result, AZ_SPAN_FROM_STR("\"ipc_query.1\",\"MY_INTERFACE_1.123\"")));
  assert_int_equal(query_out.continuation_token, 0x000200FF);

  query_1_next_model_in next_in = { .continuation_token = query_out.continuation_token };
  query_result = AZ_SPAN_FROM_BUFFER(buf);
  query_1_next_model_out next_out = { .result = &query_result, .continuation_token = 0 };
  assert_int_equal(
      az_ulib_ipc_call(query_handle, QUERY_1_NEXT_COMMAND, &next_in, &next_out), AZ_OK);
  assert_true(az_span_is_content_equal(
      *next_out.result, AZ_SPAN_FROM_STR("\"MY_INTERFACE_1.2\",\"MY_INTERFACE_2.123\"")));
  assert_int_equal(next_out.continuation_token, 0x000400FF);

  next_in.continuation_token = next_out.continuation_token;
  query_result = AZ_SPAN_FROM_BUFFER(buf); // reset az_span size.
  assert_int_equal(
      az_ulib_ipc_call(query_handle, QUERY_1_NEXT_COMMAND, &next_in, &next_out), AZ_OK);
  assert_true(
      az_span_is_content_equal(*next_out.result, AZ_SPAN_FROM_STR("\"MY_INTERFACE_3.123\"")));
  assert_int_equal(next_out.continuation_token, 0x000a00FF);

  next_in.continuation_token = next_out.continuation_token;
  query_result = AZ_SPAN_FROM_BUFFER(buf); // reset az_span size.
  assert_int_equal(
      az_ulib_ipc_call(query_handle, QUERY_1_NEXT_COMMAND, &next_in, &next_out), AZ_ULIB_EOF);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(query_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_query_query_next_w_str_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle query_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_FROM_STR(IPC_QUERY_1_INTERFACE_NAME),
          QUERY_1_INTERFACE_VERSION,
          AZ_ULIB_VERSION_EQUALS_TO,
          &query_handle),
      AZ_OK);

  uint8_t buf[170];

  /// act
  /// assert
  az_span in = AZ_SPAN_LITERAL_FROM_STR("{}");
  az_span out = AZ_SPAN_FROM_BUFFER(buf);
  assert_int_equal(az_ulib_ipc_call_w_str(query_handle, QUERY_1_QUERY_COMMAND, in, &out), AZ_OK);
  assert_true(az_span_is_content_equal(
      out,
      AZ_SPAN_FROM_STR("{\"result\":[\"ipc_query.1\",\"MY_INTERFACE_1.123\",\"MY_INTERFACE_1.2\"],"
                       "\"continuation_token\":196863}")));

  az_span in_1 = AZ_SPAN_LITERAL_FROM_STR("{\"continuation_token\":196863}");
  az_span out_1 = AZ_SPAN_FROM_BUFFER(buf);
  assert_int_equal(az_ulib_ipc_call_w_str(query_handle, QUERY_1_NEXT_COMMAND, in_1, &out_1), AZ_OK);
  assert_true(az_span_is_content_equal(
      out_1,
      AZ_SPAN_FROM_STR("{\"result\":[\"MY_INTERFACE_2.123\",\"MY_INTERFACE_3.123\"],"
                       "\"continuation_token\":655615}")));

  az_span in_2 = AZ_SPAN_LITERAL_FROM_STR("{\"continuation_token\":655615}");
  az_span out_2 = AZ_SPAN_FROM_BUFFER(buf);
  assert_int_equal(
      az_ulib_ipc_call_w_str(query_handle, QUERY_1_NEXT_COMMAND, in_2, &out_2), AZ_ULIB_EOF);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(query_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
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
    cmocka_unit_test_setup(az_ulib_ipc_query_query_all_interfaces_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_query_query_w_str_all_interfaces_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_query_query_next_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_query_query_next_w_str_succeed, setup),
  };

  return cmocka_run_group_tests_name("az_ulib_ipc_e2e", tests, NULL, NULL);
}
