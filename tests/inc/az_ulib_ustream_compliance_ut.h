// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_USTREAM_COMPLIANCE_UT_H
#define AZ_ULIB_USTREAM_COMPLIANCE_UT_H

#include "az_ulib_ctest_aux.h"
#include "az_ulib_test_precondition.h"
#include "az_ulib_ustream_mock_buffer.h"

#include "cmocka.h"

/* check for test artifacts. */
#ifndef USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH
#error "USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH not defined"
#endif

#ifndef USTREAM_COMPLIANCE_TARGET_FACTORY
#error "USTREAM_COMPLIANCE_TARGET_FACTORY not defined"
#endif

#if (USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH / 20 == 0)
#error "USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH must be at least 20 uint8_t long"
#endif

/* split the content in 4 parts. */
#define USTREAM_COMPLIANCE_LENGTH_1 (USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH >> 2)
#define USTREAM_COMPLIANCE_LENGTH_2 (USTREAM_COMPLIANCE_LENGTH_1 + USTREAM_COMPLIANCE_LENGTH_1)
#define USTREAM_COMPLIANCE_LENGTH_3 (USTREAM_COMPLIANCE_LENGTH_2 + USTREAM_COMPLIANCE_LENGTH_1)

/* create local buffer with enough size to handle the full content. */
#define USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH (USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 2)

/*
 * Start compliance tests:
 */

#ifndef AZ_NO_PRECONDITION_CHECKING
/* If the provided handle is NULL, the dispose shall fail with precondition. */
static void az_ulib_ustream_dispose_compliance_null_buffer_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED((&ustream_instance)->control_block->api->dispose(NULL));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided handle is not the implemented buffer type, the dispose shall fail with
 * precondition. */
static void az_ulib_ustream_dispose_compliance_buffer_is_not_type_of_buffer_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      (&ustream_instance)->control_block->api->dispose(ustream_mock_create()));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided source handle is NULL, the clone shall fail with precondition. */
static void az_ulib_ustream_clone_compliance_null_buffer_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  az_ulib_ustream ustream_instance_clone;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      (&ustream_instance)->control_block->api->clone(&ustream_instance_clone, NULL, 0));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided destination handle is NULL, the clone shall fail with precondition. */
static void az_ulib_ustream_clone_compliance_null_buffer_clone_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      (&ustream_instance)->control_block->api->clone(NULL, &ustream_instance, 0));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided handle is not the implemented buffer type, the clone shall fail with
 * precondition. */
static void az_ulib_ustream_clone_compliance_buffer_is_not_type_of_buffer_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  az_ulib_ustream ustream_instance_clone;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      (&ustream_instance)
          ->control_block->api->clone(&ustream_instance_clone, ustream_mock_create(), 0));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided handle is NULL, the get_remaining_size shall fail with precondition. */
static void az_ulib_ustream_get_remaining_size_compliance_null_buffer_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  size_t size;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      (&ustream_instance)->control_block->api->get_remaining_size(NULL, &size));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_get_remaining_size_compliance_buffer_is_not_type_of_buffer_failed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  size_t size;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      (&ustream_instance)->control_block->api->get_remaining_size(ustream_mock_create(), &size));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided handle is not the implemented buffer type, the get_remaining_size shall fail with
 * precondition. */
static void az_ulib_ustream_get_remaining_size_compliance_null_size_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ustream_get_remaining_size(&ustream_instance, NULL));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided instance is NULL, the get_position shall fail with precondition. */
static void az_ulib_ustream_get_current_position_compliance_null_buffer_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  offset_t position;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      (&ustream_instance)->control_block->api->get_position(NULL, &position));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided instance is not the implemented buffer type, the get_position shall fail with
 * precondition. */
static void az_ulib_ustream_get_current_position_compliance_buffer_is_not_type_of_buffer_failed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  offset_t position;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      (&ustream_instance)->control_block->api->get_position(ustream_mock_create(), &position));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided position is NULL, the get_position shall fail with precondition. */
static void az_ulib_ustream_get_current_position_compliance_null_position_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ustream_get_position(&ustream_instance, NULL));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided handle is NULL, the read shall fail with precondition. */
static void az_ulib_ustream_read_compliance_null_buffer_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      (&ustream_instance)
          ->control_block->api->read(
              NULL, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided handle is not the implemented buffer type, the read shall fail with precondition.
 */
static void az_ulib_ustream_read_compliance_non_type_of_buffer_api_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED((&ustream_instance)
                                          ->control_block->api->read(
                                              ustream_mock_create(),
                                              buf_result,
                                              USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH,
                                              &size_result));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided buffer_length is zero, the read shall fail with precondition. */
static void az_ulib_ustream_read_compliance_buffer_with_zero_size_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      az_ulib_ustream_read(&ustream_instance, buf_result, 0, &size_result));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided handle is NULL, the read shall fail with precondition. */
static void az_ulib_ustream_read_compliance_null_return_buffer_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  size_t size_result;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ustream_read(
      &ustream_instance, NULL, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided return size pointer is NULL, the read shall fail with precondition. */
static void az_ulib_ustream_read_compliance_null_return_size_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ustream_read(
      &ustream_instance, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, NULL));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided handle is NULL, the set_position shall fail with precondition. */
static void az_ulib_ustream_set_position_compliance_null_buffer_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      (&ustream_instance)->control_block->api->set_position(NULL, 1));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided handle is not the implemented buffer type, the set_position shall fail with
 * precondition. */
static void az_ulib_ustream_set_position_compliance_non_type_of_buffer_api_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      (&ustream_instance)->control_block->api->set_position(ustream_mock_create(), 1));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided handle is NULL, the release shall fail with precondition. */
static void az_ulib_ustream_release_compliance_null_buffer_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED((&ustream_instance)->control_block->api->release(NULL, 0));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided handle is not the implemented buffer type, the release shall fail with
 * precondition. */
static void az_ulib_ustream_release_compliance_non_type_of_buffer_api_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      (&ustream_instance)->control_block->api->release(ustream_mock_create(), 0));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided handle is NULL, the reset shall fail with precondition. */
static void az_ulib_ustream_reset_compliance_null_buffer_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED((&ustream_instance)->control_block->api->reset(NULL));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided handle is not the implemented buffer type, the reset shall fail with
 * precondition. */
