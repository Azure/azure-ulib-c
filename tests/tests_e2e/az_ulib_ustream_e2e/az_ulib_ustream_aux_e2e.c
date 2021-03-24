// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "az_ulib_ustream.h"
#include "az_ulib_ustream_e2e.h"

#include "az_ulib_test_thread.h"
#include "az_ulib_ustream_mock_buffer.h"

#include "cmocka.h"

#define USTREAM_AUX_E2E_EXPECTED_CONTENT_1 "0123456789"
#define USTREAM_AUX_E2E_EXPECTED_CONTENT_2 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define USTREAM_AUX_E2E_EXPECTED_CONTENT_3 "abcdefghijklmnopqrstuvwxyz"

static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1
    = (const uint8_t* const)USTREAM_AUX_E2E_EXPECTED_CONTENT_1;
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2
    = (const uint8_t* const)USTREAM_AUX_E2E_EXPECTED_CONTENT_2;
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3
    = (const uint8_t* const)USTREAM_AUX_E2E_EXPECTED_CONTENT_3;
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2_3
    = (const uint8_t* const)USTREAM_AUX_E2E_EXPECTED_CONTENT_2 USTREAM_AUX_E2E_EXPECTED_CONTENT_3;

static void create_test_default_multibuffer(az_ulib_ustream* ustream)
{
  // Set up required structs for first multi
  az_ulib_ustream_multi_data_cb* default_multi_data1
      = (az_ulib_ustream_multi_data_cb*)malloc(sizeof(az_ulib_ustream_multi_data_cb));
  assert_non_null(default_multi_data1);
  memset(default_multi_data1, 0, sizeof(az_ulib_ustream_multi_data_cb));

  // Set up required structs for second multi
  az_ulib_ustream_multi_data_cb* default_multi_data2
      = (az_ulib_ustream_multi_data_cb*)malloc(sizeof(az_ulib_ustream_multi_data_cb));
  assert_non_null(default_multi_data2);
  memset(default_multi_data2, 0, sizeof(az_ulib_ustream_multi_data_cb));

  // Set up first ustream
  az_ulib_ustream_data_cb* ustream_control_block1
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  assert_non_null(ustream_control_block1);
  memset(ustream_control_block1, 0, sizeof(az_ulib_ustream_data_cb));
  az_result result = az_ulib_ustream_init(
      ustream,
      ustream_control_block1,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
      NULL);
  assert_int_equal(result, AZ_OK);

  // Set up second ustream
  az_ulib_ustream default_buffer2;
  az_ulib_ustream_data_cb* ustream_control_block2
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  assert_non_null(ustream_control_block2);
  memset(ustream_control_block2, 0, sizeof(az_ulib_ustream_data_cb));
  result = az_ulib_ustream_init(
      &default_buffer2,
      ustream_control_block2,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2),
      NULL);
  assert_int_equal(result, AZ_OK);

  // Set up third ustream
  az_ulib_ustream default_buffer3;
  az_ulib_ustream_data_cb* ustream_control_block3
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  assert_non_null(ustream_control_block3);
  memset(ustream_control_block3, 0, sizeof(az_ulib_ustream_data_cb));
  result = az_ulib_ustream_init(
      &default_buffer3,
      ustream_control_block3,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3),
      NULL);
  assert_int_equal(result, AZ_OK);

  // Concat buffers together
  assert_int_equal(
      az_ulib_ustream_concat(ustream, &default_buffer2, default_multi_data1, free), AZ_OK);
  assert_int_equal(
      az_ulib_ustream_concat(ustream, &default_buffer3, default_multi_data2, free), AZ_OK);

  (void)az_ulib_ustream_dispose(&default_buffer2);
  (void)az_ulib_ustream_dispose(&default_buffer3);
}

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT                             \
  USTREAM_AUX_E2E_EXPECTED_CONTENT_1 USTREAM_AUX_E2E_EXPECTED_CONTENT_2 \
      USTREAM_AUX_E2E_EXPECTED_CONTENT_3
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT
    = (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;
#define USTREAM_COMPLIANCE_TARGET_FACTORY(ustream) create_test_default_multibuffer(ustream)

#define TEST_POSITION 10
#define TEST_SIZE 10
static az_ulib_ustream* thread_one_ustream;
static az_ulib_ustream* thread_two_ustream;

static int thread_one_func(void* arg)
{
  (void)arg;
  uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH] = { 0 };
  size_t size_result;
  offset_t cur_pos;
  az_ulib_ustream_read(thread_one_ustream, buf_result, TEST_SIZE, &size_result);
  az_ulib_ustream_get_position(thread_one_ustream, &cur_pos);
  assert_int_equal(cur_pos, TEST_SIZE);

  return 0;
}

