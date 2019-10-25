// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ENABLE_MOCKS
#include "az_ulib_pal_os_api.h"
#include "az_ulib_action_api.h"
#include "az_ulib_descriptor_api.h"
#undef ENABLE_MOCKS

#include "az_ulib_ipc_api.h"
#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "az_ulib_result.h"
#include "umock_c/umock_c.h"
#include "umock_c/umock_c_negative_tests.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_stdint.h"

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
  const az_ulib_interface_descriptor* descriptor;
  az_ulib_ipc_interface_handle handle;
  az_ulib_action_index method_index;
  AZ_ULIB_RESULT return_result;
} my_method_model_in;

MU_DEFINE_ENUM(
    my_method_action,
    MY_METHOD_ACTION_JUST_RETURN,
    MY_METHOD_ACTION_UNPUBLISH,
    MY_METHOD_ACTION_RELEASE_INTERFACE,
    MY_METHOD_ACTION_DEINIT,
    MY_METHOD_ACTION_CALL_AGAIN,
    MY_METHOD_ACTION_RETURN_ERROR)

static AZ_ULIB_RESULT my_method(const void* const model_in, const void* model_out) {
  my_method_model_in* in = (my_method_model_in*)model_in;
  AZ_ULIB_RESULT* result = (AZ_ULIB_RESULT*)model_out;
  my_method_model_in in_2;

  switch (in->action) {
    case MY_METHOD_ACTION_JUST_RETURN:
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

void init_ipc_and_publish_interfaces(void) {
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_init(&g_ipc));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_publish(&MY_INTERFACE_1_V123));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_publish(&MY_INTERFACE_1_V2));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_publish(&MY_INTERFACE_2_V123));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_publish(&MY_INTERFACE_3_V123));

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_release_interface(interface_handle));

  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V2.name,
          MY_INTERFACE_1_V2.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_release_interface(interface_handle));

  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_2_V123.name,
          MY_INTERFACE_2_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_release_interface(interface_handle));

  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_3_V123.name,
          MY_INTERFACE_3_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_release_interface(interface_handle));
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

AZ_PAL_OS_LOCK* g_lock;
int8_t g_count_lock;
void my_az_pal_os_lock_init(AZ_PAL_OS_LOCK* lock) { g_lock = lock; }

void my_az_pal_os_lock_acquire(AZ_PAL_OS_LOCK* lock) {
  if (lock == g_lock) {
    g_count_lock++;
  }
}

void my_az_pal_os_lock_release(AZ_PAL_OS_LOCK* lock) {
  if (lock == g_lock) {
    g_count_lock--;
  }
}

/**
 * Beginning of the UT for interface module.
 */
BEGIN_TEST_SUITE(az_ulib_ipc_ut)

TEST_SUITE_INITIALIZE(suite_init) {
  g_test_by_test = TEST_MUTEX_CREATE();
  ASSERT_IS_NOT_NULL(g_test_by_test);

  ASSERT_ARE_EQUAL(int, 0, umock_c_init(on_umock_c_error));
  ASSERT_ARE_EQUAL(int, 0, umocktypes_charptr_register_types());
  ASSERT_ARE_EQUAL(int, 0, umocktypes_stdint_register_types());
  ASSERT_ARE_EQUAL(int, 0, umocktypes_bool_register_types());

  REGISTER_GLOBAL_MOCK_HOOK(az_pal_os_lock_init, my_az_pal_os_lock_init);
  REGISTER_GLOBAL_MOCK_HOOK(az_pal_os_lock_acquire, my_az_pal_os_lock_acquire);
  REGISTER_GLOBAL_MOCK_HOOK(az_pal_os_lock_release, my_az_pal_os_lock_release);
}

TEST_SUITE_CLEANUP(suite_cleanup) {
  umock_c_deinit();

  TEST_MUTEX_DESTROY(g_test_by_test);
}

