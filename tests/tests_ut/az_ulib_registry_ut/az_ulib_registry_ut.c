// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "az_ulib_pal_api.h"
#include "az_ulib_query_1_model.h"
#include "az_ulib_registry_api.h"
#include "az_ulib_registry_ut.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"

#include "az_ulib_test_precondition.h"
#include "azure/core/az_precondition.h"

#include "cmocka.h"

az_ulib_pal_os_lock* g_lock;
int8_t g_lock_diff;
int8_t g_count_acquire;
int8_t g_count_sleep;
void az_pal_os_lock_init(az_ulib_pal_os_lock* lock) { g_lock = lock; }

void az_pal_os_lock_deinit(az_ulib_pal_os_lock* lock)
{
  if (lock == g_lock)
  {
    g_lock = NULL;
  }
}

void az_pal_os_lock_acquire(az_ulib_pal_os_lock* lock)
{
  if (lock == g_lock)
  {
    g_lock_diff++;
    g_count_acquire++;
  }
}

void az_pal_os_lock_release(az_ulib_pal_os_lock* lock)
{
  if (lock == g_lock)
  {
    g_lock_diff--;
  }
}

void az_pal_os_sleep(uint32_t sleep_time_ms)
{
  (void)sleep_time_ms;
  g_count_sleep++;
}

#ifndef AZ_NO_PRECONDITION_CHECKING
AZ_ULIB_ENABLE_PRECONDITION_CHECK_TESTS()
#endif // AZ_NO_PRECONDITION_CHECKING

#define REGISTRY_PAGE_SIZE 0x800

/* Static memory to store registry information. */
static uint8_t registry_buffer[REGISTRY_PAGE_SIZE * 2];
static uint8_t registry_informarmation_buffer[REGISTRY_PAGE_SIZE];

#define __REGISTRY_START (registry_buffer[0])
#define __REGISTRY_END (registry_buffer[(REGISTRY_PAGE_SIZE * 2)])
#define __REGISTRYINFO_START (registry_informarmation_buffer[0])
#define __REGISTRYINFO_END (registry_informarmation_buffer[REGISTRY_PAGE_SIZE])

static const az_ulib_registry_control_block registry_cb
    = { .registry_start = (void*)(&__REGISTRY_START),
        .registry_end = (void*)(&__REGISTRY_END),
        .registry_info_start = (void*)(&__REGISTRYINFO_START),
        .registry_info_end = (void*)(&__REGISTRYINFO_END),
        .page_size = REGISTRY_PAGE_SIZE };

static const az_ulib_registry_control_block registry_cb_2
    = { .registry_start = (void*)(&__REGISTRY_START),
        .registry_end = (void*)(&__REGISTRY_END),
        .registry_info_start = (void*)(&__REGISTRYINFO_START),
        .registry_info_end = (void*)(&__REGISTRYINFO_END),
        .page_size = REGISTRY_PAGE_SIZE };

const az_span TEST_KEY_1 = AZ_SPAN_LITERAL_FROM_STR("TEST_KEY_1");
const az_span TEST_VALUE_1 = AZ_SPAN_LITERAL_FROM_STR("TEST_VALUE_1");
const az_span TEST_KEY_2 = AZ_SPAN_LITERAL_FROM_STR("TEST_KEY_2");
const az_span TEST_VALUE_2 = AZ_SPAN_LITERAL_FROM_STR("TEST_VALUE_2");
const az_span TEST_KEY_3 = AZ_SPAN_LITERAL_FROM_STR("TEST_KEY_3");
const az_span TEST_VALUE_3 = AZ_SPAN_LITERAL_FROM_STR("TEST_VALUE_3");
const az_span TEST_KEY_4 = AZ_SPAN_LITERAL_FROM_STR("TEST_KEY_4");
const az_span TEST_VALUE_4 = AZ_SPAN_LITERAL_FROM_STR("TEST_VALUE_4");
const az_span TEST_KEY_A = AZ_SPAN_LITERAL_FROM_STR("TEST_KEY_A");
const az_span TEST_VALUE_A = AZ_SPAN_LITERAL_FROM_STR("TEST_VALUE_A");

