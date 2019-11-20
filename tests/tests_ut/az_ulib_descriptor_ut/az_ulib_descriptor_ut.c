// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "az_ulib_action_api.h"
#include "az_ulib_descriptor_api.h"
#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "az_ulib_result.h"
#include "umock_c/umock_c.h"
#include "umock_c/umock_c_negative_tests.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_stdint.h"

static TEST_MUTEX_HANDLE g_test_by_test;

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code) {
  ASSERT_FAIL("umock_c reported error :%i", error_code);
}

static uint32_t my_property = 0;

static az_ulib_result get_my_property(const void* model_out) {
  uint32_t* new_val = (uint32_t*)model_out;

  *new_val = my_property;

  return AZ_ULIB_SUCCESS;
}

static az_ulib_result set_my_property(const void* const model_in) {
  uint32_t* new_val = (uint32_t*)model_in;

  my_property = *new_val;

  return AZ_ULIB_SUCCESS;
}

static az_ulib_result my_method(const void* const model_in, const void* model_out) {
  (void)model_in;
  (void)model_out;

  return AZ_ULIB_SUCCESS;
}

static az_ulib_result my_method_async(
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

static az_ulib_result my_method_cancel(const az_ulib_action_token action_token) {
  (void)action_token;

  return AZ_ULIB_SUCCESS;
}

/**
 * Beginning of the UT for interface module.
 */
BEGIN_TEST_SUITE(az_ulib_descriptor_ut)

TEST_SUITE_INITIALIZE(suite_init) {
  g_test_by_test = TEST_MUTEX_CREATE();
  ASSERT_IS_NOT_NULL(g_test_by_test);

  ASSERT_ARE_EQUAL(int, 0, umock_c_init(on_umock_c_error));
  ASSERT_ARE_EQUAL(int, 0, umocktypes_charptr_register_types());
  ASSERT_ARE_EQUAL(int, 0, umocktypes_stdint_register_types());
  ASSERT_ARE_EQUAL(int, 0, umocktypes_bool_register_types());
}

TEST_SUITE_CLEANUP(suite_cleanup) {
  umock_c_deinit();

  TEST_MUTEX_DESTROY(g_test_by_test);
}

TEST_FUNCTION_INITIALIZE(test_method_initialize) {
  if (TEST_MUTEX_ACQUIRE(g_test_by_test)) {
    ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
  }

  umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(test_method_cleanup) { TEST_MUTEX_RELEASE(g_test_by_test); }

/* The AZ_ULIB_DESCRIPTOR_ADD_PROPERTY shall create an descriptor for a property with name, get, and
 * set pointers. */
TEST_FUNCTION(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_PROPERTY_succeed) {
  /// arrange

  /// act
  static az_ulib_action_descriptor action
      = AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property);

  /// assert
  ASSERT_ARE_EQUAL(char_ptr, action.name, "my_property");
  ASSERT_ARE_EQUAL(void_ptr, action.action_ptr_1.action, get_my_property);
  ASSERT_ARE_EQUAL(void_ptr, action.action_ptr_2.action, set_my_property);
  ASSERT_ARE_EQUAL(char, action.flags, (uint8_t)AZ_ULIB_ACTION_TYPE_PROPERTY);

  /// cleanup
}

/* The AZ_ULIB_DESCRIPTOR_ADD_METHOD shall create an descriptor for a method with name and pointer
 * to the method. */
TEST_FUNCTION(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_METHOD_succeed) {
  /// arrange

  /// act
  static az_ulib_action_descriptor action = AZ_ULIB_DESCRIPTOR_ADD_METHOD("my_method", my_method);

  /// assert
  ASSERT_ARE_EQUAL(char_ptr, action.name, "my_method");
  ASSERT_ARE_EQUAL(void_ptr, action.action_ptr_1.action, my_method);
  ASSERT_IS_NULL(action.action_ptr_2.action);
  ASSERT_ARE_EQUAL(char, action.flags, (uint8_t)AZ_ULIB_ACTION_TYPE_METHOD);

  /// cleanup
}

/* The AZ_ULIB_DESCRIPTOR_ADD_METHOD_ASYNC shall create an descriptor for an async method with name
 * and pointer to the method and the cancellation method. */
TEST_FUNCTION(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_METHOD_ASYNC_succeed) {
  /// arrange

  /// act
  static az_ulib_action_descriptor action
      = AZ_ULIB_DESCRIPTOR_ADD_METHOD_ASYNC("my_method_async", my_method_async, my_method_cancel);

  /// assert
  ASSERT_ARE_EQUAL(char_ptr, action.name, "my_method_async");
  ASSERT_ARE_EQUAL(void_ptr, action.action_ptr_1.action, my_method_async);
  ASSERT_ARE_EQUAL(void_ptr, action.action_ptr_2.action, my_method_cancel);
  ASSERT_ARE_EQUAL(char, action.flags, (uint8_t)AZ_ULIB_ACTION_TYPE_METHOD_ASYNC);

  /// cleanup
}

/* The AZ_ULIB_DESCRIPTOR_ADD_EVENT shall create an descriptor for a method with name and pointer to
 * the method. */
TEST_FUNCTION(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_EVENT_succeed) {
  /// arrange

  /// act
  static az_ulib_action_descriptor action = AZ_ULIB_DESCRIPTOR_ADD_EVENT("my_event");

  /// assert
  ASSERT_ARE_EQUAL(char_ptr, action.name, "my_event");
  ASSERT_IS_NULL(action.action_ptr_1.action);
  ASSERT_IS_NULL(action.action_ptr_2.action);
  ASSERT_ARE_EQUAL(char, action.flags, (uint8_t)AZ_ULIB_ACTION_TYPE_EVENT);

  /// cleanup
}

/* The AZ_ULIB_DESCRIPTOR_CREATE shall create an descriptor for a method with name and pointer to
 * the method. */
TEST_FUNCTION(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_CREATE_succeed) {
  /// arrange
  az_ulib_version version = 123;

  /// act
  AZ_ULIB_DESCRIPTOR_CREATE(
      MY_INTERFACE,
      "MY_INTERFACE",
      123,
      AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property),
      AZ_ULIB_DESCRIPTOR_ADD_EVENT("my_event"),
      AZ_ULIB_DESCRIPTOR_ADD_EVENT("my_event2"),
      AZ_ULIB_DESCRIPTOR_ADD_METHOD("my_method", my_method),
      AZ_ULIB_DESCRIPTOR_ADD_METHOD_ASYNC("my_method_async", my_method_async, my_method_cancel));

  /// assert
  /* Interface. */
  ASSERT_ARE_EQUAL(char_ptr, MY_INTERFACE.name, "MY_INTERFACE");
  ASSERT_ARE_EQUAL(int, MY_INTERFACE.version, 123);
  ASSERT_IS_TRUE(az_ulib_version_match(version, MY_INTERFACE.version, AZ_ULIB_VERSION_EQUALS_TO));
  ASSERT_ARE_EQUAL(char, MY_INTERFACE.size, 5);

  /* AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property) */
  ASSERT_ARE_EQUAL(char_ptr, MY_INTERFACE.action_list[0].name, "my_property");
  ASSERT_ARE_EQUAL(void_ptr, MY_INTERFACE.action_list[0].action_ptr_1.action, get_my_property);
  ASSERT_ARE_EQUAL(void_ptr, MY_INTERFACE.action_list[0].action_ptr_2.action, set_my_property);
  ASSERT_ARE_EQUAL(char, MY_INTERFACE.action_list[0].flags, (uint8_t)AZ_ULIB_ACTION_TYPE_PROPERTY);

  /* AZ_ULIB_DESCRIPTOR_ADD_EVENT("my_event") */
  ASSERT_ARE_EQUAL(char_ptr, MY_INTERFACE.action_list[1].name, "my_event");
  ASSERT_IS_NULL(MY_INTERFACE.action_list[1].action_ptr_1.action);
  ASSERT_IS_NULL(MY_INTERFACE.action_list[1].action_ptr_2.action);
  ASSERT_ARE_EQUAL(char, MY_INTERFACE.action_list[1].flags, (uint8_t)AZ_ULIB_ACTION_TYPE_EVENT);

  /* AZ_ULIB_DESCRIPTOR_ADD_EVENT("my_event2") */
  ASSERT_ARE_EQUAL(char_ptr, MY_INTERFACE.action_list[2].name, "my_event2");
  ASSERT_IS_NULL(MY_INTERFACE.action_list[2].action_ptr_1.action);
  ASSERT_IS_NULL(MY_INTERFACE.action_list[2].action_ptr_2.action);
  ASSERT_ARE_EQUAL(char, MY_INTERFACE.action_list[2].flags, (uint8_t)AZ_ULIB_ACTION_TYPE_EVENT);

  /* AZ_ULIB_DESCRIPTOR_ADD_METHOD("my_method", my_method) */
  ASSERT_ARE_EQUAL(char_ptr, MY_INTERFACE.action_list[3].name, "my_method");
  ASSERT_ARE_EQUAL(void_ptr, MY_INTERFACE.action_list[3].action_ptr_1.action, my_method);
  ASSERT_IS_NULL(MY_INTERFACE.action_list[3].action_ptr_2.action);
  ASSERT_ARE_EQUAL(char, MY_INTERFACE.action_list[3].flags, (uint8_t)AZ_ULIB_ACTION_TYPE_METHOD);

  /* AZ_ULIB_DESCRIPTOR_ADD_METHOD_ASYNC("my_method_async", my_method_async, my_method_cancel) */
  ASSERT_ARE_EQUAL(char_ptr, MY_INTERFACE.action_list[4].name, "my_method_async");
  ASSERT_ARE_EQUAL(void_ptr, MY_INTERFACE.action_list[4].action_ptr_1.action, my_method_async);
  ASSERT_ARE_EQUAL(void_ptr, MY_INTERFACE.action_list[4].action_ptr_2.action, my_method_cancel);
  ASSERT_ARE_EQUAL(
      char, MY_INTERFACE.action_list[4].flags, (uint8_t)AZ_ULIB_ACTION_TYPE_METHOD_ASYNC);

  /// cleanup
}

END_TEST_SUITE(az_ulib_descriptor_ut)