TEST_FUNCTION_INITIALIZE(test_method_initialize) {
  if (TEST_MUTEX_ACQUIRE(g_test_by_test)) {
    ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
  }

  g_lock = NULL;
  g_count_lock = 0;

  umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(test_method_cleanup) { TEST_MUTEX_RELEASE(g_test_by_test); }

/* The az_ulib_ipc_init shall initialize the ipc control block. */
/* The az_ulib_ipc_init shall initialize the lock mechanism. */
TEST_FUNCTION(az_ulib_ipc_init_succeed) {
  /// arrange
  STRICT_EXPECTED_CALL(az_pal_os_lock_init(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_init(&g_ipc);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_IS_NOT_NULL(g_lock);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
  /// cleanup
  az_ulib_ipc_deinit();
}

/* If the provided handle is NULL, the az_ulib_ipc_init shall return AZ_ULIB_ILLEGAL_ARGUMENT_ERROR.
 */
TEST_FUNCTION(az_ulib_ipc_init_with_null_handle_failed) {
  /// arrange

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_init(NULL);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
  ASSERT_IS_NULL(g_lock);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
}

/* If the az_ulib_ipc_init is called more then once, it shall return
 * AZ_ULIB_ALREADY_INITIALIZED_ERROR. */
TEST_FUNCTION(az_ulib_ipc_init_double_initialization_failed) {
  /// arrange
  az_ulib_ipc ipc1;
  az_ulib_ipc ipc2;
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_init(&ipc1));
  g_lock = NULL;
  g_count_lock = 0;
  umock_c_reset_all_calls();

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_init(&ipc2);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_ALREADY_INITIALIZED_ERROR, result);
  ASSERT_IS_NULL(g_lock);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  az_ulib_ipc_deinit();
}

/* The az_ulib_ipc_publish shall store the descriptor published in the IPC. The az_ulib_ipc_publish
 * shall be thread safe. */
TEST_FUNCTION(az_ulib_ipc_publish_succeed) {
  /// arrange
  ASSERT_ARE_EQUAL(int, az_ulib_ipc_init(&g_ipc), AZ_ULIB_SUCCESS);
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_publish(&MY_INTERFACE_1_V123));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_publish(&MY_INTERFACE_1_V2));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_publish(&MY_INTERFACE_2_V123));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_publish(&MY_INTERFACE_3_V123));

  /// assert
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_release_interface(interface_handle));

  /// cleanup
  az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_deinit();
}

/* If the ipc was not initialized, the az_ulib_ipc_publish shall return
 * AZ_ULIB_NOT_INITIALIZED_ERROR. */
TEST_FUNCTION(az_ulib_ipc_publish_with_ipc_not_initialized_failed) {
  /// arrange

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_publish(&MY_INTERFACE_1_V123);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_NOT_INITIALIZED_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
}

/* If the provided descriptor is NULL, the az_ulib_ipc_publish shall return
 * AZ_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(az_ulib_ipc_publish_with_null_descriptor_failed) {
  /// arrange
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_init(&g_ipc));
  umock_c_reset_all_calls();

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_publish(NULL);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  az_ulib_ipc_deinit();
}

/* If the provided descriptor already exist, the az_ulib_ipc_publish shall return
 * AZ_ULIB_ELEMENT_DUPLICATE_ERROR. */
TEST_FUNCTION(az_ulib_ipc_publish_with_descriptor_with_same_name_and_version_failed) {
  /// arrange
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_init(&g_ipc));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_publish(&MY_INTERFACE_1_V123));
  umock_c_reset_all_calls();
  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_publish(&MY_INTERFACE_1_V123);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_ELEMENT_DUPLICATE_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_deinit();
}

/* If there is no more memory to store a new descriptor, the az_ulib_ipc_publish shall return
 * AZ_ULIB_OUT_OF_MEMORY_ERROR. */
TEST_FUNCTION(az_ulib_ipc_publish_out_of_memory_failed) {
  /// arrange
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_init(&g_ipc));
  az_ulib_interface_descriptor descriptors[AZ_ULIB_CONFIG_MAX_IPC_INTERFACE];
  az_ulib_action_descriptor actions[1] = AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", NULL, NULL);
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++) {
    descriptors[i].name = "my_interface";
    descriptors[i].version = i + 1;
    descriptors[i].size = 1;
    descriptors[i].action_list = actions;
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_publish(&descriptors[i]));
  }
  umock_c_reset_all_calls();
  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_publish(&MY_INTERFACE_1_V123);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_OUT_OF_MEMORY_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++) {
    az_ulib_ipc_unpublish(&descriptors[i], AZ_ULIB_NO_WAIT);
  }
  az_ulib_ipc_deinit();
}

