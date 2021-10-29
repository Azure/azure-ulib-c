// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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
#include "az_ulib_pal_api.h"
#include "az_ulib_query_1_model.h"
#include "az_ulib_registry_api.h"
#include "az_ulib_result.h"
#include "az_ulib_test_my_interface.h"
#include "az_ulib_test_thread.h"

#include "cmocka.h"

static az_ulib_ipc_control_block g_ipc;

static void init_ipc_and_publish_interfaces(bool shall_initialize)
{
  if (shall_initialize)
  {
    assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  }
  assert_int_equal(az_ulib_test_my_interface_a_1_1_123_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_b_1_1_123_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_c_1_1_123_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_1_200_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_2_123_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_3_123_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_2_1_123_publish(), AZ_OK);
}

static void unpublish_interfaces_and_deinit_ipc(void)
{
  assert_int_equal(az_ulib_test_my_interface_a_1_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_b_1_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_c_1_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_2_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_2_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_1_200_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_3_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

#define NUMBER_CALLS_IN_THREAD 1000
#define MAX_THREAD (AZ_ULIB_CONFIG_MAX_IPC_INSTANCES - 1)

static uint32_t g_thread_max_sum;

static int call_sync_thread(void* arg)
{
  (void)arg;
  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_SUM;
  in.max_sum = g_thread_max_sum;
  in.return_result = AZ_OK;

  az_ulib_ipc_interface_handle interface_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);

  az_result result = AZ_OK;

  for (int i = 0; i < NUMBER_CALLS_IN_THREAD; i++)
  {
    az_result out = AZ_ULIB_PENDING;
    az_result local_result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_MY_COMMAND, &in, &out);
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

  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);

  return (int)result;
}

#define REGISTRY_PAGE_SIZE 0x800

/* Static memory to store registry information. */
static uint8_t registry_buffer[REGISTRY_PAGE_SIZE * 2];
static uint8_t registry_informarmation_buffer[REGISTRY_PAGE_SIZE];

#define __REGISTRY_START (registry_buffer[0])
#define __REGISTRY_END (registry_buffer[(REGISTRY_PAGE_SIZE * 2)])
#define __REGISTRYINFO_START (registry_informarmation_buffer[0])
#define __REGISTRYINFO_END (registry_informarmation_buffer[REGISTRY_PAGE_SIZE])

static const az_ulib_registry_control_block registry_cb
    = { .registry_start = (void*)(&__REGISTRY_START),
        .registry_end = (void*)(&__REGISTRY_END),
        .registry_info_start = (void*)(&__REGISTRYINFO_START),
        .registry_info_end = (void*)(&__REGISTRYINFO_END),
        .page_size = REGISTRY_PAGE_SIZE };

static int setup(void** state)
{
  (void)state;

  /* Start Registry. */
  az_ulib_registry_init(&registry_cb);
  az_ulib_registry_clean_all();

  g_sum_sleep = 0;
  g_lock_thread = 0;

  return 0;
}

static int teardown(void** state)
{
  (void)state;

  az_ulib_registry_deinit();
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

  az_ulib_ipc_interface_handle interface_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);

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

  az_ulib_ipc_interface_handle interface_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_UNPUBLISH;
  in.wait_policy_ms = AZ_ULIB_NO_WAIT;
  in.descriptor = &MY_INTERFACE_A_1_1_123;
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

static void az_ulib_ipc_e2e_deinit_ipc_in_the_call_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);

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

  az_ulib_ipc_interface_handle interface_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_CALL_AGAIN;
  in.handle = interface_handle;
  in.capability_index = MY_INTERFACE_MY_COMMAND;
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