static void init_and_add_4_keys(void)
{
  az_ulib_registry_init(&registry_cb);
  az_ulib_registry_clean_all();
  assert_int_equal(az_ulib_registry_add(TEST_KEY_1, TEST_VALUE_1), AZ_OK);
  assert_int_equal(az_ulib_registry_add(TEST_KEY_2, TEST_VALUE_2), AZ_OK);
  assert_int_equal(az_ulib_registry_add(TEST_KEY_3, TEST_VALUE_3), AZ_OK);
  assert_int_equal(az_ulib_registry_add(TEST_KEY_4, TEST_VALUE_4), AZ_OK);
  g_count_acquire = 0;
}

static int setup(void** state)
{
  (void)state;

  g_lock = NULL;
  g_lock_diff = 0;
  g_count_acquire = 0;
  g_count_sleep = 0;

  return 0;
}

static int teardown(void** state)
{
  (void)state;

  return 0;
}

/**
 * Beginning of the UT for interface module.
 */
#ifndef AZ_NO_PRECONDITION_CHECKING

/* If the provided handle is NULL, the az_ulib_registry_init shall fail with precondition. */
static void az_ulib_registry_init_with_null_handle_failed(void** state)
{
  /// arrange
  (void)state;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED_VOID_FUNCTION(az_ulib_registry_init(NULL));

  /// cleanup
}

/* If the registry was already initialized, the az_ulib_registry_init shall fail with precondition.
 */
static void az_ulib_registry_init_double_initialization_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_registry_init(&registry_cb);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED_VOID_FUNCTION(az_ulib_registry_init(&registry_cb_2));

  /// cleanup
  az_ulib_registry_deinit();
}

/* If the registry was not initialized, the az_ulib_registry_deinit shall fail with precondition.
 */
static void az_ulib_registry_deinit_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED_VOID_FUNCTION(az_ulib_registry_deinit());

  /// cleanup
}

/* If the registry was not initialized, the az_ulib_registry_delete shall fail with precondition.
 */
static void az_ulib_registry_delete_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_registry_delete(TEST_KEY_1));

  /// cleanup
}

/* If the provided key is AZ_SPAN_EMPTY, the az_ulib_registry_delete shall fail with precondition.
 */
static void az_ulib_registry_delete_empty_key_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_registry_init(&registry_cb);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_registry_delete(AZ_SPAN_EMPTY));

  /// cleanup
  az_ulib_registry_deinit();
}

/* If the registry was not initialized, the az_ulib_registry_try_get_value shall fail with
 * precondition. */
static void az_ulib_registry_try_get_value_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;
  az_span value = AZ_SPAN_EMPTY;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_registry_try_get_value(TEST_KEY_1, &value));

  /// cleanup
}

/* If the provided key is AZ_SPAN_EMPTY, the az_ulib_registry_try_get_value shall fail with
 * precondition. */
static void az_ulib_registry_try_get_value_with_empty_key_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_registry_init(&registry_cb);
  az_span value = AZ_SPAN_EMPTY;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_registry_try_get_value(AZ_SPAN_EMPTY, &value));

  /// cleanup
  az_ulib_registry_deinit();
}

/* If the provided value pointer is NULL, the az_ulib_registry_try_get_value shall fail with
 * precondition. */
static void az_ulib_registry_try_get_value_with_null_value_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_registry_init(&registry_cb);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_registry_try_get_value(TEST_KEY_1, NULL));

  /// cleanup
  az_ulib_registry_deinit();
}

/* If the registry was not initialized, the az_ulib_registry_add shall fail with
 * precondition. */
static void az_ulib_registry_add_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_registry_add(TEST_KEY_1, TEST_VALUE_1));

  /// cleanup
}

/* If the provided key is AZ_SPAN_EMPTY, the az_ulib_registry_add shall fail with
 * precondition. */
static void az_ulib_registry_add_with_empty_key_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_registry_init(&registry_cb);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_registry_add(AZ_SPAN_EMPTY, TEST_VALUE_1));

  /// cleanup
  az_ulib_registry_deinit();
}

/* If the provided value is AZ_SPAN_EMPTY, the az_ulib_registry_add shall fail with
 * precondition. */
