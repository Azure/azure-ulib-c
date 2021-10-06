// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_test_my_interface.h"
#include "az_ulib_ipc_api.h"

static my_property_model my_property = 0;

static az_result marshalling_out_to_json(my_property_model out, az_span* model_out_span)
{
  AZ_ULIB_TRY
  {
    az_json_writer jw;
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_init(&jw, *model_out_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_begin_object(&jw));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_property_name(
        &jw, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_PROPERTY_VAL_NAME)));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_int32(&jw, out));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_end_object(&jw));
    *model_out_span = az_json_writer_get_bytes_used_in_destination(&jw);
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
}

static az_result get_my_property(const my_property_model* const in, my_property_model* out)
{
  (void)in;
  *out = my_property;
  return AZ_OK;
}

static az_result get_my_property_span_wrapper(az_span model_in_span, az_span* model_out_span)
{
  (void)model_in_span;
  AZ_ULIB_TRY
  {
    // Call get.
    my_property_model out;
    AZ_ULIB_THROW_IF_AZ_ERROR(get_my_property(NULL, &out));

    // Marshalling my_property_model out to JSON in model_out_span.
    marshalling_out_to_json(out, model_out_span);
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
}

static az_result set_my_property(const my_property_model* const in, my_property_model* out)
{
  my_property = *in;
  *out = my_property;
  return AZ_OK;
}

static az_result set_my_property_span_wrapper(az_span model_in_span, az_span* model_out_span)
{
  AZ_ULIB_TRY
  {
    // Unmarshalling JSON in model_in_span to my_property_model in.
    az_json_reader jr;
    my_property_model in = 0;
    my_property_model out = 0;
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_init(&jr, model_in_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    while (jr.token.kind != AZ_JSON_TOKEN_END_OBJECT)
    {
      if (az_json_token_is_text_equal(
              &jr.token, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_PROPERTY_VAL_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_token_get_int32(&jr.token, &in));
      }
      AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    }
    AZ_ULIB_THROW_IF_AZ_ERROR(AZ_ULIB_TRY_RESULT);

    // Call get.
    AZ_ULIB_THROW_IF_AZ_ERROR(set_my_property(&in, &out));

    // Marshalling my_property_model out to JSON in model_out_span.
    marshalling_out_to_json(out, model_out_span);
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
}

volatile long g_is_running;
volatile long g_lock_thread;
volatile uint32_t g_sum_sleep;

static az_result my_command(const my_command_model_in* const in, my_command_model_out* out)
{
  my_command_model_in in_2;
  uint64_t sum = 0;

  (void)AZ_ULIB_PORT_ATOMIC_INC_W(&g_is_running);
  switch (in->capability)
  {
    case MY_COMMAND_CAPABILITY_JUST_RETURN:
      *out = in->return_result;
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
      *out = in->return_result;
      break;
    case MY_COMMAND_CAPABILITY_UNPUBLISH:
      *out = az_ulib_ipc_unpublish(in->descriptor, in->wait_policy_ms);
      break;
    case MY_COMMAND_CAPABILITY_DEINIT:
      *out = az_ulib_ipc_deinit();
      break;
    case MY_COMMAND_CAPABILITY_CALL_AGAIN:
      in_2.capability = 0;
      in_2.return_result = AZ_OK;
      *out = az_ulib_ipc_call(in->handle, in->capability_index, &in_2, out);
      break;
    default:
      *out = AZ_ERROR_ITEM_NOT_FOUND;
      break;
  }
  (void)AZ_ULIB_PORT_ATOMIC_DEC_W(&g_is_running);

  return AZ_OK;
}

static az_result my_command_span_wrapper(az_span model_in_span, az_span* model_out_span)
{
  AZ_ULIB_TRY
  {
    // Unmarshalling JSON in model_in_span to model_in.
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
        // Does not support local pointer over JSON.
        AZ_ULIB_THROW(AZ_ERROR_NOT_SUPPORTED);
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
        // Does not support local pointer over JSON.
        AZ_ULIB_THROW(AZ_ERROR_NOT_SUPPORTED);
      }
      else if (az_json_token_is_text_equal(
                   &jr.token, AZ_SPAN_FROM_STR(MY_INTERFACE_MY_COMMAND_CAPABILITY_INDEX_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        uint32_t val;
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_token_get_uint32(&jr.token, &val));
        model_in.capability_index = (az_ulib_capability_index)val;
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
    AZ_ULIB_THROW_IF_AZ_ERROR(my_command(&model_in, &model_out));

    // Marshalling model_out to JSON in model_out_span.
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

/*
 * Publish MY_INTERFACE_A_1_1_123
 */
static const az_ulib_capability_descriptor MY_INTERFACE_A_1_1_123_CAPABILITIES[]
    = { AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_GET_MY_PROPERTY_NAME,
            get_my_property,
            get_my_property_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_SET_MY_PROPERTY_NAME,
            set_my_property,
            set_my_property_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY2_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_MY_COMMAND_NAME,
            my_command,
            my_command_span_wrapper) };
const az_ulib_interface_descriptor MY_INTERFACE_A_1_1_123 = AZ_ULIB_DESCRIPTOR_CREATE(
    MY_PACKAGE_A_NAME,
    MY_PACKAGE_1_VERSION,
    MY_INTERFACE_1_NAME,
    MY_INTERFACE_123_VERSION,
    MY_INTERFACE_A_1_1_123_CAPABILITIES);

az_result az_ulib_test_my_interface_a_1_1_123_publish(void)
{
  return az_ulib_ipc_publish(&MY_INTERFACE_A_1_1_123);
}

az_result az_ulib_test_my_interface_a_1_1_123_unpublish(uint32_t wait_ms)
{
  return az_ulib_ipc_unpublish(&MY_INTERFACE_A_1_1_123, wait_ms);
}

/*
 * Publish MY_INTERFACE_B_1_1_123
 */
static const az_ulib_capability_descriptor MY_INTERFACE_B_1_1_123_CAPABILITIES[]
    = { AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_GET_MY_PROPERTY_NAME,
            get_my_property,
            get_my_property_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_SET_MY_PROPERTY_NAME,
            set_my_property,
            set_my_property_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY2_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_MY_COMMAND_NAME,
            my_command,
            my_command_span_wrapper) };
const az_ulib_interface_descriptor MY_INTERFACE_B_1_1_123 = AZ_ULIB_DESCRIPTOR_CREATE(
    MY_PACKAGE_B_NAME,
    MY_PACKAGE_1_VERSION,
    MY_INTERFACE_1_NAME,
    MY_INTERFACE_123_VERSION,
    MY_INTERFACE_B_1_1_123_CAPABILITIES);

az_result az_ulib_test_my_interface_b_1_1_123_publish(void)
{
  return az_ulib_ipc_publish(&MY_INTERFACE_B_1_1_123);
}

az_result az_ulib_test_my_interface_b_1_1_123_unpublish(uint32_t wait_ms)
{
  return az_ulib_ipc_unpublish(&MY_INTERFACE_B_1_1_123, wait_ms);
}

/*
 * Publish MY_INTERFACE_C_1_1_123
 */
static const az_ulib_capability_descriptor MY_INTERFACE_C_1_1_123_CAPABILITIES[]
    = { AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_GET_MY_PROPERTY_NAME,
            get_my_property,
            get_my_property_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_SET_MY_PROPERTY_NAME,
            set_my_property,
            set_my_property_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY2_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_MY_COMMAND_NAME,
            my_command,
            my_command_span_wrapper) };
const az_ulib_interface_descriptor MY_INTERFACE_C_1_1_123 = AZ_ULIB_DESCRIPTOR_CREATE(
    MY_PACKAGE_C_NAME,
    MY_PACKAGE_1_VERSION,
    MY_INTERFACE_1_NAME,
    MY_INTERFACE_123_VERSION,
    MY_INTERFACE_C_1_1_123_CAPABILITIES);

az_result az_ulib_test_my_interface_c_1_1_123_publish(void)
{
  return az_ulib_ipc_publish(&MY_INTERFACE_C_1_1_123);
}

az_result az_ulib_test_my_interface_c_1_1_123_unpublish(uint32_t wait_ms)
{
  return az_ulib_ipc_unpublish(&MY_INTERFACE_C_1_1_123, wait_ms);
}

/*
 * Publish MY_INTERFACE_A_2_1_123
 */
const az_ulib_interface_descriptor MY_INTERFACE_A_2_1_123 = AZ_ULIB_DESCRIPTOR_CREATE(
    MY_PACKAGE_A_NAME,
    MY_PACKAGE_2_VERSION,
    MY_INTERFACE_1_NAME,
    MY_INTERFACE_123_VERSION,
    MY_INTERFACE_A_1_1_123_CAPABILITIES);

az_result az_ulib_test_my_interface_a_2_1_123_publish(void)
{
  return az_ulib_ipc_publish(&MY_INTERFACE_A_2_1_123);
}

az_result az_ulib_test_my_interface_a_2_1_123_unpublish(uint32_t wait_ms)
{
  return az_ulib_ipc_unpublish(&MY_INTERFACE_A_2_1_123, wait_ms);
}

/*
 * Publish MY_INTERFACE_D_1_1_123
 */
static const az_ulib_capability_descriptor MY_INTERFACE_D_1_1_123_CAPABILITIES[]
    = { AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_GET_MY_PROPERTY_NAME,
            get_my_property,
            get_my_property_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_SET_MY_PROPERTY_NAME,
            set_my_property,
            set_my_property_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY2_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_MY_COMMAND_NAME,
            my_command,
            my_command_span_wrapper) };
const az_ulib_interface_descriptor MY_INTERFACE_D_1_1_123 = AZ_ULIB_DESCRIPTOR_CREATE(
    MY_PACKAGE_D_NAME,
    MY_PACKAGE_1_VERSION,
    MY_INTERFACE_1_NAME,
    MY_INTERFACE_123_VERSION,
    MY_INTERFACE_D_1_1_123_CAPABILITIES);

az_result az_ulib_test_my_interface_d_1_1_123_publish(void)
{
  return az_ulib_ipc_publish(&MY_INTERFACE_D_1_1_123);
}

az_result az_ulib_test_my_interface_d_1_1_123_unpublish(uint32_t wait_ms)
{
  return az_ulib_ipc_unpublish(&MY_INTERFACE_D_1_1_123, wait_ms);
}

/*
 * Publish MY_INTERFACE_D_2_1_123
 */
const az_ulib_interface_descriptor MY_INTERFACE_D_2_1_123 = AZ_ULIB_DESCRIPTOR_CREATE(
    MY_PACKAGE_D_NAME,
    MY_PACKAGE_2_VERSION,
    MY_INTERFACE_1_NAME,
    MY_INTERFACE_123_VERSION,
    MY_INTERFACE_D_1_1_123_CAPABILITIES);

az_result az_ulib_test_my_interface_d_2_1_123_publish(void)
{
  return az_ulib_ipc_publish(&MY_INTERFACE_D_2_1_123);
}

az_result az_ulib_test_my_interface_d_2_1_123_unpublish(uint32_t wait_ms)
{
  return az_ulib_ipc_unpublish(&MY_INTERFACE_D_2_1_123, wait_ms);
}

/*
 * Publish MY_INTERFACE_A_1_1_200
 */
static const az_ulib_capability_descriptor MY_INTERFACE_A_1_1_200_CAPABILITIES[]
    = { AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(MY_INTERFACE_GET_MY_PROPERTY_NAME, get_my_property, NULL),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(MY_INTERFACE_SET_MY_PROPERTY_NAME, set_my_property, NULL),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY2_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(MY_INTERFACE_MY_COMMAND_NAME, my_command, NULL) };
static const az_ulib_interface_descriptor MY_INTERFACE_A_1_1_200 = AZ_ULIB_DESCRIPTOR_CREATE(
    MY_PACKAGE_A_NAME,
    MY_PACKAGE_1_VERSION,
    MY_INTERFACE_1_NAME,
    MY_INTERFACE_200_VERSION,
    MY_INTERFACE_A_1_1_200_CAPABILITIES);

az_result az_ulib_test_my_interface_a_1_1_200_publish(void)
{
  return az_ulib_ipc_publish(&MY_INTERFACE_A_1_1_200);
}

az_result az_ulib_test_my_interface_a_1_1_200_unpublish(uint32_t wait_ms)
{
  return az_ulib_ipc_unpublish(&MY_INTERFACE_A_1_1_200, wait_ms);
}

/*
 * Publish MY_INTERFACE_A_1_2_123
 */
static const az_ulib_capability_descriptor MY_INTERFACE_A_1_2_123_CAPABILITIES[]
    = { AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_GET_MY_PROPERTY_NAME,
            get_my_property,
            get_my_property_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(MY_INTERFACE_SET_MY_PROPERTY_NAME, set_my_property, NULL),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY2_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_MY_COMMAND_NAME,
            my_command,
            my_command_span_wrapper) };
static const az_ulib_interface_descriptor MY_INTERFACE_A_1_2_123 = AZ_ULIB_DESCRIPTOR_CREATE(
    MY_PACKAGE_A_NAME,
    MY_PACKAGE_1_VERSION,
    MY_INTERFACE_2_NAME,
    MY_INTERFACE_123_VERSION,
    MY_INTERFACE_A_1_2_123_CAPABILITIES);

az_result az_ulib_test_my_interface_a_1_2_123_publish(void)
{
  return az_ulib_ipc_publish(&MY_INTERFACE_A_1_2_123);
}

az_result az_ulib_test_my_interface_a_1_2_123_unpublish(uint32_t wait_ms)
{
  return az_ulib_ipc_unpublish(&MY_INTERFACE_A_1_2_123, wait_ms);
}

/*
 * Publish MY_INTERFACE_A_1_3_123
 */
static const az_ulib_capability_descriptor MY_INTERFACE_A_1_3_123_CAPABILITIES[]
    = { AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_GET_MY_PROPERTY_NAME,
            get_my_property,
            get_my_property_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_SET_MY_PROPERTY_NAME,
            set_my_property,
            set_my_property_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(MY_INTERFACE_MY_TELEMETRY2_NAME),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            MY_INTERFACE_MY_COMMAND_NAME,
            my_command,
            my_command_span_wrapper) };
