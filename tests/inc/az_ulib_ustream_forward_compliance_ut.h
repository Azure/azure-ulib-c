// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_USTREAM_FORWARD_COMPLIANCE_UT_H
#define AZ_ULIB_USTREAM_FORWARD_COMPLIANCE_UT_H

#include "az_ulib_test_helpers.h"
#include "az_ulib_test_precondition.h"
#include "az_ulib_ustream_forward_mock_buffer.h"

#include "cmocka.h"

/* check for test artifacts. */
#ifndef USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH
#error "USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH not defined"
#endif

#ifndef USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY
#error "USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY not defined"
#endif

#if (USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH / 20 == 0)
#error "USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH must be at least 20 uint8_t long"
#endif

/* split the content in 4 parts. */
#define USTREAM_FORWARD_COMPLIANCE_LENGTH_1 \
  (USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH >> 2)
#define USTREAM_FORWARD_COMPLIANCE_LENGTH_2 \
  (USTREAM_FORWARD_COMPLIANCE_LENGTH_1 + USTREAM_FORWARD_COMPLIANCE_LENGTH_1)
#define USTREAM_FORWARD_COMPLIANCE_LENGTH_3 \
  (USTREAM_FORWARD_COMPLIANCE_LENGTH_2 + USTREAM_FORWARD_COMPLIANCE_LENGTH_1)

/* create local buffer with enough size to handle the full content. */
#define USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH \
  (USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 2)

// flush callback global variables
static uint8_t* flush_callback_buffer_check[USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH]
    = { 0 };
static size_t flush_callback_size_check = 0;
static az_ulib_callback_context flush_callback_context_check;

// mock callback function
static az_result mock_flush_callback(
    const uint8_t* const buffer,
    size_t size,
    az_ulib_callback_context flush_callback_context)
{
  (void)memcpy(flush_callback_buffer_check, buffer, size);
  flush_callback_size_check = size;
  flush_callback_context_check = flush_callback_context;

  return AZ_OK;
}

/*
 * Start compliance tests:
 */

#ifndef AZ_NO_PRECONDITION_CHECKING
/* If the provided handle is NULL, the dispose shall fail with precondition. */
static void az_ulib_ustream_forward_dispose_compliance_null_buffer_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED((ustream_forward)->_internal.api->dispose(NULL));

  /// cleanup
  az_result result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

/* If the provided handle is not the implemented buffer type, the dispose shall fail with
 * precondition. */
static void az_ulib_ustream_forward_dispose_compliance_buffer_is_not_type_of_buffer_failed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      ustream_forward->_internal.api->dispose(ustream_forward_mock_create()));

  /// cleanup
  az_result result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

/* If the provided handle is NULL, the get_size shall fail with precondition. */
static void az_ulib_ustream_forward_get_size_compliance_null_buffer_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(ustream_forward->_internal.api->get_size(NULL));

  /// cleanup
  az_result result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

/* If the provided handle is not properly initialized, the get_size shall fail with precondition. */
static void az_ulib_ustream_forward_get_size_compliance_buffer_is_not_type_of_buffer_failed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      ustream_forward->_internal.api->get_size(ustream_forward_mock_create()));

  /// cleanup
  az_result result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

/* If the provided handle is NULL, the flush shall fail with precondition. */
static void az_ulib_ustream_forward_flush_compliance_null_handle_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);
  az_ulib_callback_context callback_context = { 0 };

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      ustream_forward->_internal.api->flush(NULL, mock_flush_callback, callback_context));

  /// cleanup
  az_result result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

/* If the provided handle is not the implemented ustream_forward type, the flush shall fail with
 *  precondition.
 */
static void az_ulib_ustream_forward_flush_compliance_non_type_of_ustream_forward_api_failed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);
  az_ulib_callback_context callback_context = { 0 };

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(ustream_forward->_internal.api->flush(
      ustream_forward_mock_create(), mock_flush_callback, callback_context));

  /// cleanup
  az_result result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

/* If the provided flush_callback is NULL, the flush shall fail with precondition. */
static void az_ulib_ustream_forward_flush_compliance_null_flush_callback_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);
  az_ulib_callback_context callback_context = { 0 };

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      ustream_forward->_internal.api->flush(ustream_forward, NULL, callback_context));

  /// cleanup
  az_result result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

/* If the provided handle is NULL, the read shall fail with precondition. */
static void az_ulib_ustream_forward_read_compliance_null_handle_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);
  uint8_t buf_result[USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(ustream_forward->_internal.api->read(
      NULL, buf_result, USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result));

  /// cleanup
  az_result result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

/* If the provided handle is not the implemented buffer type, the read shall fail with precondition.
 */
static void az_ulib_ustream_forward_read_compliance_non_type_of_buffer_api_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);
  uint8_t buf_result[USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(ustream_forward->_internal.api->read(
      ustream_forward_mock_create(),
      buf_result,
      USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH,
      &size_result));

  /// cleanup
  az_result result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

/* If the provided buffer_length is zero, the read shall fail with precondition. */
static void az_ulib_ustream_forward_read_compliance_buffer_with_zero_size_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);
  uint8_t buf_result[USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      az_ulib_ustream_forward_read(ustream_forward, buf_result, 0, &size_result));

  /// cleanup
  az_result result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

