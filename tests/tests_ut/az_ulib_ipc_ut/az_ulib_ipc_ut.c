// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_ipc_ut.h"
#include "az_ulib_pal_os_api.h"
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

static uint32_t my_property = 0;

static az_result get_my_property(az_ulib_model_out model_out)
{
  uint32_t* new_val = (uint32_t*)model_out;

  *new_val = my_property;

  return AZ_OK;
}

static az_result set_my_property(az_ulib_model_in model_in)
{
  const uint32_t* const new_val = (const uint32_t* const)model_in;

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

static az_result my_command(az_ulib_model_in model_in, az_ulib_model_out model_out)
{
  const my_command_model_in* const in = (const my_command_model_in* const)model_in;
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
    az_ulib_model_in model_in,
    az_ulib_model_out model_out,
    const az_ulib_capability_token capability_token,
    const az_ulib_capability_cancellation_callback cancel)
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

static const az_ulib_capability_descriptor MY_INTERFACE_1_V123_CAPABILITIES[5] = {
  AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property, NULL, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
  AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
      "my_command_async",
      my_command_async,
      NULL,
      my_command_cancel)
};
static const az_ulib_interface_descriptor MY_INTERFACE_1_V123
    = AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE_1", 123, 5, MY_INTERFACE_1_V123_CAPABILITIES);

static const az_ulib_capability_descriptor MY_INTERFACE_1_V2_CAPABILITIES[5] = {
  AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property, NULL, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
  AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
      "my_command_async",
      my_command_async,
      NULL,
      my_command_cancel)
};
static const az_ulib_interface_descriptor MY_INTERFACE_1_V2
    = AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE_1", 2, 5, MY_INTERFACE_1_V2_CAPABILITIES);

static const az_ulib_capability_descriptor MY_INTERFACE_2_V123_CAPABILITIES[5] = {
  AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property, NULL, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
  AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
      "my_command_async",
      my_command_async,
      NULL,
      my_command_cancel)
};
static const az_ulib_interface_descriptor MY_INTERFACE_2_V123
    = AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE_2", 123, 5, MY_INTERFACE_2_V123_CAPABILITIES);

static const az_ulib_capability_descriptor MY_INTERFACE_3_V123_CAPABILITIES[5] = {
  AZ_ULIB_DESCRIPTOR_ADD_PROPERTY("my_property", get_my_property, set_my_property, NULL, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry"),
  AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY("my_telemetry2"),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND("my_command", my_command, NULL),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
      "my_command_async",
      my_command_async,
      NULL,
      my_command_cancel)
};
static const az_ulib_interface_descriptor MY_INTERFACE_3_V123
    = AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE_3", 123, 5, MY_INTERFACE_3_V123_CAPABILITIES);

static const az_ulib_interface_descriptor MY_DESCRIPTOR_LIST[AZ_ULIB_CONFIG_MAX_IPC_INTERFACE]
    = { AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE", 1000, 5, MY_INTERFACE_3_V123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE", 1001, 5, MY_INTERFACE_3_V123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE", 1002, 5, MY_INTERFACE_3_V123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE", 1003, 5, MY_INTERFACE_3_V123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE", 1004, 5, MY_INTERFACE_3_V123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE", 1005, 5, MY_INTERFACE_3_V123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE", 1006, 5, MY_INTERFACE_3_V123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE", 1007, 5, MY_INTERFACE_3_V123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE", 1008, 5, MY_INTERFACE_3_V123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE("MY_INTERFACE", 1009, 5, MY_INTERFACE_3_V123_CAPABILITIES) };

static az_ulib_ipc g_ipc;

static void init_ipc_and_publish_interfaces(void)
{
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_1_V123, NULL), AZ_OK);
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_1_V2, NULL), AZ_OK);
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_2_V123, NULL), AZ_OK);
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_3_V123, NULL), AZ_OK);

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);

  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V2._name,
          MY_INTERFACE_1_V2._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);

  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_2_V123._name,
          MY_INTERFACE_2_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);

  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_3_V123._name,
          MY_INTERFACE_3_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  g_count_acquire = 0;
}

