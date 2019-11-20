// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#else
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "az_ulib_ucontract.h"
#include "umock_c/umock_c.h"
#include "umock_c/umock_c_negative_tests.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_stdint.h"

static TEST_MUTEX_HANDLE g_test_by_test;

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code) {
  ASSERT_FAIL("umock_c reported error :%i", error_code);
}

/**
 * Beginning of the UT for ucontract module.
 */
BEGIN_TEST_SUITE(ucontract_ut)

TEST_SUITE_INITIALIZE(suite_init) {
  g_test_by_test = TEST_MUTEX_CREATE();
  ASSERT_IS_NOT_NULL(g_test_by_test);

  ASSERT_ARE_EQUAL(int, 0, umock_c_init(on_umock_c_error));
  ASSERT_ARE_EQUAL(int, 0, umocktypes_charptr_register_types());
  ASSERT_ARE_EQUAL(int, 0, umocktypes_stdint_register_types());
  ASSERT_ARE_EQUAL(int, 0, umocktypes_bool_register_types());
}

static int require(bool expression) {
  AZ_UCONTRACT_REQUIRE(expression, -1, "Require invoked");
  return 0;
}

static int require_equals(bool expression) {
  AZ_UCONTRACT_REQUIRE_EQUALS(expression, true, -1);
  return 0;
}

static int require_not_equals(bool expression) {
  AZ_UCONTRACT_REQUIRE_NOT_EQUALS(expression, false, -1);
  return 0;
}

static int require_not_null(void* ptr) {
  AZ_UCONTRACT_REQUIRE_NOT_NULL(ptr, -1);
  return 0;
}

static int ucontract_with_one_arg(bool expression) {
  AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(expression, -1, "Require invoked"));
  return 0;
}

static int ucontract_with_two_arg(void* ptr, bool expression) {
  AZ_UCONTRACT(
      AZ_UCONTRACT_REQUIRE_NOT_NULL(ptr, -1),
      AZ_UCONTRACT_REQUIRE(expression, -2, "Require invoked"));
  return 0;
}

static int ucontract_with_three_arg(void* ptr, bool expression, bool val_equals) {
  AZ_UCONTRACT(
      AZ_UCONTRACT_REQUIRE_NOT_NULL(ptr, -1),
      AZ_UCONTRACT_REQUIRE(expression, -2, "Require invoked"),
      AZ_UCONTRACT_REQUIRE_EQUALS(val_equals, true, -3));
  return 0;
}