static void az_ulib_ipc_e2e_release_after_unpublish_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_JUST_RETURN;
  in.return_result = AZ_OK;
  az_result out = AZ_ULIB_PENDING;
  assert_int_equal(az_ulib_ipc_call(interface_handle, MY_INTERFACE_MY_COMMAND, &in, &out), AZ_OK);

  /// act
  az_result result = az_ulib_ipc_unpublish(&MY_INTERFACE_A_1_1_123, AZ_ULIB_NO_WAIT);

  /// assert
  assert_int_equal(result, AZ_ERROR_ULIB_BUSY);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_e2e_call_sync_command_in_multiple_threads_succeed(void** state)
{
  /// arrange
  (void)state;
  g_thread_max_sum = 10;
  init_ipc_and_publish_interfaces(true);

  /// act
  THREAD_HANDLE thread_handle[MAX_THREAD];
  for (int i = 0; i < MAX_THREAD; i++)
  {
    (void)test_thread_create(&thread_handle[i], &call_sync_thread, NULL);
  }

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

  THREAD_HANDLE thread_handle;

  g_is_running = 0; // Assume that the command is not running in the thread.

  (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_W(
      &g_lock_thread, 1); // Lock the command that will run in the thread to do not finish until we
                          // complete the test.

  /// act
  // Create the thread to call the command.
  (void)test_thread_create(&thread_handle, &call_sync_thread, NULL);

  // Wait for the command start to work.
  while (g_is_running == 0)
  {
  };

  // Try to unpublish the interface during the time that one of its command is running.
  az_result result = az_ulib_ipc_unpublish(&MY_INTERFACE_A_1_1_123, 3);

  // As soon as the unpublish failed, release the command to end its execution.
  (void)AZ_ULIB_PORT_ATOMIC_DEC_W(&g_lock_thread);

  /// assert
  int res;
  test_thread_join(thread_handle, &res);
  assert_int_equal(res, AZ_OK);
  assert_int_equal(result, AZ_ERROR_ULIB_BUSY);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_query_query_all_interfaces_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle query_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(IPC_1_PACKAGE_NAME),
          IPC_1_PACKAGE_VERSION,
          AZ_SPAN_FROM_STR(QUERY_1_INTERFACE_NAME),
          QUERY_1_INTERFACE_VERSION,
          &query_handle),
      AZ_ULIB_RENEW);

  az_span in = AZ_SPAN_FROM_STR("");
  uint8_t buf[500];
  az_span query_result = AZ_SPAN_FROM_BUFFER(buf);
  query_1_query_model_out out = { .result = &query_result, .continuation_token = 0 };

  /// act
  az_result result = az_ulib_ipc_call(query_handle, QUERY_1_QUERY_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_true(az_span_is_content_equal(
      *out.result,
      AZ_SPAN_FROM_STR(
          "\"*ipc.1.query.1\",\"*ipc.1.interface_manager.1\",\"*MY_PACKAGE_A.1.MY_INTERFACE_1."
          "123\",\"*MY_PACKAGE_B.1.MY_INTERFACE_1.123\",\"*MY_PACKAGE_C.1.MY_INTERFACE_1.123\",\"*"
          "MY_PACKAGE_A.1.MY_INTERFACE_1.200\",\"*MY_PACKAGE_A.1.MY_INTERFACE_2.123\",\"*MY_"
          "PACKAGE_A.1.MY_INTERFACE_3.123\",\" MY_PACKAGE_A.2.MY_INTERFACE_1.123\"")));
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

  az_ulib_ipc_interface_handle query_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(IPC_1_PACKAGE_NAME),
          IPC_1_PACKAGE_VERSION,
          AZ_SPAN_FROM_STR(QUERY_1_INTERFACE_NAME),
          QUERY_1_INTERFACE_VERSION,
          &query_handle),
      AZ_ULIB_RENEW);

  az_span in = AZ_SPAN_LITERAL_FROM_STR("{}");
  uint8_t buf[500];
  az_span out = AZ_SPAN_FROM_BUFFER(buf);

  /// act
  az_result result = az_ulib_ipc_call_with_str(query_handle, QUERY_1_QUERY_COMMAND, in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_true(az_span_is_content_equal(
      out,
      AZ_SPAN_FROM_STR(
          "{\"result\":[\"*ipc.1.query.1\",\"*ipc.1.interface_manager.1\",\"*MY_PACKAGE_A.1.MY_"
          "INTERFACE_1.123\",\"*MY_PACKAGE_B.1.MY_INTERFACE_1.123\",\"*MY_PACKAGE_C.1.MY_INTERFACE_"
          "1.123\",\"*MY_PACKAGE_A.1.MY_INTERFACE_1.200\",\"*MY_PACKAGE_A.1.MY_INTERFACE_2.123\","
          "\"*MY_PACKAGE_A.1.MY_INTERFACE_3.123\",\" MY_PACKAGE_A.2.MY_INTERFACE_1.123\"],"
          "\"continuation_token\":655615}")));

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(query_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_query_query_next_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle query_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(IPC_1_PACKAGE_NAME),
          IPC_1_PACKAGE_VERSION,
          AZ_SPAN_FROM_STR(QUERY_1_INTERFACE_NAME),
          QUERY_1_INTERFACE_VERSION,
          &query_handle),
      AZ_ULIB_RENEW);

  az_span query_in = AZ_SPAN_FROM_STR("");
  uint8_t buf[90];

  /// act
  /// assert
  az_span query_result = AZ_SPAN_FROM_BUFFER(buf);
  query_1_query_model_out query_out = { .result = &query_result, .continuation_token = 0 };
  assert_int_equal(
      az_ulib_ipc_call(query_handle, QUERY_1_QUERY_COMMAND, &query_in, &query_out), AZ_OK);
  assert_true(az_span_is_content_equal(
      *query_out.result,
      AZ_SPAN_FROM_STR("\"*ipc.1.query.1\",\"*ipc.1.interface_manager.1\",\"*MY_PACKAGE_A.1.MY_"
                       "INTERFACE_1.123\"")));
  assert_int_equal(query_out.continuation_token, 0x000300FF);

  query_1_next_model_in next_in = query_out.continuation_token;
  query_result = AZ_SPAN_FROM_BUFFER(buf);
  query_1_next_model_out next_out = { .result = &query_result, .continuation_token = 0 };
  assert_int_equal(
      az_ulib_ipc_call(query_handle, QUERY_1_NEXT_COMMAND, &next_in, &next_out), AZ_OK);
  assert_true(az_span_is_content_equal(
      *next_out.result,
      AZ_SPAN_FROM_STR(
          "\"*MY_PACKAGE_B.1.MY_INTERFACE_1.123\",\"*MY_PACKAGE_C.1.MY_INTERFACE_1.123\"")));
  assert_int_equal(next_out.continuation_token, 0x000500FF);

  next_in = next_out.continuation_token;
  query_result = AZ_SPAN_FROM_BUFFER(buf); // reset az_span size.
  assert_int_equal(
      az_ulib_ipc_call(query_handle, QUERY_1_NEXT_COMMAND, &next_in, &next_out), AZ_OK);
  assert_true(az_span_is_content_equal(
      *next_out.result,
      AZ_SPAN_FROM_STR(
          "\"*MY_PACKAGE_A.1.MY_INTERFACE_1.200\",\"*MY_PACKAGE_A.1.MY_INTERFACE_2.123\"")));
  assert_int_equal(next_out.continuation_token, 0x000700FF);

  next_in = next_out.continuation_token;
  query_result = AZ_SPAN_FROM_BUFFER(buf); // reset az_span size.
  assert_int_equal(
      az_ulib_ipc_call(query_handle, QUERY_1_NEXT_COMMAND, &next_in, &next_out), AZ_OK);
  assert_true(az_span_is_content_equal(
      *next_out.result,
      AZ_SPAN_FROM_STR(
          "\"*MY_PACKAGE_A.1.MY_INTERFACE_3.123\",\" MY_PACKAGE_A.2.MY_INTERFACE_1.123\"")));
  assert_int_equal(next_out.continuation_token, 0x000a00FF);

  next_in = next_out.continuation_token;
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

  az_ulib_ipc_interface_handle query_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(IPC_1_PACKAGE_NAME),
          IPC_1_PACKAGE_VERSION,
          AZ_SPAN_FROM_STR(QUERY_1_INTERFACE_NAME),
          QUERY_1_INTERFACE_VERSION,
          &query_handle),
      AZ_ULIB_RENEW);

  uint8_t buf[230]; // This buffer shall fit the JSON with 3 interfaces, so query next will have
                    // some more interfaces to report.

  /// act
  /// assert
  az_span in = AZ_SPAN_LITERAL_FROM_STR("{}");
  az_span out = AZ_SPAN_FROM_BUFFER(buf);
  assert_int_equal(az_ulib_ipc_call_with_str(query_handle, QUERY_1_QUERY_COMMAND, in, &out), AZ_OK);
  assert_true(az_span_is_content_equal(
      out,
      AZ_SPAN_FROM_STR("{\"result\":[\"*ipc.1.query.1\",\"*ipc.1.interface_manager.1\",\"*MY_"
                       "PACKAGE_A.1.MY_INTERFACE_1.123\"],\"continuation_token\":196863}")));

  az_span in_1 = AZ_SPAN_LITERAL_FROM_STR("{\"continuation_token\":196863}");
  az_span out_1 = AZ_SPAN_FROM_BUFFER(buf);
  assert_int_equal(
      az_ulib_ipc_call_with_str(query_handle, QUERY_1_NEXT_COMMAND, in_1, &out_1), AZ_OK);
  assert_true(az_span_is_content_equal(
      out_1,
      AZ_SPAN_FROM_STR(
          "{\"result\":[\"*MY_PACKAGE_B.1.MY_INTERFACE_1.123\",\"*MY_PACKAGE_C.1.MY_INTERFACE_1."
          "123\",\"*MY_PACKAGE_A.1.MY_INTERFACE_1.200\"],\"continuation_token\":393471}")));

  az_span in_2 = AZ_SPAN_LITERAL_FROM_STR("{\"continuation_token\":393471}");
  az_span out_2 = AZ_SPAN_FROM_BUFFER(buf);
  assert_int_equal(
      az_ulib_ipc_call_with_str(query_handle, QUERY_1_NEXT_COMMAND, in_2, &out_2), AZ_OK);
  assert_true(az_span_is_content_equal(
      out_2,
      AZ_SPAN_FROM_STR("{\"result\":[\"*MY_PACKAGE_A.1.MY_INTERFACE_2.123\",\"*MY_PACKAGE_A.1.MY_"
                       "INTERFACE_3.123\",\" MY_PACKAGE_A.2.MY_INTERFACE_1.123\"],\"continuation_"
                       "token\":655615}")));

  az_span in_4 = AZ_SPAN_LITERAL_FROM_STR("{\"continuation_token\":655615}");
  az_span out_4 = AZ_SPAN_FROM_BUFFER(buf);
  assert_int_equal(
      az_ulib_ipc_call_with_str(query_handle, QUERY_1_NEXT_COMMAND, in_4, &out_4), AZ_ULIB_EOF);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(query_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

int az_ulib_ipc_e2e()
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test_setup_teardown(az_ulib_ipc_e2e_call_sync_command_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_e2e_unpublish_interface_in_the_call_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_e2e_deinit_ipc_in_the_call_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_e2e_call_recursive_in_the_call_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_e2e_release_after_unpublish_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_e2e_call_sync_command_in_multiple_threads_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_e2e_call_sync_command_in_multiple_threads_unpublish_timeout_failed,
        setup,
        teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_query_query_all_interfaces_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_query_query_w_str_all_interfaces_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_query_query_next_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_query_query_next_w_str_succeed, setup, teardown),
  };

  return cmocka_run_group_tests_name("az_ulib_ipc_e2e", tests, NULL, NULL);
}