static void az_ulib_registry_add_with_empty_value_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_registry_init(&registry_cb);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_registry_add(TEST_KEY_1, AZ_SPAN_EMPTY));

  /// cleanup
  az_ulib_registry_deinit();
}

/* If the registry was not initialized, the az_ulib_registry_clean_all shall fail with
 * precondition. */
static void az_ulib_registry_clean_all_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED_VOID_FUNCTION(az_ulib_registry_clean_all());

  /// cleanup
}

/* If the registry was not initialized, the az_ulib_registry_get_info shall fail with
 * precondition. */
static void az_ulib_registry_get_info_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_registry_info info;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED_VOID_FUNCTION(az_ulib_registry_get_info(&info));

  /// cleanup
}

/* If the provided pointer to info is NULL, the az_ulib_registry_get_info shall fail with
 * precondition. */
static void az_ulib_registry_get_info_with_NULL_info_pointer_failed(void** state)
{
  /// arrange
  (void)state;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED_VOID_FUNCTION(az_ulib_registry_get_info(NULL));

  /// cleanup
}

#endif // AZ_NO_PRECONDITION_CHECKING

/* The az_ulib_registry_init shall initialize the ipc control block. */
/* The az_ulib_registry_init shall initialize the lock mechanism. */
static void az_ulib_registry_init_succeed(void** state)
{
  /// arrange
  (void)state;

  /// act
  az_ulib_registry_init(&registry_cb);

  /// assert
  assert_non_null(g_lock);
  assert_int_equal(g_lock_diff, 0);

  /// cleanup
  az_ulib_registry_deinit();
}

/* The az_ulib_registry_deinit shall release all resources. */
static void az_ulib_registry_deinit_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_registry_init(&registry_cb);

  /// act
  az_ulib_registry_deinit();

  /// assert
  az_ulib_registry_init(&registry_cb);

  /// cleanup
  az_ulib_registry_deinit();
}

/* The az_ulib_registry_delete shall delete the provided key from registry. */
static void az_ulib_registry_delete_single_key_succeed(void** state)
{
  /// arrange
  (void)state;
  az_span value = AZ_SPAN_EMPTY;
  az_ulib_registry_init(&registry_cb);
  az_ulib_registry_clean_all();
  assert_int_equal(az_ulib_registry_add(TEST_KEY_1, TEST_VALUE_1), AZ_OK);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_registry_delete(TEST_KEY_1);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_1, &value), AZ_ERROR_ITEM_NOT_FOUND);

  /// cleanup
  az_ulib_registry_deinit();
}

static void az_ulib_registry_delete_first_key_succeed(void** state)
{
  /// arrange
  (void)state;
  az_span value = AZ_SPAN_EMPTY;
  init_and_add_4_keys();

  /// act
  az_result result = az_ulib_registry_delete(TEST_KEY_1);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_1, &value), AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_2, &value), AZ_OK);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_3, &value), AZ_OK);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_4, &value), AZ_OK);

  /// cleanup
  az_ulib_registry_deinit();
}

static void az_ulib_registry_delete_last_key_succeed(void** state)
{
  /// arrange
  (void)state;
  az_span value = AZ_SPAN_EMPTY;
  init_and_add_4_keys();

  /// act
  az_result result = az_ulib_registry_delete(TEST_KEY_4);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_1, &value), AZ_OK);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_2, &value), AZ_OK);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_3, &value), AZ_OK);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_4, &value), AZ_ERROR_ITEM_NOT_FOUND);

  /// cleanup
  az_ulib_registry_deinit();
}

static void az_ulib_registry_delete_second_key_succeed(void** state)
{
  /// arrange
  (void)state;
  az_span value = AZ_SPAN_EMPTY;
  init_and_add_4_keys();

  /// act
  az_result result = az_ulib_registry_delete(TEST_KEY_2);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_1, &value), AZ_OK);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_2, &value), AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_3, &value), AZ_OK);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_4, &value), AZ_OK);

  /// cleanup
  az_ulib_registry_deinit();
}