static void unpublish_interfaces_and_deinit_ipc(void)
{
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

#ifndef AZ_NO_PRECONDITION_CHECKING
AZ_ULIB_ENABLE_PRECONDITION_CHECK_TESTS()
#endif // AZ_NO_PRECONDITION_CHECKING

static int setup(void** state)
{
  (void)state;

  g_lock = NULL;
  g_lock_diff = 0;
  g_count_acquire = 0;
  g_count_sleep = 0;

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
  az_ulib_ipc ipc1;
  az_ulib_ipc ipc2;
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
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_publish(&MY_INTERFACE_1_V123, NULL));

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
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_publish(NULL, NULL));

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
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT));

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

/* If the provided interface name is NULL, the az_ulib_ipc_try_get_interface shall fail with
 * precondition. */
static void az_ulib_ipc_try_get_interface_with_null_name_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_try_get_interface(
      AZ_SPAN_EMPTY, MY_INTERFACE_1_V123._version, AZ_ULIB_VERSION_EQUALS_TO, &interface_handle));

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
      MY_INTERFACE_1_V123._name, MY_INTERFACE_1_V123._version, AZ_ULIB_VERSION_EQUALS_TO, NULL));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the ipc was not initialized, the az_ulib_ipc_try_get_interface shall fail with precondition.
 */
static void az_ulib_ipc_try_get_interface_with_ipc_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123._name,
      MY_INTERFACE_1_V123._version,
      AZ_ULIB_VERSION_EQUALS_TO,
      &interface_handle));

  /// cleanup
}

/* If the provided interface handle is NULL, the az_ulib_ipc_get_interface shall fail with
 * precondition. */
static void az_ulib_ipc_get_interface_with_null_original_interface_handle_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle new_interface_handle;
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_get_interface(NULL, &new_interface_handle));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the provided new interface handle is NULL, the az_ulib_ipc_get_interface shall fail with
 * precondition. */
static void az_ulib_ipc_get_interface_with_null_interface_handle_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_get_interface(interface_handle, NULL));

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If the ipc was not initialized, the az_ulib_ipc_get_interface shall fail with precondition. */
static void az_ulib_ipc_get_interface_with_ipc_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle = (az_ulib_ipc_interface_handle)0x1234;
  az_ulib_ipc_interface_handle new_interface_handle;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      az_ulib_ipc_get_interface(interface_handle, &new_interface_handle));

  /// cleanup
}

/* If the provided handle is NULL, the az_ulib_ipc_release_interface shall fail with precondition.
 */
