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

static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1
    = (const uint8_t* const) "0123456789";
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2
    = (const uint8_t* const) "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3
    = (const uint8_t* const) "abcdefghijklmnopqrstuvwxyz";

static void create_test_default_multibuffer(az_ulib_ustream* ustream)
{
  // Set up required structs for first multi
  az_ulib_ustream_multi_data_cb* default_multi_data1
      = (az_ulib_ustream_multi_data_cb*)malloc(sizeof(az_ulib_ustream_multi_data_cb));
  ASSERT_IS_NOT_NULL(default_multi_data1);
  memset(default_multi_data1, 0, sizeof(az_ulib_ustream_multi_data_cb));

  // Set up required structs for second multi
  az_ulib_ustream_multi_data_cb* default_multi_data2
      = (az_ulib_ustream_multi_data_cb*)malloc(sizeof(az_ulib_ustream_multi_data_cb));
  ASSERT_IS_NOT_NULL(default_multi_data2);
  memset(default_multi_data2, 0, sizeof(az_ulib_ustream_multi_data_cb));

  // Set up first ustream
  az_ulib_ustream_data_cb* ustream_control_block1
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  memset(ustream_control_block1, 0, sizeof(az_ulib_ustream_data_cb));
  az_result result = az_ulib_ustream_init(
      ustream,
      ustream_control_block1,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result);

  // Set up second ustream
  az_ulib_ustream default_buffer2;
  az_ulib_ustream_data_cb* ustream_control_block2
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  memset(ustream_control_block2, 0, sizeof(az_ulib_ustream_data_cb));
  result = az_ulib_ustream_init(
      &default_buffer2,
      ustream_control_block2,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result);

  // Set up third ustream
  az_ulib_ustream default_buffer3;
  az_ulib_ustream_data_cb* ustream_control_block3
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  memset(ustream_control_block3, 0, sizeof(az_ulib_ustream_data_cb));
  result = az_ulib_ustream_init(
      &default_buffer3,
      ustream_control_block3,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result);

  // Concat buffers together
  ASSERT_ARE_EQUAL(
      int, AZ_OK, az_ulib_ustream_concat(ustream, &default_buffer2, default_multi_data1, free));
  ASSERT_ARE_EQUAL(
      int, AZ_OK, az_ulib_ustream_concat(ustream, &default_buffer3, default_multi_data2, free));

  (void)az_ulib_ustream_dispose(&default_buffer2);
  (void)az_ulib_ustream_dispose(&default_buffer3);
}

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT \
  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT
    = (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;
#define USTREAM_COMPLIANCE_TARGET_FACTORY(ustream) create_test_default_multibuffer(ustream)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
  ASSERT_FAIL("umock_c reported error :%i", error_code);
}

#ifndef AZ_NO_PRECONDITION_CHECKING
AZ_ULIB_ENABLE_PRECONDITION_CHECK_TESTS()
#endif // AZ_NO_PRECONDITION_CHECKING

/**
 * Beginning of the UT for ustream_aux.c module.
 */
BEGIN_TEST_SUITE(ustream_aux_ut)

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

  umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(test_method_cleanup)
{
  reset_mock_buffer();

  TEST_MUTEX_RELEASE(g_test_by_test);
}

#ifndef AZ_NO_PRECONDITION_CHECKING
/* az_ulib_ustream_concat shall fail with precondition if the provided ustream is NULL. */
TEST_FUNCTION(az_ulib_ustream_concat_null_instance_failed)
{
  /// arrange
  az_ulib_ustream_data_cb* control_block
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  ASSERT_IS_NOT_NULL(control_block);
  az_ulib_ustream test_buffer;
  az_result result1 = az_ulib_ustream_init(
      &test_buffer,
      control_block,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result1);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ustream_concat(NULL, &test_buffer, NULL, free));

  /// cleanup
  az_ulib_ustream_dispose(&test_buffer);
}