/* If the provided handle is NULL, the read shall fail with precondition. */
static void az_ulib_ustream_forward_read_compliance_null_return_buffer_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);
  size_t size_result;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ustream_forward_read(
      ustream_forward, NULL, USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result));

  /// cleanup
  az_result result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

/* If the provided return size pointer is NULL, the read shall fail with precondition. */
static void az_ulib_ustream_forward_read_compliance_null_return_size_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);
  uint8_t buf_result[USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH];

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ustream_forward_read(
      ustream_forward, buf_result, USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH, NULL));

  /// cleanup
  az_result result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}
#endif // AZ_NO_PRECONDITION_CHECKING

/* The dispose shall release all allocated resources to control the buffer. */
static void az_ulib_ustream_forward_dispose_compliance_single_instance_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);

  /// act
  az_result result = az_ulib_ustream_forward_dispose(ustream_forward);

  /// assert
  assert_int_equal(result, AZ_OK);

  /// cleanup
  (void)result;
}

/* The get_size shall return the number of bytes between the current position and the end
 * of the buffer. */
static void az_ulib_ustream_forward_get_size_compliance_new_buffer_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);

  /// act
  size_t size = az_ulib_ustream_forward_get_size(ustream_forward);

  /// assert
  assert_int_equal(size, USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  az_result result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

static void az_ulib_ustream_forward_flush_compliance_single_buffer_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);
  struct _test_context
  {
    uint8_t test_value;
  } test_context = { 0 };

  az_ulib_callback_context callback_context = (az_ulib_callback_context)&test_context;

  /// act
  az_result result
      = az_ulib_ustream_forward_flush(ustream_forward, mock_flush_callback, callback_context);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(flush_callback_size_check, USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  assert_int_equal(flush_callback_context_check, callback_context);
  assert_memory_equal(
      USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      flush_callback_buffer_check,
      flush_callback_size_check);

  /// cleanup
  result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

/* [1]The read shall copy the content in the provided buffer and return the number of valid
 * <tt>uint8_t</tt> values in the local buffer in the provided `size`. */
/* [2]If the length of the content is bigger than the `buffer_length`, the read shall limit the copy
 * size to the buffer_length.*/
/* [3]If there is no more content to return, the read shall return AZ_ERROR_ITEM_NOT_FOUND,
 * size shall receive 0, and do not change the content of the local buffer. */
static void az_ulib_ustream_forward_read_compliance_get_from_original_buffer_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);
  uint8_t buf_result1[USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH];
  uint8_t buf_result2[USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH];
  uint8_t buf_result3[USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result1;
  size_t size_result2;
  size_t size_result3;

  /// act
  az_result result1 = az_ulib_ustream_forward_read(
      ustream_forward, buf_result1, USTREAM_FORWARD_COMPLIANCE_LENGTH_1, &size_result1);
  az_result result2 = az_ulib_ustream_forward_read(
      ustream_forward, buf_result2, USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result2);
  az_result result3 = az_ulib_ustream_forward_read(
      ustream_forward, buf_result3, USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result3);

  /// assert
  assert_int_equal(result1, AZ_OK);
  assert_int_equal(size_result1, USTREAM_FORWARD_COMPLIANCE_LENGTH_1);
  assert_memory_equal(USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result1, size_result1);

  assert_int_equal(result2, AZ_OK);
  assert_int_equal(
      size_result2,
      USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_FORWARD_COMPLIANCE_LENGTH_1);
  assert_memory_equal(
      (const uint8_t* const)(USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_FORWARD_COMPLIANCE_LENGTH_1),
      buf_result2,
      size_result2);

  assert_int_equal(result3, AZ_ULIB_EOF);

  /// cleanup
  az_result result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

static void az_ulib_ustream_forward_read_compliance_single_buffer_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);
  uint8_t buf_result[USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  az_result result = az_ulib_ustream_forward_read(
      ustream_forward, buf_result, USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(size_result, USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  assert_memory_equal(USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

  /// cleanup
  result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

static void az_ulib_ustream_forward_read_compliance_right_boundary_condition_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);
  uint8_t buf_result[USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  az_result result = az_ulib_ustream_forward_read(
      ustream_forward,
      buf_result,
      USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1,
      &size_result);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(size_result, USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1);
  assert_memory_equal(USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

  assert_int_equal(
      az_ulib_ustream_forward_read(
          ustream_forward, buf_result, USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result),
      AZ_OK);
  assert_int_equal(size_result, 1);
  assert_memory_equal(
      (const uint8_t* const)(USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1),
      buf_result,
      size_result);

  assert_int_equal(
      az_ulib_ustream_forward_read(
          ustream_forward, buf_result, USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result),
      AZ_ULIB_EOF);

  /// cleanup
  result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

static void az_ulib_ustream_forward_read_compliance_boundary_condition_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);
  uint8_t buf_result[USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  az_result result = az_ulib_ustream_forward_read(
      ustream_forward,
      buf_result,
      USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      &size_result);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(size_result, USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  assert_memory_equal(USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

  assert_int_equal(
      az_ulib_ustream_forward_read(
          ustream_forward, buf_result, USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result),
      AZ_ULIB_EOF);

  /// cleanup
  result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

static void az_ulib_ustream_forward_read_compliance_left_boundary_condition_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);
  uint8_t buf_result[USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  az_result result = az_ulib_ustream_forward_read(
      ustream_forward,
      buf_result,
      USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 1,
      &size_result);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(size_result, USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  assert_memory_equal(USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

  assert_int_equal(
      az_ulib_ustream_forward_read(
          ustream_forward, buf_result, USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result),
      AZ_ULIB_EOF);

  /// cleanup
  result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

static void az_ulib_ustream_forward_read_compliance_single_byte_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_forward* ustream_forward;
  USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);
  uint8_t buf_result[USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  az_result result = az_ulib_ustream_forward_read(ustream_forward, buf_result, 1, &size_result);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(size_result, 1);
  assert_memory_equal(USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

  check_ustream_forward_buffer(
      ustream_forward,
      1,
      USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  result = az_ulib_ustream_forward_dispose(ustream_forward);
  (void)result;
}

#define AZ_ULIB_USTREAM_FORWARD_PRECONDITION_COMPLIANCE_UT_LIST                                \
  cmocka_unit_test(az_ulib_ustream_forward_dispose_compliance_null_buffer_failed),             \
      cmocka_unit_test(                                                                        \
          az_ulib_ustream_forward_dispose_compliance_buffer_is_not_type_of_buffer_failed),     \
      cmocka_unit_test(az_ulib_ustream_forward_get_size_compliance_null_buffer_failed),        \
      cmocka_unit_test(                                                                        \
          az_ulib_ustream_forward_get_size_compliance_buffer_is_not_type_of_buffer_failed),    \
      cmocka_unit_test(az_ulib_ustream_forward_flush_compliance_null_handle_failed),           \
      cmocka_unit_test(                                                                        \
          az_ulib_ustream_forward_flush_compliance_non_type_of_ustream_forward_api_failed),    \
      cmocka_unit_test(az_ulib_ustream_forward_flush_compliance_null_flush_callback_failed),   \
      cmocka_unit_test(az_ulib_ustream_forward_read_compliance_null_handle_failed),            \
      cmocka_unit_test(az_ulib_ustream_forward_read_compliance_non_type_of_buffer_api_failed), \
      cmocka_unit_test(az_ulib_ustream_forward_read_compliance_buffer_with_zero_size_failed),  \
      cmocka_unit_test(az_ulib_ustream_forward_read_compliance_null_return_buffer_failed),     \
      cmocka_unit_test(az_ulib_ustream_forward_read_compliance_null_return_size_failed),

#define AZ_ULIB_USTREAM_FORWARD_COMPLIANCE_UT_LIST                                              \
  cmocka_unit_test_setup_teardown(                                                              \
      az_ulib_ustream_forward_dispose_compliance_single_instance_succeed, setup, teardown),     \
      cmocka_unit_test_setup_teardown(                                                          \
          az_ulib_ustream_forward_get_size_compliance_new_buffer_succeed, setup, teardown),     \
      cmocka_unit_test_setup_teardown(                                                          \
          az_ulib_ustream_forward_flush_compliance_single_buffer_succeed, setup, teardown),     \
      cmocka_unit_test_setup_teardown(                                                          \
          az_ulib_ustream_forward_read_compliance_get_from_original_buffer_succeed,             \
          setup,                                                                                \
          teardown),                                                                            \
      cmocka_unit_test_setup_teardown(                                                          \
          az_ulib_ustream_forward_read_compliance_single_buffer_succeed, setup, teardown),      \
      cmocka_unit_test_setup_teardown(                                                          \
          az_ulib_ustream_forward_read_compliance_right_boundary_condition_succeed,             \
          setup,                                                                                \
          teardown),                                                                            \
      cmocka_unit_test_setup_teardown(                                                          \
          az_ulib_ustream_forward_read_compliance_boundary_condition_succeed, setup, teardown), \
      cmocka_unit_test_setup_teardown(                                                          \
          az_ulib_ustream_forward_read_compliance_left_boundary_condition_succeed,              \
          setup,                                                                                \
          teardown),                                                                            \
      cmocka_unit_test_setup_teardown(                                                          \
          az_ulib_ustream_forward_read_compliance_single_byte_succeed, setup, teardown),

#endif /* AZ_ULIB_USTREAM_FORWARD_COMPLIANCE_UT_H */
