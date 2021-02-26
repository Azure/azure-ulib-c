// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ENABLE_MOCKS
#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_pal_os_api.h"
#undef ENABLE_MOCKS

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "az_ulib_test_precondition.h"
#include "azure/core/az_precondition.h"
#include "azure_macro_utils/macro_utils.h"
#include "testrunnerswitcher.h"
#include "umock_c/umock_c.h"
#include "umock_c/umock_c_negative_tests.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_stdint.h"

az_ulib_pal_os_lock* g_lock;
int8_t g_lock_diff;
int8_t g_count_acquire;
int8_t g_count_sleep;
void az_pal_os_lock_init(az_ulib_pal_os_lock* lock) { g_lock = lock; }

void az_pal_os_lock_deinit(az_ulib_pal_os_lock* lock) { g_lock = NULL; }

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

static TEST_MUTEX_HANDLE g_test_by_test;

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
  ASSERT_FAIL("umock_c reported error :%i", error_code);
}

static uint32_t my_property = 0;

static az_result get_my_property(const void* model_out)
{
  uint32_t* new_val = (uint32_t*)model_out;

  *new_val = my_property;

  return AZ_OK;
}

static az_result set_my_property(const void* const model_in)
{
  uint32_t* new_val = (uint32_t*)model_in;

  my_property = *new_val;

  return AZ_OK;
}

typedef struct my_command_model_in_tag
{
  uint8_t capability;
  const az_ulib_interface_descriptor* descriptor;
  uint32_t wait_policy_ms;
  az_ulib_ipc_interface_handle handle;
  az_ulib_capability_index command_index;
  az_result return_result;
} my_command_model_in;

typedef enum my_command_capability_tag
{
  MY_COMMAND_CAPABILITY_JUST_RETURN,
  MY_COMMAND_CAPABILITY_UNPUBLISH,
  MY_COMMAND_CAPABILITY_RELEASE_INTERFACE,
  MY_COMMAND_CAPABILITY_DEINIT,
  MY_COMMAND_CAPABILITY_CALL_AGAIN,
  MY_COMMAND_CAPABILITY_RETURN_ERROR
} my_command_capability;

static az_result my_command(const void* const model_in, const void* model_out)
{
  my_command_model_in* in = (my_command_model_in*)model_in;
  az_result* result = (az_result*)model_out;
  my_command_model_in in_2;

  switch (in->capability)
  {
    case MY_COMMAND_CAPABILITY_JUST_RETURN:
      *result = in->return_result;
      break;
    case MY_COMMAND_CAPABILITY_UNPUBLISH:
      *result = az_ulib_ipc_unpublish(in->descriptor, in->wait_policy_ms);
      break;
    case MY_COMMAND_CAPABILITY_RELEASE_INTERFACE:
      *result = az_ulib_ipc_release_interface(in->handle);
      break;
    case MY_COMMAND_CAPABILITY_DEINIT:
      *result = az_ulib_ipc_deinit();
      break;
    case MY_COMMAND_CAPABILITY_CALL_AGAIN:
      in_2.capability = 0;
      in_2.return_result = AZ_OK;
      *result = az_ulib_ipc_call(in->handle, in->command_index, &in_2, model_out);
      break;
    default:
      *result = AZ_ERROR_ITEM_NOT_FOUND;
      break;
  }

  return AZ_OK;
}

static az_result my_command_async(
    const void* const model_in,
    const void* model_out,
    const az_ulib_capability_token capability_token,
    az_ulib_capability_cancellation_callback* cancel)
{
  (void)model_in;
  (void)model_out;
  (void)capability_token;
  (void)cancel;

  return AZ_OK;
}

static az_result my_command_cancel(const az_ulib_capability_token capability_token)
{
  (void)capability_token;

  return AZ_OK;
}

typedef enum
{
  MY_INTERFACE_PROPERTY = 0,
  MY_INTERFACE_TELEMETRY = 1,
  MY_INTERFACE_TELEMETRY2 = 2,
  MY_INTERFACE_COMMAND = 3,
  MY_INTERFACE_COMMAND_ASYNC = 4
} my_interface_index;

AZ_ULIB_DESCRIPTOR_CREATE(
    MY_INTERFACE_1_V123,
    "MY_INTERFACE_1",
    123,
    AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC("my_command_async", my_command_async, my_command_cancel));