static void az_ulib_registry_delete_third_key_succeed(void** state)
{
  /// arrange
  (void)state;
  az_span value = AZ_SPAN_EMPTY;
  init_and_add_4_keys();

  /// act
  az_result result = az_ulib_registry_delete(TEST_KEY_3);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_1, &value), AZ_OK);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_2, &value), AZ_OK);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_3, &value), AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_4, &value), AZ_OK);

  /// cleanup
  az_ulib_registry_deinit();
}

/* If the provided key does not exist, the az_ulib_registry_delete shall return
 * AZ_ERROR_ITEM_NOT_FOUND. */
static void az_ulib_registry_delete_unknow_key_failed(void** state)
{
  /// arrange
  (void)state;
  az_span value = AZ_SPAN_EMPTY;
  init_and_add_4_keys();

  /// act
  az_result result = az_ulib_registry_delete(TEST_KEY_A);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_1, &value), AZ_OK);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_2, &value), AZ_OK);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_3, &value), AZ_OK);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_4, &value), AZ_OK);

  /// cleanup
  az_ulib_registry_deinit();
}

/* The az_ulib_registry_try_get_value shall return the stored value for the provided key. */
static void az_ulib_registry_try_get_value_succeed(void** state)
{
  /// arrange
  (void)state;
  az_span value = AZ_SPAN_EMPTY;
  init_and_add_4_keys();

  /// act
  az_result result = az_ulib_registry_try_get_value(TEST_KEY_1, &value);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_true(az_span_is_content_equal(value, TEST_VALUE_1));

  /// cleanup
  az_ulib_registry_deinit();
}

/* If the provided key does not exist, the az_ulib_registry_try_get_value shall return
 * AZ_ERROR_ITEM_NOT_FOUND. */
static void az_ulib_registry_try_get_value_unknow_key_failed(void** state)
{
  /// arrange
  (void)state;
  az_span value = AZ_SPAN_EMPTY;
  init_and_add_4_keys();

  /// act
  az_result result = az_ulib_registry_try_get_value(TEST_KEY_A, &value);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_true(az_span_is_content_equal(value, AZ_SPAN_EMPTY));

  /// cleanup
  az_ulib_registry_deinit();
}

/* The az_ulib_registry_add shall store value for the provided key. */
static void az_ulib_registry_add_succeed(void** state)
{
  /// arrange
  (void)state;
  az_span value = AZ_SPAN_EMPTY;
  az_ulib_registry_init(&registry_cb);
  az_ulib_registry_clean_all();
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_registry_add(TEST_KEY_1, TEST_VALUE_1);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_1, &value), AZ_OK);
  assert_true(az_span_is_content_equal(value, TEST_VALUE_1));

  /// cleanup
  az_ulib_registry_deinit();
}

/* If the provided key already exist, the az_ulib_registry_add shall return
 * AZ_ERROR_ULIB_ELEMENT_DUPLICATE. */
static void az_ulib_registry_add_duplicated_key_failed(void** state)
{
  /// arrange
  (void)state;
  az_span value = AZ_SPAN_EMPTY;
  az_ulib_registry_init(&registry_cb);
  az_ulib_registry_clean_all();
  assert_int_equal(az_ulib_registry_add(TEST_KEY_1, TEST_VALUE_1), AZ_OK);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_registry_add(TEST_KEY_1, TEST_VALUE_A);

  /// assert
  assert_int_equal(result, AZ_ERROR_ULIB_ELEMENT_DUPLICATE);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_equal(az_ulib_registry_try_get_value(TEST_KEY_1, &value), AZ_OK);
  assert_true(az_span_is_content_equal(value, TEST_VALUE_1));

  /// cleanup
  az_ulib_registry_deinit();
}

/* If there is no more space in the registry info, the az_ulib_registry_add shall return
 * AZ_ERROR_NOT_ENOUGH_SPACE. */