static void az_ulib_ustream_reset_compliance_non_type_of_buffer_api_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      (&ustream_instance)->control_block->api->reset(ustream_mock_create()));

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

#endif // AZ_NO_PRECONDITION_CHECKING

/* The dispose shall free all allocated resources for the instance of the buffer. */
static void az_ulib_ustream_dispose_compliance_cloned_instance_disposed_first_succeed(void** state)
{
  /// arrange
  (void)state;
  offset_t ustream_clone_current_position;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 0);
  assert_int_equal(result, AZ_OK);

  /// act
  result = az_ulib_ustream_dispose(&ustream_instance_clone);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(
      az_ulib_ustream_get_position(&ustream_instance, &ustream_clone_current_position), AZ_OK);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_dispose_compliance_cloned_instance_disposed_second_succeed(void** state)
{
  /// arrange
  (void)state;
  offset_t ustream_clone_current_position;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 0);
  assert_int_equal(result, AZ_OK);

  /// act
  result = az_ulib_ustream_dispose(&ustream_instance);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(
      az_ulib_ustream_get_position(&ustream_instance_clone, &ustream_clone_current_position),
      AZ_OK);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

/* If there is no more instances of the buffer, the dispose shall release all allocated
 *          resources to control the buffer. */
static void az_ulib_ustream_dispose_compliance_single_instance_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);

  /// act
  az_result result = az_ulib_ustream_dispose(&ustream_instance);

  /// assert
  assert_int_equal(result, AZ_OK);

  /// cleanup
}