AZ_ULIB_DESCRIPTOR_CREATE(
    MY_INTERFACE_1_V2,
    "MY_INTERFACE_1",
    2,
    AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC("my_command_async", my_command_async, my_command_cancel));

AZ_ULIB_DESCRIPTOR_CREATE(
    MY_INTERFACE_2_V123,
    "MY_INTERFACE_2",
    123,
    AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC("my_command_async", my_command_async, my_command_cancel));

AZ_ULIB_DESCRIPTOR_CREATE(
    MY_INTERFACE_3_V123,
    "MY_INTERFACE_3",
    123,
    AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
    AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC("my_command_async", my_command_async, my_command_cancel));

static az_ulib_ipc g_ipc;

void init_ipc_and_publish_interfaces(void)
{
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_init(&g_ipc));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_1_V123, NULL));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_1_V2, NULL));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_2_V123, NULL));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_3_V123, NULL));

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));

  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V2.name,
          MY_INTERFACE_1_V2.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));

  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_2_V123.name,
          MY_INTERFACE_2_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));

  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_3_V123.name,
          MY_INTERFACE_3_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));
  g_count_acquire = 0;
}

void unpublish_interfaces_and_deinit_ipc(void)
{
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

#ifndef AZ_NO_PRECONDITION_CHECKING
AZ_ULIB_ENABLE_PRECONDITION_CHECK_TESTS()
#endif // AZ_NO_PRECONDITION_CHECKING

/**
 * Beginning of the UT for interface module.
 */
BEGIN_TEST_SUITE(az_ulib_ipc_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
  g_test_by_test = TEST_MUTEX_CREATE();
  ASSERT_IS_NOT_NULL(g_test_by_test);

#ifndef AZ_NO_PRECONDITION_CHECKING
  AZ_ULIB_SETUP_PRECONDITION_CHECK_TESTS();
#endif // AZ_NO_PRECONDITION_CHECKING

  ASSERT_ARE_EQUAL(int, 0, umock_c_init(on_umock_c_error));
  ASSERT_ARE_EQUAL(int, 0, umocktypes_charptr_register_types());
  ASSERT_ARE_EQUAL(int, 0, umocktypes_stdint_register_types());
  ASSERT_ARE_EQUAL(int, 0, umocktypes_bool_register_types());
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
  umock_c_deinit();

  TEST_MUTEX_DESTROY(g_test_by_test);
}

TEST_FUNCTION_INITIALIZE(test_command_initialize)
{
  if (TEST_MUTEX_ACQUIRE(g_test_by_test))
  {
    ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
  }

  g_lock = NULL;
  g_lock_diff = 0;
  g_count_acquire = 0;
  g_count_sleep = 0;

  umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(test_command_cleanup) { TEST_MUTEX_RELEASE(g_test_by_test); }

#ifndef AZ_NO_PRECONDITION_CHECKING

TEST_FUNCTION(az_ulib_ipc_init_with_null_handle_failed)
{
  /// arrange

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_init(NULL));

  /// cleanup
}

TEST_FUNCTION(az_ulib_ipc_init_double_initialization_failed)
{
  /// arrange
  az_ulib_ipc ipc1;
  az_ulib_ipc ipc2;
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_init(&ipc1));

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_init(&ipc2));

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

TEST_FUNCTION(az_ulib_ipc_deinit_with_ipc_not_initialized_failed)
{
  /// arrange

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_deinit());

  /// cleanup
}

TEST_FUNCTION(az_ulib_ipc_publish_with_ipc_not_initialized_failed)
{
  /// arrange

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_publish(&MY_INTERFACE_1_V123, NULL));

  /// cleanup
}

TEST_FUNCTION(az_ulib_ipc_publish_with_null_descriptor_failed)
{
  /// arrange
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_init(&g_ipc));

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_publish(NULL, NULL));

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

TEST_FUNCTION(az_ulib_ipc_unpublish_with_ipc_not_initialized_failed)
{
  /// arrange

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));

  /// cleanup
}

TEST_FUNCTION(az_ulib_ipc_unpublish_with_null_descriptor_failed)
{
  /// arrange
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_init(&g_ipc));

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_unpublish(NULL, AZ_ULIB_NO_WAIT));

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_with_null_name_failed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_try_get_interface(
      NULL, MY_INTERFACE_1_V123.version, AZ_ULIB_VERSION_EQUALS_TO, &interface_handle));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_with_null_handle_failed)
{
  /// arrange
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123.name, MY_INTERFACE_1_V123.version, AZ_ULIB_VERSION_EQUALS_TO, NULL));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_with_ipc_not_initialized_failed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123.name,
      MY_INTERFACE_1_V123.version,
      AZ_ULIB_VERSION_EQUALS_TO,
      &interface_handle));

  /// cleanup
}