static int thread_two_func(void* arg)
{
  (void)arg;
  uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH] = { 0 };
  size_t size_result;
  offset_t cur_pos;
  az_ulib_ustream_set_position(thread_two_ustream, TEST_POSITION);
  az_ulib_ustream_read(thread_two_ustream, buf_result, TEST_SIZE, &size_result);
  az_ulib_ustream_get_position(thread_two_ustream, &cur_pos);
  assert_int_equal(cur_pos, TEST_SIZE + TEST_POSITION);

  return 0;
}

/**
 * Beginning of the E2E for ustream_aux.c module.
 */
static void az_ulib_ustream_multi_read_concurrent_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream* multi_ustream = ustream_mock_create();

  az_ulib_ustream concat_ustream;
  az_ulib_ustream_data_cb* control_block1
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  az_result result = az_ulib_ustream_init(
      &concat_ustream,
      control_block1,
      free,
      (const uint8_t*)USTREAM_COMPLIANCE_EXPECTED_CONTENT,
      strlen((const char*)USTREAM_COMPLIANCE_EXPECTED_CONTENT),
      NULL);
  assert_int_equal(result, AZ_OK);

  az_ulib_ustream_multi_data_cb* multi_data1
      = (az_ulib_ustream_multi_data_cb*)malloc(sizeof(az_ulib_ustream_multi_data_cb));

  result = az_ulib_ustream_concat(multi_ustream, &concat_ustream, multi_data1, free);
  assert_int_equal(AZ_OK, result);

  az_ulib_ustream_dispose(&concat_ustream);

  // Clone the multistream
  az_ulib_ustream multibuffer_clone;
  result = az_ulib_ustream_clone(&multibuffer_clone, multi_ustream, 0);
  assert_int_equal(result, AZ_OK);

  thread_one_ustream = multi_ustream;
  thread_two_ustream = &multibuffer_clone;

  set_concurrency_ustream();

  /// act
  THREAD_HANDLE test_thread_one;
  THREAD_HANDLE test_thread_two;
  (void)test_thread_create(&test_thread_one, &thread_one_func, NULL);
  (void)test_thread_create(&test_thread_two, &thread_two_func, NULL);

  /// assert
  int res1;
  int res2;
  test_thread_join(test_thread_one, &res1);
  test_thread_join(test_thread_two, &res2);

  /// cleanup
  (void)az_ulib_ustream_dispose(multi_ustream);
  (void)az_ulib_ustream_dispose(&multibuffer_clone);
}

/*-------------------az_ulib_ustream_split() e2e tests----------------------*/

static void az_ulib_ustream_split_split_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_data_cb* data_cb
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  az_ulib_ustream ustream_instance;
  az_result result = az_ulib_ustream_init(
      &ustream_instance,
      data_cb,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2_3,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2_3),
      NULL);
  assert_int_equal(result, AZ_OK);

  uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH] = { 0 };
  size_t split_location = strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2);
  size_t returned_size;

  az_ulib_ustream ustream_instance_split;

  /// act
  result = az_ulib_ustream_split(&ustream_instance, &ustream_instance_split, split_location);
  assert_int_equal(result, AZ_OK);

  /// assert
  size_t current_pos;
  result = az_ulib_ustream_get_position(&ustream_instance, &current_pos);
  assert_int_equal(result, AZ_OK);
  assert_int_equal(current_pos, 0);

  result = az_ulib_ustream_get_position(&ustream_instance_split, &current_pos);
  assert_int_equal(result, AZ_OK);
  assert_int_equal(current_pos, split_location);

  result = az_ulib_ustream_read(
      &ustream_instance, buf_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, &returned_size);
  assert_int_equal(result, AZ_OK);
  assert_int_equal(
      strncmp(
          (const char*)buf_result,
          (const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
          strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2)),
      0);

  memset(&buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  result = az_ulib_ustream_read(
      &ustream_instance_split,
      buf_result,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      &returned_size);
  assert_int_equal(result, AZ_OK);
  assert_int_equal(
      strncmp(
          (const char*)buf_result,
          (const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
          strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3)),
      0);

  /// cleanup
  az_ulib_ustream_dispose(&ustream_instance);
  az_ulib_ustream_dispose(&ustream_instance_split);
}