/* az_ulib_ustream_concat shall fail with precondition if the provided ustream to add is NULL. */
TEST_FUNCTION(az_ulib_ustream_concat_null_buffer_to_add_failed)
{
  /// arrange
  az_ulib_ustream_data_cb* control_block
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  ASSERT_IS_NOT_NULL(control_block);
  az_ulib_ustream default_buffer;
  az_result result1 = az_ulib_ustream_init(
      &default_buffer,
      control_block,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result1);

  az_ulib_ustream_multi_data_cb multi_data;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      az_ulib_ustream_concat(&default_buffer, NULL, &multi_data, NULL));

  /// cleanup
  az_ulib_ustream_dispose(&default_buffer);
}

/* az_ulib_ustream_concat shall fail with precondition if the provided multi data to add is NULL. */
TEST_FUNCTION(az_ulib_ustream_concat_null_multi_data_failed)
{
  /// arrange
  az_ulib_ustream_data_cb* control_block
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  ASSERT_IS_NOT_NULL(control_block);
  az_ulib_ustream default_buffer;
  az_result result1 = az_ulib_ustream_init(
      &default_buffer,
      control_block,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result1);

  az_ulib_ustream_data_cb* control_block2
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  ASSERT_IS_NOT_NULL(control_block2);
  az_ulib_ustream test_buffer2;
  az_result result2 = az_ulib_ustream_init(
      &test_buffer2,
      control_block2,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
      NULL);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      az_ulib_ustream_concat(&default_buffer, &test_buffer2, NULL, NULL));

  /// cleanup
  az_ulib_ustream_dispose(&test_buffer2);
  az_ulib_ustream_dispose(&default_buffer);
}

/* az_ulib_ustream_split shall fail with precondition if the provided ustream is NULL. */
TEST_FUNCTION(az_ulib_ustream_split_null_instance_failed)
{
  /// arrange
  az_ulib_ustream test_buffer;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ustream_split(NULL, &test_buffer, 4));

  /// cleanup
}

/* az_ulib_ustream_split shall fail with precondition if the provided ustream is NULL. */
TEST_FUNCTION(az_ulib_ustream_split_null_split_instance_failed)
{
  /// arrange
  az_ulib_ustream test_buffer;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ustream_split(&test_buffer, NULL, 4));

  /// cleanup
}

#endif // AZ_NO_PRECONDITION_CHECKING

/* az_ulib_ustream_concat shall return AZ_OK if the ustreams were concatenated successfully
 */
TEST_FUNCTION(az_ulib_ustream_concat_multiple_buffers_succeed)
{
  /// arrange
  az_ulib_ustream_data_cb* control_block1
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  ASSERT_IS_NOT_NULL(control_block1);
  az_ulib_ustream test_buffer1;
  az_result result1 = az_ulib_ustream_init(
      &test_buffer1,
      control_block1,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result1);

  az_ulib_ustream_data_cb* control_block2
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  ASSERT_IS_NOT_NULL(control_block2);
  az_ulib_ustream test_buffer2;
  az_result result2 = az_ulib_ustream_init(
      &test_buffer2,
      control_block2,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result2);

  az_ulib_ustream_data_cb* control_block3
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  ASSERT_IS_NOT_NULL(control_block3);
  az_ulib_ustream test_buffer3;
  az_result result3 = az_ulib_ustream_init(
      &test_buffer3,
      control_block3,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result3);

  az_ulib_ustream_multi_data_cb* multi_data1
      = (az_ulib_ustream_multi_data_cb*)malloc(sizeof(az_ulib_ustream_multi_data_cb));
  az_ulib_ustream_multi_data_cb* multi_data2
      = (az_ulib_ustream_multi_data_cb*)malloc(sizeof(az_ulib_ustream_multi_data_cb));

  /// act
  result1 = az_ulib_ustream_concat(&test_buffer1, &test_buffer2, multi_data1, free);
  result2 = az_ulib_ustream_concat(&test_buffer1, &test_buffer3, multi_data2, free);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result1);
  ASSERT_ARE_EQUAL(int, AZ_OK, result2);
  az_ulib_ustream_dispose(&test_buffer2);
  az_ulib_ustream_dispose(&test_buffer3);
  check_buffer(
      &test_buffer1,
      0,
      (const uint8_t*)USTREAM_COMPLIANCE_EXPECTED_CONTENT,
      (uint8_t)strlen(USTREAM_COMPLIANCE_EXPECTED_CONTENT));

  /// cleanup
  az_ulib_ustream_dispose(&test_buffer1);
}

