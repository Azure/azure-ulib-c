// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "_az_ulib_interfaces.h"
#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_interface_manager_1_model.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_ipc_ut.h"
#include "az_ulib_pal_api.h"
#include "az_ulib_query_1_model.h"
#include "az_ulib_registry_api.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"

#include "az_ulib_test_my_interface.h"
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

static az_ulib_ipc_control_block g_ipc;

#define assert_handle_equal(h1, h2)                                         \
  assert_int_equal(h1._internal.ipc_interface, h2._internal.ipc_interface); \
  assert_int_equal(h1._internal.interface_hash, h2._internal.interface_hash);

#define assert_handle_not_equal(h1, h2)                                         \
  assert_int_not_equal(h1._internal.ipc_interface, h2._internal.ipc_interface); \
  assert_int_not_equal(h1._internal.interface_hash, h2._internal.interface_hash);

static void init_ipc_and_publish_interfaces(void)
{
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_1_123_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_b_1_1_123_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_c_1_1_123_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_1_200_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_2_123_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_3_123_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_2_1_123_publish(), AZ_OK);
  g_count_acquire = 0;
}

static void unpublish_interfaces_and_deinit_ipc(void)
{
  assert_int_equal(az_ulib_test_my_interface_a_1_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_b_1_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_c_1_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_2_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_2_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_1_200_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_3_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
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

static int setup(void** state)
{
  (void)state;

  /* Start Registry. */
  az_ulib_registry_init(&registry_cb);
  az_ulib_registry_clean_all();

  g_lock = NULL;
  g_lock_diff = 0;
  g_count_acquire = 0;
  g_count_sleep = 0;

  return 0;
}

static int teardown(void** state)
{
  (void)state;

  az_ulib_registry_deinit();
  return 0;
}

/**
 * Beginning of the UT for interface module.
 */
#ifndef AZ_NO_PRECONDITION_CHECKING

/* If the provided handle is NULL, the az_ulib_ipc_init shall fail with precondition. */
static void az_ulib_ipc_init_with_null_handle_failed(void** state)
{
  /// arrange
  (void)state;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_init(NULL));

  /// cleanup
}

/* If the az_ulib_ipc_init is called more then once, it shall fail with precondition. */
static void az_ulib_ipc_init_double_initialization_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_control_block ipc1;
  az_ulib_ipc_control_block ipc2;
  assert_int_equal(az_ulib_ipc_init(&ipc1), AZ_OK);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_init(&ipc2));

  /// cleanup
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* If the IPC was not initialized, the az_ulib_ipc_deinit shall fail with precondition. */
static void az_ulib_ipc_deinit_with_ipc_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_deinit());

  /// cleanup
}

/* If the ipc was not initialized, the az_ulib_ipc_publish shall fail with precondition. */
static void az_ulib_ipc_publish_with_ipc_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_test_my_interface_a_1_1_123_publish());

  /// cleanup
}

/* If the provided descriptor is NULL, the az_ulib_ipc_publish shall fail with precondition. */
static void az_ulib_ipc_publish_with_null_descriptor_failed(void** state)
{
  /// arrange
  (void)state;
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_publish(NULL));

  /// cleanup
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* If the ipc was not initialized, the az_ulib_ipc_set_default shall fail with precondition. */
static void az_ulib_ipc_set_default_with_ipc_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_set_default(
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      MY_PACKAGE_1_VERSION,
      AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
      MY_INTERFACE_123_VERSION));

  /// cleanup
}

/* If the provided package name is empty, the az_ulib_ipc_set_default shall fail with precondition.
 */
static void az_ulib_ipc_set_default_with_empty_package_name_failed(void** state)
{
  /// arrange
  (void)state;
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_set_default(
      AZ_SPAN_EMPTY,
      MY_PACKAGE_1_VERSION,
      AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
      MY_INTERFACE_123_VERSION));

  /// cleanup
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* If the provided interface name is empty, the az_ulib_ipc_set_default shall fail with
 * precondition. */
static void az_ulib_ipc_set_default_with_empty_interface_name_failed(void** state)
{
  /// arrange
  (void)state;
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_set_default(
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      MY_PACKAGE_1_VERSION,
      AZ_SPAN_EMPTY,
      MY_INTERFACE_123_VERSION));

  /// cleanup
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* If the ipc was not initialized, the az_ulib_ipc_unpublish shall fail with precondition. */
static void az_ulib_ipc_unpublish_with_ipc_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      az_ulib_test_my_interface_a_1_1_123_unpublish(AZ_ULIB_NO_WAIT));

  /// cleanup
}

/* If the provided descriptor is NULL, the az_ulib_ipc_unpublish shall fail with precondition. */
static void az_ulib_ipc_unpublish_with_null_descriptor_failed(void** state)
{
  /// arrange
  (void)state;
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_unpublish(NULL, AZ_ULIB_NO_WAIT));

  /// cleanup
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* If the ipc was not initialized, the az_ulib_ipc_try_get_interface shall fail with precondition.
 */
static void az_ulib_ipc_try_get_interface_with_ipc_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle = { 0 };

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_try_get_interface(
      AZ_SPAN_EMPTY,
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      MY_PACKAGE_1_VERSION,
      AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
      MY_INTERFACE_123_VERSION,
      &interface_handle));

  /// cleanup
}

/* If the provided package name is AZ_SPAN_EMPTY, the az_ulib_ipc_try_get_interface shall fail
 * with precondition. */
static void az_ulib_ipc_try_get_interface_with_empty_package_name_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_try_get_interface(
      AZ_SPAN_EMPTY,
      AZ_SPAN_EMPTY,
      MY_PACKAGE_1_VERSION,
      AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
      MY_INTERFACE_123_VERSION,
      &interface_handle));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the provided interface name is AZ_SPAN_EMPTY, the az_ulib_ipc_try_get_interface shall fail
 * with precondition. */
static void az_ulib_ipc_try_get_interface_with_empty_interface_name_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_try_get_interface(
      AZ_SPAN_EMPTY,
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      MY_PACKAGE_1_VERSION,
      AZ_SPAN_EMPTY,
      MY_INTERFACE_123_VERSION,
      &interface_handle));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the provided interface version is AZ_ULIB_VERSION_DEFAULT, the az_ulib_ipc_try_get_interface
 * shall fail with precondition. */
static void az_ulib_ipc_try_get_interface_with_default_interface_version_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_try_get_interface(
      AZ_SPAN_EMPTY,
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      MY_PACKAGE_1_VERSION,
      AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
      AZ_ULIB_VERSION_DEFAULT,
      &interface_handle));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the provided handle is NULL, the az_ulib_ipc_try_get_interface shall fail with precondition.
 */
static void az_ulib_ipc_try_get_interface_with_null_handle_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_try_get_interface(
      AZ_SPAN_EMPTY,
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      MY_PACKAGE_1_VERSION,
      AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
      MY_INTERFACE_123_VERSION,
      NULL));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the ipc was not initialized, the az_ulib_ipc_try_get_capability shall fail with precondition.
 */
static void az_ulib_ipc_try_get_capability_with_ipc_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_capability_index capability_index;
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  init_ipc_and_publish_interfaces();
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_try_get_capability(
      interface_handle,
      AZ_SPAN_FROM_STR(MY_INTERFACE_MY_COMMAND_CAPABILITY_NAME),
      &capability_index));

  /// cleanup
}

/* If the provided pointer to return the capability index is NULL, the
 * az_ulib_ipc_try_get_capability shall fail with precondition. */
static void az_ulib_ipc_try_get_capability_with_null_return_capability_handle_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  init_ipc_and_publish_interfaces();
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_try_get_capability(
      interface_handle, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_COMMAND_CAPABILITY_NAME), NULL));

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If the capability name is invalid, the az_ulib_ipc_try_get_capability shall fail with
 * precondition. */