static void az_ulib_ipc_release_interface_with_null_interface_handle_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(az_ulib_ipc_release_interface(NULL));

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If the ipc is not initialized, the az_ulib_ipc_release_interface shall fail with precondition.
/*/
static void az_ulib_ipc_release_interface_with_ipc_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;

  /// act
  /// assert
  AZ_ULIB_ASSERT_PRECONDITION_CHECKED(
      az_ulib_ipc_release_interface((az_ulib_ipc_interface_handle)0x1234));

  /// cleanup
}

/* If the IPC is not initialized, the az_ulib_ipc_call shall fail with precondition. */
static void az_ulib_ipc_call_with_ipc_not_initialized_failed(void** state)
{
  /// arrange
  (void)state;
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

/* If the interface handle is NULL, the az_ulib_ipc_call shall fail with precondition. */
static void az_ulib_ipc_call_with_null_interface_handle_failed(void** state)
{
  /// arrange
  (void)state;
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
static void az_ulib_ipc_init_succeed(void** state)
{
  /// arrange
  (void)state;
  g_lock = NULL;

  /// act
  az_result result = az_ulib_ipc_init(&g_ipc);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_non_null(g_lock);
  assert_int_equal(g_lock_diff, 0);
  /// cleanup
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* The az_ulib_ipc_publish shall store the descriptor published in the IPC. The
 az_ulib_ipc_publish
 * shall be thread safe. */
static void az_ulib_ipc_publish_succeed(void** state)
{
  /// arrange
  (void)state;
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);

  /// act
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_1_V123, NULL), AZ_OK);
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_1_V2, NULL), AZ_OK);
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_2_V123, NULL), AZ_OK);
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_3_V123, NULL), AZ_OK);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 5);
  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);

  /// cleanup
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* The az_ulib_ipc_publish shall return a valid handle for the published interface. */
static void az_ulib_ipc_publish_return_handle_succeed(void** state)
{
  /// arrange
  (void)state;
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  az_ulib_ipc_interface_handle interface_handle[4];

  /// act
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_1_V123, &(interface_handle[0])), AZ_OK);
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_1_V2, &(interface_handle[1])), AZ_OK);
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_2_V123, &(interface_handle[2])), AZ_OK);
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_3_V123, &(interface_handle[3])), AZ_OK);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 5);
  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_JUST_RETURN;
  in.return_result = AZ_OK;
  az_result out = AZ_ULIB_PENDING;
  assert_int_equal(az_ulib_ipc_call(interface_handle[0], MY_INTERFACE_COMMAND, &in, &out), AZ_OK);
  assert_int_equal(out, AZ_OK);
  az_ulib_ipc_interface_handle interface_handle_copy;
  assert_int_equal(az_ulib_ipc_get_interface(interface_handle[0], &interface_handle_copy), AZ_OK);
  out = AZ_ULIB_PENDING;
  assert_int_equal(az_ulib_ipc_call(interface_handle_copy, MY_INTERFACE_COMMAND, &in, &out), AZ_OK);
  assert_int_equal(out, AZ_OK);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle_copy), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* If the provided descriptor already exist, the az_ulib_ipc_publish shall return
 * AZ_ERROR_ULIB_ELEMENT_DUPLICATE. */
static void az_ulib_ipc_publish_with_descriptor_with_same_name_and_version_failed(void** state)
{
  /// arrange
  (void)state;
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  assert_int_equal(az_ulib_ipc_publish(&MY_INTERFACE_1_V123, NULL), AZ_OK);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_publish(&MY_INTERFACE_1_V123, NULL);

  /// assert
  assert_int_equal(result, AZ_ERROR_ULIB_ELEMENT_DUPLICATE);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* If there is no more memory to store a new descriptor, the az_ulib_ipc_publish shall return
 * AZ_ERROR_NOT_ENOUGH_SPACE. */
static void az_ulib_ipc_publish_out_of_memory_failed(void** state)
{
  /// arrange
  (void)state;
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE - 1; i++)
  {
    assert_int_equal(az_ulib_ipc_publish(&MY_DESCRIPTOR_LIST[i], NULL), AZ_OK);
  }
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_publish(&MY_INTERFACE_1_V123, NULL);

  /// assert
  assert_int_equal(result, AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE - 1; i++)
  {
    assert_int_equal(az_ulib_ipc_unpublish(&MY_DESCRIPTOR_LIST[i], AZ_ULIB_NO_WAIT), AZ_OK);
  }
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* The az_ulib_ipc_unpublish shall remove a descriptor for the IPC. The az_ulib_ipc_unpublish
 shall
 * be thread safe. */
/* The az_ulib_ipc_unpublish shall wait as long as the caller wants.*/
static void az_ulib_ipc_unpublish_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  /// act
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_WAIT_FOREVER), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, 10000), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT), AZ_OK);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 4);
  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V2._name,
          MY_INTERFACE_1_V2._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_2_V123._name,
          MY_INTERFACE_2_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_3_V123._name,
          MY_INTERFACE_3_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
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
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT), AZ_OK);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 4);
  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V2._name,
          MY_INTERFACE_1_V2._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_2_V123._name,
          MY_INTERFACE_2_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_3_V123._name,
          MY_INTERFACE_3_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
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
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_WAIT_FOREVER), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT), AZ_OK);

  /// assert
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 2);

  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE - 3; i++)
  {
    assert_int_equal(az_ulib_ipc_publish(&MY_DESCRIPTOR_LIST[i], NULL), AZ_OK);
  }
  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INTERFACE - 3; i++)
  {
    assert_int_equal(az_ulib_ipc_unpublish(&MY_DESCRIPTOR_LIST[i], AZ_ULIB_NO_WAIT), AZ_OK);
  }

  /// cleanup
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* If the provided descriptor was not published, the az_ulib_ipc_unpublish shall return
 * AZ_ERROR_ITEM_NOT_FOUND. */