static void az_ulib_registry_add_out_of_registry_info_space_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_registry_init(&registry_cb);
  az_ulib_registry_clean_all();

  az_ulib_registry_info info;
  az_ulib_registry_get_info(&info);
  /* Fill all available registry info. */
  for (size_t i = 0; i < info.free_registry_info; i++)
  {
    az_span key = az_span_create((uint8_t*)&i, (int32_t)sizeof(i));
    assert_int_equal(az_ulib_registry_add(key, key), AZ_OK);
  }
  az_ulib_registry_get_info(&info);
  assert_int_equal(info.free_registry_info, 0);
  assert_int_not_equal(info.free_registry_data, 0);

  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_registry_add(TEST_KEY_1, TEST_VALUE_1);

  /// assert
  assert_int_equal(result, AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  az_ulib_registry_deinit();
}

/* If there is no more space in the registry data, the az_ulib_registry_add shall return
 * AZ_ERROR_OUT_OF_MEMORY. */
static void az_ulib_registry_add_out_of_registry_data_space_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_registry_init(&registry_cb);
  az_ulib_registry_clean_all();

  az_ulib_registry_info info;
  az_ulib_registry_get_info(&info);
  /* Fill all available registry. */
  uint8_t val[0xff];
  for (size_t i = 0; i < sizeof(val); i++)
  {
    val[i] = 'a';
  }
  az_span val_span = az_span_create(val, (int32_t)sizeof(val));
  size_t max = (info.total_registry_data / (sizeof(val) + sizeof(uint64_t))); // key needs 64bits.
  for (size_t i = 0; i < max; i++)
  {
    az_span key = az_span_create((uint8_t*)&i, (int32_t)sizeof(i));
    assert_int_equal(az_ulib_registry_add(key, val_span), AZ_OK);
  }
  az_ulib_registry_get_info(&info);
  val_span = az_span_create(
      val,
      (int32_t)(info.free_registry_data - (2 * sizeof(uint64_t)))); // TEST_KEY_1 needs 2 x 64bits.
  assert_int_equal(az_ulib_registry_add(TEST_KEY_1, val_span), AZ_OK);
  az_ulib_registry_get_info(&info);
  assert_int_not_equal(info.free_registry_info, 0);
  assert_int_equal(info.free_registry_data, 0);

  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_registry_add(TEST_KEY_2, TEST_VALUE_2);

  /// assert
  assert_int_equal(result, AZ_ERROR_OUT_OF_MEMORY);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  az_ulib_registry_deinit();
}

static void az_ulib_registry_add_no_space_for_key_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_registry_init(&registry_cb);
  az_ulib_registry_clean_all();

  az_ulib_registry_info info;
  az_ulib_registry_get_info(&info);
  /* Fill all available registry. */
  uint8_t val[0xff];
  for (size_t i = 0; i < sizeof(val); i++)
  {
    val[i] = 'a';
  }
  az_span val_span = az_span_create(val, (int32_t)sizeof(val));
  size_t max = (info.total_registry_data / (sizeof(val) + sizeof(uint64_t))); // key needs 64bits.
  for (size_t i = 0; i < max; i++)
  {
    az_span key = az_span_create((uint8_t*)&i, (int32_t)sizeof(i));
    assert_int_equal(az_ulib_registry_add(key, val_span), AZ_OK);
  }
  az_ulib_registry_get_info(&info);

  // TEST_KEY_1 needs 2 x 64bits. Only 1 x 64bits left, and Key needs 2 x 64bits.
  val_span = az_span_create(val, (int32_t)(info.free_registry_data - (3 * sizeof(uint64_t))));

  assert_int_equal(az_ulib_registry_add(TEST_KEY_1, val_span), AZ_OK);
  az_ulib_registry_get_info(&info);
  assert_int_not_equal(info.free_registry_info, 0);
  assert_int_equal(info.free_registry_data, 8);

  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_registry_add(TEST_KEY_2, TEST_VALUE_2);

  /// assert
  assert_int_equal(result, AZ_ERROR_OUT_OF_MEMORY);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  az_ulib_registry_deinit();
}