static void az_ulib_ipc_try_get_capability_with_invalid_name_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_capability_index capability_index;
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  init_ipc_and_publish_interfaces();
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      az_ulib_ipc_try_get_capability(interface_handle, AZ_SPAN_EMPTY, &capability_index));

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If the ipc is not initialized, the az_ulib_ipc_release_interface shall fail with precondition.
/*/
static void az_ulib_ipc_release_interface_with_ipc_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle handle = { 0 };

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_release_interface(handle));

  /// cleanup
}

/* If the IPC is not initialized, the az_ulib_ipc_call shall fail with precondition. */
static void az_ulib_ipc_call_with_ipc_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle handle = { 0 };
  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_JUST_RETURN;
  in.return_result = AZ_OK;
  az_result out = AZ_ULIB_PENDING;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_call(handle, MY_INTERFACE_MY_COMMAND, &in, &out));

  /// cleanup
}

/* If the IPC is not initialized, the az_ulib_ipc_call_with_str shall fail with precondition. */
static void az_ulib_ipc_call_with_str_with_ipc_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle handle = { 0 };
  az_span in = AZ_SPAN_LITERAL_FROM_STR("{ \"capability\":0, \"return_result\":65536 }");
  uint8_t buf[100];
  az_span out = AZ_SPAN_FROM_BUFFER(buf);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      az_ulib_ipc_call_with_str(handle, MY_INTERFACE_MY_COMMAND, in, &out));

  /// cleanup
}

/* If the IPC is not initialized, the az_ulib_ipc_split_method_name shall fail with precondition. */
static void az_ulib_ipc_split_method_name_with_ipc_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;
  az_span full_name = AZ_SPAN_LITERAL_FROM_STR("device.package.1.interface.2.capability");
  az_span device_name = AZ_SPAN_EMPTY;
  az_span package_name = AZ_SPAN_EMPTY;
  uint32_t package_version = 0;
  az_span interface_name = AZ_SPAN_EMPTY;
  uint32_t interface_version = 0;
  az_span capability_name = AZ_SPAN_EMPTY;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_split_method_name(
      full_name,
      &device_name,
      &package_name,
      &package_version,
      &interface_name,
      &interface_version,
      &capability_name));

  /// cleanup
}

/* If the method full name is AZ_SPAN_EMPTY, the az_ulib_ipc_split_method_name shall fail with
 * precondition. */
static void az_ulib_ipc_split_method_name_with_empty_method_full_name_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  az_span full_name = AZ_SPAN_EMPTY;
  az_span device_name = AZ_SPAN_EMPTY;
  az_span package_name = AZ_SPAN_EMPTY;
  uint32_t package_version = 0;
  az_span interface_name = AZ_SPAN_EMPTY;
  uint32_t interface_version = 0;
  az_span capability_name = AZ_SPAN_EMPTY;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_split_method_name(
      full_name,
      &device_name,
      &package_name,
      &package_version,
      &interface_name,
      &interface_version,
      &capability_name));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the device_name is NULL, the az_ulib_ipc_split_method_name shall fail with precondition. */
static void az_ulib_ipc_split_method_name_with_null_device_name_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  az_span full_name = AZ_SPAN_LITERAL_FROM_STR("device.package.1.interface.2.capability");
  az_span package_name = AZ_SPAN_EMPTY;
  uint32_t package_version = 0;
  az_span interface_name = AZ_SPAN_EMPTY;
  uint32_t interface_version = 0;
  az_span capability_name = AZ_SPAN_EMPTY;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_split_method_name(
      full_name,
      NULL,
      &package_name,
      &package_version,
      &interface_name,
      &interface_version,
      &capability_name));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the package_name is NULL, the az_ulib_ipc_split_method_name shall fail with precondition. */
static void az_ulib_ipc_split_method_name_with_null_package_name_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  az_span full_name = AZ_SPAN_LITERAL_FROM_STR("device.package.1.interface.2.capability");
  az_span device_name = AZ_SPAN_EMPTY;
  uint32_t package_version = 0;
  az_span interface_name = AZ_SPAN_EMPTY;
  uint32_t interface_version = 0;
  az_span capability_name = AZ_SPAN_EMPTY;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_split_method_name(
      full_name,
      &device_name,
      NULL,
      &package_version,
      &interface_name,
      &interface_version,
      &capability_name));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the package_version is NULL, the az_ulib_ipc_split_method_name shall fail with precondition.
 */
static void az_ulib_ipc_split_method_name_with_null_package_version_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  az_span full_name = AZ_SPAN_LITERAL_FROM_STR("device.package.1.interface.2.capability");
  az_span device_name = AZ_SPAN_EMPTY;
  az_span package_name = AZ_SPAN_EMPTY;
  az_span interface_name = AZ_SPAN_EMPTY;
  uint32_t interface_version = 0;
  az_span capability_name = AZ_SPAN_EMPTY;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_split_method_name(
      full_name,
      &device_name,
      &package_name,
      NULL,
      &interface_name,
      &interface_version,
      &capability_name));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the interface_name is NULL, the az_ulib_ipc_split_method_name shall fail with precondition. */
static void az_ulib_ipc_split_method_name_with_null_interface_name_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  az_span full_name = AZ_SPAN_LITERAL_FROM_STR("device.package.1.interface.2.capability");
  az_span device_name = AZ_SPAN_EMPTY;
  az_span package_name = AZ_SPAN_EMPTY;
  uint32_t package_version = 0;
  uint32_t interface_version = 0;
  az_span capability_name = AZ_SPAN_EMPTY;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_split_method_name(
      full_name,
      &device_name,
      &package_name,
      &package_version,
      NULL,
      &interface_version,
      &capability_name));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the interface_version is NULL, the az_ulib_ipc_split_method_name shall fail with precondition.
 */
static void az_ulib_ipc_split_method_name_with_null_interface_version_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  az_span full_name = AZ_SPAN_LITERAL_FROM_STR("device.package.1.interface.2.capability");
  az_span device_name = AZ_SPAN_EMPTY;
  az_span package_name = AZ_SPAN_EMPTY;
  uint32_t package_version = 0;
  az_span interface_name = AZ_SPAN_EMPTY;
  az_span capability_name = AZ_SPAN_EMPTY;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_split_method_name(
      full_name,
      &device_name,
      &package_name,
      &package_version,
      &interface_name,
      NULL,
      &capability_name));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the capability_name is NULL, the az_ulib_ipc_split_method_name shall fail with precondition.
 */
static void az_ulib_ipc_split_method_name_with_null_capability_name_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  az_span full_name = AZ_SPAN_LITERAL_FROM_STR("device.package.1.interface.2.capability");
  az_span device_name = AZ_SPAN_EMPTY;
  az_span package_name = AZ_SPAN_EMPTY;
  uint32_t package_version = 0;
  az_span interface_name = AZ_SPAN_EMPTY;
  uint32_t interface_version = 0;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_split_method_name(
      full_name,
      &device_name,
      &package_name,
      &package_version,
      &interface_name,
      &interface_version,
      NULL));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the IPC is not initialized, the az_ulib_ipc_query shall fail with precondition. */
static void az_ulib_ipc_query_with_ipc_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;
  az_span query = AZ_SPAN_LITERAL_FROM_STR("");
  uint8_t buf[100];
  az_span query_result = AZ_SPAN_FROM_BUFFER(buf);
  uint32_t token = 0;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_query(query, &query_result, &token));

  /// cleanup
}