static void az_ulib_ipc_unpublish_with_unknown_descriptor_failed(void** state)
{
  /// arrange
  (void)state;
  assert_int_equal(az_ulib_ipc_init(&g_ipc), AZ_OK);

  /// act
  az_result result = az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 2);

  /// cleanup
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* If one of the command in the interface is running and the wait policy is AZ_ULIB_NO_WAIT, the
 * az_ulib_ipc_unpublish shall return AZ_ERROR_ULIB_BUSY. */
static void az_ulib_ipc_unpublish_with_command_running_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_UNPUBLISH;
  in.descriptor = &MY_INTERFACE_1_V123;
  in.wait_policy_ms = AZ_ULIB_NO_WAIT;
  az_result out = AZ_ULIB_PENDING;

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  g_count_acquire = 0;

  /// act
  // call unpublish inside of the command.
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(out, AZ_ERROR_ULIB_BUSY);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If one of the command in the interface is running and the wait policy is not big enough, the
 * az_ulib_ipc_unpublish shall return AZ_ERROR_ULIB_BUSY. */
static void az_ulib_ipc_unpublish_with_command_running_with_small_timeout_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_UNPUBLISH;
  in.descriptor = &MY_INTERFACE_1_V123;
  in.wait_policy_ms = 10000;
  az_result out = AZ_ULIB_PENDING;

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  g_count_acquire = 0;

  /// act
  // call unpublish inside of the command.
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(out, AZ_ERROR_ULIB_BUSY);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);
  assert_int_equal(g_count_sleep, 8);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If there are valid instances of the interface, the az_ulib_ipc_unpublish shall return
 * AZ_OK. */