static void az_ulib_registry_add_space_only_for_key_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_registry_init(&registry_cb);
  az_ulib_registry_clean_all();

  az_ulib_registry_info info;
  az_ulib_registry_get_info(&info);
  /* Fill all available registry. */
  uint8_t val[0xff];
  for (size_t i = 0; i < sizeof(val); i++)
  {
    val[i] = 'a';
  }
  az_span val_span = az_span_create(val, (int32_t)sizeof(val));
  size_t max = (info.total_registry_data / (sizeof(val) + sizeof(uint64_t))); // key needs 64bits.
  for (size_t i = 0; i < max; i++)
  {
    az_span key = az_span_create((uint8_t*)&i, (int32_t)sizeof(i));
    assert_int_equal(az_ulib_registry_add(key, val_span), AZ_OK);
  }
  az_ulib_registry_get_info(&info);

  // TEST_KEY_1 needs 2 x 64bits. Only 3 x 64bits left that only fits the Key, but not the value.
  val_span = az_span_create(val, (int32_t)(info.free_registry_data - (5 * sizeof(uint64_t))));

  assert_int_equal(az_ulib_registry_add(TEST_KEY_1, val_span), AZ_OK);
  az_ulib_registry_get_info(&info);
  assert_int_not_equal(info.free_registry_info, 0);
  assert_int_equal(info.free_registry_data, 24);

  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_registry_add(TEST_KEY_2, TEST_VALUE_2);

  /// assert
  assert_int_equal(result, AZ_ERROR_OUT_OF_MEMORY);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  az_ulib_registry_deinit();
}

/* The az_ulib_registry_clean_all shall Delete all stored registries. */
static void az_ulib_registry_clean_all_succeed(void** state)
{
  /// arrange
  (void)state;
  init_and_add_4_keys();
  az_ulib_registry_info info;
  az_ulib_registry_get_info(&info);
  assert_int_not_equal(info.in_use_registry_info, 0);
  g_count_acquire = 0;

  /// act
  az_ulib_registry_clean_all();

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  az_ulib_registry_get_info(&info);
  assert_int_equal(info.in_use_registry_info, 0);
  assert_int_equal(info.in_use_registry_data, 0);

  /// cleanup
  az_ulib_registry_deinit();
}

/* The az_ulib_registry_get_info shall return the registry information in the #az_ulib_registry_info
 * structure. */
static void az_ulib_registry_get_info_succeed(void** state)
{
  /// arrange
  (void)state;
  init_and_add_4_keys();
  az_ulib_registry_info info;
  g_count_acquire = 0;

  /// act
  az_ulib_registry_get_info(&info);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_not_equal(info.total_registry_info, 0);
  assert_int_equal(info.in_use_registry_info, 4);
  assert_int_equal(info.free_registry_info, info.total_registry_info - info.in_use_registry_info);
  assert_int_equal(info.total_registry_data, sizeof(registry_buffer));
  assert_int_not_equal(info.in_use_registry_data, 0);
  assert_int_equal(info.free_registry_data, info.total_registry_data - info.in_use_registry_data);

  /// cleanup
  az_ulib_registry_deinit();
}

int az_ulib_registry_ut()
{
#ifndef AZ_NO_PRECONDITION_CHECKING
  AZ_ULIB_SETUP_PRECONDITION_CHECK_TESTS();
#endif // AZ_NO_PRECONDITION_CHECKING

  const struct CMUnitTest tests[] = {
#ifndef AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test_setup_teardown(az_ulib_registry_init_with_null_handle_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_registry_init_double_initialization_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_registry_deinit_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_registry_delete_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_delete_empty_key_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_registry_try_get_value_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_registry_try_get_value_with_empty_key_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_registry_try_get_value_with_null_value_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_add_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_add_with_empty_key_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_add_with_empty_value_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_registry_clean_all_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_registry_get_info_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_registry_get_info_with_NULL_info_pointer_failed, setup, teardown),
#endif // AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test_setup_teardown(az_ulib_registry_init_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_deinit_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_delete_single_key_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_delete_first_key_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_delete_last_key_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_delete_second_key_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_delete_third_key_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_delete_unknow_key_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_try_get_value_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_registry_try_get_value_unknow_key_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_add_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_add_duplicated_key_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_registry_add_out_of_registry_info_space_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_registry_add_out_of_registry_data_space_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_add_no_space_for_key_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_registry_add_space_only_for_key_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_clean_all_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_registry_get_info_succeed, setup, teardown),
  };

  return cmocka_run_group_tests_name("az_ulib_registry_ut", tests, NULL, NULL);
}