TEST_FUNCTION(az_ulib_ipc_get_interface_with_null_original_interface_handle_failed)
{
  /// arrange
  az_ulib_ipc_interface_handle new_interface_handle;
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_get_interface(NULL, &new_interface_handle));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_get_interface_with_null_interface_handle_failed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_get_interface(interface_handle, NULL));

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_get_interface_with_ipc_not_initialized_failed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle = (az_ulib_ipc_interface_handle)0x1234;
  az_ulib_ipc_interface_handle new_interface_handle;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      az_ulib_ipc_get_interface(interface_handle, &new_interface_handle));

  /// cleanup
}

TEST_FUNCTION(az_ulib_ipc_release_interface_with_null_interface_handle_failed)
{
  /// arrange
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_release_interface(NULL));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_release_interface_with_ipc_not_initialized_failed)
{
  /// arrange

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      az_ulib_ipc_release_interface((az_ulib_ipc_interface_handle)0x1234));

  /// cleanup
}

TEST_FUNCTION(az_ulib_ipc_call_with_ipc_not_initialized_failed)
{
  /// arrange
  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_JUST_RETURN;
  in.return_result = AZ_OK;
  az_result out = AZ_ULIB_PENDING;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      az_ulib_ipc_call((az_ulib_ipc_interface_handle)0x1234, MY_INTERFACE_COMMAND, &in, &out));

  /// cleanup
}

TEST_FUNCTION(az_ulib_ipc_call_with_null_interface_handle_failed)
{
  /// arrange
  init_ipc_and_publish_interfaces();

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_JUST_RETURN;
  in.return_result = AZ_OK;
  az_result out = AZ_ULIB_PENDING;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_call(NULL, MY_INTERFACE_COMMAND, &in, &out));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

#endif // AZ_NO_PRECONDITION_CHECKING

/* The az_ulib_ipc_init shall initialize the ipc control block. */
/* The az_ulib_ipc_init shall initialize the lock mechanism. */
TEST_FUNCTION(az_ulib_ipc_init_succeed)
{
  /// arrange
  g_lock = NULL;

  /// act
  az_result result = az_ulib_ipc_init(&g_ipc);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_IS_NOT_NULL(g_lock);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

/* The az_ulib_ipc_publish shall store the descriptor published in the IPC. The az_ulib_ipc_publish
 * shall be thread safe. */
TEST_FUNCTION(az_ulib_ipc_publish_succeed)
{
  /// arrange
  ASSERT_ARE_EQUAL(int, az_ulib_ipc_init(&g_ipc), AZ_OK);

  /// act
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_1_V123, NULL));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_1_V2, NULL));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_2_V123, NULL));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_3_V123, NULL));

  /// assert
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 4, g_count_acquire);
  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

/* The az_ulib_ipc_publish shall return a valid handle for the published interface. */
TEST_FUNCTION(az_ulib_ipc_publish_return_handle_succeed)
{
  /// arrange
  ASSERT_ARE_EQUAL(int, az_ulib_ipc_init(&g_ipc), AZ_OK);
  az_ulib_ipc_interface_handle interface_handle[4];

  /// act
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_1_V123, &(interface_handle[0])));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_1_V2, &(interface_handle[1])));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_2_V123, &(interface_handle[2])));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_3_V123, &(interface_handle[3])));

  /// assert
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 4, g_count_acquire);
  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_JUST_RETURN;
  in.return_result = AZ_OK;
  az_result out = AZ_ULIB_PENDING;
  ASSERT_ARE_EQUAL(
      int, AZ_OK, az_ulib_ipc_call(interface_handle[0], MY_INTERFACE_COMMAND, &in, &out));
  ASSERT_ARE_EQUAL(int, AZ_OK, out);
  az_ulib_ipc_interface_handle interface_handle_copy;
  ASSERT_ARE_EQUAL(
      int, AZ_OK, az_ulib_ipc_get_interface(interface_handle[0], &interface_handle_copy));
  out = AZ_ULIB_PENDING;
  ASSERT_ARE_EQUAL(
      int, AZ_OK, az_ulib_ipc_call(interface_handle_copy, MY_INTERFACE_COMMAND, &in, &out));
  ASSERT_ARE_EQUAL(int, AZ_OK, out);

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle_copy));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

