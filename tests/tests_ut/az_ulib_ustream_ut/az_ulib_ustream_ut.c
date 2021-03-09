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

#include "az_ulib_ctest_aux.h"
#include "az_ulib_test_precondition.h"
#include "az_ulib_ustream_mock_buffer.h"
#include "azure/core/az_precondition.h"
#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umock_c_negative_tests.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_stdint.h"

static TEST_MUTEX_HANDLE g_test_by_test;

#include "az_ulib_ustream.h"
#include "az_ulib_ustream_base.h"

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT \
  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62

static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT
    = (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;
static az_ulib_ustream test_ustream_instance;
static void ustream_factory(az_ulib_ustream* ustream)
{
  az_ulib_ustream_data_cb* ustream_control_block
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t) * USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  (void)memcpy(
      buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ustream_init(
          ustream,
          ustream_control_block,
          free,
          buf,
          USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
          free));
}
#define USTREAM_COMPLIANCE_TARGET_FACTORY(ustream) ustream_factory(ustream)

#define TEST_CONST_BUFFER_LENGTH (USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 2)
#define TEST_CONST_MAX_BUFFER_SIZE (TEST_CONST_BUFFER_LENGTH - 1)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
  ASSERT_FAIL("umock_c reported error :%i", error_code);
}

#ifndef AZ_NO_PRECONDITION_CHECKING
AZ_ULIB_ENABLE_PRECONDITION_CHECK_TESTS()
#endif // AZ_NO_PRECONDITION_CHECKING

/**
 * Beginning of the UT for ustream.c on ownership model.
 */
BEGIN_TEST_SUITE(ustream_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
  int result;

  g_test_by_test = TEST_MUTEX_CREATE();
  ASSERT_IS_NOT_NULL(g_test_by_test);

#ifndef AZ_NO_PRECONDITION_CHECKING
  AZ_ULIB_SETUP_PRECONDITION_CHECK_TESTS();
#endif // AZ_NO_PRECONDITION_CHECKING

  result = umock_c_init(on_umock_c_error);
  ASSERT_ARE_EQUAL(int, 0, result);
  result = umocktypes_charptr_register_types();
  ASSERT_ARE_EQUAL(int, 0, result);
  result = umocktypes_stdint_register_types();
  ASSERT_ARE_EQUAL(int, 0, result);
  result = umocktypes_bool_register_types();
  ASSERT_ARE_EQUAL(int, 0, result);

  REGISTER_UMOCK_ALIAS_TYPE(az_ulib_ustream, void*);
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

  memset(&test_ustream_instance, 0, sizeof(az_ulib_ustream));

  umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(test_method_cleanup)
{
  reset_mock_buffer();

  TEST_MUTEX_RELEASE(g_test_by_test);
}

#ifndef AZ_NO_PRECONDITION_CHECKING
/* az_ulib_ustream_init shall fail with precondition if the provided constant buffer is NULL. */
TEST_FUNCTION(az_ulib_ustream_init_null_buffer_failed)
{
  /// arrange
  az_ulib_ustream ustream_instance;
  az_ulib_ustream_data_cb control_block;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ustream_init(
      &ustream_instance,
      &control_block,
      NULL,
      NULL,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      NULL));

  /// cleanup
}

/* az_ulib_ustream_init shall fail with precondition if the provided buffer length is zero. */
TEST_FUNCTION(az_ulib_ustream_init_zero_length_failed)
{
  /// arrange
  az_ulib_ustream ustream_instance;
  az_ulib_ustream_data_cb control_block;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ustream_init(
      &ustream_instance, &control_block, NULL, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 0, NULL));

  /// cleanup
}

/* az_ulib_ustream_init shall fail with precondition if the provided buffer length is zero. */
TEST_FUNCTION(az_ulib_ustream_init_NULL_ustream_instance_failed)
{
  /// arrange
  az_ulib_ustream_data_cb control_block;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ustream_init(
      NULL,
      &control_block,
      NULL,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      NULL));

  /// cleanup
}

/* az_ulib_ustream_init shall fail with precondition if the provided buffer length is zero. */
TEST_FUNCTION(az_ulib_ustream_init_NULL_control_block_failed)
{
  /// arrange
  az_ulib_ustream ustream_instance;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ustream_init(
      &ustream_instance,
      NULL,
      NULL,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      NULL));

  /// cleanup
}
#endif // AZ_NO_PRECONDITION_CHECKING

/* az_ulib_ustream_init shall create an instance of the ustream and initialize the instance. */
TEST_FUNCTION(az_ulib_ustream_init_const_succeed)
{
  /// arrange
  az_ulib_ustream_data_cb* control_block
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  umock_c_reset_all_calls();
  az_ulib_ustream ustream_instance;

  /// act
  az_result result = az_ulib_ustream_init(
      &ustream_instance,
      control_block,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      NULL);

  /// assert
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
  ASSERT_ARE_EQUAL(int, AZ_OK, result);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* az_ulib_ustream_init shall create an instance of the ustream and initialize the instance. */
TEST_FUNCTION(az_ulib_ustream_init_succeed)
{
  /// arrange
  uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t) * USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  (void)memcpy(
      buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  az_ulib_ustream_data_cb* control_block
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  umock_c_reset_all_calls();
  az_ulib_ustream ustream_instance;

  /// act
  az_result result = az_ulib_ustream_init(
      &ustream_instance,
      control_block,
      free,
      buf,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      free);

  /// assert
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
  ASSERT_ARE_EQUAL(int, AZ_OK, result);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

#include "az_ulib_ustream_compliance_ut.h"

END_TEST_SUITE(ustream_ut)