static void az_ulib_ipc_unpublish_with_valid_interface_instance_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
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
static void az_ulib_ipc_try_get_interface_version_equals_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123._name,
      MY_INTERFACE_1_V123._version,
      AZ_ULIB_VERSION_EQUALS_TO,
      &interface_handle);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_try_get_interface_version_any_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123._name, 0, AZ_ULIB_VERSION_ANY, &interface_handle);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_try_get_interface_version_greater_than_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle;
  az_ulib_ipc_interface_handle greater_interface_handle;
  init_ipc_and_publish_interfaces();
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V2._name,
          MY_INTERFACE_1_V2._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V2._name,
      MY_INTERFACE_1_V2._version,
      AZ_ULIB_VERSION_GREATER_THAN,
      &greater_interface_handle);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_ptr_not_equal(interface_handle, greater_interface_handle);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(greater_interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_try_get_interface_version_lower_than_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle;
  az_ulib_ipc_interface_handle lower_interface_handle;
  init_ipc_and_publish_interfaces();
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123._name,
      MY_INTERFACE_1_V123._version,
      AZ_ULIB_VERSION_LOWER_THAN,
      &lower_interface_handle);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_ptr_not_equal(interface_handle, lower_interface_handle);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(lower_interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_try_get_interface_version_lower_or_equal_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle;
  az_ulib_ipc_interface_handle lower_interface_handle;
  init_ipc_and_publish_interfaces();
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V2._name,
          MY_INTERFACE_1_V2._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V2._name,
      MY_INTERFACE_1_V2._version,
      AZ_ULIB_VERSION_LOWER_THAN | AZ_ULIB_VERSION_EQUALS_TO,
      &lower_interface_handle);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_ptr_equal(interface_handle, lower_interface_handle);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(lower_interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If the IPC reach the maximum number of allowed instances for a single interface, the
 * az_ulib_ipc_try_get_interface shall return AZ_ERROR_NOT_ENOUGH_SPACE. */
static void az_ulib_ipc_try_get_interface_with_max_interface_instances_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle[AZ_ULIB_CONFIG_MAX_IPC_INSTANCES];
  az_ulib_ipc_interface_handle interface_handle_plus_one;

  init_ipc_and_publish_interfaces();

  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INSTANCES; i++)
  {
    assert_int_equal(
        az_ulib_ipc_try_get_interface(
            MY_INTERFACE_1_V123._name,
            MY_INTERFACE_1_V123._version,
            AZ_ULIB_VERSION_EQUALS_TO,
            &interface_handle[i]),
        AZ_OK);
  }
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123._name,
      MY_INTERFACE_1_V123._version,
      AZ_ULIB_VERSION_EQUALS_TO,
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
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      AZ_SPAN_FROM_STR("unknown_name"),
      MY_INTERFACE_1_V123._version,
      AZ_ULIB_VERSION_EQUALS_TO,
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
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123._name, 9999, AZ_ULIB_VERSION_EQUALS_TO, &interface_handle);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_try_get_interface_without_version_greater_than_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V123._name,
      MY_INTERFACE_1_V123._version,
      AZ_ULIB_VERSION_GREATER_THAN,
      &interface_handle);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

static void az_ulib_ipc_try_get_interface_without_version_lower_than_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_try_get_interface(
      MY_INTERFACE_1_V2._name,
      MY_INTERFACE_1_V2._version,
      AZ_ULIB_VERSION_LOWER_THAN,
      &interface_handle);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* The az_ulib_ipc_get_interface shall return the handle for the interface. */
/* The az_ulib_ipc_get_interface shall return AZ_OK. */
static void az_ulib_ipc_get_interface_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle;
  az_ulib_ipc_interface_handle new_interface_handle;
  init_ipc_and_publish_interfaces();
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_get_interface(interface_handle, &new_interface_handle);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(new_interface_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If the IPC reach the maximum number of allowed instances for a single interface, the
 * az_ulib_ipc_get_interface shall return AZ_ERROR_NOT_ENOUGH_SPACE. */
static void az_ulib_ipc_get_interface_with_max_interface_instances_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle[AZ_ULIB_CONFIG_MAX_IPC_INSTANCES];
  az_ulib_ipc_interface_handle interface_handle_plus_one;

  init_ipc_and_publish_interfaces();

  for (int i = 0; i < AZ_ULIB_CONFIG_MAX_IPC_INSTANCES; i++)
  {
    assert_int_equal(
        az_ulib_ipc_try_get_interface(
            MY_INTERFACE_1_V123._name,
            MY_INTERFACE_1_V123._version,
            AZ_ULIB_VERSION_EQUALS_TO,
            &interface_handle[i]),
        AZ_OK);
  }
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_get_interface(interface_handle[0], &interface_handle_plus_one);

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

/* If the provided interface name does not exist, the az_ulib_ipc_get_interface shall return
 * AZ_ERROR_ITEM_NOT_FOUND. */
static void az_ulib_ipc_get_interface_with_unpublished_interface_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle;
  az_ulib_ipc_interface_handle new_interface_handle;
  init_ipc_and_publish_interfaces();
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_get_interface(interface_handle, &new_interface_handle);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* The az_ulib_ipc_release_interface shall release the instance of the interface. */
/* The az_ulib_ipc_release_interface shall return AZ_OK. */
static void az_ulib_ipc_release_interface_succeed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
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

/* If the interface is already released, the az_ulib_ipc_release_interface shall return
 * AZ_ERROR_ULIB_PRECONDITION. */
static void az_ulib_ipc_release_interface_double_release_failed(void** state)
{
  /// arrange
  (void)state;
  az_ulib_ipc_interface_handle interface_handle;
  init_ipc_and_publish_interfaces();
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  g_count_acquire = 0;

  /// act
  az_result result = az_ulib_ipc_release_interface(interface_handle);

  /// assert
  assert_int_equal(result, AZ_ERROR_ULIB_PRECONDITION);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If one of the command in the interface is running, the az_ulib_ipc_release_interface shall
 return
 * AZ_OK. */
static void az_ulib_ipc_release_interface_with_command_running_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  g_count_acquire = 0;

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_RELEASE_INTERFACE;
  in.handle = interface_handle;
  az_result out = AZ_ULIB_PENDING;

  /// act
  // call release inside of the command.
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(out, AZ_OK);
  assert_int_equal(g_lock_diff, 0);
  assert_int_equal(g_count_acquire, 1);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* The az_ulib_ipc_call shall call the command published by the interface. */
/* The az_ulib_ipc_call shall return AZ_OK. */
static void az_ulib_ipc_call_calls_the_command_succeed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_JUST_RETURN;
  in.return_result = AZ_OK;
  az_result out = AZ_ULIB_PENDING;

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_OK);
  assert_int_equal(out, AZ_OK);
  assert_int_equal(g_lock_diff, 0);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  unpublish_interfaces_and_deinit_ipc();
}