/* az_ulib_ustream_read shall read from all the concatenated ustreams and return AZ_OK */
TEST_FUNCTION(az_ulib_ustream_concat_read_from_multiple_buffers_succeed)
{
  /// arrange
  az_ulib_ustream_data_cb* control_block1
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  ASSERT_IS_NOT_NULL(control_block1);
  az_ulib_ustream test_buffer1;
  az_result result1 = az_ulib_ustream_init(
      &test_buffer1,
      control_block1,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result1);

  az_ulib_ustream_data_cb* control_block2
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  ASSERT_IS_NOT_NULL(control_block2);
  az_ulib_ustream test_buffer2;
  az_result result2 = az_ulib_ustream_init(
      &test_buffer2,
      control_block2,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result2);

  az_ulib_ustream_data_cb* control_block3
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  ASSERT_IS_NOT_NULL(control_block3);
  az_ulib_ustream test_buffer3;
  az_result result3 = az_ulib_ustream_init(
      &test_buffer3,
      control_block3,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result3);

  az_ulib_ustream_multi_data_cb* multi_data1
      = (az_ulib_ustream_multi_data_cb*)malloc(sizeof(az_ulib_ustream_multi_data_cb));
  az_ulib_ustream_multi_data_cb* multi_data2
      = (az_ulib_ustream_multi_data_cb*)malloc(sizeof(az_ulib_ustream_multi_data_cb));

  result1 = az_ulib_ustream_concat(&test_buffer1, &test_buffer2, multi_data1, free);
  result2 = az_ulib_ustream_concat(&test_buffer1, &test_buffer3, multi_data2, free);
  ASSERT_ARE_EQUAL(int, AZ_OK, result1);
  ASSERT_ARE_EQUAL(int, AZ_OK, result2);

  uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
  size_t size_result;

  /// act
  az_result result = az_ulib_ustream_read(
      &test_buffer1, buf_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, &size_result);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_BUFFER_ARE_EQUAL(
      uint8_t_ptr, USTREAM_COMPLIANCE_EXPECTED_CONTENT, buf_result, size_result);

  /// cleanup
  az_ulib_ustream_dispose(&test_buffer2);
  az_ulib_ustream_dispose(&test_buffer3);
  az_ulib_ustream_dispose(&test_buffer1);
}

/* az_ulib_ustream_multi_concat shall return an error if the internal call to
   az_ulib_ustream_get_remaining_size failed when concatenating two ustreams */
TEST_FUNCTION(az_ulib_ustream_concat_new_control_block_failed_on_get_remaining_size_failed)
{
  /// arrange
  az_ulib_ustream multibuffer;
  az_ulib_ustream_data_cb* control_block1
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  az_result result = az_ulib_ustream_init(
      &multibuffer,
      control_block1,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result);

  az_ulib_ustream* test_buffer2 = ustream_mock_create();

  az_ulib_ustream_multi_data_cb* multi_data1
      = (az_ulib_ustream_multi_data_cb*)malloc(sizeof(az_ulib_ustream_multi_data_cb));

  set_get_remaining_size_result(AZ_ERROR_ULIB_SYSTEM);

  /// act
  result = az_ulib_ustream_concat(&multibuffer, test_buffer2, multi_data1, free);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ULIB_SYSTEM, result);

  /// cleanup
  (void)az_ulib_ustream_dispose(test_buffer2);
  (void)az_ulib_ustream_dispose(&multibuffer);
}