/* If the pointer to result is NULL, the az_ulib_ipc_query shall fail with precondition. */
static void az_ulib_ipc_query_with_null_result_failed(void** state)
{
  /// arrange
  (void)state;
  az_span query = AZ_SPAN_LITERAL_FROM_STR("");
  uint32_t token = 0;
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_query(query, NULL, &token));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the az_span for the result is empty, the az_ulib_ipc_query shall fail with precondition. */
static void az_ulib_ipc_query_with_empty_result_failed(void** state)
{
  /// arrange
  (void)state;
  az_span query = AZ_SPAN_LITERAL_FROM_STR("");
  az_span query_result = AZ_SPAN_EMPTY;
  uint32_t token = 0;
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_query(query, &query_result, &token));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the pointer to continuation token is null, the az_ulib_ipc_query shall fail with precondition.
 */
static void az_ulib_ipc_query_with_null_continuation_token_failed(void** state)
{
  /// arrange
  (void)state;
  az_span query = AZ_SPAN_LITERAL_FROM_STR("");
  uint8_t buf[100];
  az_span query_result = AZ_SPAN_FROM_BUFFER(buf);
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_query(query, &query_result, NULL));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the IPC is not initialized, the az_ulib_ipc_query_next shall fail with precondition. */
static void az_ulib_ipc_query_next_with_ipc_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;
  uint8_t buf[100];
  az_span query_result = AZ_SPAN_FROM_BUFFER(buf);
  uint32_t token = 0;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_query_next(&token, &query_result));

  /// cleanup
}

/* If the pointer to result is NULL, the az_ulib_ipc_query_next shall fail with precondition. */
static void az_ulib_ipc_query_next_with_null_result_failed(void** state)
{
  /// arrange
  (void)state;
  uint32_t token = 0;
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_query_next(&token, NULL));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the az_span for the result is empty, the az_ulib_ipc_query_next shall fail with precondition.
 */
static void az_ulib_ipc_query_next_with_empty_result_failed(void** state)
{
  /// arrange
  (void)state;
  az_span query_result = AZ_SPAN_EMPTY;
  uint32_t token = 0;
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_query_next(&token, &query_result));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the pointer to continuation token is null, the az_ulib_ipc_query_next shall fail with
 * precondition.
 */
static void az_ulib_ipc_query_next_with_null_continuation_token_failed(void** state)
{
  /// arrange
  (void)state;
  uint8_t buf[100];
  az_span query_result = AZ_SPAN_FROM_BUFFER(buf);
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_query_next(NULL, &query_result));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

#endif // AZ_NO_PRECONDITION_CHECKING

/* The az_ulib_ipc_init shall initialize the ipc control block. */
/* The az_ulib_ipc_init shall initialize the lock mechanism. */
/* The az_ulib_ipc_init shall publish a query interface. */
/* The az_ulib_ipc_init shall publish an interface manager interface. */
static void az_ulib_ipc_init_succeed(void** state)
{
  /// arrange
  (void)state;
  g_lock = NULL;
  az_ulib_ipc_interface_handle ipc_query_handle = { 0 };
  az_ulib_ipc_interface_handle ipc_interface_manager_handle = { 0 };

  /// act
  az_result result = az_ulib_ipc_init(&g_ipc);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_non_null(g_lock);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(IPC_1_PACKAGE_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(QUERY_1_INTERFACE_NAME),
          QUERY_1_INTERFACE_VERSION,
          &ipc_query_handle),
      AZ_ULIB_RENEW);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(IPC_1_PACKAGE_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(INTERFACE_MANAGER_1_INTERFACE_NAME),
          INTERFACE_MANAGER_1_INTERFACE_VERSION,
          &ipc_interface_manager_handle),
      AZ_ULIB_RENEW);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(ipc_query_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(ipc_interface_manager_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* The az_ulib_ipc_deinit shall release all resources associate with ipc. */
/* The az_ulib_ipc_deinit shall return AZ_OK. */
/* The az_ulib_ipc_init shall unpublish its own query interface. */
static void az_ulib_ipc_deinit_succeed(void** state)
{
  /// arrange
  (void)state;
  g_lock = NULL;
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  assert_non_null(g_lock);

  /// act
  az_result result = az_ulib_ipc_deinit();

  /// assert
  assert_null(g_lock);
  assert_int_equal(result, AZ_OK);

  /// cleanup
}

/* If there is published interface, the az_ulib_ipc_deinit shall return AZ_ERROR_ULIB_BUSY. */
static void az_ulib_ipc_deinit_with_published_interface_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle ipc_query_handle = { 0 };
  az_ulib_ipc_interface_handle ipc_interface_manager_handle = { 0 };
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_deinit();

  /// assert
  assert_int_equal(result, AZ_ERROR_ULIB_BUSY);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(IPC_1_PACKAGE_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(QUERY_1_INTERFACE_NAME),
          QUERY_1_INTERFACE_VERSION,
          &ipc_query_handle),
      AZ_ULIB_RENEW);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(IPC_1_PACKAGE_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(INTERFACE_MANAGER_1_INTERFACE_NAME),
          INTERFACE_MANAGER_1_INTERFACE_VERSION,
          &ipc_interface_manager_handle),
      AZ_ULIB_RENEW);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(ipc_query_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(ipc_interface_manager_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If there is instances of the IPC owned interface, the az_ulib_ipc_deinit shall return
 * AZ_ERROR_ULIB_BUSY.
 */
static void az_ulib_ipc_deinit_with_instace_of_ipc_query_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle ipc_query_handle = { 0 };
  az_ulib_ipc_interface_handle ipc_interface_manager_handle = { 0 };
  init_ipc_and_publish_interfaces();
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(IPC_1_PACKAGE_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(QUERY_1_INTERFACE_NAME),
          QUERY_1_INTERFACE_VERSION,
          &ipc_query_handle),
      AZ_ULIB_RENEW);

  /// act
  az_result result = az_ulib_ipc_deinit();

  /// assert
  assert_int_equal(result, AZ_ERROR_ULIB_BUSY);
  assert_int_equal(az_ulib_ipc_release_interface(ipc_query_handle), AZ_OK);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(IPC_1_PACKAGE_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(QUERY_1_INTERFACE_NAME),
          QUERY_1_INTERFACE_VERSION,
          &ipc_query_handle),
      AZ_OK);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(IPC_1_PACKAGE_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(INTERFACE_MANAGER_1_INTERFACE_NAME),
          INTERFACE_MANAGER_1_INTERFACE_VERSION,
          &ipc_interface_manager_handle),
      AZ_ULIB_RENEW);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(ipc_query_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(ipc_interface_manager_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_deinit_with_instace_of_ipc_interface_manager_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle ipc_query_handle = { 0 };
  az_ulib_ipc_interface_handle ipc_interface_manager_handle = { 0 };
  init_ipc_and_publish_interfaces();
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(IPC_1_PACKAGE_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(INTERFACE_MANAGER_1_INTERFACE_NAME),
          INTERFACE_MANAGER_1_INTERFACE_VERSION,
          &ipc_interface_manager_handle),
      AZ_ULIB_RENEW);

  /// act
  az_result result = az_ulib_ipc_deinit();

  /// assert
  assert_int_equal(result, AZ_ERROR_ULIB_BUSY);
  assert_int_equal(az_ulib_ipc_release_interface(ipc_interface_manager_handle), AZ_OK);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(IPC_1_PACKAGE_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(QUERY_1_INTERFACE_NAME),
          QUERY_1_INTERFACE_VERSION,
          &ipc_query_handle),
      AZ_ULIB_RENEW);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(IPC_1_PACKAGE_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(INTERFACE_MANAGER_1_INTERFACE_NAME),
          INTERFACE_MANAGER_1_INTERFACE_VERSION,
          &ipc_interface_manager_handle),
      AZ_OK);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(ipc_query_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(ipc_interface_manager_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* The az_ulib_ipc_publish shall store the descriptor published in the IPC. The az_ulib_ipc_publish
 * shall be thread safe. */
/* The az_ulib_ipc_publish shall make the first published interface.version as default. */
static void az_ulib_ipc_publish_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  az_ulib_ipc_interface_handle interface_handle_p2 = { 0 };
  az_ulib_ipc_interface_handle default_interface_handle = { 0 };
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  g_count_acquire = 0;

  /// act
  assert_int_equal(az_ulib_test_my_interface_a_1_1_123_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_2_1_123_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_1_200_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_2_123_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_3_123_publish(), AZ_OK);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 5);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_2_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle_p2),
      AZ_ULIB_RENEW);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &default_interface_handle),
      AZ_ULIB_RENEW);
  assert_handle_equal(interface_handle, default_interface_handle);
  assert_handle_not_equal(interface_handle_p2, default_interface_handle);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle_p2), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(default_interface_handle), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_2_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_1_200_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_2_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_3_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* The az_ulib_ipc_publish shall create a different hash for each time that an interface is
 * published. */
