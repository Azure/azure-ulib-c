// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_result.h"
#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umock_c_negative_tests.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_stdint.h"

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

static az_result my_command(const void* const model_in, const void* model_out)
{
  (void)model_in;
  (void)model_out;

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

/**
 * Beginning of the UT for interface module.
 */
BEGIN_TEST_SUITE(az_ulib_descriptor_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
  g_test_by_test = TEST_MUTEX_CREATE();
  ASSERT_IS_NOT_NULL(g_test_by_test);

  ASSERT_ARE_EQUAL(int, 0, umock_c_init(on_umock_c_error));
  ASSERT_ARE_EQUAL(int, 0, umocktypes_charptr_register_types());
  ASSERT_ARE_EQUAL(int, 0, umocktypes_stdint_register_types());
  ASSERT_ARE_EQUAL(int, 0, umocktypes_bool_register_types());
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
  umock_c_deinit();

  TEST_MUTEX_DESTROY(g_test_by_test);
}

TEST_FUNCTION_INITIALIZE(test_method_initialize)
{
  if (TEST_MUTEX_ACQUIRE(g_test_by_test))
  {
    ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
  }

  umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(test_method_cleanup) { TEST_MUTEX_RELEASE(g_test_by_test); }

/* The AZ_ULIB_DESCRIPTOR_ADD_PROPERTY shall create an descriptor for a property with name, get, and
 * set pointers. */
TEST_FUNCTION(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_PROPERTY_succeed)
{
  /// arrange

  /// act
  static az_ulib_capability_descriptor capability
      = AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property);

  /// assert
  ASSERT_ARE_EQUAL(char_ptr, capability.name, "my_property");
  ASSERT_ARE_EQUAL(void_ptr, capability.capability_ptr_1.capability, get_my_property);
  ASSERT_ARE_EQUAL(void_ptr, capability.capability_ptr_2.capability, set_my_property);
  ASSERT_ARE_EQUAL(char, capability.flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_PROPERTY);

  /// cleanup
}

/* The AZ_ULIB_DESCRIPTOR_ADD_COMMAND shall create an descriptor for a command with name and pointer
 * to the command. */
TEST_FUNCTION(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_COMMAND_succeed)
{
  /// arrange

  /// act
  static az_ulib_capability_descriptor capability
      = AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command);

  /// assert
  ASSERT_ARE_EQUAL(char_ptr, capability.name, "my_command");
  ASSERT_ARE_EQUAL(void_ptr, capability.capability_ptr_1.capability, my_command);
  ASSERT_IS_NULL(capability.capability_ptr_2.capability);
  ASSERT_ARE_EQUAL(char, capability.flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_COMMAND);

  /// cleanup
}

/* The AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC shall create an descriptor for an async command with
 * name and pointer to the command and the cancellation command. */
TEST_FUNCTION(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC_succeed)
{
  /// arrange

  /// act
  static az_ulib_capability_descriptor capability = AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
      "my_command_async", my_command_async, my_command_cancel);

  /// assert
  ASSERT_ARE_EQUAL(char_ptr, capability.name, "my_command_async");
  ASSERT_ARE_EQUAL(void_ptr, capability.capability_ptr_1.capability, my_command_async);
  ASSERT_ARE_EQUAL(void_ptr, capability.capability_ptr_2.capability, my_command_cancel);
  ASSERT_ARE_EQUAL(char, capability.flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_COMMAND_ASYNC);

  /// cleanup
}

/* The AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY shall create an descriptor for a command with name and
 * pointer to the command. */
TEST_FUNCTION(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY_succeed)
{
  /// arrange

  /// act
  static az_ulib_capability_descriptor capability
      = AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry");

  /// assert
  ASSERT_ARE_EQUAL(char_ptr, capability.name, "my_telemetry");
  ASSERT_IS_NULL(capability.capability_ptr_1.capability);
  ASSERT_IS_NULL(capability.capability_ptr_2.capability);
  ASSERT_ARE_EQUAL(char, capability.flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_TELEMETRY);

  /// cleanup
}