/* If the interface was unpublished, the az_ulib_ipc_call shall return AZ_ERROR_ITEM_NOT_FOUND
 * and do not call the command. */
static void az_ulib_ipc_call_unpublished_interface_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();

  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT), AZ_OK);

  my_command_model_in in;
  in.capability = MY_COMMAND_CAPABILITY_JUST_RETURN;
  in.return_result = AZ_OK;
  az_result out = AZ_ULIB_PENDING;

  /// act
  az_result result = az_ulib_ipc_call(interface_handle, MY_INTERFACE_COMMAND, &in, &out);

  /// assert
  assert_int_equal(result, AZ_ERROR_ITEM_NOT_FOUND);
  assert_int_equal(out, AZ_ULIB_PENDING);
  assert_int_equal(g_lock_diff, 0);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

/* The az_ulib_ipc_deinit shall release all resources associate with ipc. */
/* The az_ulib_ipc_deinit shall return AZ_OK. */
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
  init_ipc_and_publish_interfaces();

  /// act
  az_result result = az_ulib_ipc_deinit();

  /// assert
  assert_int_equal(result, AZ_ERROR_ULIB_BUSY);

  /// cleanup
  unpublish_interfaces_and_deinit_ipc();
}

/* If there is instances of the interface, the az_ulib_ipc_deinit shall return AZ_ERROR_ULIB_BUSY.
 */
static void az_ulib_ipc_deinit_with_instace_failed(void** state)
{
  /// arrange
  (void)state;
  init_ipc_and_publish_interfaces();
  az_ulib_ipc_interface_handle interface_handle;
  assert_int_equal(
      az_ulib_ipc_try_get_interface(
          MY_INTERFACE_1_V123._name,
          MY_INTERFACE_1_V123._version,
          AZ_ULIB_VERSION_EQUALS_TO,
          &interface_handle),
      AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, AZ_ULIB_NO_WAIT), AZ_OK);
  assert_int_equal(az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, AZ_ULIB_NO_WAIT), AZ_OK);

  /// act
  az_result result = az_ulib_ipc_deinit();

  /// assert
  assert_int_equal(result, AZ_ERROR_ULIB_BUSY);

  /// cleanup
  assert_int_equal(az_ulib_ipc_release_interface(interface_handle), AZ_OK);
  assert_int_equal(az_ulib_ipc_deinit(), AZ_OK);
}