static void az_ulib_ipc_publish_new_hash_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  init_ipc_and_publish_interfaces();
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_OK); // AZ_OK => Return the same handle.
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  g_count_acquire = 0;

  /// act
  assert_int_equal(az_ulib_test_my_interface_a_1_1_123_publish(), AZ_OK);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW); // AZ_ULIB_RENEW => Return a new handle because the hash is different.

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If the provided package version is AZ_ULIB_VERSION_DEFAULT, the az_ulib_ipc_publish shall return
 * AZ_ERROR_ARG. */
static void az_ulib_ipc_publish_with_any_package_version_failed(void** state)
{
  /// arrange
  (void)state;
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  static const az_ulib_capability_descriptor MY_INTERFACE_CAPABILITIES[]
      = { AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY_NAME) };
  const az_ulib_interface_descriptor MY_INTERFACE = AZ_ULIB_DESCRIPTOR_CREATE(
      "MY_PACKAGE", AZ_ULIB_VERSION_DEFAULT, "MY_INTERFACE", 1, MY_INTERFACE_CAPABILITIES);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_publish(&MY_INTERFACE);

  /// assert
  assert_int_equal(result, AZ_ERROR_ARG);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 0);

  /// cleanup
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* If the provided interface version is AZ_ULIB_VERSION_DEFAULT, the az_ulib_ipc_publish shall
 * return AZ_ERROR_ARG. */
static void az_ulib_ipc_publish_with_any_interface_version_failed(void** state)
{
  /// arrange
  (void)state;
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  g_count_acquire = 0;
  static const az_ulib_capability_descriptor MY_INTERFACE_CAPABILITIES[]
      = { AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY_NAME) };
  const az_ulib_interface_descriptor MY_INTERFACE = AZ_ULIB_DESCRIPTOR_CREATE(
      "MY_PACKAGE", 1, "MY_INTERFACE", AZ_ULIB_VERSION_DEFAULT, MY_INTERFACE_CAPABILITIES);

  /// act
  az_result result = az_ulib_ipc_publish(&MY_INTERFACE);

  /// assert
  assert_int_equal(result, AZ_ERROR_ARG);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 0);

  /// cleanup
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* If the provided descriptor already exist, the az_ulib_ipc_publish shall return
 * AZ_ERROR_ULIB_ELEMENT_DUPLICATE. */
static void az_ulib_ipc_publish_with_descriptor_with_same_name_and_version_failed(void** state)
{
  /// arrange
  (void)state;
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_1_123_publish(), AZ_OK);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_test_my_interface_a_1_1_123_publish();

  /// assert
  assert_int_equal(result, AZ_ERROR_ULIB_ELEMENT_DUPLICATE);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  assert_int_equal(az_ulib_test_my_interface_a_1_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* If there is no more memory to store a new descriptor, the az_ulib_ipc_publish shall return
 * AZ_ERROR_NOT_ENOUGH_SPACE. */
static void az_ulib_ipc_publish_out_of_memory_failed(void** state)
{
  /// arrange
  (void)state;
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE - 2; i++)
  {
    assert_int_equal(az_ulib_test_my_interface_publish(i), AZ_OK);
  }
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_test_my_interface_a_1_1_123_publish();

  /// assert
  assert_int_equal(result, AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE - 2; i++)
  {
    assert_int_equal(az_ulib_test_my_interface_unpublish(i), AZ_OK);
  }
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* The az_ulib_ipc_set_default shall make the provided package.version the default package for an
 * interface.version.*/
static void az_ulib_ipc_set_default_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle_p2 = { 0 };
  az_ulib_ipc_interface_handle default_interface_handle = { 0 };
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_1_123_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_2_1_123_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  // There is no current default for the interface MY_INTERFACE_1_NAME.
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &default_interface_handle),
      AZ_ERROR_ITEM_NOT_FOUND);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_set_default(
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      MY_PACKAGE_2_VERSION,
      AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
      MY_INTERFACE_123_VERSION);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_equal(result, AZ_OK);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_2_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle_p2),
      AZ_ULIB_RENEW);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &default_interface_handle),
      AZ_ULIB_RENEW);
  // Check that the current default is the package 2.
  assert_handle_equal(interface_handle_p2, default_interface_handle);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(default_interface_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle_p2), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_2_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* If another package is already default, the az_ulib_ipc_set_default shall move the default to the
 * provided one. */
static void az_ulib_ipc_set_default_move_default_version_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  az_ulib_ipc_interface_handle interface_handle_p2 = { 0 };
  az_ulib_ipc_interface_handle default_interface_handle = { 0 };
  init_ipc_and_publish_interfaces();
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_2_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle_p2),
      AZ_ULIB_RENEW);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &default_interface_handle),
      AZ_ULIB_RENEW);
  // Check that the current default is the package 1.
  assert_handle_equal(interface_handle, default_interface_handle);
  assert_handle_not_equal(interface_handle_p2, default_interface_handle);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_set_default(
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      MY_PACKAGE_2_VERSION,
      AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
      MY_INTERFACE_123_VERSION);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_equal(result, AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(default_interface_handle), AZ_OK);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &default_interface_handle),
      AZ_ULIB_RENEW);
  // Check that the current default is the package 2.
  assert_handle_equal(interface_handle_p2, default_interface_handle);
  assert_handle_not_equal(interface_handle, default_interface_handle);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle_p2), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(default_interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If the interface version is #AZ_ULIB_VERSION_DEFAULT, The az_ulib_ipc_set_default shall return
 * AZ_ERROR_ARG.*/
static void az_ulib_ipc_set_default_any_interface_version_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_set_default(
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      MY_PACKAGE_2_VERSION,
      AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
      AZ_ULIB_VERSION_DEFAULT);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 0);
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If there is no package.version.interface.version, The az_ulib_ipc_set_default shall return
 * AZ_ERROR_ITEM_NOT_FOUND.*/
static void az_ulib_ipc_set_default_unknown_interface_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_set_default(
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      MY_PACKAGE_2_VERSION,
      AZ_SPAN_FROM_STR("unknown_interface"),
      MY_INTERFACE_123_VERSION);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_set_default_unknown_interface_version_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_set_default(
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      MY_PACKAGE_2_VERSION,
      AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
      999);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_set_default_unknown_package_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_set_default(
      AZ_SPAN_FROM_STR("unknown_package"),
      MY_PACKAGE_2_VERSION,
      AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
      MY_INTERFACE_123_VERSION);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_set_default_any_package_version_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_set_default(
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      AZ_ULIB_VERSION_DEFAULT,
      AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
      MY_INTERFACE_123_VERSION);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 0);
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_set_default_unknown_package_version_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_set_default(
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      999,
      AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
      MY_INTERFACE_123_VERSION);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* The az_ulib_ipc_unpublish shall remove a descriptor for the IPC. The az_ulib_ipc_unpublish shall
 * be thread safe. */