static void az_ulib_ustream_split_split_and_reset_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_data_cb* data_cb
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  az_ulib_ustream ustream_instance;
  az_result result = az_ulib_ustream_init(
      &ustream_instance,
      data_cb,
      free,
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2_3,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2_3),
      NULL);
  assert_int_equal(result, AZ_OK);

  uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH] = { 0 };
  size_t split_location = strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2);
  size_t returned_size;

  az_ulib_ustream ustream_instance_split;

  result = az_ulib_ustream_read(&ustream_instance, buf_result, 2, &returned_size);
  assert_int_equal(result, AZ_OK);
  assert_int_equal(returned_size, 2);

  /// act
  result = az_ulib_ustream_split(&ustream_instance, &ustream_instance_split, split_location);
  assert_int_equal(result, AZ_OK);
  result = az_ulib_ustream_reset(&ustream_instance);
  assert_int_equal(result, AZ_OK);

  /// assert
  size_t remaining_size;
  result = az_ulib_ustream_get_remaining_size(&ustream_instance, &remaining_size);
  assert_int_equal(result, AZ_OK);
  assert_int_equal(remaining_size, split_location);

  /// cleanup
  az_ulib_ustream_dispose(&ustream_instance);
  az_ulib_ustream_dispose(&ustream_instance_split);
}

static void az_ulib_ustream_split_split_and_concat_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream_data_cb* data_cb
      = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
  az_ulib_ustream ustream_instance;
  assert_int_equal(
      az_ulib_ustream_init(
          &ustream_instance,
          data_cb,
          free,
          USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2_3,
          strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2_3),
          NULL),
      AZ_OK);

  uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH] = { 0 };
  size_t split_location = strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2);
  size_t returned_size;

  az_ulib_ustream ustream_instance_split;

  az_result result
      = az_ulib_ustream_split(&ustream_instance, &ustream_instance_split, split_location);
  assert_int_equal(result, AZ_OK);

  /// act
  result = az_ulib_ustream_read(
      &ustream_instance, buf_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, &returned_size);
  assert_int_equal(result, AZ_OK);
  assert_memory_equal(
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
      buf_result,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2));

  memset(&buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

  result = az_ulib_ustream_read(
      &ustream_instance_split,
      buf_result,
      USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
      &returned_size);
  assert_int_equal(result, AZ_OK);
  assert_memory_equal(
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
      buf_result,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3));

  result = az_ulib_ustream_reset(&ustream_instance);
  assert_int_equal(result, AZ_OK);
  result = az_ulib_ustream_reset(&ustream_instance_split);
  assert_int_equal(result, AZ_OK);
  az_ulib_ustream_multi_data_cb* multi_data
      = (az_ulib_ustream_multi_data_cb*)malloc(sizeof(az_ulib_ustream_multi_data_cb));
  result = az_ulib_ustream_concat(&ustream_instance, &ustream_instance_split, multi_data, free);
  assert_int_equal(result, AZ_OK);

  /// assert
  result = az_ulib_ustream_read(
      &ustream_instance, buf_result, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, &returned_size);
  assert_int_equal(result, AZ_OK);
  assert_memory_equal(
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2_3,
      buf_result,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2_3));

  /// cleanup
  az_ulib_ustream_dispose(&ustream_instance);
  az_ulib_ustream_dispose(&ustream_instance_split);
}