/* If the provided descriptor already exist, the az_ulib_ipc_publish shall return
 * AZ_ERROR_ULIB_ELEMENT_DUPLICATE. */
TEST_FUNCTION(az_ulib_ipc_publish_with_descriptor_with_same_name_and_version_failed)
{
  /// arrange
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_init(&g_ipc));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&MY_INTERFACE_1_V123, NULL));
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_publish(&MY_INTERFACE_1_V123, NULL);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ULIB_ELEMENT_DUPLICATE, result);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

/* If there is no more memory to store a new descriptor, the az_ulib_ipc_publish shall return
 * AZ_ERROR_NOT_ENOUGH_SPACE. */
TEST_FUNCTION(az_ulib_ipc_publish_out_of_memory_failed)
{
  /// arrange
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_init(&g_ipc));
  az_ulib_interface_descriptor descriptors[AZ_ULIB_CONFIG_MAX_IPC_INTERFACE];
  az_ulib_capability_descriptor capabilities[1]
      = AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", NULL, NULL);
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++)
  {
    descriptors[i].name = "my_interface";
    descriptors[i].version = i + 1;
    descriptors[i].size = 1;
    descriptors[i].capability_list = capabilities;
    ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&descriptors[i], NULL));
  }
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_publish(&MY_INTERFACE_1_V123, NULL);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_NOT_ENOUGH_SPACE, result);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE; i++)
  {
    ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&descriptors[i], AZ_ULIB_NO_WAIT));
  }
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

/* The az_ulib_ipc_unpublish shall remove a descriptor for the IPC. The az_ulib_ipc_unpublish shall
 * be thread safe. */
/* The az_ulib_ipc_unpublish shall wait as long as the caller wants.*/
TEST_FUNCTION(az_ulib_ipc_unpublish_succeed)
{
  /// arrange
  init_ipc_and_publish_interfaces();

  /// act
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_WAIT_FOREVER));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, 10000));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));

  /// assert
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 4, g_count_acquire);
  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ERROR_ITEM_NOT_FOUND,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(
      int,
      AZ_ERROR_ITEM_NOT_FOUND,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V2.name,
          MY_INTERFACE_1_V2.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(
      int,
      AZ_ERROR_ITEM_NOT_FOUND,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_2_V123.name,
          MY_INTERFACE_2_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(
      int,
      AZ_ERROR_ITEM_NOT_FOUND,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_3_V123.name,
          MY_INTERFACE_3_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

TEST_FUNCTION(az_ulib_ipc_unpublish_random_order_succeed)
{
  /// arrange
  init_ipc_and_publish_interfaces();

  /// act
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));

  /// assert
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 4, g_count_acquire);
  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_ERROR_ITEM_NOT_FOUND,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(
      int,
      AZ_ERROR_ITEM_NOT_FOUND,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V2.name,
          MY_INTERFACE_1_V2.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(
      int,
      AZ_ERROR_ITEM_NOT_FOUND,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_2_V123.name,
          MY_INTERFACE_2_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(
      int,
      AZ_ERROR_ITEM_NOT_FOUND,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_3_V123.name,
          MY_INTERFACE_3_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

/* The az_ulib_ipc_unpublish shall release the descriptor position to be used by another descriptor.
 */
TEST_FUNCTION(az_ulib_ipc_unpublish_release_resource_succeed)
{
  /// arrange
  init_ipc_and_publish_interfaces();

  /// act
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_WAIT_FOREVER));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));

  /// assert
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 2, g_count_acquire);

  az_ulib_interface_descriptor descriptors[AZ_ULIB_CONFIG_MAX_IPC_INTERFACE - 2];
  az_ulib_capability_descriptor capabilities[1]
      = AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", NULL, NULL);
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE - 2; i++)
  {
    descriptors[i].name = "my_interface";
    descriptors[i].version = i + 1;
    descriptors[i].size = 1;
    descriptors[i].capability_list = capabilities;
    ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_publish(&descriptors[i], NULL));
  }
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE - 2; i++)
  {
    ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&descriptors[i], AZ_ULIB_NO_WAIT));
  }

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

/* If the provided descriptor was not published, the az_ulib_ipc_unpublish shall return
 * AZ_ERROR_ITEM_NOT_FOUND. */