static const az_ulib_interface_descriptor MY_INTERFACE_A_1_3_123 = AZ_ULIB_DESCRIPTOR_CREATE(
    MY_PACKAGE_A_NAME,
    MY_PACKAGE_1_VERSION,
    MY_INTERFACE_3_NAME,
    MY_INTERFACE_123_VERSION,
    MY_INTERFACE_A_1_3_123_CAPABILITIES);

az_result az_ulib_test_my_interface_a_1_3_123_publish(void)
{
  return az_ulib_ipc_publish(&MY_INTERFACE_A_1_3_123);
}

az_result az_ulib_test_my_interface_a_1_3_123_unpublish(uint32_t wait_ms)
{
  return az_ulib_ipc_unpublish(&MY_INTERFACE_A_1_3_123, wait_ms);
}

static const az_ulib_interface_descriptor MY_DESCRIPTOR_LIST[AZ_ULIB_CONFIG_MAX_IPC_INTERFACE]
    = { AZ_ULIB_DESCRIPTOR_CREATE(
            MY_PACKAGE_A_NAME,
            MY_PACKAGE_1_VERSION,
            "MY_INTERFACE",
            1000,
            MY_INTERFACE_A_1_3_123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE(
            MY_PACKAGE_A_NAME,
            MY_PACKAGE_1_VERSION,
            "MY_INTERFACE",
            1001,
            MY_INTERFACE_A_1_3_123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE(
            MY_PACKAGE_A_NAME,
            MY_PACKAGE_1_VERSION,
            "MY_INTERFACE",
            1002,
            MY_INTERFACE_A_1_3_123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE(
            MY_PACKAGE_A_NAME,
            MY_PACKAGE_1_VERSION,
            "MY_INTERFACE",
            1003,
            MY_INTERFACE_A_1_3_123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE(
            MY_PACKAGE_A_NAME,
            MY_PACKAGE_1_VERSION,
            "MY_INTERFACE",
            1004,
            MY_INTERFACE_A_1_3_123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE(
            MY_PACKAGE_A_NAME,
            MY_PACKAGE_1_VERSION,
            "MY_INTERFACE",
            1005,
            MY_INTERFACE_A_1_3_123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE(
            MY_PACKAGE_A_NAME,
            MY_PACKAGE_1_VERSION,
            "MY_INTERFACE",
            1006,
            MY_INTERFACE_A_1_3_123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE(
            MY_PACKAGE_A_NAME,
            MY_PACKAGE_1_VERSION,
            "MY_INTERFACE",
            1007,
            MY_INTERFACE_A_1_3_123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE(
            MY_PACKAGE_A_NAME,
            MY_PACKAGE_1_VERSION,
            "MY_INTERFACE",
            1008,
            MY_INTERFACE_A_1_3_123_CAPABILITIES),
        AZ_ULIB_DESCRIPTOR_CREATE(
            MY_PACKAGE_A_NAME,
            MY_PACKAGE_1_VERSION,
            "MY_INTERFACE",
            1009,
            MY_INTERFACE_A_1_3_123_CAPABILITIES) };

az_result az_ulib_test_my_interface_publish(int i)
{
  return az_ulib_ipc_publish(&(MY_DESCRIPTOR_LIST[i]));
}

az_result az_ulib_test_my_interface_unpublish(int i)
{
  return az_ulib_ipc_unpublish(&(MY_DESCRIPTOR_LIST[i]), AZ_ULIB_NO_WAIT);
}