/* az_ulib_ustream_read shall return partial result if one of the internal buffers failed. */
TEST_FUNCTION(
    az_ulib_ustream_multi_read_control_block_failed_in_read_with_some_valid_content_succeed)
{
  /// arrange
  az_ulib_ustream multibuffer;
  az_ulib_ustream_data_cb* control_block1
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  az_result result = az_ulib_ustream_init(
      &multibuffer,
      control_block1,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result);

  az_ulib_ustream* test_buffer2 = ustream_mock_create();

  az_ulib_ustream_multi_data_cb* multi_data1
      = (az_ulib_ustream_multi_data_cb*)malloc(sizeof(az_ulib_ustream_multi_data_cb));

  result = az_ulib_ustream_concat(&multibuffer, test_buffer2, multi_data1, free);
  set_read_result(AZ_ERROR_ULIB_SYSTEM);

  uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
  size_t size_result;

  /// act
  result = az_ulib_ustream_read(
      &multibuffer, buf_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, &size_result);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_BUFFER_ARE_EQUAL(
      uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, buf_result, size_result);

  /// cleanup
  (void)az_ulib_ustream_dispose(&multibuffer);
  (void)az_ulib_ustream_dispose(test_buffer2);
}

/* az_ulib_ustream_read shall return AZ_ERROR_ULIB_SYSTEM if it failed to read the requested bytes
 */
TEST_FUNCTION(az_ulib_ustream_multi_read_control_block_failed_in_read_failed)
{
  /// arrange
  az_ulib_ustream multibuffer;
  az_ulib_ustream_data_cb* control_block1
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  az_result result = az_ulib_ustream_init(
      &multibuffer,
      control_block1,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result);

  az_ulib_ustream* test_buffer2 = ustream_mock_create();

  az_ulib_ustream_multi_data_cb* multi_data1
      = (az_ulib_ustream_multi_data_cb*)malloc(sizeof(az_ulib_ustream_multi_data_cb));

  result = az_ulib_ustream_concat(&multibuffer, test_buffer2, multi_data1, free);
  set_read_result(AZ_ERROR_ULIB_SYSTEM);

  uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
  size_t size_result;

  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ustream_read(
          &multibuffer, buf_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, &size_result));
  ASSERT_BUFFER_ARE_EQUAL(
      uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, buf_result, size_result);

  set_read_result(AZ_ERROR_ULIB_SYSTEM);

  /// act
  result = az_ulib_ustream_read(
      &multibuffer, buf_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, &size_result);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ULIB_SYSTEM, result);

  /// cleanup
  (void)az_ulib_ustream_dispose(&multibuffer);
  (void)az_ulib_ustream_dispose(test_buffer2);
}

/* az_ulib_ustream_read shall set the position to an instance's position before reading and return
 * AZ_OK */
TEST_FUNCTION(az_ulib_ustream_multi_read_clone_and_original_in_parallel_succeed)
{
  /// arrange
  az_ulib_ustream multibuffer;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&multibuffer);

  az_ulib_ustream multibuffer_clone;

  az_ulib_ustream_clone(&multibuffer_clone, &multibuffer, 0);

  uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
  size_t size_result;

  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ustream_read(
          &multibuffer,
          buf_result,
          strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
          &size_result));

  ASSERT_BUFFER_ARE_EQUAL(
      uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, buf_result, size_result);

  memset(buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ustream_read(
          &multibuffer_clone,
          buf_result,
          strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
          &size_result));
  ASSERT_BUFFER_ARE_EQUAL(
      uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, buf_result, size_result);

  memset(buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ustream_reset(&multibuffer));

  /// act
  az_result result = az_ulib_ustream_read(
      &multibuffer, buf_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, &size_result);

  ASSERT_BUFFER_ARE_EQUAL(
      uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

  ASSERT_ARE_EQUAL(int, AZ_OK, result);

  /// assert
  result = az_ulib_ustream_read(
      &multibuffer, buf_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, &size_result);

  ASSERT_ARE_EQUAL(int, AZ_ULIB_EOF, result);

  (void)az_ulib_ustream_dispose(&multibuffer);

  memset(buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ustream_read(
          &multibuffer_clone,
          buf_result,
          strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2),
          &size_result));
  ASSERT_BUFFER_ARE_EQUAL(
      uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2, buf_result, size_result);

  memset(buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ustream_read(
          &multibuffer_clone,
          buf_result,
          strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3),
          &size_result));
  ASSERT_BUFFER_ARE_EQUAL(
      uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3, buf_result, size_result);

  ASSERT_ARE_EQUAL(
      int,
      AZ_ULIB_EOF,
      az_ulib_ustream_read(
          &multibuffer_clone,
          buf_result,
          strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3),
          &size_result));

  /// cleanup
  (void)az_ulib_ustream_dispose(&multibuffer_clone);
}