TEST_FUNCTION(az_ulib_ipc_unpublish_with_unknown_descriptor_failed)
{
  /// arrange
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_init(&g_ipc));

  /// act
  az_result result = az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ITEM_NOT_FOUND, result);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

/* If one of the command in the interface is running and the wait policy is AZ_ULIB_NO_WAIT, the
 * az_ulib_ipc_unpublish shall return AZ_ERROR_ULIB_BUSY. */
TEST_FUNCTION(az_ulib_ipc_unpublish_with_command_running_failed)
{
  /// arrange
  init_ipc_and_publish_interfaces();

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_UNPUBLISH;
  in.descriptor = &MY_INTERFACE_1_V123;
  in.wait_policy_ms = AZ_ULIB_NO_WAIT;
  az_result out = AZ_ULIB_PENDING;

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  g_count_acquire = 0;

  /// act
  // call unpublish inside of the command.
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ULIB_BUSY, out);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));
  unpublish_interfaces_and_deinit_ipc();
}

/* If one of the command in the interface is running and the wait policy is not big enough, the
 * az_ulib_ipc_unpublish shall return AZ_ERROR_ULIB_BUSY. */
TEST_FUNCTION(az_ulib_ipc_unpublish_with_command_running_with_small_timeout_failed)
{
  /// arrange
  init_ipc_and_publish_interfaces();

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_UNPUBLISH;
  in.descriptor = &MY_INTERFACE_1_V123;
  in.wait_policy_ms = 10000;
  az_result out = AZ_ULIB_PENDING;

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  g_count_acquire = 0;

  /// act
  // call unpublish inside of the command.
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ULIB_BUSY, out);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);
  ASSERT_ARE_EQUAL(int, 8, g_count_sleep);

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));
  unpublish_interfaces_and_deinit_ipc();
}

/* If there are valid instances of the interface, the az_ulib_ipc_unpublish shall return
 * AZ_OK. */
TEST_FUNCTION(az_ulib_ipc_unpublish_with_valid_interface_instance_succeed)
{
  /// arrange
  init_ipc_and_publish_interfaces();

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

/* If one of the command in the interface is running, the wait policy is different than
 * AZ_ULIB_NO_WAIT and the call ends before the timeout, the az_ulib_ipc_unpublish shall return
 * AZ_ULIB_SUCCEESS. */
// TODO: implement the test when the code is ready.

/* If one of the command in the interface is running, the wait policy is different than
 * AZ_ULIB_NO_WAIT and the call ends after the timeout, the az_ulib_ipc_unpublish shall return
 * AZ_ERROR_ULIB_BUSY. */
// TODO: implement the test when the code is ready.

/* The az_ulib_ipc_try_get_interface shall return the handle for the interface. */
/* The az_ulib_ipc_try_get_interface shall return AZ_OK. */
TEST_FUNCTION(az_ulib_ipc_try_get_interface_version_equals_succeed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123.name,
      MY_INTERFACE_1_V123.version,
      AZ_ULIB_VERSION_EQUALS_TO,
      &interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_version_any_succeed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123.name, 0, AZ_ULIB_VERSION_ANY, &interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_version_greater_than_succeed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  az_ulib_ipc_interface_handle greater_interface_handle;
  init_ipc_and_publish_interfaces();
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V2.name,
          MY_INTERFACE_1_V2.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V2.name,
      MY_INTERFACE_1_V2.version,
      AZ_ULIB_VERSION_GREATER_THAN,
      &greater_interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_NOT_EQUAL(void_ptr, interface_handle, greater_interface_handle);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(greater_interface_handle));
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_version_lower_than_succeed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  az_ulib_ipc_interface_handle lower_interface_handle;
  init_ipc_and_publish_interfaces();
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123.name,
      MY_INTERFACE_1_V123.version,
      AZ_ULIB_VERSION_LOWER_THAN,
      &lower_interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_NOT_EQUAL(void_ptr, interface_handle, lower_interface_handle);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(lower_interface_handle));
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_version_lower_or_equal_succeed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  az_ulib_ipc_interface_handle lower_interface_handle;
  init_ipc_and_publish_interfaces();
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V2.name,
          MY_INTERFACE_1_V2.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V2.name,
      MY_INTERFACE_1_V2.version,
      AZ_ULIB_VERSION_LOWER_THAN | AZ_ULIB_VERSION_EQUALS_TO,
      &lower_interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_EQUAL(void_ptr, interface_handle, lower_interface_handle);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(lower_interface_handle));
  unpublish_interfaces_and_deinit_ipc();
}