/* The az_ulib_ipc_unpublish shall remove a descriptor for the IPC. The az_ulib_ipc_unpublish shall
 * be thread safe. */
TEST_FUNCTION(az_ulib_ipc_unpublish_succeed) {
  /// arrange
  init_ipc_and_publish_interfaces();
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));

  /// assert
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_NO_SUCH_ELEMENT_ERROR,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_NO_SUCH_ELEMENT_ERROR,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V2.name,
          MY_INTERFACE_1_V2.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_NO_SUCH_ELEMENT_ERROR,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_2_V123.name,
          MY_INTERFACE_2_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_NO_SUCH_ELEMENT_ERROR,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_3_V123.name,
          MY_INTERFACE_3_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  /// cleanup
  az_ulib_ipc_deinit();
}

TEST_FUNCTION(az_ulib_ipc_unpublish_randon_order_succeed) {
  /// arrange
  init_ipc_and_publish_interfaces();
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));

  /// assert
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_NO_SUCH_ELEMENT_ERROR,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_NO_SUCH_ELEMENT_ERROR,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V2.name,
          MY_INTERFACE_1_V2.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_NO_SUCH_ELEMENT_ERROR,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_2_V123.name,
          MY_INTERFACE_2_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_NO_SUCH_ELEMENT_ERROR,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_3_V123.name,
          MY_INTERFACE_3_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  /// cleanup
  az_ulib_ipc_deinit();
}

/* The az_ulib_ipc_unpublish shall release the descriptor position to be used by another descriptor.
 */
TEST_FUNCTION(az_ulib_ipc_unpublish_release_resource_succeed) {
  /// arrange
  init_ipc_and_publish_interfaces();
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));

  /// assert
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  az_ulib_interface_descriptor descriptors[AZ_ULIB_CONFIG_MAX_IPC_INTERFACE - 2];
  az_ulib_action_descriptor actions[1] = AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", NULL, NULL);
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE - 2; i++) {
    descriptors[i].name = "my_interface";
    descriptors[i].version = i + 1;
    descriptors[i].size = 1;
    descriptors[i].action_list = actions;
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_publish(&descriptors[i]));
  }
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE - 2; i++) {
    az_ulib_ipc_unpublish(&descriptors[i], AZ_ULIB_NO_WAIT);
  }

  /// cleanup
  az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_deinit();
}

/* If the ipc was not initialized, the az_ulib_ipc_unpublish shall return
 * AZ_ULIB_NOT_INITIALIZED_ERROR. */
TEST_FUNCTION(az_ulib_ipc_unpublish_with_ipc_not_initialized_failed) {
  /// arrange

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_NOT_INITIALIZED_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
}

/* If the provided descriptor is NULL, the az_ulib_ipc_unpublish shall return
 * AZ_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(az_ulib_ipc_unpublish_with_null_descriptor_failed) {
  /// arrange
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_init(&g_ipc));
  umock_c_reset_all_calls();

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_unpublish(NULL, AZ_ULIB_NO_WAIT);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  az_ulib_ipc_deinit();
}

/* If the provided descriptor was not published, the az_ulib_ipc_unpublish shall return
 * AZ_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(az_ulib_ipc_unpublish_with_unknown_descriptor_failed) {
  /// arrange
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_init(&g_ipc));
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_NO_SUCH_ELEMENT_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  az_ulib_ipc_deinit();
}

/* If one of the method in the interface is running and the wait policy is AZ_ULIB_NO_WAIT, the
 * az_ulib_ipc_unpublish shall return AZ_ULIB_BUSY_ERROR. */
TEST_FUNCTION(az_ulib_ipc_unpublish_with_method_running_failed) {
  /// arrange
  init_ipc_and_publish_interfaces();

  my_method_model_in in;
  in.action = MY_METHOD_ACTION_UNPUBLISH;
  in.descriptor = &MY_INTERFACE_1_V123;
  AZ_ULIB_RESULT out = AZ_ULIB_PENDING;

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  // call umpublish inside of the methond.
  AZ_ULIB_RESULT result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_METHOD, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_BUSY_ERROR, out);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