/*-------------------az_ulib_ustream_split() unit tests----------------------*/

/* az_ulib_ustream_split shall return AZ_OK if the split is successful */
TEST_FUNCTION(az_ulib_ustream_split_success)
{
  /// arrange
  az_ulib_ustream* test_ustream = ustream_mock_create();

  az_ulib_ustream ustream_split;

  /// act
  az_result result = az_ulib_ustream_split(test_ustream, &ustream_split, 5);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);

  /// cleanup
  az_ulib_ustream_dispose(test_ustream);
}

/* az_ulib_ustream_split shall return AZ_ERROR_ITEM_NOT_FOUND if the provided split position
 * is invalid */
TEST_FUNCTION(az_ulib_ustream_split_invalid_split_position_failed)
{
  /// arrange
  az_ulib_ustream_data_cb* control_block
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  ASSERT_IS_NOT_NULL(control_block);
  az_ulib_ustream test_ustream;
  az_result result = az_ulib_ustream_init(
      &test_ustream,
      control_block,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result);

  az_ulib_ustream ustream_split;

  /// act
  result = az_ulib_ustream_split(&test_ustream, &ustream_split, -1);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ITEM_NOT_FOUND, result);

  /// cleanup
  az_ulib_ustream_dispose(&test_ustream);
}

/* az_ulib_ustream_split shall return AZ_ERROR_ITEM_NOT_FOUND if the provided split position
 * is invalid */
TEST_FUNCTION(az_ulib_ustream_split_invalid_split_position_with_offset_failed)
{
  /// arrange
  az_ulib_ustream_data_cb* control_block
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  ASSERT_IS_NOT_NULL(control_block);
  az_ulib_ustream test_ustream;
  az_result result = az_ulib_ustream_init(
      &test_ustream,
      control_block,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result);

  az_ulib_ustream test_ustream_clone;

  az_ulib_ustream_clone(&test_ustream_clone, &test_ustream, 10);

  az_ulib_ustream ustream_split;

  /// act
  result = az_ulib_ustream_split(&test_ustream_clone, &ustream_split, 5);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ITEM_NOT_FOUND, result);

  /// cleanup
  az_ulib_ustream_dispose(&test_ustream);
  az_ulib_ustream_dispose(&test_ustream_clone);
}

/* az_ulib_ustream_split shall return AZ_ERROR_ITEM_NOT_FOUND if the provided split position is
 * invalid */
TEST_FUNCTION(az_ulib_ustream_split_invalid_split_position_with_offset_after_failed)
{
  /// arrange
  az_ulib_ustream_data_cb* control_block
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  ASSERT_IS_NOT_NULL(control_block);
  az_ulib_ustream test_ustream;
  size_t content_length = strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1);
  az_result result = az_ulib_ustream_init(
      &test_ustream,
      control_block,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
      content_length,
      NULL);
  ASSERT_ARE_EQUAL(int, AZ_OK, result);

  az_ulib_ustream test_ustream_clone;

  az_ulib_ustream_clone(&test_ustream_clone, &test_ustream, 10);

  az_ulib_ustream ustream_split;

  /// act
  result = az_ulib_ustream_split(&test_ustream_clone, &ustream_split, 10 + content_length + 1);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ITEM_NOT_FOUND, result);

  /// cleanup
  az_ulib_ustream_dispose(&test_ustream);
  az_ulib_ustream_dispose(&test_ustream_clone);
}

/* az_ulib_ustream_split shall return the return value of az_ulib_ustream_get_position if it fails
 */
TEST_FUNCTION(az_ulib_ustream_split_get_position_failed)
{
  /// arrange
  az_ulib_ustream* test_ustream = ustream_mock_create();

  az_ulib_ustream ustream_split;

  set_get_position_result(AZ_ERROR_ULIB_SYSTEM);

  /// act
  az_result result = az_ulib_ustream_split(test_ustream, &ustream_split, 5);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ULIB_SYSTEM, result);

  /// cleanup
  az_ulib_ustream_dispose(test_ustream);
}