static void az_ulib_ustream_split_concat_and_split_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;

  create_test_default_multibuffer(&ustream_instance);

  uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH] = { 0 };
  size_t split_location = USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH / 2;
  size_t returned_size;

  az_ulib_ustream ustream_instance_split;

  /// act
  az_result result
      = az_ulib_ustream_split(&ustream_instance, &ustream_instance_split, split_location);
  assert_int_equal(result, AZ_OK);

  // assert
  // read the ustream in two parts to ensure properly split
  size_t read_length;
  read_length = strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1);
  // read until the first expected content is done
  result = az_ulib_ustream_read(&ustream_instance, buf_result, read_length, &returned_size);
  assert_int_equal(result, AZ_OK);
  assert_memory_equal(
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
      buf_result,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));

  memset(&buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  read_length = split_location - read_length;

  // read ustream_instance through second expected content
  result = az_ulib_ustream_read(&ustream_instance, buf_result, read_length, &returned_size);
  assert_int_equal(result, AZ_OK);
  uint8_t* test_buf = (uint8_t*)malloc(read_length);
  memcpy(test_buf, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2, read_length);
  assert_memory_equal(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2, buf_result, read_length);

  size_t remaining_size;
  result = az_ulib_ustream_get_remaining_size(&ustream_instance, &remaining_size);
  assert_int_equal(remaining_size, 0);

  // check the ustream_instance_split
  memset(&buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  size_t remaining_from_second
      = strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2) - read_length;
  // read starting from the second expected content + what was read in previous ustream
  result = az_ulib_ustream_read(
      &ustream_instance_split, buf_result, remaining_from_second, &returned_size);
  assert_int_equal(result, AZ_OK);
  assert_memory_equal(
      (const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2 + read_length,
      buf_result,
      returned_size);

  memset(&buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  // read through third expected content
  read_length = strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3);
  result = az_ulib_ustream_read(&ustream_instance_split, buf_result, read_length, &returned_size);
  assert_int_equal(result, AZ_OK);
  assert_memory_equal(
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
      buf_result,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3));

  result = az_ulib_ustream_get_remaining_size(&ustream_instance_split, &remaining_size);
  assert_int_equal(remaining_size, 0);

  /// cleanup
  free(test_buf);
  az_ulib_ustream_dispose(&ustream_instance);
  az_ulib_ustream_dispose(&ustream_instance_split);
}

static void az_ulib_ustream_split_concat_and_split_and_dispose_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ustream ustream_instance;

  create_test_default_multibuffer(&ustream_instance);

  uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH] = { 0 };
  size_t split_location = USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH / 2;
  size_t returned_size;

  az_ulib_ustream ustream_instance_split;

  az_result result
      = az_ulib_ustream_split(&ustream_instance, &ustream_instance_split, split_location);
  assert_int_equal(result, AZ_OK);

  /// act
  // read the ustream in two parts to ensure properly split
  size_t read_length;
  read_length = strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1);
  result = az_ulib_ustream_read(&ustream_instance, buf_result, read_length, &returned_size);
  assert_int_equal(result, AZ_OK);
  assert_memory_equal(
      USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
      buf_result,
      strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));

  memset(&buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  read_length = split_location - read_length;
  az_ulib_ustream_dispose(&ustream_instance_split);

  // assert
  result = az_ulib_ustream_read(&ustream_instance, buf_result, read_length, &returned_size);
  assert_int_equal(result, AZ_OK);
  uint8_t* test_buf = (uint8_t*)malloc(read_length);
  memcpy(test_buf, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2, read_length);
  assert_memory_equal(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2, buf_result, read_length);

  /// cleanup
  free(test_buf);
  az_ulib_ustream_dispose(&ustream_instance);
}

#include "az_ulib_ustream_compliance_e2e.h"

int az_ulib_ustream_aux_e2e()
{
  const struct CMUnitTest tests[]
      = { cmocka_unit_test(az_ulib_ustream_multi_read_concurrent_succeed),
          cmocka_unit_test(az_ulib_ustream_split_split_succeed),
          cmocka_unit_test(az_ulib_ustream_split_split_and_reset_succeed),
          cmocka_unit_test(az_ulib_ustream_split_split_and_concat_succeed),
          cmocka_unit_test(az_ulib_ustream_split_concat_and_split_succeed),
          cmocka_unit_test(az_ulib_ustream_split_concat_and_split_and_dispose_succeed),

          AZ_ULIB_USTREAM_COMPLIANCE_E2E_LIST };

  return cmocka_run_group_tests_name("az_ulib_ustream_aux_e2e", tests, NULL, NULL);
}