/* The az_ulib_ipc_unpublish shall wait as long as the caller wants.*/
static void az_ulib_ipc_unpublish_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  /// act
  assert_int_equal(az_ulib_test_my_interface_a_1_1_123_unpublish(AZ_ULIB_WAIT_FOREVER), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_b_1_1_123_unpublish(AZ_ULIB_WAIT_FOREVER), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_c_1_1_123_unpublish(AZ_ULIB_WAIT_FOREVER), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_2_1_123_unpublish(AZ_ULIB_WAIT_FOREVER), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_1_200_unpublish(10000), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_2_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_3_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 7);
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_200_VERSION,
          &interface_handle),
      AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_2_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_3_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ERROR_ITEM_NOT_FOUND);

  /// cleanup
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

static void az_ulib_ipc_unpublish_random_order_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  /// act
  assert_int_equal(az_ulib_test_my_interface_a_1_3_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_1_200_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_2_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_2_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_b_1_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_c_1_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 7);
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_200_VERSION,
          &interface_handle),
      AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_2_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_3_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ERROR_ITEM_NOT_FOUND);

  /// cleanup
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* The az_ulib_ipc_unpublish shall release the descriptor position to be used by another
 descriptor.
 */
static void az_ulib_ipc_unpublish_release_resource_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  /// act
  assert_int_equal(az_ulib_test_my_interface_a_1_1_123_unpublish(AZ_ULIB_WAIT_FOREVER), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_b_1_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_c_1_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_2_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_2_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 5);

  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE - 4; i++)
  {
    assert_int_equal(az_ulib_test_my_interface_publish(i), AZ_OK);
  }
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE - 4; i++)
  {
    assert_int_equal(az_ulib_test_my_interface_unpublish(i), AZ_OK);
  }

  /// cleanup
  assert_int_equal(az_ulib_test_my_interface_a_1_1_200_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_a_1_3_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* If the provided descriptor was not published, the az_ulib_ipc_unpublish shall return
 * AZ_ERROR_ITEM_NOT_FOUND. */
static void az_ulib_ipc_unpublish_with_unknown_descriptor_failed(void** state)
{
  /// arrange
  (void)state;
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_test_my_interface_a_1_1_123_unpublish(AZ_ULIB_NO_WAIT);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 0);

  /// cleanup
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* If one of the capability in the interface is running and the wait policy is AZ_ULIB_NO_WAIT, the
 * az_ulib_ipc_unpublish shall return AZ_ERROR_ULIB_BUSY. */
static void az_ulib_ipc_unpublish_with_capability_running_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_UNPUBLISH;
  in.descriptor = &MY_INTERFACE_A_1_1_123;
  in.wait_policy_ms = AZ_ULIB_NO_WAIT;
  az_result out = AZ_ULIB_PENDING;

  az_ulib_ipc_interface_handle interface_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);
  g_count_acquire = 0;

  /// act
  // call unpublish inside of my_command.
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_MY_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(out, AZ_ERROR_ULIB_BUSY);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If one of the capability in the interface is running and the wait policy is not big enough, the
 * az_ulib_ipc_unpublish shall return AZ_ERROR_ULIB_BUSY. */
static void az_ulib_ipc_unpublish_with_capability_running_with_small_timeout_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_UNPUBLISH;
  in.descriptor = &MY_INTERFACE_A_1_1_123;
  in.wait_policy_ms = 10000;
  az_result out = AZ_ULIB_PENDING;

  az_ulib_ipc_interface_handle interface_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);
  g_count_acquire = 0;

  /// act
  // call unpublish inside of my_command.
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_MY_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(out, AZ_ERROR_ULIB_BUSY);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, g_count_sleep + 1);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If there are valid instances of the interface, the az_ulib_ipc_unpublish shall return
 * AZ_ERROR_ULIB_BUSY. */
static void az_ulib_ipc_unpublish_with_valid_interface_instance_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  az_ulib_ipc_interface_handle interface_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_test_my_interface_a_1_1_123_unpublish(AZ_ULIB_NO_WAIT);

  /// assert
  assert_int_equal(result, AZ_ERROR_ULIB_BUSY);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If one of the capability in the interface is running, the wait policy is different than
 * AZ_ULIB_NO_WAIT and the call ends before the timeout, the az_ulib_ipc_unpublish shall return
 * AZ_OK. */
// TODO: implement the test when the code is ready.

/* If one of the capability in the interface is running, the wait policy is different than
 * AZ_ULIB_NO_WAIT and the call ends after the timeout, the az_ulib_ipc_unpublish shall return
 * AZ_ERROR_ULIB_BUSY. */
// TODO: implement the test when the code is ready.

/* The az_ulib_ipc_try_get_interface shall return the handle for the interface. */
/* The az_ulib_ipc_try_get_interface shall return AZ_OK. */
static void az_ulib_ipc_try_get_interface_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle lowest_version = { 0 };
  az_ulib_ipc_interface_handle highest_version = { 0 };
  init_ipc_and_publish_interfaces();

  /// act
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &lowest_version),
      AZ_ULIB_RENEW);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_200_VERSION,
          &highest_version),
      AZ_ULIB_RENEW);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 2);
  assert_handle_not_equal(lowest_version, highest_version);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(lowest_version), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(highest_version), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If the package name is AZ_SPAN_EMPTY, the az_ulib_ipc_try_get_interface shall return
 * the interface in the default package. */
/* If the package version is AZ_ULIB_VERSION_DEFAULT, the az_ulib_ipc_try_get_interface shall return
 * the interface in the default package. */
static void az_ulib_ipc_try_get_interface_default_name_and_version_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle package_1 = { 0 };
  az_ulib_ipc_interface_handle package_2 = { 0 };
  az_ulib_ipc_interface_handle default_package_version = { 0 };
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_d_1_1_123_publish(), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_d_2_1_123_publish(), AZ_OK);
  g_count_acquire = 0;

  /// act
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_D_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &package_1),
      AZ_ULIB_RENEW);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_D_NAME),
          MY_PACKAGE_2_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &package_2),
      AZ_ULIB_RENEW);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_D_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &default_package_version),
      AZ_ULIB_RENEW);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 3);
  assert_handle_equal(package_1, default_package_version);
  assert_handle_not_equal(package_1, package_2);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(package_1), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(package_2), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(default_package_version), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_d_1_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_test_my_interface_d_2_1_123_unpublish(AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

static void az_ulib_ipc_try_get_interface_default_name_only_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle package_1 = { 0 };
  az_ulib_ipc_interface_handle package_2 = { 0 };
  az_ulib_ipc_interface_handle default_package_version = { 0 };
  init_ipc_and_publish_interfaces();

  /// act
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &package_1),
      AZ_ULIB_RENEW);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_2_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &package_2),
      AZ_ULIB_RENEW);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          AZ_ULIB_VERSION_DEFAULT,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &default_package_version),
      AZ_ULIB_RENEW);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 3);
  assert_handle_equal(package_1, default_package_version);
  assert_handle_not_equal(package_1, package_2);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(package_1), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(package_2), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(default_package_version), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If the device is not AZ_SPAN_EMPTY, the az_ulib_ipc_try_get_interface shall return
 * AZ_ERROR_NOT_IMPLEMENTED. */
/* NOTE: This will be implemented in the future and this test shall reflect it. */
static void az_ulib_ipc_try_get_interface_for_specific_device_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      AZ_SPAN_FROM_STR("device_2"),
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      MY_PACKAGE_1_VERSION,
      AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
      MY_INTERFACE_123_VERSION,
      &interface_handle);

  /// assert
  assert_int_equal(result, AZ_ERROR_NOT_IMPLEMENTED);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 0);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the IPC reach the maximum number of allowed instances for a single interface, the
 * az_ulib_ipc_try_get_interface shall return AZ_ERROR_NOT_ENOUGH_SPACE. */