/* az_ulib_ustream_split shall return AZ_ILLEGAL_ARGUMENT_ERROR if the input size_pos is equal to
 * the current position */
TEST_FUNCTION(az_ulib_ustream_split_position_same_as_current_failed)
{
  /// arrange
  az_ulib_ustream* test_ustream = ustream_mock_create();

  az_ulib_ustream ustream_split;

  /// act
  az_result result = az_ulib_ustream_split(test_ustream, &ustream_split, 0);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ARG, result);

  /// cleanup
  az_ulib_ustream_dispose(test_ustream);
}

/* az_ulib_ustream_split shall return the return value of az_ulib_ustream_get_position if it fails
 */
TEST_FUNCTION(az_ulib_ustream_split_get_remaining_size_failed)
{
  /// arrange
  az_ulib_ustream* test_ustream = ustream_mock_create();

  az_ulib_ustream ustream_split;

  set_get_remaining_size_result(AZ_ERROR_ULIB_SYSTEM);

  /// act
  az_result result = az_ulib_ustream_split(test_ustream, &ustream_split, 5);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ULIB_SYSTEM, result);

  /// cleanup
  az_ulib_ustream_dispose(test_ustream);
}

/* az_ulib_ustream_split shall return AZ_ILLEGAL_ARGUMENT_ERROR if the input size_pos is equal to
 * the current position + remaining size */
TEST_FUNCTION(az_ulib_ustream_split_position_end_of_ustream_failed)
{
  /// arrange
  az_ulib_ustream* test_ustream = ustream_mock_create();

  az_ulib_ustream ustream_split;

  size_t remaining_size;
  az_ulib_ustream_get_remaining_size(test_ustream, &remaining_size);

  /// act
  az_result result = az_ulib_ustream_split(test_ustream, &ustream_split, remaining_size);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ARG, result);

  /// cleanup
  az_ulib_ustream_dispose(test_ustream);
}

/* az_ulib_ustream_split shall return the return value of az_ulib_ustream_set_position if it fails
 */
TEST_FUNCTION(az_ulib_ustream_split_set_position_failed)
{
  /// arrange
  az_ulib_ustream* test_ustream = ustream_mock_create();

  az_ulib_ustream ustream_split;

  set_set_position_result(AZ_ERROR_ULIB_SYSTEM);

  /// act
  az_result result = az_ulib_ustream_split(test_ustream, &ustream_split, 5);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ULIB_SYSTEM, result);

  /// cleanup
  az_ulib_ustream_dispose(test_ustream);
}

/* az_ulib_ustream_split shall return the return value of az_ulib_ustream_clone if it fails */
TEST_FUNCTION(az_ulib_ustream_split_clone_failed)
{
  /// arrange
  az_ulib_ustream* test_ustream = ustream_mock_create();

  az_ulib_ustream ustream_split;

  set_clone_result(AZ_ERROR_ULIB_SYSTEM);

  /// act
  az_result result = az_ulib_ustream_split(test_ustream, &ustream_split, 5);

  /// assert]
  size_t cur_pos;
  az_ulib_ustream_get_position(test_ustream, &cur_pos);
  ASSERT_ARE_EQUAL(int, 0, cur_pos);

  ASSERT_ARE_EQUAL(int, AZ_ERROR_ULIB_SYSTEM, result);

  /// cleanup
  az_ulib_ustream_dispose(test_ustream);
}

/* az_ulib_ustream_split shall return the return value of az_ulib_ustream_set_position if it fails
 */
TEST_FUNCTION(az_ulib_ustream_split_set_position_second_failed)
{
  /// arrange
  az_ulib_ustream* test_ustream = ustream_mock_create();

  az_ulib_ustream ustream_split;

  set_set_position_result(AZ_ERROR_ULIB_SYSTEM);
  set_delay_return_value(1);

  /// act
  az_result result = az_ulib_ustream_split(test_ustream, &ustream_split, 5);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ULIB_SYSTEM, result);

  /// cleanup
  az_ulib_ustream_dispose(test_ustream);
}

#include "az_ulib_ustream_compliance_ut.h"

END_TEST_SUITE(ustream_aux_ut)
