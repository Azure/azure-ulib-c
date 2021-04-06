// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from cipher v1 DL.
 *
 * Implement the code under the concrete functions.
 *
 ********************************************************************/

#include "cipher_v1i1_interface.h"
#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "cipher_1_interface.h"
#include "cipher_v1i1.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static az_result cipher_1_call_w_str(az_span name, az_span model_in, az_span* model_out);

/*
 * Concrete implementations of the cipher 1 encrypt commands.
 */
static az_result cipher_1_encrypt_concrete(az_ulib_model_in model_in, az_ulib_model_out model_out)
{
  az_result result;

  /*
   * ==================
   * The user code starts here.
   */
  const cipher_1_encrypt_model_in* const in = (const cipher_1_encrypt_model_in* const)model_in;
  cipher_1_encrypt_model_out* out = (cipher_1_encrypt_model_out*)model_out;

  result = cipher_v1i1_encrypt(in->context, in->src, out->dest);

  /*
   * The user code ends here.
   * ==================
   */

  return result;
}

/*
 * Concrete implementations of the cipher 1 decrypt commands.
 */
static az_result cipher_1_decrypt_concrete(az_ulib_model_in model_in, az_ulib_model_out model_out)
{
  az_result result;

  /*
   * ==================
   * The user code starts here.
   */
  const cipher_1_decrypt_model_in* const in = (const cipher_1_decrypt_model_in* const)model_in;
  cipher_1_decrypt_model_out* out = (cipher_1_decrypt_model_out*)model_out;

  result = cipher_v1i1_decrypt(in->src, out->dest);

  /*
   * The user code ends here.
   * ==================
   */

  return result;
}

static const az_ulib_capability_descriptor CIPHER_1_CAPABILITIES[CIPHER_1_CAPABILITY_SIZE] = {
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND(
      CIPHER_1_INTERFACE_ENCRYPT_COMMAND_NAME,
      cipher_1_encrypt_concrete),
  AZ_ULIB_DESCRIPTOR_ADD_COMMAND(CIPHER_1_INTERFACE_DECRYPT_COMMAND_NAME, cipher_1_decrypt_concrete)
};

static const az_ulib_interface_descriptor CIPHER_1_DESCRIPTOR = AZ_ULIB_DESCRIPTOR_CREATE(
    CIPHER_1_INTERFACE_NAME,
    CIPHER_1_INTERFACE_VERSION,
    CIPHER_1_CAPABILITY_SIZE,
    cipher_1_call_w_str,
    CIPHER_1_CAPABILITIES);

static az_result cipher_1_encrypt_span_wrapper(az_span model_in_span, az_span* model_out_span)
{
  az_result result = AZ_OK;

  AZ_ULIB_TRY
  {
    // Unmarshalling JSON to model_in_span.
    az_json_reader jr;
    cipher_1_encrypt_model_in encrypt_model_in = { 0 };
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_init(&jr, model_in_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    while (jr.token.kind != AZ_JSON_TOKEN_END_OBJECT)
    {
      if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR("context")))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_token_get_uint32(&jr.token, &encrypt_model_in.context));
      }
      else if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR("src")))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        encrypt_model_in.src
            = az_span_create(az_span_ptr(jr.token.slice), az_span_size(jr.token.slice));
      }
      AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    }
    AZ_ULIB_THROW_IF_AZ_ERROR(AZ_ULIB_TRY_RESULT);

    // Build model_out_span.
    char dest_buffer[200];
    az_span dest_span = AZ_SPAN_FROM_BUFFER(dest_buffer);
    cipher_1_encrypt_model_out encrypt_model_out = { .dest = &dest_span };

    // Call.
    result = cipher_1_encrypt_concrete(
        (az_ulib_model_in)&encrypt_model_in, (az_ulib_model_out)&encrypt_model_out);

    // Marshalling model_out_span to JSON.
    az_json_writer jw;
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_init(&jw, *model_out_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_begin_object(&jw));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR("dest")));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_string(&jw, *encrypt_model_out.dest));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_end_object(&jw));
    *model_out_span = az_json_writer_get_bytes_used_in_destination(&jw);
  }
  AZ_ULIB_CATCH(...) { result = AZ_ULIB_TRY_RESULT; }

  return result;
}

static az_result cipher_1_decrypt_span_wrapper(az_span model_in_span, az_span* model_out_span)
{
  az_result result = AZ_OK;

  AZ_ULIB_TRY
  {
    // Unmarshalling JSON to model_in_span.
    az_json_reader jr;
    cipher_1_decrypt_model_in decrypt_model_in = { 0 };
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_init(&jr, model_in_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    while (jr.token.kind != AZ_JSON_TOKEN_END_OBJECT)
    {
      if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR("src")))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        decrypt_model_in.src
            = az_span_create(az_span_ptr(jr.token.slice), az_span_size(jr.token.slice));
      }
      AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    }
    AZ_ULIB_THROW_IF_AZ_ERROR(AZ_ULIB_TRY_RESULT);

    // Build model_out_span.
    char dest_buffer[200];
    az_span dest_span = AZ_SPAN_FROM_BUFFER(dest_buffer);
    cipher_1_decrypt_model_out decrypt_model_out = { .dest = &dest_span };

    // Call.
    result = cipher_1_decrypt_concrete(
        (az_ulib_model_in)&decrypt_model_in, (az_ulib_model_out)&decrypt_model_out);

    // Marshalling model_out_span to JSON.
    az_json_writer jw;
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_init(&jw, *model_out_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_begin_object(&jw));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR("dest")));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_string(&jw, *decrypt_model_out.dest));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_end_object(&jw));
    *model_out_span = az_json_writer_get_bytes_used_in_destination(&jw);
  }
  AZ_ULIB_CATCH(...) { result = AZ_ULIB_TRY_RESULT; }

  return result;
}

static az_result cipher_1_call_w_str(az_span name, az_span model_in_span, az_span* model_out_span)
{
  az_ulib_capability_index capability_index;
  az_result result;

  for (capability_index = 0; capability_index < CIPHER_1_DESCRIPTOR.size; capability_index++)
  {
    if (az_span_is_content_equal(name, CIPHER_1_DESCRIPTOR.capability_list[capability_index].name))
    {
      break;
    }
  }

  switch (capability_index)
  {
    case CIPHER_1_INTERFACE_ENCRYPT_COMMAND:
      result = cipher_1_encrypt_span_wrapper(model_in_span, model_out_span);
      break;
    case CIPHER_1_INTERFACE_DECRYPT_COMMAND:
    {
      result = cipher_1_decrypt_span_wrapper(model_in_span, model_out_span);
      break;
    }
    default:
      result = AZ_ERROR_ITEM_NOT_FOUND;
      break;
  }

  return result;
}

az_result publish_cipher_v1i1_interface(void)
{
  return az_ulib_ipc_publish(&CIPHER_1_DESCRIPTOR, NULL);
}

az_result unpublish_cipher_v1i1_interface(void)
{
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
  return az_ulib_ipc_unpublish(&CIPHER_1_DESCRIPTOR, AZ_ULIB_NO_WAIT);
#else
  return AZ_OK;
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
}