static int ucontract_with_four_arg(void* ptr, bool expression, bool val_equals, int num) {
  AZ_UCONTRACT(
      AZ_UCONTRACT_REQUIRE_NOT_NULL(ptr, -1),
      AZ_UCONTRACT_REQUIRE(expression, -2, "Require invoked"),
      AZ_UCONTRACT_REQUIRE_EQUALS(val_equals, true, -3),
      AZ_UCONTRACT_REQUIRE_NOT_EQUALS(num, 0, -4));
  return 0;
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

/* The AZ_UCONTRACT_REQUIRE shall do nothing if the required expression returns true. */
TEST_FUNCTION(AZ_UCONTRACT_REQUIRE_Succeed) {
  /// arrange

  /// act
  int result = require(true);

  /// assert
  ASSERT_ARE_EQUAL(int, 0, result);

  /// cleanup
}

TEST_FUNCTION(AZ_UCONTRACT_REQUIRE_EQUALS_Succeed) {
  /// arrange

  /// act
  int result = require_equals(true);

  /// assert
  ASSERT_ARE_EQUAL(int, 0, result);

  /// cleanup
}

TEST_FUNCTION(AZ_UCONTRACT_REQUIRE_NOT_EQUALS_Succeed) {
  /// arrange

  /// act
  int result = require_not_equals(true);

  /// assert
  ASSERT_ARE_EQUAL(int, 0, result);

  /// cleanup
}

TEST_FUNCTION(AZ_UCONTRACT_REQUIRE_NOT_NULL_Succeed) {
  /// arrange
  int i = 0;

  /// act
  int result = require_not_null(&i);

  /// assert
  ASSERT_ARE_EQUAL(int, 0, result);

  /// cleanup
}

/* The AZ_UCONTRACT_REQUIRE shall return `result` if the required expression returns false. */
TEST_FUNCTION(AZ_UCONTRACT_REQUIRE_Failed) {
  /// arrange

  /// act
  int result = require(false);

  /// assert
  ASSERT_ARE_EQUAL(int, -1, result);

  /// cleanup
}

TEST_FUNCTION(AZ_UCONTRACT_REQUIRE_EQUALS_Failed) {
  /// arrange

  /// act
  int result = require_equals(false);

  /// assert
  ASSERT_ARE_EQUAL(int, -1, result);

  /// cleanup
}

TEST_FUNCTION(AZ_UCONTRACT_REQUIRE_NOT_EQUALS_Failed) {
  /// arrange

  /// act
  int result = require_not_equals(false);

  /// assert
  ASSERT_ARE_EQUAL(int, -1, result);

  /// cleanup
}

TEST_FUNCTION(AZ_UCONTRACT_REQUIRE_NOT_NULL_Failed) {
  /// arrange

  /// act
  int result = require_not_null(NULL);

  /// assert
  ASSERT_ARE_EQUAL(int, -1, result);

  /// cleanup
}

/* The AZ_UCONTRACT shall do nothing if the required expressions return true. */
TEST_FUNCTION(AZ_UCONTRACT_ucontract_with_one_arg_succeed) {
  /// arrange

  /// act
  int result = ucontract_with_one_arg(true);

  /// assert
  ASSERT_ARE_EQUAL(int, 0, result);

  /// cleanup
}

TEST_FUNCTION(AZ_UCONTRACT_ucontract_with_two_arg_succeed) {
  /// arrange
  int i = 0;

  /// act
  int result = ucontract_with_two_arg(&i, true);

  /// assert
  ASSERT_ARE_EQUAL(int, 0, result);

  /// cleanup
}

TEST_FUNCTION(AZ_UCONTRACT_ucontract_with_three_arg_succeed) {
  /// arrange
  int i = 0;

  /// act
  int result = ucontract_with_three_arg(&i, true, true);

  /// assert
  ASSERT_ARE_EQUAL(int, 0, result);

  /// cleanup
}

TEST_FUNCTION(AZ_UCONTRACT_ucontract_with_four_arg_succeed) {
  /// arrange
  int i = 0;

  /// act
  int result = ucontract_with_four_arg(&i, true, true, 5);

  /// assert
  ASSERT_ARE_EQUAL(int, 0, result);

  /// cleanup
}

/* The AZ_UCONTRACT_REQUIRE shall return `result` if at least one of the required expressions return
 * false. */
TEST_FUNCTION(AZ_UCONTRACT_REQUIRE_ucontract_with_one_arg_failed) {
  /// arrange

  /// act
  int result = ucontract_with_one_arg(false);

  /// assert
  ASSERT_ARE_EQUAL(int, -1, result);

  /// cleanup
}

TEST_FUNCTION(AZ_UCONTRACT_ucontract_with_two_arg_first_failed) {
  /// arrange

  /// act
  int result = ucontract_with_two_arg(NULL, true);

  /// assert
  ASSERT_ARE_EQUAL(int, -1, result);

  /// cleanup
}

TEST_FUNCTION(AZ_UCONTRACT_ucontract_with_two_arg_second_failed) {
  /// arrange
  int i = 0;

  /// act
  int result = ucontract_with_two_arg(&i, false);

  /// assert
  ASSERT_ARE_EQUAL(int, -2, result);

  /// cleanup
}

TEST_FUNCTION(AZ_UCONTRACT_ucontract_with_three_arg_failed) {
  /// arrange
  int i = 0;

  /// act
  int result = ucontract_with_three_arg(&i, true, false);

  /// assert
  ASSERT_ARE_EQUAL(int, -3, result);

  /// cleanup
}

TEST_FUNCTION(AZ_UCONTRACT_ucontract_with_four_arg_failed) {
  /// arrange
  int i = 0;

  /// act
  int result = ucontract_with_four_arg(&i, true, true, 0);

  /// assert
  ASSERT_ARE_EQUAL(int, -4, result);

  /// cleanup
}

END_TEST_SUITE(ucontract_ut)