static void az_ulib_ipc_try_get_interface_with_max_interface_instances_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle[AZ_ULIB_CONFIG_MAX_IPC_INSTANCES];
  az_ulib_ipc_interface_handle interface_handle_plus_one = { 0 };
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INSTANCES; i++)
  {
    interface_handle[i]._internal.ipc_interface = NULL;
    interface_handle[i]._internal.interface_hash = 0;
  }

  init_ipc_and_publish_interfaces();

  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INSTANCES; i++)
  {
    assert_int_equal(
        az_ulib_ipc_try_get_interface(
            AZ_SPAN_EMPTY,
            AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
            MY_PACKAGE_1_VERSION,
            AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
            MY_INTERFACE_123_VERSION,
            &interface_handle[i]),
        AZ_ULIB_RENEW);
  }
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      AZ_SPAN_EMPTY,
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      MY_PACKAGE_1_VERSION,
      AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
      MY_INTERFACE_123_VERSION,
      &interface_handle_plus_one);

  /// assert
  assert_int_equal(result, AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INSTANCES; i++)
  {
    assert_int_equal(az_ulib_ipc_release_interface(interface_handle[i]), AZ_OK);
  }
  unpublish_interfaces_and_deinit_ipc();
}

/* If the provided interface name does not exist, the az_ulib_ipc_try_get_interface shall return
 * AZ_ERROR_ITEM_NOT_FOUND. */
static void az_ulib_ipc_try_get_interface_with_unknown_name_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      AZ_SPAN_EMPTY,
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      MY_PACKAGE_1_VERSION,
      AZ_SPAN_FROM_STR("unknown_name"),
      MY_INTERFACE_123_VERSION,
      &interface_handle);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_try_get_interface_with_unknown_version_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      AZ_SPAN_EMPTY,
      AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
      MY_PACKAGE_1_VERSION,
      AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
      9999,
      &interface_handle);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* The az_ulib_ipc_try_get_capability shall return the capability index in the interface. */
/* The az_ulib_ipc_try_get_capability shall return AZ_OK. */
static void az_ulib_ipc_try_get_capability_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  az_ulib_capability_index index0 = 0xFFFF;
  az_ulib_capability_index index1 = 0xFFFF;
  az_ulib_capability_index index2 = 0xFFFF;
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);
  g_count_acquire = 0;

  /// act
  az_result result0 = az_ulib_ipc_try_get_capability(
      interface_handle, AZ_SPAN_FROM_STR(MY_INTERFACE_GET_MY_PROPERTY_NAME), &index0);
  az_result result1 = az_ulib_ipc_try_get_capability(
      interface_handle, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_TELEMETRY_NAME), &index1);
  az_result result2 = az_ulib_ipc_try_get_capability(
      interface_handle, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_COMMAND_NAME), &index2);

  /// assert
  assert_int_equal(result0, AZ_OK);
  assert_int_equal(index0, 0);
  assert_int_equal(result1, AZ_OK);
  assert_int_equal(index1, 2);
  assert_int_equal(result2, AZ_OK);
  assert_int_equal(index2, 4);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 0);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If the capability name does not belongs to the interface, the az_ulib_ipc_try_get_capability
 * shall not change the content of capability_index and return AZ_ERROR_ITEM_NOT_FOUND. */
static void az_ulib_ipc_try_get_capability_with_not_capability_name_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  az_ulib_capability_index index = 0xFFFF;
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_try_get_capability(
      interface_handle, AZ_SPAN_FROM_STR("not_in_interface"), &index);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(index, 0xFFFF);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 0);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* The az_ulib_ipc_release_interface shall release the instance of the interface. */
/* The az_ulib_ipc_release_interface shall return AZ_OK. */
static void az_ulib_ipc_release_interface_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle = { 0 };
  init_ipc_and_publish_interfaces();
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_release_interface(interface_handle);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* The az_ulib_ipc_call shall call the capability published by the interface. */
/* The az_ulib_ipc_call shall return AZ_OK. */
static void az_ulib_ipc_call_calls_the_capability_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  az_ulib_ipc_interface_handle interface_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_JUST_RETURN;
  in.return_result = AZ_OK;
  az_result out = AZ_ULIB_PENDING;

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_MY_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(out, AZ_OK);
  assert_int_equal(g_lock_diff, 0);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* The az_ulib_ipc_call_with_str shall call the capability published by the interface. */
