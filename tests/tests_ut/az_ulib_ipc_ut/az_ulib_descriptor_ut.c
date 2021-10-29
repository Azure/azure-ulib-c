// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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

#include "az_ulib_test_my_interface.h"
#include "cmocka.h"

static az_result get_my_property(const my_property_model* const in, my_property_model* out)
{
  (void)in;
  (void)out;
  return AZ_OK;
}

static az_result get_my_property_span_wrapper(az_span model_in_span, az_span* model_out_span)
{
  (void)model_in_span;
  (void)model_out_span;
  return AZ_OK;
}

static az_result set_my_property(const my_property_model* const in, my_property_model* out)
{
  (void)in;
  (void)out;
  return AZ_OK;
}

static az_result set_my_property_span_wrapper(az_span model_in_span, az_span* model_out_span)
{
  (void)model_in_span;
  (void)model_out_span;
  return AZ_OK;
}

static az_result my_command(
    const my_command_model_in* const model_in,
    my_command_model_out* model_out)
{
  (void)model_in;
  (void)model_out;
  return AZ_OK;
}

static az_result my_command_span_wrapper(az_span model_in_span, az_span* model_out_span)
{
  (void)model_in_span;
  (void)model_out_span;
  return AZ_OK;
}

/**
 * Beginning of the UT for interface module.
 */
/* The AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY shall create an descriptor for a capability with name and
 * pointer to the capability call. */
static void az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY_w_null_wrapper_succeed(
    void** state)
{
  /// arrange
  (void)state;

  /// act
  static az_ulib_capability_descriptor capability
      = AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(MY_INTERFACE_MY_COMMAND_NAME, my_command, NULL);

  /// assert
  assert_true(az_span_is_content_equal(
      capability._internal.name, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_COMMAND_NAME)));
  assert_ptr_equal(capability._internal.capability_ptr, my_command);
  assert_ptr_equal(capability._internal.capability_span_wrapper, NULL);

  /// cleanup
}

static void az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY_succeed(void** state)
{
  /// arrange
  (void)state;

  /// act
  static az_ulib_capability_descriptor capability = AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
      MY_INTERFACE_MY_COMMAND_NAME, my_command, my_command_span_wrapper);

  /// assert
  assert_true(az_span_is_content_equal(
      capability._internal.name, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_COMMAND_NAME)));
  assert_ptr_equal(capability._internal.capability_ptr, my_command);
  assert_ptr_equal(capability._internal.capability_span_wrapper, my_command_span_wrapper);

  /// cleanup
}

/* The AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY shall create an descriptor for a capability. */
static void az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY_succeed(void** state)
{
  /// arrange
  (void)state;

  /// act
  static az_ulib_capability_descriptor capability
      = AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY_NAME);

  /// assert
  assert_true(az_span_is_content_equal(
      capability._internal.name, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_TELEMETRY_NAME)));
  assert_ptr_equal(capability._internal.capability_ptr, NULL);
  assert_ptr_equal(capability._internal.capability_span_wrapper, NULL);

  /// cleanup
}

/* The az_ulib_interface_descriptor shall create an descriptor for a capability with name and
 * pointer to the capability call. */
static const az_ulib_capability_descriptor MY_INTERFACE_1_V123_CAPABILITIES[]
    = { AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_GET_MY_PROPERTY_NAME,
            get_my_property,
            get_my_property_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_SET_MY_PROPERTY_NAME,
            set_my_property,
            set_my_property_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY2_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_MY_COMMAND_NAME,
            my_command,
            my_command_span_wrapper) };
const az_ulib_interface_descriptor MY_INTERFACE = AZ_ULIB_DESCRIPTOR_CREATE(
    MY_PACKAGE_A_NAME,
    MY_PACKAGE_1_VERSION,
    MY_INTERFACE_1_NAME,
    MY_INTERFACE_123_VERSION,
    MY_INTERFACE_1_V123_CAPABILITIES);

