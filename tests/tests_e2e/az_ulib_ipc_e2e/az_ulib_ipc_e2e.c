// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "az_ulib_pal_os_api.h"
#include "az_ulib_action_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "azure_macro_utils/macro_utils.h"
#include "az_ulib_test_thread.h"
#include "testrunnerswitcher.h"
#include "az_ulib_result.h"
#include "umock_c/umock_c.h"

static TEST_MUTEX_HANDLE g_test_by_test;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code) {
  ASSERT_FAIL("umock_c reported error :%s", MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
}

static uint32_t my_property = 0;

static AZ_ULIB_RESULT get_my_property(const void* model_out) {
  uint32_t* new_val = (uint32_t*)model_out;

  *new_val = my_property;

  return AZ_ULIB_SUCCESS;
}

static AZ_ULIB_RESULT set_my_property(const void* const model_in) {
  uint32_t* new_val = (uint32_t*)model_in;

  my_property = *new_val;

  return AZ_ULIB_SUCCESS;
}

typedef struct my_method_model_in_tag {
  uint8_t action;
  uint32_t max_sum;
  const az_ulib_interface_descriptor* descriptor;
  az_ulib_ipc_interface_handle handle;
  az_ulib_action_index method_index;
  AZ_ULIB_RESULT return_result;
} my_method_model_in;

MU_DEFINE_ENUM(
    my_method_action,
    MY_METHOD_ACTION_JUST_RETURN,
    MY_METHOD_ACTION_SUM,
    MY_METHOD_ACTION_UNPUBLISH,
    MY_METHOD_ACTION_RELEASE_INTERFACE,
    MY_METHOD_ACTION_DEINIT,
    MY_METHOD_ACTION_CALL_AGAIN,
    MY_METHOD_ACTION_RETURN_ERROR)

static volatile long g_is_running;
static volatile long g_lock_thread;
static volatile long g_sum_sleep;

static AZ_ULIB_RESULT my_method(const void* const model_in, const void* model_out) {
  my_method_model_in* in = (my_method_model_in*)model_in;
  AZ_ULIB_RESULT* result = (AZ_ULIB_RESULT*)model_out;
  my_method_model_in in_2;
  uint64_t sum = 0;

  (void)AZ_ULIB_PORT_ATOMIC_INC_W(&g_is_running);
  switch (in->action) {
    case MY_METHOD_ACTION_JUST_RETURN:
      *result = in->return_result;
      break;
    case MY_METHOD_ACTION_SUM:
      while (g_lock_thread != 0) {
        az_pal_os_sleep(10);
      };
      for (uint32_t i = 0; i < in->max_sum; i++) {
        if (g_sum_sleep != 0) {
          az_pal_os_sleep(g_sum_sleep);
        }
        sum += i;
      }
      *result = in->return_result;
      break;
    case MY_METHOD_ACTION_UNPUBLISH:
      *result = az_ulib_ipc_unpublish(in->descriptor, AZ_ULIB_NO_WAIT);
      break;
    case MY_METHOD_ACTION_RELEASE_INTERFACE:
      *result = az_ulib_ipc_release_interface(in->handle);
      break;
    case MY_METHOD_ACTION_DEINIT:
      *result = az_ulib_ipc_deinit();
      break;
    case MY_METHOD_ACTION_CALL_AGAIN:
      in_2.action = 0;
      in_2.return_result = AZ_ULIB_SUCCESS;
      *result = az_ulib_ipc_call(in->handle, in->method_index, &in_2, model_out);
      break;
    default:
      *result = AZ_ULIB_NO_SUCH_ELEMENT_ERROR;
      break;
  }
  (void)AZ_ULIB_PORT_ATOMIC_DEC_W(&g_is_running);

  return AZ_ULIB_SUCCESS;
}

static AZ_ULIB_RESULT my_method_async(
    const void* const model_in,
    const void* model_out,
    const az_ulib_action_token action_token,
    az_ulib_action_cancellation_callback* cancel) {
  (void)model_in;
  (void)model_out;
  (void)action_token;
  (void)cancel;

  return AZ_ULIB_SUCCESS;
}

static AZ_ULIB_RESULT my_method_cancel(const az_ulib_action_token action_token) {
  (void)action_token;

  return AZ_ULIB_SUCCESS;
}

typedef enum {
  MY_INTERFACE_PROPERTY = 0,
  MY_INTERFACE_EVENT = 1,
  MY_INTERFACE_EVENT2 = 2,
  MY_INTERFACE_METHOD = 3,
  MY_INTERFACE_METHOD_ASYNC = 4
} my_interface_index;

AZ_ULIB_DESCRIPTOR_CREATE(
    MY_INTERFACE_1_V123,
    "MY_INTERFACE_1",
    123,
    AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property),
    AZ_ULIB_DESCRIPTOR_ADD_EVENT("my_event"),
    AZ_ULIB_DESCRIPTOR_ADD_EVENT("my_event2"),
    AZ_ULIB_DESCRIPTOR_ADD_METHOD("my_method", my_method),
    AZ_ULIB_DESCRIPTOR_ADD_METHOD_ASYNC("my_method_async", my_method_async, my_method_cancel));