/* The az_ulib_ipc_call_with_str shall return AZ_OK. */
static void az_ulib_ipc_call_with_str_calls_the_capability_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  az_ulib_ipc_interface_handle interface_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_123_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);

  az_span in = AZ_SPAN_LITERAL_FROM_STR("{ \"capability\":0, \"return_result\":65536 }");
  uint8_t buf[100];
  az_span out = AZ_SPAN_FROM_BUFFER(buf);

  /// act
  az_result result = az_ulib_ipc_call_with_str(interface_handle, MY_INTERFACE_MY_COMMAND, in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_true(az_span_is_content_equal(out, AZ_SPAN_FROM_STR("{\"result\":65536}")));
  assert_int_equal(g_lock_diff, 0);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If the interface does not support call with string, the az_ulib_ipc_call_with_str shall return
 * AZ_ERROR_NOT_SUPPORTED. */
static void az_ulib_ipc_call_with_str_calls_not_supporte_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  az_ulib_ipc_interface_handle interface_handle = { 0 };
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          AZ_SPAN_EMPTY,
          AZ_SPAN_FROM_STR(MY_PACKAGE_A_NAME),
          MY_PACKAGE_1_VERSION,
          AZ_SPAN_FROM_STR(MY_INTERFACE_1_NAME),
          MY_INTERFACE_200_VERSION,
          &interface_handle),
      AZ_ULIB_RENEW);

  az_span in = AZ_SPAN_LITERAL_FROM_STR("{ \"capability\":0, \"return_result\":65536 }");
  uint8_t buf[100];
  az_span out = AZ_SPAN_FROM_BUFFER(buf);

  /// act
  az_result result = az_ulib_ipc_call_with_str(interface_handle, MY_INTERFACE_MY_COMMAND, in, &out);

  /// assert
  assert_int_equal(result, AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(g_lock_diff, 0);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* The az_ulib_ipc_get_function_table shall return the IPC table. */
static void az_ulib_ipc_get_table_succeed(void** state)
{
  /// arrange
  (void)state;

  /// act
  const az_ulib_ipc_function_table* table = az_ulib_ipc_get_function_table();

  /// assert
  assert_non_null(table);
  assert_ptr_equal(table->publish, az_ulib_ipc_publish);
  assert_ptr_equal(table->unpublish, az_ulib_ipc_unpublish);
  assert_ptr_equal(table->try_get_interface, az_ulib_ipc_try_get_interface);
  assert_ptr_equal(table->try_get_capability, az_ulib_ipc_try_get_capability);
  assert_ptr_equal(table->release_interface, az_ulib_ipc_release_interface);
  assert_ptr_equal(table->call, az_ulib_ipc_call);
  assert_ptr_equal(table->call_with_str, az_ulib_ipc_call_with_str);
  assert_ptr_equal(table->query, az_ulib_ipc_query);
  assert_ptr_equal(table->query_next, az_ulib_ipc_query_next);

  /// cleanup
}

/* The az_ulib_ipc_split_method_name shall split the method full name returning device, package,
 * interface and capability information. */
/* The az_ulib_ipc_split_method_name shall return AZ_OK. */
typedef struct
{
  char* full_name;
  char* device_name;
  char* package_name;
  uint32_t package_version;
  char* interface_name;
  uint32_t interface_version;
  char* capability_name;
} method_name_ut;

static const method_name_ut method_name_ut_list[]
    = { { .full_name = "deviceA@packageA.1.interfaceA.2:capabilityA",
          .device_name = "deviceA",
          .package_name = "packageA",
          .package_version = 1,
          .interface_name = "interfaceA",
          .interface_version = 2,
          .capability_name = "capabilityA" },
        { .full_name = "deviceB@packageB.3.interfaceB.4",
          .device_name = "deviceB",
          .package_name = "packageB",
          .package_version = 3,
          .interface_name = "interfaceB",
          .interface_version = 4,
          .capability_name = NULL },
        { .full_name = "deviceC@packageC.*.interfaceC.5:capabilityC",
          .device_name = "deviceC",
          .package_name = "packageC",
          .package_version = AZ_ULIB_VERSION_DEFAULT,
          .interface_name = "interfaceC",
          .interface_version = 5,
          .capability_name = "capabilityC" },
        { .full_name = "packageD.6.interfaceD.7:capabilityD",
          .device_name = NULL,
          .package_name = "packageD",
          .package_version = 6,
          .interface_name = "interfaceD",
          .interface_version = 7,
          .capability_name = "capabilityD" } };
static void az_ulib_ipc_split_method_name_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  az_span device_name = AZ_SPAN_EMPTY;
  az_span package_name = AZ_SPAN_EMPTY;
  uint32_t package_version = 0;
  az_span interface_name = AZ_SPAN_EMPTY;
  uint32_t interface_version = 0;
  az_span capability_name = AZ_SPAN_EMPTY;

  for (uint32_t i = 0; i < (sizeof(method_name_ut_list) / sizeof(method_name_ut)); i++)
  {
    /// act
    az_result result = az_ulib_ipc_split_method_name(
        az_span_create_from_str(method_name_ut_list[i].full_name),
        &device_name,
        &package_name,
        &package_version,
        &interface_name,
        &interface_version,
        &capability_name);

    /// assert
    assert_int_equal(result, AZ_OK);

    if (method_name_ut_list[i].device_name == NULL)
    {
      assert_null(az_span_ptr(device_name));
    }
    else
    {
      assert_true(az_span_is_content_equal(
          device_name, az_span_create_from_str(method_name_ut_list[i].device_name)));
    }

    if (method_name_ut_list[i].package_name == NULL)
    {
      assert_null(az_span_ptr(package_name));
    }
    else
    {
      assert_true(az_span_is_content_equal(
          package_name, az_span_create_from_str(method_name_ut_list[i].package_name)));
    }

    assert_int_equal(package_version, method_name_ut_list[i].package_version);

    assert_true(az_span_is_content_equal(
        interface_name, az_span_create_from_str(method_name_ut_list[i].interface_name)));

    assert_int_equal(interface_version, method_name_ut_list[i].interface_version);

    if (method_name_ut_list[i].capability_name == NULL)
    {
      assert_null(az_span_ptr(capability_name));
    }
    else
    {
      assert_true(az_span_is_content_equal(
          capability_name, az_span_create_from_str(method_name_ut_list[i].capability_name)));
    }
  }

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the provided method full name does not match the rules, the az_ulib_ipc_split_method_name
 * shall return AZ_ERROR_UNEXPECTED_CHAR. */
typedef struct
{
  char* full_name;
} method_bad_name_ut;
static const method_bad_name_ut method_bad_name_ut_list[]
    = { { .full_name = "interface" },
        { .full_name = "1.capability" },
        { .full_name = ".2.capability" },
        { .full_name = "interface:capability" },
        { .full_name = "interface.:capability" },
        { .full_name = "2.interface.1:capability" },
        { .full_name = "2.interface.1" },
        { .full_name = "interface.8:capability" },
        { .full_name = "package.interface.8:capability" },
        { .full_name = "package..interface.8:capability" },
        { .full_name = "package.*1.interface.8:capability" },
        { .full_name = "package.1.interface.*:capability" },
        { .full_name = "interface.9" } };
static void az_ulib_ipc_split_bad_method_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  az_span device_name = AZ_SPAN_EMPTY;
  az_span package_name = AZ_SPAN_EMPTY;
  uint32_t package_version = 0;
  az_span interface_name = AZ_SPAN_EMPTY;
  uint32_t interface_version = 0;
  az_span capability_name = AZ_SPAN_EMPTY;

  for (uint32_t i = 0; i < (sizeof(method_bad_name_ut_list) / sizeof(char*)); i++)
  {
    /// act
    az_result result = az_ulib_ipc_split_method_name(
        az_span_create_from_str(method_bad_name_ut_list[i].full_name),
        &device_name,
        &package_name,
        &package_version,
        &interface_name,
        &interface_version,
        &capability_name);

    /// assert
    assert_int_equal(result, AZ_ERROR_UNEXPECTED_CHAR);
    assert_null(az_span_ptr(device_name));
    assert_null(az_span_ptr(package_name));
    assert_int_equal(package_version, 0);
    assert_null(az_span_ptr(interface_name));
    assert_int_equal(interface_version, 0);
    assert_null(az_span_ptr(capability_name));
  }

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the query is empty, the az_ulib_ipc_query shall return a list of interfaces name and version,
 * separated by comma. */
/* The az_ulib_ipc_query shall return AZ_OK. */
static void az_ulib_ipc_query_succeed(void** state)
{
  /// arrange
  (void)state;
  az_span query = AZ_SPAN_LITERAL_FROM_STR("");
  uint8_t buf[400];
  az_span query_result = AZ_SPAN_FROM_BUFFER(buf);
  uint32_t token = 0;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_query(query, &query_result, &token);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_true(az_span_is_content_equal(
      query_result,
      AZ_SPAN_FROM_STR(
          "\"*ipc.1.query.1\",\"*ipc.1.interface_manager.1\",\"*MY_PACKAGE_A.1.MY_INTERFACE_1."
          "123\",\"*MY_PACKAGE_B.1.MY_INTERFACE_1.123\",\"*MY_PACKAGE_C.1.MY_INTERFACE_1.123\",\"*"
          "MY_PACKAGE_A.1.MY_INTERFACE_1.200\",\"*MY_PACKAGE_A.1.MY_INTERFACE_2.123\",\"*MY_"
          "PACKAGE_A.1.MY_INTERFACE_3.123\",\" MY_PACKAGE_A.2.MY_INTERFACE_1.123\"")));
  assert_int_equal(token, 0x000a00FF);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_query_small_buffer_succeed(void** state)
{
  /// arrange
  (void)state;
  az_span query = AZ_SPAN_LITERAL_FROM_STR("");
  uint8_t buf[110];
  az_span query_result = AZ_SPAN_FROM_BUFFER(buf);
  uint32_t token = 0;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_query(query, &query_result, &token);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_true(az_span_is_content_equal(
      query_result,
      AZ_SPAN_FROM_STR("\"*ipc.1.query.1\",\"*ipc.1.interface_manager.1\",\"*MY_PACKAGE_A.1.MY_"
                       "INTERFACE_1.123\"")));
  assert_int_equal(token, 0x000300FF);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the buffer does not fit, at least, one interface name and version, the az_ulib_ipc_query shall
 * return AZ_ERROR_NOT_ENOUGH_SPACE. */
static void az_ulib_ipc_query_buffer_too_small_failed(void** state)
{
  /// arrange
  (void)state;
  az_span query = AZ_SPAN_LITERAL_FROM_STR("");
  uint8_t buf[3];
  az_span query_result = AZ_SPAN_FROM_BUFFER(buf);
  uint32_t token = 0;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_query(query, &query_result, &token);

  /// assert
  assert_int_equal(result, AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the query is not supported, the az_ulib_ipc_query shall return AZ_ERROR_NOT_SUPPORTED. */
static void az_ulib_ipc_query_not_supported_failed(void** state)
{
  /// arrange
  (void)state;
  az_span query = AZ_SPAN_LITERAL_FROM_STR("\"query\":\"not supported\"");
  uint8_t buf[300];
  az_span query_result = AZ_SPAN_FROM_BUFFER(buf);
  uint32_t token = 0;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_query(query, &query_result, &token);

  /// assert
  assert_int_equal(result, AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If there is no more interfaces to report, the az_ulib_ipc_query shall return AZ_ULIB_EOF. */
static void az_ulib_ipc_query_eof_succeed(void** state)
{
  /// arrange
  (void)state;
  az_span query = AZ_SPAN_LITERAL_FROM_STR("");
  uint8_t buf[300];
  az_span query_result = AZ_SPAN_FROM_BUFFER(buf);
  uint32_t token = 0;
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  _az_ulib_ipc_query_interface_unpublish();
  _az_ulib_ipc_interface_manager_interface_unpublish();
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_query(query, &query_result, &token);

  /// assert
  assert_int_equal(result, AZ_ULIB_EOF);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  _az_ulib_ipc_query_interface_publish();
  _az_ulib_ipc_interface_manager_interface_publish();
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* The az_ulib_ipc_query_next shall return the next part of the query information based on the
 * continuation token. */
/* If there are more information, the az_ulib_ipc_query_next shall return AZ_OK. */
/* If there is no more interfaces to report, the az_ulib_ipc_query_next shall return AZ_ULIB_EOF. */
static void az_ulib_ipc_query_next_succeed(void** state)
{
  /// arrange
  (void)state;
  az_span query = AZ_SPAN_LITERAL_FROM_STR("");
  uint8_t buf[90];
  az_span query_result = AZ_SPAN_FROM_BUFFER(buf);
  uint32_t token = 0;
  init_ipc_and_publish_interfaces();
  assert_int_equal(az_ulib_ipc_query(query, &query_result, &token), AZ_OK);

  /// act
  /// assert
  query_result = AZ_SPAN_FROM_BUFFER(buf);
  assert_int_equal(az_ulib_ipc_query_next(&token, &query_result), AZ_OK);
  assert_true(az_span_is_content_equal(
      query_result,
      AZ_SPAN_FROM_STR(
          "\"*MY_PACKAGE_B.1.MY_INTERFACE_1.123\",\"*MY_PACKAGE_C.1.MY_INTERFACE_1.123\"")));
  assert_int_equal(token, 0x000500FF);
  query_result = AZ_SPAN_FROM_BUFFER(buf);
  assert_int_equal(az_ulib_ipc_query_next(&token, &query_result), AZ_OK);
  assert_true(az_span_is_content_equal(
      query_result,
      AZ_SPAN_FROM_STR(
          "\"*MY_PACKAGE_A.1.MY_INTERFACE_1.200\",\"*MY_PACKAGE_A.1.MY_INTERFACE_2.123\"")));
  assert_int_equal(token, 0x000700FF);
  query_result = AZ_SPAN_FROM_BUFFER(buf);
  assert_int_equal(az_ulib_ipc_query_next(&token, &query_result), AZ_OK);
  assert_true(az_span_is_content_equal(
      query_result,
      AZ_SPAN_FROM_STR(
          "\"*MY_PACKAGE_A.1.MY_INTERFACE_3.123\",\" MY_PACKAGE_A.2.MY_INTERFACE_1.123\"")));
  assert_int_equal(token, 0x000a00FF);
  query_result = AZ_SPAN_FROM_BUFFER(buf);
  assert_int_equal(az_ulib_ipc_query_next(&token, &query_result), AZ_ULIB_EOF);

  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 5);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the continuation token is not supported, the az_ulib_ipc_query_next shall return
 * AZ_ERROR_NOT_SUPPORTED. */
static void az_ulib_ipc_query_next_not_supported_failed(void** state)
{
  /// arrange
  (void)state;
  uint8_t buf[300];
  az_span query_result = AZ_SPAN_FROM_BUFFER(buf);
  uint32_t token = 0;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_query_next(&token, &query_result);

  /// assert
  assert_int_equal(result, AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

int az_ulib_ipc_ut()
{
#ifndef AZ_NO_PRECONDITION_CHECKING
  AZ_ULIB_SETUP_PRECONDITION_CHECK_TESTS();
#endif // AZ_NO_PRECONDITION_CHECKING

  const struct CMUnitTest tests[] = {
#ifndef AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test_setup_teardown(az_ulib_ipc_init_with_null_handle_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_init_double_initialization_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_deinit_with_ipc_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_publish_with_ipc_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_publish_with_null_descriptor_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_set_default_with_ipc_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_set_default_with_empty_package_name_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_set_default_with_empty_interface_name_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_unpublish_with_ipc_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_unpublish_with_null_descriptor_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_try_get_interface_with_empty_interface_name_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_try_get_interface_with_default_interface_version_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_try_get_interface_with_empty_package_name_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_try_get_interface_with_null_handle_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_try_get_interface_with_ipc_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_try_get_capability_with_null_return_capability_handle_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_try_get_capability_with_ipc_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_try_get_capability_with_invalid_name_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_release_interface_with_ipc_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_call_with_ipc_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_call_with_str_with_ipc_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_split_method_name_with_ipc_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_split_method_name_with_empty_method_full_name_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_split_method_name_with_null_device_name_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_split_method_name_with_null_package_name_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_split_method_name_with_null_package_version_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_split_method_name_with_null_interface_name_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_split_method_name_with_null_interface_version_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_split_method_name_with_null_capability_name_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_query_with_ipc_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_query_with_null_result_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_query_with_empty_result_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_query_with_null_continuation_token_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_query_next_with_ipc_not_initialized_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_query_next_with_null_result_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_query_next_with_empty_result_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_query_next_with_null_continuation_token_failed, setup, teardown),
#endif // AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test_setup_teardown(az_ulib_ipc_init_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_deinit_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_deinit_with_published_interface_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_deinit_with_instace_of_ipc_query_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_deinit_with_instace_of_ipc_interface_manager_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_publish_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_publish_new_hash_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_publish_with_any_package_version_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_publish_with_any_interface_version_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_publish_with_descriptor_with_same_name_and_version_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_publish_out_of_memory_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_set_default_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_set_default_move_default_version_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_set_default_unknown_interface_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_set_default_any_interface_version_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_set_default_unknown_interface_version_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_set_default_unknown_package_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_set_default_any_package_version_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_set_default_unknown_package_version_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_unpublish_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_unpublish_random_order_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_unpublish_release_resource_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_unpublish_with_unknown_descriptor_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_unpublish_with_capability_running_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_unpublish_with_capability_running_with_small_timeout_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_unpublish_with_valid_interface_instance_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_try_get_interface_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_try_get_interface_default_name_and_version_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_try_get_interface_default_name_only_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_try_get_interface_for_specific_device_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_try_get_interface_with_max_interface_instances_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_try_get_interface_with_unknown_name_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_try_get_interface_with_unknown_version_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_try_get_capability_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_try_get_capability_with_not_capability_name_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_release_interface_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_call_calls_the_capability_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_call_with_str_calls_the_capability_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(
        az_ulib_ipc_call_with_str_calls_not_supporte_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_get_table_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_split_method_name_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_split_bad_method_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_query_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_query_small_buffer_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_query_buffer_too_small_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_query_not_supported_failed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_query_eof_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_query_next_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(az_ulib_ipc_query_next_not_supported_failed, setup, teardown),
  };

  return cmocka_run_group_tests_name("az_ulib_ipc_ut", tests, NULL, NULL);
}