static void az_ulib_descriptor_interface_descriptor_succeed(void** state)
{
  /// arrange
  (void)state;

  /// act
  /// assert
  /* Interface. */
  assert_true(az_span_is_content_equal(
      MY_INTERFACE._internal.pkg_name, AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME)));
  assert_int_equal(MY_INTERFACE._internal.pkg_version, MY_PACKAGE_1_VERSION);
  assert_true(az_span_is_content_equal(
      MY_INTERFACE._internal.intf_name, AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME)));
  assert_int_equal(MY_INTERFACE._internal.intf_version, MY_INTERFACE_123_VERSION);
  assert_int_equal(MY_INTERFACE._internal.size, 5);

  /* AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
   *          MY_INTERFACE_GET_MY_PROPERTY_NAME, get_my_property, get_my_property_span_wrapper)
   */
  assert_true(az_span_is_content_equal(
      MY_INTERFACE._internal.capability_list[0]._internal.name,
      AZ_SPAN_FROM_STR(MY_INTERFACE_GET_MY_PROPERTY_NAME)));
  assert_ptr_equal(
      MY_INTERFACE._internal.capability_list[0]._internal.capability_ptr, get_my_property);
  assert_ptr_equal(
      MY_INTERFACE._internal.capability_list[0]._internal.capability_span_wrapper,
      get_my_property_span_wrapper);

  /* AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
   *          MY_INTERFACE_SET_MY_PROPERTY_NAME, set_my_property, set_my_property_span_wrapper)
   */
  assert_true(az_span_is_content_equal(
      MY_INTERFACE._internal.capability_list[1]._internal.name,
      AZ_SPAN_FROM_STR(MY_INTERFACE_SET_MY_PROPERTY_NAME)));
  assert_ptr_equal(
      MY_INTERFACE._internal.capability_list[1]._internal.capability_ptr, set_my_property);
  assert_ptr_equal(
      MY_INTERFACE._internal.capability_list[1]._internal.capability_span_wrapper,
      set_my_property_span_wrapper);

  /* AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY_NAME) */
  assert_true(az_span_is_content_equal(
      MY_INTERFACE._internal.capability_list[2]._internal.name,
      AZ_SPAN_FROM_STR(MY_INTERFACE_MY_TELEMETRY_NAME)));
  assert_ptr_equal(MY_INTERFACE._internal.capability_list[2]._internal.capability_ptr, NULL);
  assert_ptr_equal(
      MY_INTERFACE._internal.capability_list[2]._internal.capability_span_wrapper, NULL);

  /* AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY2_NAME) */
  assert_true(az_span_is_content_equal(
      MY_INTERFACE._internal.capability_list[3]._internal.name,
      AZ_SPAN_FROM_STR(MY_INTERFACE_MY_TELEMETRY2_NAME)));
  assert_ptr_equal(MY_INTERFACE._internal.capability_list[3]._internal.capability_ptr, NULL);
  assert_ptr_equal(
      MY_INTERFACE._internal.capability_list[3]._internal.capability_span_wrapper, NULL);

  /* AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
   *          MY_INTERFACE_MY_COMMAND_NAME, my_command, my_command_span_wrapper)
   */
  assert_true(az_span_is_content_equal(
      MY_INTERFACE._internal.capability_list[4]._internal.name,
      AZ_SPAN_FROM_STR(MY_INTERFACE_MY_COMMAND_NAME)));
  assert_ptr_equal(MY_INTERFACE._internal.capability_list[4]._internal.capability_ptr, my_command);
  assert_ptr_equal(
      MY_INTERFACE._internal.capability_list[4]._internal.capability_span_wrapper,
      my_command_span_wrapper);

  /// cleanup
}

int az_ulib_descriptor_ut()
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY_w_null_wrapper_succeed),
    cmocka_unit_test(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY_succeed),
    cmocka_unit_test(az_ulib_descriptor_AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY_succeed),
    cmocka_unit_test(az_ulib_descriptor_interface_descriptor_succeed),
  };

  return cmocka_run_group_tests_name("az_ulib_descriptor_ut", tests, NULL, NULL);
}