AZ_ULIB_DESCRIPTOR_CREATE(
    MY_INTERFACE_1_V2,
    "MY_INTERFACE_1",
    2,
    AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property),
    AZ_ULIB_DESCRIPTOR_ADD_EVENT("my_event"),
    AZ_ULIB_DESCRIPTOR_ADD_EVENT("my_event2"),
    AZ_ULIB_DESCRIPTOR_ADD_METHOD("my_method", my_method),
    AZ_ULIB_DESCRIPTOR_ADD_METHOD_ASYNC("my_method_async", my_method_async, my_method_cancel));

AZ_ULIB_DESCRIPTOR_CREATE(
    MY_INTERFACE_2_V123,
    "MY_INTERFACE_2",
    123,
    AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property),
    AZ_ULIB_DESCRIPTOR_ADD_EVENT("my_event"),
    AZ_ULIB_DESCRIPTOR_ADD_EVENT("my_event2"),
    AZ_ULIB_DESCRIPTOR_ADD_METHOD("my_method", my_method),
    AZ_ULIB_DESCRIPTOR_ADD_METHOD_ASYNC("my_method_async", my_method_async, my_method_cancel));

AZ_ULIB_DESCRIPTOR_CREATE(
    MY_INTERFACE_3_V123,
    "MY_INTERFACE_3",
    123,
    AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property),
    AZ_ULIB_DESCRIPTOR_ADD_EVENT("my_event"),
    AZ_ULIB_DESCRIPTOR_ADD_EVENT("my_event2"),
    AZ_ULIB_DESCRIPTOR_ADD_METHOD("my_method", my_method),
    AZ_ULIB_DESCRIPTOR_ADD_METHOD_ASYNC("my_method_async", my_method_async, my_method_cancel));

static az_ulib_ipc g_ipc;

void init_ipc_and_publish_interfaces(bool shall_initialize) {
  if (shall_initialize) {
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_init(&g_ipc));
  }
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_publish(&MY_INTERFACE_1_V123, NULL));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_publish(&MY_INTERFACE_1_V2, NULL));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_publish(&MY_INTERFACE_2_V123, NULL));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_publish(&MY_INTERFACE_3_V123, NULL));
}

void unpublish_interfaces_and_deinit_ipc(void) {
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_deinit());
}

#define NUMBER_CALLS_IN_THREAD 1000
#define MAX_THREAD (AZ_ULIB_CONFIG_MAX_IPC_INSTANCES - 1)
#define SMALL_NUMBER_THREAD (AZ_ULIB_CONFIG_MAX_IPC_INSTANCES >> 1)

static uint32_t g_thread_max_sum;