/* If the IPC reach the maximum number of allowed instances for a single interface, the
 * az_ulib_ipc_try_get_interface shall return AZ_ERROR_NOT_ENOUGH_SPACE. */
TEST_FUNCTION(az_ulib_ipc_try_get_interface_with_max_interface_instances_failed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle[AZ_ULIB_CONFIG_MAX_IPC_INSTANCES];
  az_ulib_ipc_interface_handle interface_handle_plus_one;

  init_ipc_and_publish_interfaces();

  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INSTANCES; i++)
  {
    ASSERT_ARE_EQUAL(
        int,
        AZ_OK,
        az_ulib_ipc_try_get_interface(
            MY_INTERFACE_1_V123.name,
            MY_INTERFACE_1_V123.version,
            AZ_ULIB_VERSION_EQUALS_TO,
            &interface_handle[i]));
  }
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123.name,
      MY_INTERFACE_1_V123.version,
      AZ_ULIB_VERSION_EQUALS_TO,
      &interface_handle_plus_one);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_NOT_ENOUGH_SPACE, result);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INSTANCES; i++)
  {
    ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle[i]));
  }
  unpublish_interfaces_and_deinit_ipc();
}

/* If the provided interface name does not exist, the az_ulib_ipc_try_get_interface shall return
 * AZ_ERROR_ITEM_NOT_FOUND. */
TEST_FUNCTION(az_ulib_ipc_try_get_interface_with_unknown_name_failed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      "unknown_name", MY_INTERFACE_1_V123.version, AZ_ULIB_VERSION_EQUALS_TO, &interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ITEM_NOT_FOUND, result);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_with_unknown_version_failed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123.name, 9999, AZ_ULIB_VERSION_EQUALS_TO, &interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ITEM_NOT_FOUND, result);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_without_version_greater_than_failed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123.name,
      MY_INTERFACE_1_V123.version,
      AZ_ULIB_VERSION_GREATER_THAN,
      &interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ITEM_NOT_FOUND, result);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

TEST_FUNCTION(az_ulib_ipc_try_get_interface_without_version_lower_than_failed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V2.name,
      MY_INTERFACE_1_V2.version,
      AZ_ULIB_VERSION_LOWER_THAN,
      &interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ITEM_NOT_FOUND, result);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* The az_ulib_ipc_get_interface shall return the handle for the interface. */
/* The az_ulib_ipc_get_interface shall return AZ_OK. */
TEST_FUNCTION(az_ulib_ipc_get_interface_succeed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  az_ulib_ipc_interface_handle new_interface_handle;
  init_ipc_and_publish_interfaces();
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_get_interface(interface_handle, &new_interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(new_interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));
  unpublish_interfaces_and_deinit_ipc();
}

/* If the IPC reach the maximum number of allowed instances for a single interface, the
 * az_ulib_ipc_get_interface shall return AZ_ERROR_NOT_ENOUGH_SPACE. */
TEST_FUNCTION(az_ulib_ipc_get_interface_with_max_interface_instances_failed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle[AZ_ULIB_CONFIG_MAX_IPC_INSTANCES];
  az_ulib_ipc_interface_handle interface_handle_plus_one;

  init_ipc_and_publish_interfaces();

  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INSTANCES; i++)
  {
    ASSERT_ARE_EQUAL(
        int,
        AZ_OK,
        az_ulib_ipc_try_get_interface(
            MY_INTERFACE_1_V123.name,
            MY_INTERFACE_1_V123.version,
            AZ_ULIB_VERSION_EQUALS_TO,
            &interface_handle[i]));
  }
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_get_interface(interface_handle[0], &interface_handle_plus_one);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_NOT_ENOUGH_SPACE, result);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INSTANCES; i++)
  {
    ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle[i]));
  }
  unpublish_interfaces_and_deinit_ipc();
}

/* If the provided interface name does not exist, the az_ulib_ipc_get_interface shall return
 * AZ_ERROR_ITEM_NOT_FOUND. */
