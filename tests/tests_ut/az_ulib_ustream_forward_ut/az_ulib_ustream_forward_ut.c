// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "az_ulib_ustream_forward.h"
#include "az_ulib_ustream_forward_ut.h"

#include "az_ulib_ustream_forward_mock_buffer.h"

#include "az_ulib_test_precondition.h"
#include "azure/core/az_precondition.h"

#include "cmocka.h"

/* define constants for the compliance test */
#define USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT \
  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62

static const uint8_t* const USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT
    = (const uint8_t* const)USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT;

static az_ulib_ustream_forward test_ustream_forward;
static void ustream_forward_factory(az_ulib_ustream_forward** ustream_forward)
{
  *ustream_forward
      = (az_ulib_ustream_forward*)malloc(sizeof(az_ulib_ustream_forward));
  uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t) * USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  assert_non_null(buf);
  (void)memcpy(
      buf, USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT, USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  assert_int_equal(
      az_ulib_ustream_forward_init(
          *ustream_forward,
          free,
          buf,
          USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
          free),
      AZ_OK);
}
#define USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(ustream_forward) ustream_forward_factory(ustream_forward)

#define TEST_CONST_BUFFER_LENGTH (USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 2)
#define TEST_CONST_MAX_BUFFER_SIZE (TEST_CONST_BUFFER_LENGTH - 1)

#ifndef AZ_NO_PRECONDITION_CHECKING
AZ_ULIB_ENABLE_PRECONDITION_CHECK_TESTS()
#endif // AZ_NO_PRECONDITION_CHECKING

/**
 * Beginning of the UT for ustream_forward.c on ownership model.
 */
static int setup(void** state)
{
  (void)state;

  memset(&test_ustream_forward, 0, sizeof(az_ulib_ustream_forward));

  return 0;
}

static int teardown(void** state)
{
  (void)state;

  reset_mock_buffer();

  return 0;
}

#ifndef AZ_NO_PRECONDITION_CHECKING
/* az_ulib_ustream_forward_init shall fail with precondition if the provided buffer is NULL. */
static void az_ulib_ustream_forward_init_null_buffer_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward ustream_forward;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ustream_forward_init(
      &ustream_forward,
      NULL,
      NULL,
      USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      NULL));

  /// cleanup
}

/* az_ulib_ustream_forward_init shall fail with precondition if the provided buffer length is zero. */
static void az_ulib_ustream_forward_init_zero_length_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward ustream_forward;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ustream_forward_init(
      &ustream_forward, 
      NULL, 
      USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
      0, 
      NULL));

  /// cleanup
}

/* az_ulib_ustream_forward_init shall fail with precondition if the provided ustream_forward 
    is NULL. */
static void az_ulib_ustream_forward_init_NULL_ustream_forward_instance_failed(void** state)
{
  /// arrange
  (void)state;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ustream_forward_init(
      NULL,
      NULL,
      USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      NULL));

  /// cleanup
}
#endif // AZ_NO_PRECONDITION_CHECKING

/* az_ulib_ustream_forward_init shall create an instance of the ustream_forward and initialize the instance. */
static void az_ulib_ustream_forward_init_const_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward ustream_forward;

  /// act
  az_result result = az_ulib_ustream_forward_init(
      &ustream_forward,
      NULL,
      USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      NULL);

  /// assert
  assert_int_equal(result, AZ_OK);

  /// cleanup
  result = az_ulib_ustream_forward_dispose(&ustream_forward);
  (void)result;
}

/* az_ulib_ustream_forward_init shall create an instance of the ustream_forward and initialize the instance. */
static void az_ulib_ustream_forward_init_succeed(void** state)
{
  /// arrange
  (void)state;
  uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t) * USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  assert_non_null(buf);
  (void)memcpy(
      buf, USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT, USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  az_ulib_ustream_forward ustream_forward;

  /// act
  az_result result = az_ulib_ustream_forward_init(
      &ustream_forward,
      NULL,
      buf,
      USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      free);

  /// assert
  assert_int_equal(result, AZ_OK);

  /// cleanup
  result = az_ulib_ustream_forward_dispose(&ustream_forward);
  (void)result;
}

#include "az_ulib_ustream_forward_compliance_ut.h"

int az_ulib_ustream_forward_ut()
{
#ifndef AZ_NO_PRECONDITION_CHECKING
  AZ_ULIB_SETUP_PRECONDITION_CHECK_TESTS();
#endif // AZ_NO_PRECONDITION_CHECKING

  const struct CMUnitTest tests[] = {
#ifndef AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test(az_ulib_ustream_forward_init_null_buffer_failed),
    cmocka_unit_test(az_ulib_ustream_forward_init_zero_length_failed),
    cmocka_unit_test(az_ulib_ustream_forward_init_NULL_ustream_forward_instance_failed),
#endif // AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test_setup_teardown(az_ulib_ustream_forward_init_const_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ustream_forward_init_succeed, setup, teardown),
#ifndef AZ_NO_PRECONDITION_CHECKING
    AZ_ULIB_USTREAM_FORWARD_PRECONDITION_COMPLIANCE_UT_LIST
#endif // AZ_NO_PRECONDITION_CHECKING
        AZ_ULIB_USTREAM_FORWARD_COMPLIANCE_UT_LIST
  };

  return cmocka_run_group_tests_name("az_ulib_ustream_forward_ut", tests, NULL, NULL);
}