static int call_sync_thread(void* arg) {
  my_method_model_in in;
  in.action = MY_METHOD_ACTION_SUM;
  in.max_sum = g_thread_max_sum;
  in.return_result = AZ_ULIB_SUCCESS;

  az_ulib_ipc_interface_handle local_handle;

  AZ_ULIB_RESULT result
      = az_ulib_ipc_get_interface((az_ulib_ipc_interface_handle)arg, &local_handle);

  if (result != AZ_ULIB_SUCCESS) {
    if (result != AZ_ULIB_NO_SUCH_ELEMENT_ERROR) {
      (void)printf("get interface returned: %d\r\n", result);
    }
  } else {
    for (int i = 0; i < NUMBER_CALLS_IN_THREAD; i++) {
      AZ_ULIB_RESULT out = AZ_ULIB_PENDING;
      AZ_ULIB_RESULT local_result
          = az_ulib_ipc_call((az_ulib_ipc_interface_handle)arg, MY_INTERFACE_METHOD, &in, &out);
      if (result == AZ_ULIB_SUCCESS) {
        if (local_result != AZ_ULIB_SUCCESS) {
          result = local_result;
          if (result != AZ_ULIB_NO_SUCH_ELEMENT_ERROR) {
            (void)printf("ipc call returned: %d\r\n", result);
          }
        } else if (out != AZ_ULIB_SUCCESS) {
          result = local_result;
          (void)printf("method returned: %d\r\n", result);
        }
      }
    }

    AZ_ULIB_RESULT release_result;
    if ((release_result = az_ulib_ipc_release_interface(local_handle)) != AZ_ULIB_SUCCESS) {
      (void)printf("release interface returned: %d\r\n", release_result);
      if (result == AZ_ULIB_SUCCESS) {
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

TEST_SUITE_INITIALIZE(suite_init) {
  g_test_by_test = TEST_MUTEX_CREATE();
  ASSERT_IS_NOT_NULL(g_test_by_test);

  ASSERT_ARE_EQUAL(int, 0, umock_c_init(on_umock_c_error));
}

TEST_SUITE_CLEANUP(suite_cleanup) {
  umock_c_deinit();

  TEST_MUTEX_DESTROY(g_test_by_test);
}

TEST_FUNCTION_INITIALIZE(test_method_initialize) {
  if (TEST_MUTEX_ACQUIRE(g_test_by_test)) {
    ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
  }

  g_sum_sleep = 0;
  g_lock_thread = 0;

  umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(test_method_cleanup) { TEST_MUTEX_RELEASE(g_test_by_test); }

TEST_FUNCTION(az_ulib_ipc_e2e_call_sync_method_succeed) {
  /// arrange
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  my_method_model_in in;
  in.action = MY_METHOD_ACTION_SUM;
  in.max_sum = 10000;
  in.return_result = AZ_ULIB_SUCCESS;
  AZ_ULIB_RESULT out = AZ_ULIB_PENDING;

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_METHOD, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, out);

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_e2e_unpublish_interface_in_the_call_failed) {
  /// arrange
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  my_method_model_in in;
  in.action = MY_METHOD_ACTION_UNPUBLISH;
  in.descriptor = &MY_INTERFACE_1_V123;
  AZ_ULIB_RESULT out = AZ_ULIB_PENDING;

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_METHOD, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_BUSY_ERROR, out);

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_e2e_release_interface_in_the_call_succeed) {
  /// arrange
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  my_method_model_in in;
  in.action = MY_METHOD_ACTION_RELEASE_INTERFACE;
  in.handle = interface_handle;
  AZ_ULIB_RESULT out = AZ_ULIB_PENDING;

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_METHOD, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, out);

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_e2e_deinit_ipc_in_the_call_failed) {
  /// arrange
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  my_method_model_in in;
  in.action = MY_METHOD_ACTION_DEINIT;
  AZ_ULIB_RESULT out = AZ_ULIB_PENDING;

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_METHOD, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_BUSY_ERROR, out);

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_e2e_call_recursive_in_the_call_succeed) {
  /// arrange
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  my_method_model_in in;
  in.action = MY_METHOD_ACTION_CALL_AGAIN;
  in.handle = interface_handle;
  in.method_index = MY_INTERFACE_METHOD;
  AZ_ULIB_RESULT out = AZ_ULIB_PENDING;

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_METHOD, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, out);

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_e2e_unpublish_interface_before_call_succeed) {
  /// arrange
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  my_method_model_in in;
  in.action = MY_METHOD_ACTION_UNPUBLISH;
  in.descriptor = &MY_INTERFACE_1_V123;
  AZ_ULIB_RESULT out = AZ_ULIB_PENDING;

  /// act
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  AZ_ULIB_RESULT result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_METHOD, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_NO_SUCH_ELEMENT_ERROR, result);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_PENDING, out);

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_deinit();
}