TEST_FUNCTION(az_ulib_ipc_get_interface_with_unpublished_interface_failed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  az_ulib_ipc_interface_handle new_interface_handle;
  init_ipc_and_publish_interfaces();
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_get_interface(interface_handle, &new_interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ITEM_NOT_FOUND, result);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

/* The az_ulib_ipc_release_interface shall release the instance of the interface. */
/* The az_ulib_ipc_release_interface shall return AZ_OK. */
TEST_FUNCTION(az_ulib_ipc_release_interface_succeed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_release_interface(interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the interface is already released, the az_ulib_ipc_release_interface shall return
 * AZ_ERROR_ULIB_PRECONDITION. */
TEST_FUNCTION(az_ulib_ipc_release_interface_double_release_failed)
{
  /// arrange
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_release_interface(interface_handle);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ULIB_PRECONDITION, result);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If one of the command in the interface is running, the az_ulib_ipc_release_interface shall return
 * AZ_OK. */
TEST_FUNCTION(az_ulib_ipc_release_interface_with_command_running_failed)
{
  /// arrange
  init_ipc_and_publish_interfaces();

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  g_count_acquire = 0;

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_RELEASE_INTERFACE;
  in.handle = interface_handle;
  az_result out = AZ_ULIB_PENDING;

  /// act
  // call release inside of the command.
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_EQUAL(int, AZ_OK, out);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(int, 1, g_count_acquire);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* The az_ulib_ipc_call shall call the command published by the interface. */
/* The az_ulib_ipc_call shall return AZ_OK. */
TEST_FUNCTION(az_ulib_ipc_call_calls_the_command_succeed)
{
  /// arrange
  init_ipc_and_publish_interfaces();

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_JUST_RETURN;
  in.return_result = AZ_OK;
  az_result out = AZ_ULIB_PENDING;

  umock_c_reset_all_calls();

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_OK, result);
  ASSERT_ARE_EQUAL(int, AZ_OK, out);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));
  unpublish_interfaces_and_deinit_ipc();
}

/* If the interface was unpublished, the az_ulib_ipc_call shall return AZ_ERROR_ITEM_NOT_FOUND
 * and do not call the command. */
TEST_FUNCTION(az_ulib_ipc_call_unpublished_interface_failed)
{
  /// arrange
  init_ipc_and_publish_interfaces();

  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_JUST_RETURN;
  in.return_result = AZ_OK;
  az_result out = AZ_ULIB_PENDING;

  umock_c_reset_all_calls();

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ITEM_NOT_FOUND, result);
  ASSERT_ARE_EQUAL(int, AZ_ULIB_PENDING, out);
  ASSERT_ARE_EQUAL(int, 0, g_lock_diff);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

/* The az_ulib_ipc_deinit shall release all resources associate with ipc. */
/* The az_ulib_ipc_deinit shall return AZ_OK. */
TEST_FUNCTION(az_ulib_ipc_deinit_succeed)
{
  /// arrange
  g_lock = NULL;
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_init(&g_ipc));
  ASSERT_IS_NOT_NULL(g_lock);

  /// act
  az_result result = az_ulib_ipc_deinit();

  /// assert
  ASSERT_IS_NULL(g_lock);
  ASSERT_ARE_EQUAL(int, AZ_OK, result);

  /// cleanup
}

/* If there is published interface, the az_ulib_ipc_deinit shall return AZ_ERROR_ULIB_BUSY. */
TEST_FUNCTION(az_ulib_ipc_deinit_with_published_interface_failed)
{
  /// arrange
  init_ipc_and_publish_interfaces();
  umock_c_reset_all_calls();

  /// act
  az_result result = az_ulib_ipc_deinit();

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ULIB_BUSY, result);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If there is instances of the interface, the az_ulib_ipc_deinit shall return AZ_ERROR_ULIB_BUSY.
 */
TEST_FUNCTION(az_ulib_ipc_deinit_with_instace_failed)
{
  /// arrange
  init_ipc_and_publish_interfaces();
  az_ulib_ipc_interface_handle interface_handle;
  ASSERT_ARE_EQUAL(
      int,
      AZ_OK,
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123.name,
          MY_INTERFACE_1_V123.version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT));
  umock_c_reset_all_calls();

  /// act
  az_result result = az_ulib_ipc_deinit();

  /// assert
  ASSERT_ARE_EQUAL(int, AZ_ERROR_ULIB_BUSY, result);
  ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

  /// cleanup
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_release_interface(interface_handle));
  ASSERT_ARE_EQUAL(int, AZ_OK, az_ulib_ipc_deinit());
}

END_TEST_SUITE(az_ulib_ipc_ut)