int az_ulib_ipc_ut()
{
#ifndef AZ_NO_PRECONDITION_CHECKING
  AZ_ULIB_SETUP_PRECONDITION_CHECK_TESTS();
#endif // AZ_NO_PRECONDITION_CHECKING

  const struct CMUnitTest tests[] = {
#ifndef AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test(az_ulib_ipc_init_with_null_handle_failed),
    cmocka_unit_test(az_ulib_ipc_init_double_initialization_failed),
    cmocka_unit_test(az_ulib_ipc_deinit_with_ipc_not_initialized_failed),
    cmocka_unit_test(az_ulib_ipc_publish_with_ipc_not_initialized_failed),
    cmocka_unit_test(az_ulib_ipc_publish_with_null_descriptor_failed),
    cmocka_unit_test(az_ulib_ipc_unpublish_with_ipc_not_initialized_failed),
    cmocka_unit_test(az_ulib_ipc_unpublish_with_null_descriptor_failed),
    cmocka_unit_test(az_ulib_ipc_try_get_interface_with_null_name_failed),
    cmocka_unit_test(az_ulib_ipc_try_get_interface_with_null_handle_failed),
    cmocka_unit_test(az_ulib_ipc_try_get_interface_with_ipc_not_initialized_failed),
    cmocka_unit_test(az_ulib_ipc_get_interface_with_null_original_interface_handle_failed),
    cmocka_unit_test(az_ulib_ipc_get_interface_with_null_interface_handle_failed),
    cmocka_unit_test(az_ulib_ipc_get_interface_with_ipc_not_initialized_failed),
    cmocka_unit_test(az_ulib_ipc_release_interface_with_null_interface_handle_failed),
    cmocka_unit_test(az_ulib_ipc_release_interface_with_ipc_not_initialized_failed),
    cmocka_unit_test(az_ulib_ipc_call_with_ipc_not_initialized_failed),
    cmocka_unit_test(az_ulib_ipc_call_with_null_interface_handle_failed),
#endif // AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test_setup(az_ulib_ipc_init_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_publish_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_publish_return_handle_succeed, setup),
    cmocka_unit_test_setup(
        az_ulib_ipc_publish_with_descriptor_with_same_name_and_version_failed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_publish_out_of_memory_failed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_unpublish_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_unpublish_random_order_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_unpublish_release_resource_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_unpublish_with_unknown_descriptor_failed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_unpublish_with_command_running_failed, setup),
    cmocka_unit_test_setup(
        az_ulib_ipc_unpublish_with_command_running_with_small_timeout_failed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_unpublish_with_valid_interface_instance_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_try_get_interface_version_equals_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_try_get_interface_version_any_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_try_get_interface_version_greater_than_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_try_get_interface_version_lower_than_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_try_get_interface_version_lower_or_equal_succeed, setup),
    cmocka_unit_test_setup(
        az_ulib_ipc_try_get_interface_with_max_interface_instances_failed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_try_get_interface_with_unknown_name_failed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_try_get_interface_with_unknown_version_failed, setup),
    cmocka_unit_test_setup(
        az_ulib_ipc_try_get_interface_without_version_greater_than_failed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_try_get_interface_without_version_lower_than_failed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_get_interface_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_get_interface_with_max_interface_instances_failed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_get_interface_with_unpublished_interface_failed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_release_interface_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_release_interface_double_release_failed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_release_interface_with_command_running_failed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_call_calls_the_command_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_call_unpublished_interface_failed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_deinit_succeed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_deinit_with_published_interface_failed, setup),
    cmocka_unit_test_setup(az_ulib_ipc_deinit_with_instace_failed, setup),
  };

  return cmocka_run_group_tests_name("az_ulib_ipc_ut", tests, NULL, NULL);
}