/* The clone shall return a buffer with the same content of the original buffer. */
/* The clone shall start the buffer on the current position of the original buffer. */
/* The clone shall move the current position to the start position. */
/* The clone shall move the released position to the one immediately before the current position. */
/* The cloned buffer shall not interfere in the instance of the original buffer and vice versa. */
static void az_ulib_ustream_clone_compliance_new_buffer_cloned_with_zero_offset_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  az_ulib_ustream ustream_instance_clone;

  /// act
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 0);

  /// assert
  /* clone succeed */
  assert_int_equal(result, AZ_OK);

  /* current position was moved to the start of the buffer. */
  offset_t ustream_clone_current_position;
  assert_int_equal(
      az_ulib_ustream_get_position(&ustream_instance_clone, &ustream_clone_current_position),
      AZ_OK);
  assert_int_equal(ustream_clone_current_position, 0);

  /* release position was moved to the start menus one. */
  assert_int_equal(az_ulib_ustream_reset(&ustream_instance_clone), AZ_OK);
  assert_int_equal(
      az_ulib_ustream_get_position(&ustream_instance_clone, &ustream_clone_current_position),
      AZ_OK);
  assert_int_equal(ustream_clone_current_position, 0);

  /* Full content was copied. */
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance_clone, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result),
      AZ_OK);
  assert_int_equal(size_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  assert_memory_equal(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

  /* Keep original buffer instance */
  check_buffer(
      &ustream_instance,
      0,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

static void az_ulib_ustream_clone_compliance_new_buffer_cloned_with_offset_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  az_ulib_ustream ustream_instance_clone;

  /// act
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 10000);

  /// assert
  /* clone succeed */
  assert_int_equal(result, AZ_OK);

  /* current position was moved to the start of the buffer with the offset. */
  offset_t ustream_clone_current_position;
  assert_int_equal(
      az_ulib_ustream_get_position(&ustream_instance_clone, &ustream_clone_current_position),
      AZ_OK);
  assert_int_equal(ustream_clone_current_position, 10000);

  /* release position was moved to the start menus one. */
  assert_int_equal(az_ulib_ustream_reset(&ustream_instance_clone), AZ_OK);
  assert_int_equal(
      az_ulib_ustream_get_position(&ustream_instance_clone, &ustream_clone_current_position),
      AZ_OK);
  assert_int_equal(ustream_clone_current_position, 10000);

  /* Full content was copied. */
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance_clone, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result),
      AZ_OK);
  assert_int_equal(size_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  assert_memory_equal(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

  /* Keep original buffer instance */
  check_buffer(
      &ustream_instance,
      0,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

static void az_ulib_ustream_clone_compliance_empty_buffer_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  assert_int_equal(
      az_ulib_ustream_set_position(&ustream_instance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH),
      AZ_OK);
  assert_int_equal(
      az_ulib_ustream_release(&ustream_instance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1),
      AZ_OK);
  az_ulib_ustream ustream_instance_clone;

  /// act
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 0);

  /// assert
  /* clone succeed */
  assert_int_equal(result, AZ_OK);

  /* current position was moved to the start of the buffer. */
  offset_t ustream_clone_current_position;
  assert_int_equal(
      az_ulib_ustream_get_position(&ustream_instance_clone, &ustream_clone_current_position),
      AZ_OK);
  assert_int_equal(ustream_clone_current_position, 0);

  /* release position was moved to the start menus one. */
  assert_int_equal(az_ulib_ustream_reset(&ustream_instance_clone), AZ_OK);
  assert_int_equal(
      az_ulib_ustream_get_position(&ustream_instance_clone, &ustream_clone_current_position),
      AZ_OK);
  assert_int_equal(ustream_clone_current_position, 0);

  /* Full content was copied. */
  check_buffer(
      &ustream_instance_clone,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /* Keep original buffer instance */
  check_buffer(
      &ustream_instance,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

static void
az_ulib_ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size;
  assert_int_equal(
      az_ulib_ustream_read(&ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_2, &size), AZ_OK);
  assert_int_equal(az_ulib_ustream_release(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1), AZ_OK);
  az_ulib_ustream ustream_instance_clone;

  /// act
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 100);

  /// assert
  /* clone succeed */
  assert_int_equal(result, AZ_OK);

  /* current position was moved to the start of the buffer with the offset. */
  offset_t ustream_clone_current_position;
  assert_int_equal(
      az_ulib_ustream_get_position(&ustream_instance_clone, &ustream_clone_current_position),
      AZ_OK);
  assert_int_equal(ustream_clone_current_position, 100);

  /* release position was moved to the start menus one. */
  assert_int_equal(az_ulib_ustream_reset(&ustream_instance_clone), AZ_OK);
  assert_int_equal(
      az_ulib_ustream_get_position(&ustream_instance_clone, &ustream_clone_current_position),
      AZ_OK);
  assert_int_equal(ustream_clone_current_position, 100);

  /* Full content was copied. */
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance_clone, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result),
      AZ_OK);
  assert_int_equal(
      size_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2);
  assert_memory_equal(
      (const uint8_t*)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_2),
      buf_result,
      size_result);

  /* Keep original buffer instance */
  check_buffer(
      &ustream_instance,
      USTREAM_COMPLIANCE_LENGTH_2,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

static void
az_ulib_ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_negative_offset_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size;
  assert_int_equal(
      az_ulib_ustream_read(&ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_2, &size), AZ_OK);
  assert_int_equal(az_ulib_ustream_release(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1), AZ_OK);
  az_ulib_ustream ustream_instance_clone;

  /// act
  az_result result = az_ulib_ustream_clone(
      &ustream_instance_clone, &ustream_instance, USTREAM_COMPLIANCE_LENGTH_1);

  /// assert
  /* clone succeed */
  assert_int_equal(result, AZ_OK);

  /* current position was moved to the start of the buffer with the offset. */
  offset_t ustream_clone_current_position;
  assert_int_equal(
      az_ulib_ustream_get_position(&ustream_instance_clone, &ustream_clone_current_position),
      AZ_OK);
  assert_int_equal(ustream_clone_current_position, USTREAM_COMPLIANCE_LENGTH_1);

  /* release position was moved to the start menus one. */
  assert_int_equal(az_ulib_ustream_reset(&ustream_instance_clone), AZ_OK);
  assert_int_equal(
      az_ulib_ustream_get_position(&ustream_instance_clone, &ustream_clone_current_position),
      AZ_OK);
  assert_int_equal(ustream_clone_current_position, USTREAM_COMPLIANCE_LENGTH_1);

  /* Full content was copied. */
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance_clone, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result),
      AZ_OK);
  assert_int_equal(
      size_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2);
  assert_memory_equal(
      (const uint8_t*)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_2),
      buf_result,
      size_result);

  /* Keep original buffer instance */
  check_buffer(
      &ustream_instance,
      USTREAM_COMPLIANCE_LENGTH_2,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

static void
az_ulib_ustream_clone_compliance_cloned_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size;
  assert_int_equal(
      az_ulib_ustream_read(&ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_1, &size), AZ_OK);
  assert_int_equal(
      az_ulib_ustream_release(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1 - 1), AZ_OK);
  az_ulib_ustream ustream_instance_clone1;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone1, &ustream_instance, 100);
  assert_int_equal(result, AZ_OK);
  assert_int_equal(
      az_ulib_ustream_read(&ustream_instance_clone1, buf, USTREAM_COMPLIANCE_LENGTH_1, &size),
      AZ_OK);
  assert_int_equal(
      az_ulib_ustream_release(&ustream_instance_clone1, 100 + USTREAM_COMPLIANCE_LENGTH_1 - 1),
      AZ_OK);
  az_ulib_ustream ustream_instance_clone2;

  /// act
  result = az_ulib_ustream_clone(&ustream_instance_clone2, &ustream_instance_clone1, 0);

  /// assert
  /* clone succeed */
  assert_int_equal(result, AZ_OK);

  /* current position was moved to the start of the buffer with the offset. */
  offset_t ustream_clone_current_position;
  assert_int_equal(
      az_ulib_ustream_get_position(&ustream_instance_clone2, &ustream_clone_current_position),
      AZ_OK);
  assert_int_equal(ustream_clone_current_position, 0);

  /* release position was moved to the start menus one. */
  assert_int_equal(az_ulib_ustream_reset(&ustream_instance_clone2), AZ_OK);
  assert_int_equal(
      az_ulib_ustream_get_position(&ustream_instance_clone2, &ustream_clone_current_position),
      AZ_OK);
  assert_int_equal(ustream_clone_current_position, 0);

  /* Full content was copied. */
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance_clone2,
          buf_result,
          USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH,
          &size_result),
      AZ_OK);
  assert_int_equal(
      size_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2);
  assert_memory_equal(
      (const uint8_t*)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_2),
      buf_result,
      size_result);

  /* Keep original buffer instance */
  check_buffer(
      &ustream_instance,
      USTREAM_COMPLIANCE_LENGTH_1,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  check_buffer(
      &ustream_instance_clone1,
      USTREAM_COMPLIANCE_LENGTH_2,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
  (void)az_ulib_ustream_dispose(&ustream_instance_clone1);
  (void)az_ulib_ustream_dispose(&ustream_instance_clone2);
}

/* If the offset plus the buffer length bypass UINT32_MAX, the clone shall return NULL. */
static void az_ulib_ustream_clone_compliance_offset_exceed_size_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  az_ulib_ustream ustream_instance_clone;

  /// act
  az_result result
      = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, UINT32_MAX - 2);

  /// assert
  assert_int_equal(result, AZ_ERROR_ARG);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* The get_remaining_size shall return the number of bytes between the current position and the end
 * of the buffer. */