/* If there are valid instances of the interface, the az_ulib_ipc_unpublish shall return
 * AZ_ULIB_SUCCESS. */
TEST_FUNCTION(az_ulib_ipc_unpublish_with_valid_interface_instance_succeed) {
  /// arrange
  init_ipc_and_publish_interfaces();

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_deinit();
}

/* If one of the method in the interface is running, the wait policy is different than
 * AZ_ULIB_NO_WAIT and the call ends before the timeout, the az_ulib_ipc_unpublish shall return
 * AZ_ULIB_SUCCEESS. */
// TODO: implement the test when the code is ready.

/* If one of the method in the interface is running, the wait policy is different than
 * AZ_ULIB_NO_WAIT and the call ends after the timeout, the az_ulib_ipc_unpublish shall return
 * AZ_ULIB_BUSY_ERROR. */
// TODO: implement the test when the code is ready.

/* The az_ulib_ipc_try_get_interface shall return the handle for the interface. */
/* The az_ulib_ipc_try_get_interface shall return AZ_ULIB_SUCCESS. */
TEST_FUNCTION(az_ulib_ipc_try_get_interface_version_equals_succeed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123.name,
      MY_INTERFACE_1_V123.version,
      AZ_ULIB_VERSION_EQUALS_TO,
      &interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_version_any_succeed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123.name, 0, AZ_ULIB_VERSION_ANY, &interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_version_greater_than_succeed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  az_ulib_ipc_interface_handle greater_interface_handle;
  init_ipc_and_publish_interfaces();
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V2.name,
          MY_INTERFACE_1_V2.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V2.name,
      MY_INTERFACE_1_V2.version,
      AZ_ULIB_VERSION_GREATER_THAN,
      &greater_interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_NOT_EQUAL(void_ptr, interface_handle, greater_interface_handle);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  az_ulib_ipc_release_interface(greater_interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_version_lower_than_succeed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  az_ulib_ipc_interface_handle lower_interface_handle;
  init_ipc_and_publish_interfaces();
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123.name,
      MY_INTERFACE_1_V123.version,
      AZ_ULIB_VERSION_LOWER_THAN,
      &lower_interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_NOT_EQUAL(void_ptr, interface_handle, lower_interface_handle);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  az_ulib_ipc_release_interface(lower_interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_version_lower_or_equal_succeed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  az_ulib_ipc_interface_handle lower_interface_handle;
  init_ipc_and_publish_interfaces();
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V2.name,
          MY_INTERFACE_1_V2.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V2.name,
      MY_INTERFACE_1_V2.version,
      AZ_ULIB_VERSION_LOWER_THAN | AZ_ULIB_VERSION_EQUALS_TO,
      &lower_interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_EQUAL(void_ptr, interface_handle, lower_interface_handle);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  az_ulib_ipc_release_interface(lower_interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

/* If the IPC reach the maximun number of allawed instances for a single interface, the
 * az_ulib_ipc_try_get_interface shall return AZ_ULIB_BUSY_ERROR. */
TEST_FUNCTION(az_ulib_ipc_try_get_interface_with_max_interface_instances_failed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle[AZ_ULIB_CONFIG_MAX_IPC_INSTANCES];
  az_ulib_ipc_interface_handle interface_handle_plus_one;

  init_ipc_and_publish_interfaces();

  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INSTANCES; i++) {
    ASSERT_ARE_EQUAL(
        int,
        AZ_ULIB_SUCCESS,
        az_ulib_ipc_try_get_interface(
            MY_INTERFACE_1_V123.name,
            MY_INTERFACE_1_V123.version,
            AZ_ULIB_VERSION_EQUALS_TO,
            &interface_handle[i]));
  }

  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123.name,
      MY_INTERFACE_1_V123.version,
      AZ_ULIB_VERSION_EQUALS_TO,
      &interface_handle_plus_one);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_BUSY_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INSTANCES; i++) {
    az_ulib_ipc_release_interface(interface_handle[i]);
  }
  unpublish_interfaces_and_deinit_ipc();
}

/* If the provided interface name does not exist, the az_ulib_ipc_try_get_interface shall return
 * AZ_ULIB_NO_SUCH_ELEMENT_ERROR. */
TEST_FUNCTION(az_ulib_ipc_try_get_interface_with_unknown_name_failed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_try_get_interface(
      "unknown_name", MY_INTERFACE_1_V123.version, AZ_ULIB_VERSION_EQUALS_TO, &interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_NO_SUCH_ELEMENT_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_with_unknown_version_failed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123.name, 9999, AZ_ULIB_VERSION_EQUALS_TO, &interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_NO_SUCH_ELEMENT_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_without_version_greater_than_failed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123.name,
      MY_INTERFACE_1_V123.version,
      AZ_ULIB_VERSION_GREATER_THAN,
      &interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_NO_SUCH_ELEMENT_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_without_version_lower_than_failed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V2.name,
      MY_INTERFACE_1_V2.version,
      AZ_ULIB_VERSION_LOWER_THAN,
      &interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_NO_SUCH_ELEMENT_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the provided interface name is NULL, the az_ulib_ipc_try_get_interface shall return
 * AZ_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(az_ulib_ipc_try_get_interface_with_null_name_failed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();
  umock_c_reset_all_calls();

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_try_get_interface(
      NULL, MY_INTERFACE_1_V123.version, AZ_ULIB_VERSION_EQUALS_TO, &interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the provided handle is NULL, the az_ulib_ipc_try_get_interface shall return
 * AZ_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(az_ulib_ipc_try_get_interface_with_null_handle_failed) {
  /// arrange
  init_ipc_and_publish_interfaces();
  umock_c_reset_all_calls();

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123.name, MY_INTERFACE_1_V123.version, AZ_ULIB_VERSION_EQUALS_TO, NULL);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the ipc was not initialized, the az_ulib_ipc_try_get_interface shall return
 * AZ_ULIB_NOT_INITIALIZED_ERROR. */
TEST_FUNCTION(az_ulib_ipc_try_get_interface_with_ipc_not_initialized_failed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123.name,
      MY_INTERFACE_1_V123.version,
      AZ_ULIB_VERSION_EQUALS_TO,
      &interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_NOT_INITIALIZED_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
}

/* The az_ulib_ipc_get_interface shall return the handle for the interface. */
/* The az_ulib_ipc_get_interface shall return AZ_ULIB_SUCCESS. */
TEST_FUNCTION(az_ulib_ipc_get_interface_succeed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  az_ulib_ipc_interface_handle new_interface_handle;
  init_ipc_and_publish_interfaces();
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_get_interface(interface_handle, &new_interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  az_ulib_ipc_release_interface(new_interface_handle);
  az_ulib_ipc_release_interface(interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

/* If the IPC reach the maximun number of allawed instances for a single interface, the
 * az_ulib_ipc_get_interface shall return AZ_ULIB_BUSY_ERROR. */
TEST_FUNCTION(az_ulib_ipc_get_interface_with_max_interface_instances_failed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle[AZ_ULIB_CONFIG_MAX_IPC_INSTANCES];
  az_ulib_ipc_interface_handle interface_handle_plus_one;

  init_ipc_and_publish_interfaces();

  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INSTANCES; i++) {
    ASSERT_ARE_EQUAL(
        int,
        AZ_ULIB_SUCCESS,
        az_ulib_ipc_try_get_interface(
            MY_INTERFACE_1_V123.name,
            MY_INTERFACE_1_V123.version,
            AZ_ULIB_VERSION_EQUALS_TO,
            &interface_handle[i]));
  }

  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result
      = az_ulib_ipc_get_interface(interface_handle[0], &interface_handle_plus_one);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_BUSY_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INSTANCES; i++) {
    az_ulib_ipc_release_interface(interface_handle[i]);
  }
  unpublish_interfaces_and_deinit_ipc();
}

/* If the provided interface name does not exist, the az_ulib_ipc_get_interface shall return
 * AZ_ULIB_NO_SUCH_ELEMENT_ERROR. */
TEST_FUNCTION(az_ulib_ipc_get_interface_with_unpublished_interface_failed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  az_ulib_ipc_interface_handle new_interface_handle;
  init_ipc_and_publish_interfaces();
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_get_interface(interface_handle, &new_interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_NO_SUCH_ELEMENT_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_release_interface(interface_handle));
  az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_deinit();
}

/* If the provided interface handle is NULL, the az_ulib_ipc_get_interface shall return
 * AZ_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(az_ulib_ipc_get_interface_with_null_original_interface_handle_failed) {
  /// arrange
  az_ulib_ipc_interface_handle new_interface_handle;
  init_ipc_and_publish_interfaces();
  umock_c_reset_all_calls();

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_get_interface(NULL, &new_interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the provided new interface handle is NULL, the az_ulib_ipc_get_interface shall return
 * AZ_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(az_ulib_ipc_get_interface_with_null_interface_handle_failed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  umock_c_reset_all_calls();

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_get_interface(interface_handle, NULL);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_release_interface(interface_handle));
  unpublish_interfaces_and_deinit_ipc();
}

/* If the ipc was not initialized, the az_ulib_ipc_get_interface shall return
 * AZ_ULIB_NOT_INITIALIZED_ERROR. */
TEST_FUNCTION(az_ulib_ipc_get_interface_with_ipc_not_initialized_failed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle = (az_ulib_ipc_interface_handle)0x1234;
  az_ulib_ipc_interface_handle new_interface_handle;

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_get_interface(interface_handle, &new_interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_NOT_INITIALIZED_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
}

/* The az_ulib_ipc_release_interface shall release the instance of the interface. */
/* The az_ulib_ipc_release_interface shall return AZ_ULIB_SUCCESS. */
TEST_FUNCTION(az_ulib_ipc_release_interface_succeed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_release_interface(interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the interface is already released, the az_ulib_ipc_release_interface shall return
 * AZ_ULIB_PRECONDITION_ERROR. */
TEST_FUNCTION(az_ulib_ipc_release_interface_double_release_failed) {
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_release_interface(interface_handle));
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_release_interface(interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_PRECONDITION_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the provided handle is NULL, the az_ulib_ipc_release_interface shall return
 * AZ_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(az_ulib_ipc_release_interface_with_null_interface_handle_failed) {
  /// arrange
  init_ipc_and_publish_interfaces();
  umock_c_reset_all_calls();

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_release_interface(NULL);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the ipc is not initialized, the az_ulib_ipc_release_interface shall return
 * AZ_ULIB_NOT_INITIALIZED_ERROR. */
TEST_FUNCTION(az_ulib_ipc_release_interface_with_ipc_not_initialized_failed) {
  /// arrange
  umock_c_reset_all_calls();

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_release_interface((az_ulib_ipc_interface_handle)0x1234);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_NOT_INITIALIZED_ERROR, result);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
}

/* If one of the method in the interface is running, the az_ulib_ipc_release_interface shall return
 * AZ_ULIB_SUCCESS. */
TEST_FUNCTION(az_ulib_ipc_release_interface_with_method_running_failed) {
  /// arrange
  init_ipc_and_publish_interfaces();

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

  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_acquire(IGNORED_PTR_ARG));
  STRICT_EXPECTED_CALL(az_pal_os_lock_release(IGNORED_PTR_ARG));

  /// act
  // call umpublish inside of the methond.
  AZ_ULIB_RESULT result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_METHOD, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, out);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* The az_ulib_ipc_call shall call the method published by the interface. */
/* The az_ulib_ipc_call shall return AZ_ULIB_SUCCESS. */
TEST_FUNCTION(az_ulib_ipc_call_calls_the_method_succeed) {
  /// arrange
  init_ipc_and_publish_interfaces();

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

  umock_c_reset_all_calls();

  /// act
  // call umpublish inside of the methond.
  AZ_ULIB_RESULT result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_METHOD, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, out);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  unpublish_interfaces_and_deinit_ipc();
}

/* If the IPC is not initialized, the az_ulib_ipc_call shall return AZ_ULIB_NOT_INITIALIZED_ERROR
 * and do not call the method.
 */
TEST_FUNCTION(az_ulib_ipc_call_with_ipc_not_initialized_failed) {
  /// arrange
  my_method_model_in in;
  in.action = MY_METHOD_ACTION_JUST_RETURN;
  in.return_result = AZ_ULIB_SUCCESS;
  AZ_ULIB_RESULT out = AZ_ULIB_PENDING;

  umock_c_reset_all_calls();

  /// act
  // call umpublish inside of the methond.
  AZ_ULIB_RESULT result
      = az_ulib_ipc_call((az_ulib_ipc_interface_handle)0x1234, MY_INTERFACE_METHOD, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_NOT_INITIALIZED_ERROR, result);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_PENDING, out);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
}

/* If the interface handle is NULL, the az_ulib_ipc_call shall return AZ_ULIB_ILLEGAL_ARGUMENT_ERROR
 * and do not call the method. */
TEST_FUNCTION(az_ulib_ipc_call_with_null_interface_handle_failed) {
  /// arrange
  init_ipc_and_publish_interfaces();

  my_method_model_in in;
  in.action = MY_METHOD_ACTION_JUST_RETURN;
  in.return_result = AZ_ULIB_SUCCESS;
  AZ_ULIB_RESULT out = AZ_ULIB_PENDING;

  umock_c_reset_all_calls();

  /// act
  // call umpublish inside of the methond.
  AZ_ULIB_RESULT result = az_ulib_ipc_call(NULL, MY_INTERFACE_METHOD, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_PENDING, out);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the interface was unpublished, the az_ulib_ipc_call shall return AZ_ULIB_NO_SUCH_ELEMENT_ERROR
 * and do not call the method. */
TEST_FUNCTION(az_ulib_ipc_call_unpublished_interface_failed) {
  /// arrange
  init_ipc_and_publish_interfaces();

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));

  my_method_model_in in;
  in.action = MY_METHOD_ACTION_JUST_RETURN;
  in.return_result = AZ_ULIB_SUCCESS;
  AZ_ULIB_RESULT out = AZ_ULIB_PENDING;

  umock_c_reset_all_calls();

  /// act
  // call umpublish inside of the methond.
  AZ_ULIB_RESULT result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_METHOD, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_NO_SUCH_ELEMENT_ERROR, result);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_PENDING, out);
  ASSERT_ARE_EQUAL(int, 0, g_count_lock);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT);
  az_ulib_ipc_deinit();
}

/* The az_ulib_ipc_deinit shall release all resources associate with ipc. */
/* The az_ulib_ipc_deinit shall return AZ_ULIB_SUCCESS. */
TEST_FUNCTION(az_ulib_ipc_deinit_succeed) {
  /// arrange
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ulib_ipc_init(&g_ipc));
  umock_c_reset_all_calls();

  STRICT_EXPECTED_CALL(az_pal_os_lock_deinit(IGNORED_PTR_ARG));

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_deinit();

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
}

/* If there is published interface, the az_ulib_ipc_deinit shall return AZ_ULIB_BUSY_ERROR. */
TEST_FUNCTION(az_ulib_ipc_deinit_with_published_interface_failed) {
  /// arrange
  init_ipc_and_publish_interfaces();
  umock_c_reset_all_calls();

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_deinit();

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_BUSY_ERROR, result);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If there is instances of the interface, the az_ulib_ipc_deinit shall return AZ_ULIB_BUSY_ERROR.
 */
TEST_FUNCTION(az_ulib_ipc_deinit_with_instace_failed) {
  /// arrange
  init_ipc_and_publish_interfaces();
  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_SUCCESS,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(
      int, AZ_ULIB_SUCCESS, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));
  umock_c_reset_all_calls();

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_deinit();

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_BUSY_ERROR, result);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  az_ulib_ipc_release_interface(interface_handle);
  az_ulib_ipc_deinit();
}

/* If the IPC was not initialized, the az_ulib_ipc_deinit shall return
 * AZ_ULIB_NOT_INITIALIZED_ERROR. */
TEST_FUNCTION(az_ulib_ipc_deinit_with_ipc_not_initialized_failed) {
  /// arrange

  /// act
  AZ_ULIB_RESULT result = az_ulib_ipc_deinit();

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ULIB_NOT_INITIALIZED_ERROR, result);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
}

END_TEST_SUITE(az_ulib_ipc_ut)