TEST_FUNCTION(az_ulib_ipc_e2e_release_after_unpublish_succeed) {
  /// arrange
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  my_method_model_in in;
  in.action = MY_METHOD_ACTION_JUST_RETURN;
  in.return_result = AZ_ULIB_SUCCESS;
  AZ_ULIB_RESULT out = AZ_ULIB_PENDING;

  /// act
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_call(interface_handle, MY_INTERFACE_METHOD, &in, &out));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  AZ_ULIB_RESULT result = az_ulib_ipc_release_interface(interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, out);

  /// cleanup
  az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_deinit();
}

TEST_FUNCTION(az_ulib_ipc_e2e_call_sync_method_in_multiple_threads_succeed) {
  /// arrange
  g_thread_max_sum = 10;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  /// act
  THREAD_HANDLE thread_handle[MAX_THREAD];
  for (int i = 0; i < MAX_THREAD; i++) {
    (void)test_thread_create(&thread_handle[i], &call_sync_thread, interface_handle);
  }
  az_ulib_ipc_release_interface(interface_handle);

  /// assert
  for (int i = 0; i < MAX_THREAD; i++) {
    int res;
    test_thread_join(thread_handle[i], &res);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, res);
  }

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_e2e_call_sync_method_in_multiple_threads_unpublish_timeout_failed) {
  /// arrange
  g_thread_max_sum = 100;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  THREAD_HANDLE thread_handle;

  g_is_running = 0; // Assume that the method is not running in the thread.

  (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_W(
      &g_lock_thread, 1); // Lock the method that will run in the thread to do not finish until we
                          // complete the test.

  /// act
  // Create the thread to call the method.
  (void)test_thread_create(&thread_handle, &call_sync_thread, interface_handle);

  // Wait for the method start to work.
  while (g_is_running == 0) {
  };

  // Try to unpublish the interface during the time that one of its method is running.
  AZ_ULIB_RESULT result = az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, 3);

  // As soon as the unpublish failed, release the method to end its execution.
  (void)AZ_ULIB_PORT_ATOMIC_DEC_W(&g_lock_thread);
  az_ulib_ipc_release_interface(interface_handle);

  /// assert
  int res;
  test_thread_join(thread_handle, &res);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, res);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_BUSY_ERROR, result);

  /// cleanup
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_deinit());
}

TEST_FUNCTION(az_ulib_ipc_e2e_call_sync_method_in_multiple_threads_and_unpublish_succeed) {
  /// arrange
  g_thread_max_sum = 30;
  g_sum_sleep = 10;
  init_ipc_and_publish_interfaces(true);

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  g_is_running = 0; // Assume that the method is not running in the thread.

  (void)AZ_ULIB_PORT_ATOMIC_EXCHANGE_W(
      &g_lock_thread, 0); // Lock the method that will run in the thread to do not finish until we
                          // complete the test.


  /// act
  THREAD_HANDLE thread_handle[SMALL_NUMBER_THREAD];
  for (int count_thread = 0; count_thread < SMALL_NUMBER_THREAD; count_thread++) {
    az_pal_os_sleep(100);
    (void)test_thread_create(&thread_handle[count_thread], &call_sync_thread, interface_handle);
  }
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, 10000));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, 10000));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, 10000));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, 10000));
  az_ulib_ipc_release_interface(interface_handle);

  /// assert
  for (int i = 0; i < SMALL_NUMBER_THREAD; i++) {
    int res;
    test_thread_join(thread_handle[i], &res);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_NO_SUCH_ELEMENT_ERROR, res);
  }

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_deinit());
}

END_TEST_SUITE(az_ulib_ipc_e2e)