static void az_ulib_ustream_get_remaining_size_compliance_new_buffer_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  size_t size;

  /// act
  az_result result = az_ulib_ustream_get_remaining_size(&ustream_instance, &size);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(size, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void
az_ulib_ustream_get_remaining_size_compliance_new_buffer_with_non_zero_current_position_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size;
  assert_int_equal(
      az_ulib_ustream_read(&ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_2, &size), AZ_OK);
  assert_int_equal(az_ulib_ustream_release(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1), AZ_OK);

  /// act
  az_result result = az_ulib_ustream_get_remaining_size(&ustream_instance, &size);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(size, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void
az_ulib_ustream_get_remaining_size_compliance_cloned_buffer_with_non_zero_current_position_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size;
  assert_int_equal(
      az_ulib_ustream_read(&ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_1, &size), AZ_OK);
  assert_int_equal(
      az_ulib_ustream_release(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1 - 1), AZ_OK);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 100);
  assert_int_equal(result, AZ_OK);
  assert_int_equal(
      az_ulib_ustream_read(&ustream_instance_clone, buf, USTREAM_COMPLIANCE_LENGTH_1, &size),
      AZ_OK);
  assert_int_equal(
      az_ulib_ustream_release(&ustream_instance_clone, 100 + USTREAM_COMPLIANCE_LENGTH_1 - 1),
      AZ_OK);

  /// act
  result = az_ulib_ustream_get_remaining_size(&ustream_instance_clone, &size);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(size, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* The get_position shall return the logical current position of the buffer. */
static void az_ulib_ustream_get_current_position_compliance_new_buffer_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  offset_t position;

  /// act
  az_result result = az_ulib_ustream_get_position(&ustream_instance, &position);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(position, 0);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void
az_ulib_ustream_get_current_position_compliance_new_buffer_with_non_zero_current_position_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size;
  offset_t position;
  assert_int_equal(
      az_ulib_ustream_read(&ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_2, &size), AZ_OK);
  assert_int_equal(az_ulib_ustream_release(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1), AZ_OK);

  /// act
  az_result result = az_ulib_ustream_get_position(&ustream_instance, &position);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(position, USTREAM_COMPLIANCE_LENGTH_2);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void
az_ulib_ustream_get_current_position_compliance_cloned_buffer_with_non_zero_current_position_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size;
  offset_t position;
  assert_int_equal(
      az_ulib_ustream_read(&ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_1, &size), AZ_OK);
  assert_int_equal(
      az_ulib_ustream_release(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1 - 1), AZ_OK);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 100);
  assert_int_equal(result, AZ_OK);
  assert_int_equal(
      az_ulib_ustream_read(&ustream_instance_clone, buf, USTREAM_COMPLIANCE_LENGTH_1, &size),
      AZ_OK);
  assert_int_equal(
      az_ulib_ustream_release(&ustream_instance_clone, 100 + USTREAM_COMPLIANCE_LENGTH_1 - 1),
      AZ_OK);

  /// act
  result = az_ulib_ustream_get_position(&ustream_instance_clone, &position);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(position, 100 + USTREAM_COMPLIANCE_LENGTH_1);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* [1]The read shall copy the content in the provided buffer and return the number of valid
 * <tt>uint8_t</tt> values in the local buffer in the provided `size`. */
/* [2]If the length of the content is bigger than the `buffer_length`, the read shall limit the copy
 * size to the buffer_length.*/
/* [3]If there is no more content to return, the read shall return AZ_ERROR_ITEM_NOT_FOUND,
 * size shall receive 0, and do not change the content of the local buffer. */
static void az_ulib_ustream_read_compliance_get_from_original_buffer_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result1[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  uint8_t buf_result2[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  uint8_t buf_result3[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result1;
  size_t size_result2;
  size_t size_result3;

  /// act
  az_result result1 = az_ulib_ustream_read(
      &ustream_instance, buf_result1, USTREAM_COMPLIANCE_LENGTH_1, &size_result1);
  az_result result2 = az_ulib_ustream_read(
      &ustream_instance, buf_result2, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result2);
  az_result result3 = az_ulib_ustream_read(
      &ustream_instance, buf_result3, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result3);

  /// assert
  assert_int_equal(result1, AZ_OK);
  assert_int_equal(size_result1, USTREAM_COMPLIANCE_LENGTH_1);
  assert_memory_equal(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result1, size_result1);

  assert_int_equal(result2, AZ_OK);
  assert_int_equal(
      size_result2, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1);
  assert_memory_equal(
      (const uint8_t* const)(
          USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_1),
      buf_result2,
      size_result2);

  assert_int_equal(result3, AZ_ULIB_EOF);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_read_compliance_get_from_cloned_buffer_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  assert_int_equal(
      az_ulib_ustream_set_position(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1), AZ_OK);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 100);
  assert_int_equal(result, AZ_OK);
  (void)az_ulib_ustream_dispose(&ustream_instance);

  uint8_t buf_result1[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  uint8_t buf_result2[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  uint8_t buf_result3[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result1;
  size_t size_result2;
  size_t size_result3;

  /// act
  az_result result1 = az_ulib_ustream_read(
      &ustream_instance_clone, buf_result1, USTREAM_COMPLIANCE_LENGTH_1, &size_result1);
  az_result result2 = az_ulib_ustream_read(
      &ustream_instance_clone, buf_result2, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result2);
  az_result result3 = az_ulib_ustream_read(
      &ustream_instance_clone, buf_result3, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result3);

  /// assert
  assert_int_equal(result1, AZ_OK);
  assert_int_equal(size_result1, USTREAM_COMPLIANCE_LENGTH_1);
  assert_memory_equal(
      (const uint8_t* const)(
          USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_1),
      buf_result1,
      size_result1);

  assert_int_equal(result2, AZ_OK);
  assert_int_equal(
      size_result2, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2);
  assert_memory_equal(
      (const uint8_t* const)(
          USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_2),
      buf_result2,
      size_result2);

  assert_int_equal(result3, AZ_ULIB_EOF);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

static void az_ulib_ustream_read_compliance_single_buffer_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  az_result result = az_ulib_ustream_read(
      &ustream_instance, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(size_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  assert_memory_equal(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_read_compliance_right_boundary_condition_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  az_result result = az_ulib_ustream_read(
      &ustream_instance, buf_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1, &size_result);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(size_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1);
  assert_memory_equal(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result),
      AZ_OK);
  assert_int_equal(size_result, 1);
  assert_memory_equal(
      (const uint8_t* const)(
          USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH
          - 1),
      buf_result,
      size_result);

  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result),
      AZ_ULIB_EOF);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_read_compliance_cloned_buffer_right_boundary_condition_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  assert_int_equal(
      az_ulib_ustream_set_position(
          &ustream_instance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1),
      AZ_OK);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 0);
  assert_int_equal(result, AZ_OK);
  (void)az_ulib_ustream_dispose(&ustream_instance);

  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  result = az_ulib_ustream_read(
      &ustream_instance_clone, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(size_result, 1);
  assert_memory_equal(
      (const uint8_t* const)(
          USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH
          - 1),
      buf_result,
      size_result);

  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance_clone, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result),
      AZ_ULIB_EOF);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

static void az_ulib_ustream_read_compliance_boundary_condition_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  az_result result = az_ulib_ustream_read(
      &ustream_instance, buf_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, &size_result);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(size_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  assert_memory_equal(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result),
      AZ_ULIB_EOF);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_read_compliance_left_boundary_condition_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  az_result result = az_ulib_ustream_read(
      &ustream_instance, buf_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 1, &size_result);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(size_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  assert_memory_equal(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result),
      AZ_ULIB_EOF);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_read_compliance_single_byte_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;

  /// act
  az_result result = az_ulib_ustream_read(&ustream_instance, buf_result, 1, &size_result);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(size_result, 1);
  assert_memory_equal(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

  check_buffer(
      &ustream_instance,
      1,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* The set_position shall change the current position of the buffer. */
static void az_ulib_ustream_set_position_compliance_back_to_beginning_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance, buf_result, USTREAM_COMPLIANCE_LENGTH_1, &size_result),
      AZ_OK);

  /// act
  az_result result = az_ulib_ustream_set_position(&ustream_instance, 0);

  /// assert
  assert_int_equal(result, AZ_OK);
  check_buffer(
      &ustream_instance,
      0,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_set_position_compliance_back_position_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result),
      AZ_OK);

  /// act
  az_result result = az_ulib_ustream_set_position(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1);

  /// assert
  assert_int_equal(result, AZ_OK);
  check_buffer(
      &ustream_instance,
      USTREAM_COMPLIANCE_LENGTH_1,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_set_position_compliance_forward_position_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance, buf_result, USTREAM_COMPLIANCE_LENGTH_1, &size_result),
      AZ_OK);

  /// act
  az_result result = az_ulib_ustream_set_position(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_2);

  /// assert
  assert_int_equal(result, AZ_OK);
  check_buffer(
      &ustream_instance,
      USTREAM_COMPLIANCE_LENGTH_2,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_set_position_compliance_forward_to_the_end_position_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);

  /// act
  az_result result
      = az_ulib_ustream_set_position(&ustream_instance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// assert
  assert_int_equal(result, AZ_OK);
  check_buffer(
      &ustream_instance,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_set_position_compliance_run_full_buffer_byte_by_byte_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[1];
  size_t size_result;

  for (offset_t i = 0; i < USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH; i++)
  {
    /// act - assert
    assert_int_equal(az_ulib_ustream_set_position(&ustream_instance, i), AZ_OK);

    assert_int_equal(az_ulib_ustream_read(&ustream_instance, buf_result, 1, &size_result), AZ_OK);

    assert_int_equal(size_result, 1);
    assert_int_equal(
        *buf_result, *(const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + i));
  }
  check_buffer(
      &ustream_instance,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void
az_ulib_ustream_set_position_compliance_run_full_buffer_byte_by_byte_reverse_order_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[1];
  size_t size_result;

  for (offset_t i = USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1; i > 0; i--)
  {
    /// act - assert
    assert_int_equal(az_ulib_ustream_set_position(&ustream_instance, i), AZ_OK);

    assert_int_equal(az_ulib_ustream_read(&ustream_instance, buf_result, 1, &size_result), AZ_OK);

    assert_int_equal(size_result, 1);
    assert_int_equal(
        *buf_result, *(const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + i));
  }
  assert_int_equal(az_ulib_ustream_set_position(&ustream_instance, 0), AZ_OK);
  check_buffer(
      &ustream_instance,
      0,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_set_position_compliance_cloned_buffer_back_to_beginning_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  assert_int_equal(
      az_ulib_ustream_set_position(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1), AZ_OK);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 0);
  assert_int_equal(result, AZ_OK);
  (void)az_ulib_ustream_dispose(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance_clone, buf_result, USTREAM_COMPLIANCE_LENGTH_1, &size_result),
      AZ_OK);

  /// act
  result = az_ulib_ustream_set_position(&ustream_instance_clone, 0);

  /// assert
  assert_int_equal(result, AZ_OK);
  check_buffer(
      &ustream_instance_clone,
      USTREAM_COMPLIANCE_LENGTH_1,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

static void az_ulib_ustream_set_position_compliance_cloned_buffer_back_position_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  assert_int_equal(
      az_ulib_ustream_set_position(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1), AZ_OK);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 0);
  assert_int_equal(result, AZ_OK);
  (void)az_ulib_ustream_dispose(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance_clone, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result),
      AZ_OK);

  /// act
  result = az_ulib_ustream_set_position(&ustream_instance_clone, USTREAM_COMPLIANCE_LENGTH_1);

  /// assert
  assert_int_equal(result, AZ_OK);
  check_buffer(
      &ustream_instance_clone,
      USTREAM_COMPLIANCE_LENGTH_2,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

static void az_ulib_ustream_set_position_compliance_cloned_buffer_forward_position_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  assert_int_equal(
      az_ulib_ustream_set_position(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1), AZ_OK);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 0);
  assert_int_equal(result, AZ_OK);
  (void)az_ulib_ustream_dispose(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance_clone, buf_result, USTREAM_COMPLIANCE_LENGTH_1, &size_result),
      AZ_OK);

  /// act
  result = az_ulib_ustream_set_position(&ustream_instance_clone, USTREAM_COMPLIANCE_LENGTH_2);

  /// assert
  assert_int_equal(result, AZ_OK);
  check_buffer(
      &ustream_instance_clone,
      USTREAM_COMPLIANCE_LENGTH_3,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

static void
az_ulib_ustream_set_position_compliance_cloned_buffer_forward_to_the_end_position_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  assert_int_equal(
      az_ulib_ustream_set_position(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1), AZ_OK);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 100);
  assert_int_equal(result, AZ_OK);
  (void)az_ulib_ustream_dispose(&ustream_instance);

  /// act
  result = az_ulib_ustream_set_position(
      &ustream_instance_clone,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1 + 100);

  /// assert
  assert_int_equal(result, AZ_OK);
  check_buffer(
      &ustream_instance_clone,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

static void
az_ulib_ustream_set_position_compliance_cloned_buffer_run_full_buffer_byte_by_byte_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  assert_int_equal(
      az_ulib_ustream_set_position(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1), AZ_OK);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 0);
  assert_int_equal(result, AZ_OK);
  (void)az_ulib_ustream_dispose(&ustream_instance);
  uint8_t buf_result[1];
  size_t size_result;

  for (offset_t i = 0; i < USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1;
       i++)
  {
    /// act - assert
    assert_int_equal(az_ulib_ustream_set_position(&ustream_instance_clone, i), AZ_OK);

    assert_int_equal(
        az_ulib_ustream_read(&ustream_instance_clone, buf_result, 1, &size_result), AZ_OK);

    assert_int_equal(size_result, 1);
    assert_int_equal(
        *buf_result,
        *(const uint8_t* const)(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_1 + i));
  }
  check_buffer(
      &ustream_instance_clone,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

static void
az_ulib_ustream_set_position_compliance_cloned_buffer_run_full_buffer_byte_by_byte_reverse_order_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  assert_int_equal(
      az_ulib_ustream_set_position(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1), AZ_OK);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 0);
  assert_int_equal(result, AZ_OK);
  (void)az_ulib_ustream_dispose(&ustream_instance);
  uint8_t buf_result[1];
  size_t size_result;

  for (offset_t i = USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1 - 1;
       i > 0;
       i--)
  {
    /// act - assert
    assert_int_equal(az_ulib_ustream_set_position(&ustream_instance_clone, i), AZ_OK);

    assert_int_equal(
        az_ulib_ustream_read(&ustream_instance_clone, buf_result, 1, &size_result), AZ_OK);

    assert_int_equal(size_result, 1);
    assert_int_equal(
        *buf_result,
        *(const uint8_t* const)(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_1 + i));
  }
  assert_int_equal(az_ulib_ustream_set_position(&ustream_instance_clone, 0), AZ_OK);
  check_buffer(
      &ustream_instance_clone,
      USTREAM_COMPLIANCE_LENGTH_1,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

/* If the provided position is out of the range of the buffer, the set_position shall return
 * AZ_ERROR_ITEM_NOT_FOUND, and do not change the current position. */
static void az_ulib_ustream_set_position_compliance_forward_out_of_the_buffer_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);

  /// act
  az_result result = az_ulib_ustream_set_position(
      &ustream_instance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 1);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  check_buffer(
      &ustream_instance,
      0,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided position is before the first valid position, the set_position shall return
 * AZ_ERROR_ITEM_NOT_FOUND, and do not change the current position. */
static void az_ulib_ustream_set_position_compliance_back_before_first_valid_position_failed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size;
  assert_int_equal(
      az_ulib_ustream_read(&ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_1, &size), AZ_OK);
  assert_int_equal(
      az_ulib_ustream_release(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1 - 1), AZ_OK);

  /// act
  az_result result = az_ulib_ustream_set_position(&ustream_instance, 0);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  check_buffer(
      &ustream_instance,
      USTREAM_COMPLIANCE_LENGTH_1,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the provided position is before the first valid position, the set_position shall return
 * AZ_ERROR_ITEM_NOT_FOUND, and do not change the current position. */
static void
az_ulib_ustream_set_position_compliance_back_before_first_valid_position_with_offset_failed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 100);
  assert_int_equal(result, AZ_OK);

  /// act
  result = az_ulib_ustream_set_position(&ustream_instance_clone, 0);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  check_buffer(
      &ustream_instance,
      0,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* The release shall do nothing for &ustream_instance. */
static void az_ulib_ustream_release_compliance_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance, buf_result, USTREAM_COMPLIANCE_LENGTH_1, &size_result),
      AZ_OK);
  offset_t current_position;
  assert_int_equal(az_ulib_ustream_get_position(&ustream_instance, &current_position), AZ_OK);

  /// act
  az_result result = az_ulib_ustream_release(&ustream_instance, current_position - 1);

  /// assert
  assert_int_equal(result, AZ_OK);
  check_buffer(
      &ustream_instance,
      USTREAM_COMPLIANCE_LENGTH_1,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_release_compliance_release_all_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  assert_int_equal(
      az_ulib_ustream_set_position(&ustream_instance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH),
      AZ_OK);

  /// act
  az_result result
      = az_ulib_ustream_release(&ustream_instance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(az_ulib_ustream_reset(&ustream_instance), AZ_OK);
  check_buffer(
      &ustream_instance,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_release_compliance_run_full_buffer_byte_by_byte_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[1];
  size_t size_result;

  for (offset_t i = 1; i < USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1; i++)
  {
    /// act - assert
    assert_int_equal(az_ulib_ustream_set_position(&ustream_instance, i + 1), AZ_OK);

    assert_int_equal(az_ulib_ustream_release(&ustream_instance, i - 1), AZ_OK);

    assert_int_equal(az_ulib_ustream_reset(&ustream_instance), AZ_OK);

    assert_int_equal(az_ulib_ustream_read(&ustream_instance, buf_result, 1, &size_result), AZ_OK);

    assert_int_equal(size_result, 1);
    assert_int_equal(
        *buf_result, *(const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + i));
  }
  check_buffer(
      &ustream_instance,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_release_compliance_cloned_buffer_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  assert_int_equal(
      az_ulib_ustream_set_position(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1), AZ_OK);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 0);
  assert_int_equal(result, AZ_OK);
  (void)az_ulib_ustream_dispose(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance_clone, buf_result, USTREAM_COMPLIANCE_LENGTH_1, &size_result),
      AZ_OK);
  offset_t current_position;
  assert_int_equal(az_ulib_ustream_get_position(&ustream_instance_clone, &current_position), AZ_OK);

  /// act
  result = az_ulib_ustream_release(&ustream_instance_clone, current_position - 1);

  /// assert
  assert_int_equal(result, AZ_OK);
  check_buffer(
      &ustream_instance_clone,
      USTREAM_COMPLIANCE_LENGTH_2,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

static void az_ulib_ustream_release_compliance_cloned_buffer_release_all_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  assert_int_equal(
      az_ulib_ustream_set_position(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1), AZ_OK);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 1000);
  assert_int_equal(result, AZ_OK);
  (void)az_ulib_ustream_dispose(&ustream_instance);
  assert_int_equal(
      az_ulib_ustream_set_position(
          &ustream_instance_clone,
          USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1 + 1000),
      AZ_OK);

  /// act
  result = az_ulib_ustream_release(
      &ustream_instance_clone,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1 + 999);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(az_ulib_ustream_reset(&ustream_instance_clone), AZ_OK);
  check_buffer(
      &ustream_instance_clone,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

static void az_ulib_ustream_release_compliance_cloned_buffer_run_full_buffer_byte_by_byte_succeed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  assert_int_equal(
      az_ulib_ustream_set_position(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1), AZ_OK);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 0);
  assert_int_equal(result, AZ_OK);
  (void)az_ulib_ustream_dispose(&ustream_instance);
  uint8_t buf_result[1];
  size_t size_result;

  for (offset_t i = 1;
       i < USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1 - 1;
       i++)
  {
    /// act - assert
    assert_int_equal(az_ulib_ustream_set_position(&ustream_instance_clone, i + 1), AZ_OK);

    assert_int_equal(az_ulib_ustream_release(&ustream_instance_clone, i - 1), AZ_OK);

    assert_int_equal(az_ulib_ustream_reset(&ustream_instance_clone), AZ_OK);

    assert_int_equal(
        az_ulib_ustream_read(&ustream_instance_clone, buf_result, 1, &size_result), AZ_OK);

    assert_int_equal(size_result, 1);
    assert_int_equal(
        *buf_result,
        *(const uint8_t* const)(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_1 + i));
  }
  check_buffer(
      &ustream_instance_clone,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

/* If the release position is after the current position, the release shall return
 * AZ_ERROR_ITEM_NOT_FOUND, and do not release any resource. */
static void az_ulib_ustream_release_compliance_release_after_current_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance, buf_result, USTREAM_COMPLIANCE_LENGTH_1, &size_result),
      AZ_OK);
  offset_t current_position;
  assert_int_equal(az_ulib_ustream_get_position(&ustream_instance, &current_position), AZ_OK);

  /// act
  az_result result = az_ulib_ustream_release(&ustream_instance, current_position);

  /// assert
  assert_int_equal(result, AZ_ERROR_ARG);
  check_buffer(
      &ustream_instance,
      USTREAM_COMPLIANCE_LENGTH_1,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* If the release position is already released, the release shall return
 * AZ_ERROR_ARG, and do not release any resource. */
static void az_ulib_ustream_release_compliance_release_position_already_released_failed(
    void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance, buf_result, USTREAM_COMPLIANCE_LENGTH_2, &size_result),
      AZ_OK);
  assert_int_equal(
      az_ulib_ustream_release(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_2 - 1), AZ_OK);

  /// act
  az_result result = az_ulib_ustream_release(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1);

  /// assert
  assert_int_equal(result, AZ_ERROR_ARG);
  assert_int_equal(az_ulib_ustream_reset(&ustream_instance), AZ_OK);
  check_buffer(
      &ustream_instance,
      USTREAM_COMPLIANCE_LENGTH_2,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

/* The reset shall change the current position of the buffer. */
static void az_ulib_ustream_reset_compliance_back_to_beginning_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(az_ulib_ustream_read(&ustream_instance, buf_result, 5, &size_result), AZ_OK);

  /// act
  az_result result = az_ulib_ustream_reset(&ustream_instance);

  /// assert
  assert_int_equal(result, AZ_OK);
  check_buffer(
      &ustream_instance,
      0,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_reset_compliance_back_position_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size_result;
  assert_int_equal(
      az_ulib_ustream_read(
          &ustream_instance, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result),
      AZ_OK);
  assert_int_equal(
      az_ulib_ustream_release(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1 - 1), AZ_OK);

  /// act
  az_result result = az_ulib_ustream_reset(&ustream_instance);

  /// assert
  assert_int_equal(result, AZ_OK);
  check_buffer(
      &ustream_instance,
      USTREAM_COMPLIANCE_LENGTH_1,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
}

static void az_ulib_ustream_reset_compliance_cloned_buffer_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;
  USTREAM_COMPLIANCE_TARGET_FACTORY(&ustream_instance);
  uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
  size_t size;
  assert_int_equal(
      az_ulib_ustream_read(&ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_1, &size), AZ_OK);
  assert_int_equal(
      az_ulib_ustream_release(&ustream_instance, USTREAM_COMPLIANCE_LENGTH_1 - 1), AZ_OK);
  az_ulib_ustream ustream_instance_clone;
  az_result result = az_ulib_ustream_clone(&ustream_instance_clone, &ustream_instance, 100);
  assert_int_equal(result, AZ_OK);
  assert_int_equal(
      az_ulib_ustream_read(&ustream_instance_clone, buf, USTREAM_COMPLIANCE_LENGTH_1, &size),
      AZ_OK);
  assert_int_equal(
      az_ulib_ustream_release(&ustream_instance_clone, 100 + USTREAM_COMPLIANCE_LENGTH_1 - 1),
      AZ_OK);

  /// act
  result = az_ulib_ustream_reset(&ustream_instance_clone);

  /// assert
  assert_int_equal(result, AZ_OK);
  check_buffer(
      &ustream_instance_clone,
      USTREAM_COMPLIANCE_LENGTH_2,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  check_buffer(
      &ustream_instance,
      USTREAM_COMPLIANCE_LENGTH_1,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  /// cleanup
  (void)az_ulib_ustream_dispose(&ustream_instance);
  (void)az_ulib_ustream_dispose(&ustream_instance_clone);
}

#define AZ_ULIB_USTREAM_PRECONDITION_COMPLIANCE_UT_LIST                                         \
  cmocka_unit_test(az_ulib_ustream_dispose_compliance_null_buffer_failed),                      \
      cmocka_unit_test(az_ulib_ustream_dispose_compliance_buffer_is_not_type_of_buffer_failed), \
      cmocka_unit_test(az_ulib_ustream_clone_compliance_null_buffer_failed),                    \
      cmocka_unit_test(az_ulib_ustream_clone_compliance_null_buffer_clone_failed),              \
      cmocka_unit_test(az_ulib_ustream_clone_compliance_buffer_is_not_type_of_buffer_failed),   \
      cmocka_unit_test(az_ulib_ustream_get_remaining_size_compliance_null_buffer_failed),       \
      cmocka_unit_test(                                                                         \
          az_ulib_ustream_get_remaining_size_compliance_buffer_is_not_type_of_buffer_failed),   \
      cmocka_unit_test(az_ulib_ustream_get_remaining_size_compliance_null_size_failed),         \
      cmocka_unit_test(az_ulib_ustream_get_current_position_compliance_null_buffer_failed),     \
      cmocka_unit_test(                                                                         \
          az_ulib_ustream_get_current_position_compliance_buffer_is_not_type_of_buffer_failed), \
      cmocka_unit_test(az_ulib_ustream_get_current_position_compliance_null_position_failed),   \
      cmocka_unit_test(az_ulib_ustream_read_compliance_null_buffer_failed),                     \
      cmocka_unit_test(az_ulib_ustream_read_compliance_non_type_of_buffer_api_failed),          \
      cmocka_unit_test(az_ulib_ustream_read_compliance_buffer_with_zero_size_failed),           \
      cmocka_unit_test(az_ulib_ustream_read_compliance_null_return_buffer_failed),              \
      cmocka_unit_test(az_ulib_ustream_read_compliance_null_return_size_failed),                \
      cmocka_unit_test(az_ulib_ustream_set_position_compliance_null_buffer_failed),             \
      cmocka_unit_test(az_ulib_ustream_set_position_compliance_non_type_of_buffer_api_failed),  \
      cmocka_unit_test(az_ulib_ustream_release_compliance_null_buffer_failed),                  \
      cmocka_unit_test(az_ulib_ustream_release_compliance_non_type_of_buffer_api_failed),       \
      cmocka_unit_test(az_ulib_ustream_reset_compliance_null_buffer_failed),                    \
      cmocka_unit_test(az_ulib_ustream_reset_compliance_non_type_of_buffer_api_failed),

#define AZ_ULIB_USTREAM_COMPLIANCE_UT_LIST                                                                                              \
  cmocka_unit_test_setup_teardown(                                                                                                      \
      az_ulib_ustream_dispose_compliance_cloned_instance_disposed_first_succeed, setup, teardown),                                      \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_dispose_compliance_cloned_instance_disposed_second_succeed,                                                   \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_dispose_compliance_single_instance_succeed, setup, teardown),                                                 \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_clone_compliance_new_buffer_cloned_with_zero_offset_succeed,                                                  \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_clone_compliance_new_buffer_cloned_with_offset_succeed,                                                       \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_clone_compliance_empty_buffer_succeed, setup, teardown),                                                      \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed,          \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_negative_offset_succeed, \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_clone_compliance_cloned_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed,       \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_clone_compliance_offset_exceed_size_failed, setup, teardown),                                                 \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_get_remaining_size_compliance_new_buffer_succeed, setup, teardown),                                           \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_get_remaining_size_compliance_new_buffer_with_non_zero_current_position_succeed,                              \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_get_remaining_size_compliance_cloned_buffer_with_non_zero_current_position_succeed,                           \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_get_current_position_compliance_new_buffer_succeed, setup, teardown),                                         \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_get_current_position_compliance_new_buffer_with_non_zero_current_position_succeed,                            \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_get_current_position_compliance_cloned_buffer_with_non_zero_current_position_succeed,                         \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_read_compliance_get_from_original_buffer_succeed, setup, teardown),                                           \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_read_compliance_get_from_cloned_buffer_succeed, setup, teardown),                                             \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_read_compliance_single_buffer_succeed, setup, teardown),                                                      \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_read_compliance_right_boundary_condition_succeed, setup, teardown),                                           \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_read_compliance_cloned_buffer_right_boundary_condition_succeed,                                               \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_read_compliance_boundary_condition_succeed, setup, teardown),                                                 \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_read_compliance_left_boundary_condition_succeed, setup, teardown),                                            \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_read_compliance_single_byte_succeed, setup, teardown),                                                        \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_set_position_compliance_back_to_beginning_succeed, setup, teardown),                                          \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_set_position_compliance_back_position_succeed, setup, teardown),                                              \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_set_position_compliance_forward_position_succeed, setup, teardown),                                           \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_set_position_compliance_forward_to_the_end_position_succeed,                                                  \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_set_position_compliance_run_full_buffer_byte_by_byte_succeed,                                                 \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_set_position_compliance_run_full_buffer_byte_by_byte_reverse_order_succeed,                                   \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_set_position_compliance_cloned_buffer_back_to_beginning_succeed,                                              \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_set_position_compliance_cloned_buffer_back_position_succeed,                                                  \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_set_position_compliance_cloned_buffer_forward_position_succeed,                                               \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_set_position_compliance_cloned_buffer_forward_to_the_end_position_succeed,                                    \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_set_position_compliance_cloned_buffer_run_full_buffer_byte_by_byte_succeed,                                   \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_set_position_compliance_cloned_buffer_run_full_buffer_byte_by_byte_reverse_order_succeed,                     \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_set_position_compliance_forward_out_of_the_buffer_failed,                                                     \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_set_position_compliance_back_before_first_valid_position_failed,                                              \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_set_position_compliance_back_before_first_valid_position_with_offset_failed,                                  \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_release_compliance_succeed, setup, teardown),                                                                 \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_release_compliance_release_all_succeed, setup, teardown),                                                     \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_release_compliance_run_full_buffer_byte_by_byte_succeed,                                                      \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_release_compliance_cloned_buffer_succeed, setup, teardown),                                                   \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_release_compliance_cloned_buffer_release_all_succeed, setup, teardown),                                       \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_release_compliance_cloned_buffer_run_full_buffer_byte_by_byte_succeed,                                        \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_release_compliance_release_after_current_failed, setup, teardown),                                            \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_release_compliance_release_position_already_released_failed,                                                  \
          setup,                                                                                                                        \
          teardown),                                                                                                                    \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_reset_compliance_back_to_beginning_succeed, setup, teardown),                                                 \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_reset_compliance_back_position_succeed, setup, teardown),                                                     \
      cmocka_unit_test_setup_teardown(                                                                                                  \
          az_ulib_ustream_reset_compliance_cloned_buffer_succeed, setup, teardown),

#endif /* AZ_ULIB_USTREAM_COMPLIANCE_UT_H */
