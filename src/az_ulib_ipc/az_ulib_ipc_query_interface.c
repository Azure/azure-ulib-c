// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "_az_ulib_interfaces.h"
#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_query_1_model.h"
#include "az_ulib_result.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static az_result query_1_query_concrete(
    const query_1_query_model_in* const in,
    query_1_query_model_out* out)
{
  return az_ulib_ipc_query(*in, out->result, &(out->continuation_token));
}

AZ_INLINE int32_t model_out_span_min_size(void)
{
  return (int32_t)(
      2 + // {}
      sizeof(QUERY_1_NEXT_RESULT_NAME) + 6 + // "result":[],
      sizeof(QUERY_1_NEXT_CONTINUATION_TOKEN_NAME) + 13 + //"continuation_token":4294967295
      64); // az_json_writer requires a leftover of _az_MINIMUM_STRING_CHUNK_SIZE to properly work.
}

static az_result marshalling_model_out_to_json(
    query_1_query_model_out* model_out,
    az_span* model_out_span)
{
  AZ_ULIB_TRY
  {
    az_json_writer jw;
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_init(&jw, *model_out_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_begin_object(&jw));
    AZ_ULIB_THROW_IF_AZ_ERROR(
        az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR(QUERY_1_QUERY_RESULT_NAME)));

    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_begin_array(&jw));

    az_span remaining
        = az_span_create(az_span_ptr(*(model_out->result)), az_span_size(*(model_out->result)));
    int32_t split_pos = az_span_find(remaining, AZ_SPAN_FROM_STR(","));
    while (split_pos != -1)
    {
      az_span interface_name = az_span_slice(remaining, 1, split_pos - 1);
      AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_string(&jw, interface_name));
      remaining = az_span_slice_to_end(remaining, split_pos + 1);
      split_pos = az_span_find(remaining, AZ_SPAN_FROM_STR(","));
    }
    AZ_ULIB_THROW_IF_AZ_ERROR(AZ_ULIB_TRY_RESULT);
    if (az_span_size(remaining) > 0)
    {
      az_span interface_name = az_span_slice(remaining, 1, az_span_size(remaining) - 1);
      AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_string(&jw, interface_name));
    }

    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_end_array(&jw));

    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_property_name(
        &jw, AZ_SPAN_FROM_STR(QUERY_1_QUERY_CONTINUATION_TOKEN_NAME)));
    AZ_ULIB_THROW_IF_AZ_ERROR(
        az_json_writer_append_int32(&jw, (int32_t)model_out->continuation_token));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_end_object(&jw));
    *model_out_span = az_json_writer_get_bytes_used_in_destination(&jw);
  }
  AZ_ULIB_CATCH(...) { return AZ_ULIB_TRY_RESULT; }

  return AZ_OK;
}

static az_result query_1_query_span_wrapper(az_span model_in_span, az_span* model_out_span)
{
  AZ_ULIB_TRY
  {
    // Unmarshalling JSON in model_in_span to query_model_in.
    az_json_reader jr;
    az_span query_model_in = AZ_SPAN_EMPTY;
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_init(&jr, model_in_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    while (jr.token.kind != AZ_JSON_TOKEN_END_OBJECT)
    {
      if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR(QUERY_1_QUERY_QUERY_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        query_model_in = az_span_create(az_span_ptr(jr.token.slice), az_span_size(jr.token.slice));
      }
      AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    }
    AZ_ULIB_THROW_IF_AZ_ERROR(AZ_ULIB_TRY_RESULT);

    // Create a temporary buffer to store the query_model_out.
    az_span dest_span = az_span_slice_to_end(*model_out_span, model_out_span_min_size() + 1);
    query_1_query_model_out query_model_out = { .result = &dest_span, .continuation_token = 0 };

    // Call.
    AZ_ULIB_THROW_IF_AZ_ERROR(query_1_query_concrete(&query_model_in, &query_model_out));

    // Marshalling query_model_out to JSON in model_out_span.
    AZ_ULIB_THROW_IF_AZ_ERROR(marshalling_model_out_to_json(&query_model_out, model_out_span));
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
}

static az_result query_1_next_concrete(
    const query_1_next_model_in* const in,
    query_1_next_model_out* out)
{
  out->continuation_token = *in;
  return az_ulib_ipc_query_next(&(out->continuation_token), out->result);
}

static az_result query_1_next_span_wrapper(az_span model_in_span, az_span* model_out_span)
{
  AZ_ULIB_TRY
  {
    // Unmarshalling JSON in model_in_span to next_model_in.
    az_json_reader jr;
    uint32_t next_model_in = 0;
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_init(&jr, model_in_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    while (jr.token.kind != AZ_JSON_TOKEN_END_OBJECT)
    {
      if (az_json_token_is_text_equal(
              &jr.token, AZ_SPAN_FROM_STR(QUERY_1_NEXT_CONTINUATION_TOKEN_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        AZ_ULIB_THROW_IF_AZ_ERROR(az_span_atou32(jr.token.slice, &next_model_in));
      }
      AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    }
    AZ_ULIB_THROW_IF_AZ_ERROR(AZ_ULIB_TRY_RESULT);

    // Create a temporary buffer to store the next_model_out.
    az_span dest_span = az_span_slice_to_end(*model_out_span, model_out_span_min_size() + 1);
    query_1_next_model_out next_model_out = { .result = &dest_span, .continuation_token = 0 };

    // Call.
    AZ_ULIB_THROW_IF_AZ_ERROR(query_1_next_concrete(&next_model_in, &next_model_out));

    if (AZ_ULIB_TRY_RESULT != AZ_ULIB_EOF)
    {
      // Marshalling next_model_out to JSON in model_out_span.
      AZ_ULIB_THROW_IF_AZ_ERROR(
          marshalling_model_out_to_json((query_1_query_model_out*)&next_model_out, model_out_span));
    }
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
}

static const az_ulib_capability_descriptor QUERY_1_CAPABILITIES[]
    = { AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            QUERY_1_QUERY_COMMAND_NAME,
            query_1_query_concrete,
            query_1_query_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
            QUERY_1_NEXT_COMMAND_NAME,
            query_1_next_concrete,
            query_1_next_span_wrapper) };

static const az_ulib_interface_descriptor QUERY_1_DESCRIPTOR = AZ_ULIB_DESCRIPTOR_CREATE(
    IPC_1_PACKAGE_NAME,
    IPC_1_PACKAGE_VERSION,
    QUERY_1_INTERFACE_NAME,
    QUERY_1_INTERFACE_VERSION,
    QUERY_1_CAPABILITIES);

az_result _az_ulib_ipc_query_interface_publish(void)
{
  return az_ulib_ipc_publish(&QUERY_1_DESCRIPTOR);
}

az_result _az_ulib_ipc_query_interface_unpublish(void)
{
  return az_ulib_ipc_unpublish(&QUERY_1_DESCRIPTOR, AZ_ULIB_NO_WAIT);
}