/* The AZ_ULIB_DESCRIPTOR_CREATE shall create an descriptor for a command with name and pointer to
 * the command. */
TEST_FUNCTION(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_CREATE_succeed)
{
  /// arrange
  az_ulib_version version = 123;

  /// act
  AZ_ULIB_DESCRIPTOR_CREATE(
      MY_INTERFACE,
      "MY_INTERFACE",
      123,
      AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property),
      AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
      AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
      AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command),
      AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
          "my_command_async", my_command_async, my_command_cancel));

  /// assert
  /* Interface. */
  ASSERT_ARE_EQUAL(char_ptr, MY_INTERFACE.name, "MY_INTERFACE");
  ASSERT_ARE_EQUAL(int, MY_INTERFACE.version, 123);
  ASSERT_IS_TRUE(az_ulib_version_match(version, MY_INTERFACE.version, AZ_ULIB_VERSION_EQUALS_TO));
  ASSERT_ARE_EQUAL(char, MY_INTERFACE.size, 5);

  /* AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property) */
  ASSERT_ARE_EQUAL(char_ptr, MY_INTERFACE.capability_list[0].name, "my_property");
  ASSERT_ARE_EQUAL(
      void_ptr, MY_INTERFACE.capability_list[0].capability_ptr_1.capability, get_my_property);
  ASSERT_ARE_EQUAL(
      void_ptr, MY_INTERFACE.capability_list[0].capability_ptr_2.capability, set_my_property);
  ASSERT_ARE_EQUAL(
      char, MY_INTERFACE.capability_list[0].flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_PROPERTY);

  /* AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry") */
  ASSERT_ARE_EQUAL(char_ptr, MY_INTERFACE.capability_list[1].name, "my_telemetry");
  ASSERT_IS_NULL(MY_INTERFACE.capability_list[1].capability_ptr_1.capability);
  ASSERT_IS_NULL(MY_INTERFACE.capability_list[1].capability_ptr_2.capability);
  ASSERT_ARE_EQUAL(
      char, MY_INTERFACE.capability_list[1].flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_TELEMETRY);

  /* AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2") */
  ASSERT_ARE_EQUAL(char_ptr, MY_INTERFACE.capability_list[2].name, "my_telemetry2");
  ASSERT_IS_NULL(MY_INTERFACE.capability_list[2].capability_ptr_1.capability);
  ASSERT_IS_NULL(MY_INTERFACE.capability_list[2].capability_ptr_2.capability);
  ASSERT_ARE_EQUAL(
      char, MY_INTERFACE.capability_list[2].flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_TELEMETRY);

  /* AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command) */
  ASSERT_ARE_EQUAL(char_ptr, MY_INTERFACE.capability_list[3].name, "my_command");
  ASSERT_ARE_EQUAL(
      void_ptr, MY_INTERFACE.capability_list[3].capability_ptr_1.capability, my_command);
  ASSERT_IS_NULL(MY_INTERFACE.capability_list[3].capability_ptr_2.capability);
  ASSERT_ARE_EQUAL(
      char, MY_INTERFACE.capability_list[3].flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_COMMAND);

  /* AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC("my_command_async", my_command_async, my_command_cancel)
   */
  ASSERT_ARE_EQUAL(char_ptr, MY_INTERFACE.capability_list[4].name, "my_command_async");
  ASSERT_ARE_EQUAL(
      void_ptr, MY_INTERFACE.capability_list[4].capability_ptr_1.capability, my_command_async);
  ASSERT_ARE_EQUAL(
      void_ptr, MY_INTERFACE.capability_list[4].capability_ptr_2.capability, my_command_cancel);
  ASSERT_ARE_EQUAL(
      char, MY_INTERFACE.capability_list[4].flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_COMMAND_ASYNC);

  /// cleanup
}

END_TEST_SUITE(az_ulib_descriptor_ut)
