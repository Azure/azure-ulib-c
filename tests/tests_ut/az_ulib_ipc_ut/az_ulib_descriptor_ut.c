// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_result.h"

#include "cmocka.h"

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
/* The AZ_ULIB_DESCRIPTOR_ADD_PROPERTY shall create an descriptor for a property with name, get, and
 * set pointers. */
static void az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_PROPERTY_succeed(void** state)
{
  /// arrange
  (void)state;

  /// act
  static az_ulib_capability_descriptor capability
      = AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property);

  /// assert
  assert_string_equal(capability.name, "my_property");
  assert_ptr_equal(capability.capability_ptr_1.capability, get_my_property);
  assert_ptr_equal(capability.capability_ptr_2.capability, set_my_property);
  assert_int_equal(capability.flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_PROPERTY);

  /// cleanup
}

/* The AZ_ULIB_DESCRIPTOR_ADD_COMMAND shall create an descriptor for a command with name and pointer
 * to the command. */
static void az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_COMMAND_succeed(void** state)
{
  /// arrange
  (void)state;

  /// act
  static az_ulib_capability_descriptor capability
      = AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command);

  /// assert
  assert_string_equal(capability.name, "my_command");
  assert_ptr_equal(capability.capability_ptr_1.capability, my_command);
  assert_null(capability.capability_ptr_2.capability);
  assert_int_equal(capability.flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_COMMAND);

  /// cleanup
}

/* The AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC shall create an descriptor for an async command with
 * name and pointer to the command and the cancellation command. */
static void az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC_succeed(void** state)
{
  /// arrange
  (void)state;

  /// act
  static az_ulib_capability_descriptor capability = AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
      "my_command_async", my_command_async, my_command_cancel);

  /// assert
  assert_string_equal(capability.name, "my_command_async");
  assert_ptr_equal(capability.capability_ptr_1.capability, my_command_async);
  assert_ptr_equal(capability.capability_ptr_2.capability, my_command_cancel);
  assert_int_equal(capability.flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_COMMAND_ASYNC);

  /// cleanup
}

/* The AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY shall create an descriptor for a command with name and
 * pointer to the command. */
static void az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY_succeed(void** state)
{
  /// arrange
  (void)state;

  /// act
  static az_ulib_capability_descriptor capability
      = AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry");

  /// assert
  assert_string_equal(capability.name, "my_telemetry");
  assert_null(capability.capability_ptr_1.capability);
  assert_null(capability.capability_ptr_2.capability);
  assert_int_equal(capability.flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_TELEMETRY);

  /// cleanup
}

/* The az_ulib_interface_descriptor shall create an descriptor for a command with name and pointer
 * to the command. */
static void az_ulib_descriptor_interface_descriptor_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_version version = 123;

  /// act
  static const az_ulib_capability_descriptor MY_INTERFACE_CAPABILITIES[5] = {
    AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC("my_command_async", my_command_async, my_command_cancel)
  };
  static const az_ulib_interface_descriptor MY_INTERFACE
      = { "MY_INTERFACE", 123, 5, (az_ulib_capability_descriptor*)MY_INTERFACE_CAPABILITIES };

  /// assert
  /* Interface. */
  assert_string_equal(MY_INTERFACE.name, "MY_INTERFACE");
  assert_int_equal(MY_INTERFACE.version, 123);
  assert_true(az_ulib_version_match(version, MY_INTERFACE.version, AZ_ULIB_VERSION_EQUALS_TO));
  assert_int_equal(MY_INTERFACE.size, 5);

  /* AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property) */
  assert_string_equal(MY_INTERFACE.capability_list[0].name, "my_property");
  assert_ptr_equal(MY_INTERFACE.capability_list[0].capability_ptr_1.capability, get_my_property);
  assert_ptr_equal(MY_INTERFACE.capability_list[0].capability_ptr_2.capability, set_my_property);
  assert_int_equal(
      MY_INTERFACE.capability_list[0].flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_PROPERTY);

  /* AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry") */
  assert_string_equal(MY_INTERFACE.capability_list[1].name, "my_telemetry");
  assert_null(MY_INTERFACE.capability_list[1].capability_ptr_1.capability);
  assert_null(MY_INTERFACE.capability_list[1].capability_ptr_2.capability);
  assert_int_equal(
      MY_INTERFACE.capability_list[1].flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_TELEMETRY);

  /* AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2") */
  assert_string_equal(MY_INTERFACE.capability_list[2].name, "my_telemetry2");
  assert_null(MY_INTERFACE.capability_list[2].capability_ptr_1.capability);
  assert_null(MY_INTERFACE.capability_list[2].capability_ptr_2.capability);
  assert_int_equal(
      MY_INTERFACE.capability_list[2].flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_TELEMETRY);

  /* AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command) */
  assert_string_equal(MY_INTERFACE.capability_list[3].name, "my_command");
  assert_ptr_equal(MY_INTERFACE.capability_list[3].capability_ptr_1.capability, my_command);
  assert_null(MY_INTERFACE.capability_list[3].capability_ptr_2.capability);
  assert_int_equal(MY_INTERFACE.capability_list[3].flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_COMMAND);

  /* AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC("my_command_async", my_command_async, my_command_cancel)
   */
  assert_string_equal(MY_INTERFACE.capability_list[4].name, "my_command_async");
  assert_ptr_equal(MY_INTERFACE.capability_list[4].capability_ptr_1.capability, my_command_async);
  assert_ptr_equal(MY_INTERFACE.capability_list[4].capability_ptr_2.capability, my_command_cancel);
  assert_int_equal(
      MY_INTERFACE.capability_list[4].flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_COMMAND_ASYNC);

  /// cleanup
}

int az_ulib_descriptor_ut()
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_PROPERTY_succeed),
    cmocka_unit_test(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_COMMAND_succeed),
    cmocka_unit_test(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC_succeed),
    cmocka_unit_test(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY_succeed),
    cmocka_unit_test(az_ulib_descriptor_interface_descriptor_succeed),
  };

  return cmocka_run_group_tests_name("az_ulib_descriptor_ut", tests, NULL, NULL);
}
