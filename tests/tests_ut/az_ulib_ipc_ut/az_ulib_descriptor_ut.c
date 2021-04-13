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
#include "az_ulib_ipc_ut.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"

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

static az_result my_command(az_ulib_model_in model_in, az_ulib_model_out model_out)
{
  (void)model_in;
  (void)model_out;

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

/**
 * Beginning of the UT for interface module.
 */
/* The AZ_ULIB_DESCRIPTOR_ADD_PROPERTY shall create an descriptor for a property with name, get, and
 * set pointers. */
static void az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_PROPERTY_succeed(void** state)
{
  /// arrange
  (void)state;
  const char property_name[] = "my_property";

  /// act
  static az_ulib_capability_descriptor capability = AZ_ULIB_DESCRIPTOR_ADD_PROPERTY(
      "my_property", get_my_property, set_my_property, NULL, NULL);

  /// assert
  assert_int_equal(az_span_size(capability._name), _az_COUNTOF(property_name) - 1);
  assert_memory_equal(az_span_ptr(capability._name), property_name, _az_COUNTOF(property_name) - 1);
  assert_ptr_equal(capability._capability_ptr_1._get, get_my_property);
  assert_ptr_equal(capability._capability_ptr_2._set, set_my_property);
  assert_int_equal(capability._flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_PROPERTY);

  /// cleanup
}

/* The AZ_ULIB_DESCRIPTOR_ADD_COMMAND shall create an descriptor for a command with name and pointer
 * to the command. */
static void az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_COMMAND_succeed(void** state)
{
  /// arrange
  (void)state;
  const char command_name[] = "my_command";

  /// act
  static az_ulib_capability_descriptor capability
      = AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command, NULL);

  /// assert
  assert_int_equal(az_span_size(capability._name), _az_COUNTOF(command_name) - 1);
  assert_memory_equal(az_span_ptr(capability._name), command_name, _az_COUNTOF(command_name) - 1);
  assert_ptr_equal(capability._capability_ptr_1._command, my_command);
  assert_int_equal(capability._flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_COMMAND);

  /// cleanup
}

/* The AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC shall create an descriptor for an async command with
 * name and pointer to the command and the cancellation command. */
static void az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC_succeed(void** state)
{
  /// arrange
  (void)state;
  const char command_async_name[] = "my_command_async";

  /// act
  static az_ulib_capability_descriptor capability = AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
      "my_command_async", my_command_async, NULL, my_command_cancel);

  /// assert
  assert_int_equal(az_span_size(capability._name), _az_COUNTOF(command_async_name) - 1);
  assert_memory_equal(
      az_span_ptr(capability._name), command_async_name, _az_COUNTOF(command_async_name) - 1);
  assert_ptr_equal(capability._capability_ptr_1._command_async, my_command_async);
  assert_ptr_equal(capability._capability_ptr_2._cancel, my_command_cancel);
  assert_int_equal(capability._flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_COMMAND_ASYNC);

  /// cleanup
}

/* The AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY shall create an descriptor for a command with name and
 * pointer to the command. */
static void az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY_succeed(void** state)
{
  /// arrange
  (void)state;
  const char telemetry_name[] = "my_telemetry";

  /// act
  static az_ulib_capability_descriptor capability
      = AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry");

  /// assert
  assert_int_equal(az_span_size(capability._name), _az_COUNTOF(telemetry_name) - 1);
  assert_memory_equal(
      az_span_ptr(capability._name), telemetry_name, _az_COUNTOF(telemetry_name) - 1);
  assert_int_equal(capability._flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_TELEMETRY);

  /// cleanup
}

/* The az_ulib_interface_descriptor shall create an descriptor for a command with name and pointer
 * to the command. */
