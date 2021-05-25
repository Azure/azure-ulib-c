// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_test_my_interface.h"
#include "az_ulib_ipc_api.h"

static my_property_model my_property = 0;

static az_result get_my_property(az_ulib_model_out model_out)
{
  my_property_model* new_val = (my_property_model*)model_out;

  *new_val = my_property;

  return AZ_OK;
}

static az_result get_my_property_span_wrapper(az_span* model_out_span)
{
  AZ_ULIB_TRY
  {
    // Call get.
    my_property_model val;
    AZ_ULIB_THROW_IF_AZ_ERROR(get_my_property((az_ulib_model_out)&val));

    // Marshalling val to JSON in model_out_span.
    az_json_writer jw;
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_init(&jw, *model_out_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_begin_object(&jw));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_property_name(
        &jw, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_PROPERTY_VAL_NAME)));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_int32(&jw, val));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_end_object(&jw));
    *model_out_span = az_json_writer_get_bytes_used_in_destination(&jw);
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
}

static az_result set_my_property(az_ulib_model_in model_in)
{
  const my_property_model* const new_val = (const my_property_model* const)model_in;

  my_property = *new_val;

  return AZ_OK;
}

static az_result set_my_property_span_wrapper(az_span model_in_span)
{
  AZ_ULIB_TRY
  {
    // Unmarshalling JSON in model_in_span to val.
    az_json_reader jr;
    my_property_model val = 0;
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_init(&jr, model_in_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    while (jr.token.kind != AZ_JSON_TOKEN_END_OBJECT)
    {
      if (az_json_token_is_text_equal(
              &jr.token, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_PROPERTY_VAL_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_token_get_int32(&jr.token, &val));
      }
      AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    }
    AZ_ULIB_THROW_IF_AZ_ERROR(AZ_ULIB_TRY_RESULT);

    // Call get.
    AZ_ULIB_THROW_IF_AZ_ERROR(set_my_property((az_ulib_model_in)&val));
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
}

volatile long g_is_running;
volatile long g_lock_thread;
volatile uint32_t g_sum_sleep;

static az_result my_command(az_ulib_model_in model_in, az_ulib_model_out model_out)
{
  const my_command_model_in* const in = (const my_command_model_in* const)model_in;
  my_command_model_out* result = (my_command_model_out*)model_out;
  my_command_model_in in_2;
  uint64_t sum = 0;

  (void)AZ_ULIB_PORT_ATOMIC_INC_W(&g_is_running);
  switch (in->capability)
  {
    case MY_COMMAND_CAPABILITY_JUST_RETURN:
      *result = in->return_result;
      break;
    case MY_COMMAND_CAPABILITY_SUM:
      while (g_lock_thread != 0)
      {
        az_pal_os_sleep(10);
      };
      for (uint32_t i = 0; i < in->max_sum; i++)
      {
        if (g_sum_sleep != 0)
        {
          az_pal_os_sleep(g_sum_sleep);
        }
        sum += i;
      }
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
  (void)AZ_ULIB_PORT_ATOMIC_DEC_W(&g_is_running);

  return AZ_OK;
}

static az_result my_command_span_wrapper(az_span model_in_span, az_span* model_out_span)
{
  AZ_ULIB_TRY
  {
    // Unmarshalling JSON in model_in_span to val.
    az_json_reader jr;
    my_command_model_in model_in = { 0 };
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_init(&jr, model_in_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    while (jr.token.kind != AZ_JSON_TOKEN_END_OBJECT)
    {
      if (az_json_token_is_text_equal(
              &jr.token, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_COMMAND_CAPABILITY_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        int32_t val;
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_token_get_int32(&jr.token, &val));
        model_in.capability = (uint8_t)val;
      }
      else if (az_json_token_is_text_equal(
                   &jr.token, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_COMMAND_MAX_SUM_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_token_get_uint32(&jr.token, &model_in.max_sum));
      }
      else if (az_json_token_is_text_equal(
                   &jr.token, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_COMMAND_DESCRIPTOR_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        uint64_t val;
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_token_get_uint64(&jr.token, &val));
        model_in.descriptor = (az_ulib_interface_descriptor*)val;
      }
      else if (az_json_token_is_text_equal(
                   &jr.token, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_COMMAND_WAIT_POLICY_MS_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_token_get_uint32(&jr.token, &model_in.wait_policy_ms));
      }
      else if (az_json_token_is_text_equal(
                   &jr.token, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_COMMAND_HANDLE_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        uint64_t val;
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_token_get_uint64(&jr.token, &val));
        model_in.handle = (az_ulib_ipc_interface_handle)val;
      }
      else if (az_json_token_is_text_equal(
                   &jr.token, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_COMMAND_COMMAND_INDEX_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        uint32_t val;
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_token_get_uint32(&jr.token, &val));
        model_in.command_index = (az_ulib_capability_index)val;
      }
      else if (az_json_token_is_text_equal(
                   &jr.token, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_COMMAND_RETURN_RESULT_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        uint32_t val;
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_token_get_uint32(&jr.token, &val));
        model_in.return_result = (az_result)val;
      }
      AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    }
    AZ_ULIB_THROW_IF_AZ_ERROR(AZ_ULIB_TRY_RESULT);

    // Call get.
    my_command_model_out model_out;
    AZ_ULIB_THROW_IF_AZ_ERROR(
        my_command((az_ulib_model_in)&model_in, (az_ulib_model_out)&model_out));

    // Marshalling encrypt_model_out to JSON in model_out_span.
    az_json_writer jw;
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_init(&jw, *model_out_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_begin_object(&jw));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_property_name(
        &jw, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_COMMAND_RESULT_NAME)));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_int32(&jw, (int32_t)model_out));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_end_object(&jw));
    *model_out_span = az_json_writer_get_bytes_used_in_destination(&jw);
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
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

static az_result my_command_async_span_wrapper(
    az_span model_in_span,
    az_span* model_out_span,
    const az_ulib_capability_token capability_token,
    const az_ulib_capability_cancellation_callback cancel)
{
  (void)model_in_span;
  (void)model_out_span;
  (void)capability_token;
  (void)cancel;

  return AZ_OK;
}

static az_result my_command_cancel(const az_ulib_capability_token capability_token)
{
  (void)capability_token;

  return AZ_OK;
}

/*
 * Publish MY_INTERFACE_1_V123
 */
static const az_ulib_capability_descriptor
    MY_INTERFACE_1_V123_CAPABILITIES[MY_INTERFACE_1_123_CAPABILITY_SIZE]
    = { AZ_ULIB_DESCRIPTOR_ADD_PROPERTY(
            MY_INTERFACE_MY_PROPERTY_NAME,
            get_my_property,
            set_my_property,
            get_my_property_span_wrapper,
            set_my_property_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY2_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_COMMAND(
            MY_INTERFACE_MY_COMMAND_NAME,
            my_command,
            my_command_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
            MY_INTERFACE_MY_COMMAND_ASYNC_NAME,
            my_command_async,
            my_command_async_span_wrapper,
            my_command_cancel) };
const az_ulib_interface_descriptor MY_INTERFACE_1_V123 = AZ_ULIB_DESCRIPTOR_CREATE(
    MY_INTERFACE_1_123_INTERFACE_NAME,
    MY_INTERFACE_1_123_INTERFACE_VERSION,
    MY_INTERFACE_1_123_CAPABILITY_SIZE,
    MY_INTERFACE_1_V123_CAPABILITIES);

az_result az_ulib_test_my_interface_1_v123_publish(az_ulib_ipc_interface_handle* interface_handle)
{
  return az_ulib_ipc_publish(&MY_INTERFACE_1_V123, interface_handle);
}

az_result az_ulib_test_my_interface_1_v123_unpublish(uint32_t wait_ms)
{
  return az_ulib_ipc_unpublish(&MY_INTERFACE_1_V123, wait_ms);
}

/*
 * Publish MY_INTERFACE_1_V2
 */
static const az_ulib_capability_descriptor
    MY_INTERFACE_1_V2_CAPABILITIES[MY_INTERFACE_1_2_CAPABILITY_SIZE]
    = { AZ_ULIB_DESCRIPTOR_ADD_PROPERTY(
            MY_INTERFACE_MY_PROPERTY_NAME,
            get_my_property,
            set_my_property,
            NULL,
            NULL),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY2_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_COMMAND(MY_INTERFACE_MY_COMMAND_NAME, my_command, NULL),
        AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
            MY_INTERFACE_MY_COMMAND_ASYNC_NAME,
            my_command_async,
            NULL,
            my_command_cancel) };
static const az_ulib_interface_descriptor MY_INTERFACE_1_V2 = AZ_ULIB_DESCRIPTOR_CREATE(
    MY_INTERFACE_1_2_INTERFACE_NAME,
    MY_INTERFACE_1_2_INTERFACE_VERSION,
    MY_INTERFACE_1_2_CAPABILITY_SIZE,
    MY_INTERFACE_1_V2_CAPABILITIES);

az_result az_ulib_test_my_interface_1_v2_publish(az_ulib_ipc_interface_handle* interface_handle)
{
  return az_ulib_ipc_publish(&MY_INTERFACE_1_V2, interface_handle);
}

az_result az_ulib_test_my_interface_1_v2_unpublish(uint32_t wait_ms)
{
  return az_ulib_ipc_unpublish(&MY_INTERFACE_1_V2, wait_ms);
}

/*
 * Publish MY_INTERFACE_2_V123
 */
static const az_ulib_capability_descriptor
    MY_INTERFACE_2_V123_CAPABILITIES[MY_INTERFACE_2_123_CAPABILITY_SIZE]
    = { AZ_ULIB_DESCRIPTOR_ADD_PROPERTY(
            MY_INTERFACE_MY_PROPERTY_NAME,
            get_my_property,
            set_my_property,
            get_my_property_span_wrapper,
            set_my_property_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY2_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_COMMAND(
            MY_INTERFACE_MY_COMMAND_NAME,
            my_command,
            my_command_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
            MY_INTERFACE_MY_COMMAND_ASYNC_NAME,
            my_command_async,
            my_command_async_span_wrapper,
            my_command_cancel) };
static const az_ulib_interface_descriptor MY_INTERFACE_2_V123 = AZ_ULIB_DESCRIPTOR_CREATE(
    MY_INTERFACE_2_123_INTERFACE_NAME,
    MY_INTERFACE_2_123_INTERFACE_VERSION,
    MY_INTERFACE_2_123_CAPABILITY_SIZE,
    MY_INTERFACE_2_V123_CAPABILITIES);

az_result az_ulib_test_my_interface_2_v123_publish(az_ulib_ipc_interface_handle* interface_handle)
{
  return az_ulib_ipc_publish(&MY_INTERFACE_2_V123, interface_handle);
}

az_result az_ulib_test_my_interface_2_v123_unpublish(uint32_t wait_ms)
{
  return az_ulib_ipc_unpublish(&MY_INTERFACE_2_V123, wait_ms);
}

/*
 * Publish MY_INTERFACE_3_V123
 */
static const az_ulib_capability_descriptor
    MY_INTERFACE_3_V123_CAPABILITIES[MY_INTERFACE_3_123_CAPABILITY_SIZE]
    = { AZ_ULIB_DESCRIPTOR_ADD_PROPERTY(
            MY_INTERFACE_MY_PROPERTY_NAME,
            get_my_property,
            set_my_property,
            get_my_property_span_wrapper,
            set_my_property_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY2_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_COMMAND(
            MY_INTERFACE_MY_COMMAND_NAME,
            my_command,
            my_command_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(
            MY_INTERFACE_MY_COMMAND_ASYNC_NAME,
            my_command_async,
            my_command_async_span_wrapper,
            my_command_cancel) };
static const az_ulib_interface_descriptor MY_INTERFACE_3_V123 = AZ_ULIB_DESCRIPTOR_CREATE(
    MY_INTERFACE_3_123_INTERFACE_NAME,
    MY_INTERFACE_3_123_INTERFACE_VERSION,
    MY_INTERFACE_3_123_CAPABILITY_SIZE,
    MY_INTERFACE_3_V123_CAPABILITIES);

az_result az_ulib_test_my_interface_3_v123_publish(az_ulib_ipc_interface_handle* interface_handle)
{
  return az_ulib_ipc_publish(&MY_INTERFACE_3_V123, interface_handle);
}

az_result az_ulib_test_my_interface_3_v123_unpublish(uint32_t wait_ms)
{
  return az_ulib_ipc_unpublish(&MY_INTERFACE_3_V123, wait_ms);
}

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

az_result az_ulib_test_my_interface_publish(int i)
{
  return az_ulib_ipc_publish(&(MY_DESCRIPTOR_LIST[i]), NULL);
}

az_result az_ulib_test_my_interface_unpublish(int i)
{
  return az_ulib_ipc_unpublish(&(MY_DESCRIPTOR_LIST[i]), AZ_ULIB_NO_WAIT);
}