static void az_ulib_descriptor_interface_descriptor_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_version version = 123;
  const char interface_name[] = "MY_INTERFACE";
  const char property_name[] = "my_property";
  const char telemetry_name[] = "my_telemetry";
  const char telemetry2_name[] = "my_telemetry2";
  const char command_name[] = "my_command";
  const char command_async_name[] = "my_command_async";

  /// act
  static const az_ulib_capability_descriptor MY_INTERFACE_CAPABILITIES[5] = {
    AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property, NULL, NULL),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command, NULL),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
        "my_command_async", my_command_async, NULL, my_command_cancel)
  };
  static const az_ulib_interface_descriptor MY_INTERFACE
      = AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE", 123, 5, MY_INTERFACE_CAPABILITIES);

  /// assert
  /* Interface. */
  assert_int_equal(az_span_size(MY_INTERFACE._name), _az_COUNTOF(interface_name) - 1);
  assert_memory_equal(
      az_span_ptr(MY_INTERFACE._name), interface_name, _az_COUNTOF(interface_name) - 1);
  assert_int_equal(MY_INTERFACE._version, 123);
  assert_true(az_ulib_version_match(version, MY_INTERFACE._version, AZ_ULIB_VERSION_EQUALS_TO));
  assert_int_equal(MY_INTERFACE._size, 5);

  /* AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property) */
  assert_int_equal(
      az_span_size(MY_INTERFACE._capability_list[0]._name), _az_COUNTOF(property_name) - 1);
  assert_memory_equal(
      az_span_ptr(MY_INTERFACE._capability_list[0]._name),
      property_name,
      _az_COUNTOF(property_name) - 1);
  assert_ptr_equal(MY_INTERFACE._capability_list[0]._capability_ptr_1._get, get_my_property);
  assert_ptr_equal(MY_INTERFACE._capability_list[0]._capability_ptr_2._set, set_my_property);
  assert_int_equal(
      MY_INTERFACE._capability_list[0]._flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_PROPERTY);

  /* AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry") */
  assert_int_equal(
      az_span_size(MY_INTERFACE._capability_list[1]._name), _az_COUNTOF(telemetry_name) - 1);
  assert_memory_equal(
      az_span_ptr(MY_INTERFACE._capability_list[1]._name),
      telemetry_name,
      _az_COUNTOF(telemetry_name) - 1);
  assert_int_equal(
      MY_INTERFACE._capability_list[1]._flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_TELEMETRY);

  /* AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2") */
  assert_int_equal(
      az_span_size(MY_INTERFACE._capability_list[2]._name), _az_COUNTOF(telemetry2_name) - 1);
  assert_memory_equal(
      az_span_ptr(MY_INTERFACE._capability_list[2]._name),
      telemetry2_name,
      _az_COUNTOF(telemetry2_name) - 1);
  assert_int_equal(
      MY_INTERFACE._capability_list[2]._flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_TELEMETRY);

  /* AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command) */
  assert_int_equal(
      az_span_size(MY_INTERFACE._capability_list[3]._name), _az_COUNTOF(command_name) - 1);
  assert_memory_equal(
      az_span_ptr(MY_INTERFACE._capability_list[3]._name),
      command_name,
      _az_COUNTOF(command_name) - 1);
  assert_ptr_equal(MY_INTERFACE._capability_list[3]._capability_ptr_1._command, my_command);
  assert_int_equal(
      MY_INTERFACE._capability_list[3]._flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_COMMAND);

  /* AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC("my_command_async", my_command_async, my_command_cancel)
   */
  assert_int_equal(
      az_span_size(MY_INTERFACE._capability_list[4]._name), _az_COUNTOF(command_async_name) - 1);
  assert_memory_equal(
      az_span_ptr(MY_INTERFACE._capability_list[4]._name),
      command_async_name,
      _az_COUNTOF(command_async_name) - 1);
  assert_ptr_equal(
      MY_INTERFACE._capability_list[4]._capability_ptr_1._command_async, my_command_async);
  assert_ptr_equal(MY_INTERFACE._capability_list[4]._capability_ptr_2._cancel, my_command_cancel);
  assert_int_equal(
      MY_INTERFACE._capability_list[4]._flags, (uint8_t)AZ_ULIB_CAPABILITY_TYPE_COMMAND_ASYNC);

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
