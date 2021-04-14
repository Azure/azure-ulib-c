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
#include "cipher_1_model.h"
#include "cipher_v1i1.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static az_result cipher_1_encrypt_concrete(az_ulib_model_in model_in, az_ulib_model_out model_out)
{
  const cipher_1_encrypt_model_in* const in = (const cipher_1_encrypt_model_in* const)model_in;
  cipher_1_encrypt_model_out* out = (cipher_1_encrypt_model_out*)model_out;
  return cipher_v1i1_encrypt(in->context, in->src, out->dest);
}

static az_result cipher_1_encrypt_span_wrapper(az_span model_in_span, az_span* model_out_span)
{
  AZ_ULIB_TRY
  {
    // Unmarshalling JSON in model_in_span to encrypt_model_in.
    az_json_reader jr;
    cipher_1_encrypt_model_in encrypt_model_in = { 0 };
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_init(&jr, model_in_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    while (jr.token.kind != AZ_JSON_TOKEN_END_OBJECT)
    {
      if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR(CIPHER_1_ENCRYPT_CONTEXT_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_token_get_uint32(&jr.token, &encrypt_model_in.context));
      }
      else if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR(CIPHER_1_ENCRYPT_SRC_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        encrypt_model_in.src
            = az_span_create(az_span_ptr(jr.token.slice), az_span_size(jr.token.slice));
      }
      AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    }
    AZ_ULIB_THROW_IF_AZ_ERROR(AZ_ULIB_TRY_RESULT);

    // Create a temporary buffer to store the encrypt_model_out.
    char dest_buffer[200];
    az_span dest_span = AZ_SPAN_FROM_BUFFER(dest_buffer);
    cipher_1_encrypt_model_out encrypt_model_out = { .dest = &dest_span };

    // Call.
    AZ_ULIB_THROW_IF_AZ_ERROR(cipher_1_encrypt_concrete(
        (az_ulib_model_in)&encrypt_model_in, (az_ulib_model_out)&encrypt_model_out));

    // Marshalling encrypt_model_out to JSON in model_out_span.
    az_json_writer jw;
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_init(&jw, *model_out_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_begin_object(&jw));
    AZ_ULIB_THROW_IF_AZ_ERROR(
        az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR(CIPHER_1_ENCRYPT_DEST_NAME)));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_string(&jw, *encrypt_model_out.dest));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_end_object(&jw));
    *model_out_span = az_json_writer_get_bytes_used_in_destination(&jw);
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
}

static az_result cipher_1_decrypt_concrete(az_ulib_model_in model_in, az_ulib_model_out model_out)
{
  const cipher_1_decrypt_model_in* const in = (const cipher_1_decrypt_model_in* const)model_in;
  cipher_1_decrypt_model_out* out = (cipher_1_decrypt_model_out*)model_out;
  return cipher_v1i1_decrypt(in->src, out->dest);
}

static az_result cipher_1_decrypt_span_wrapper(az_span model_in_span, az_span* model_out_span)
{
  AZ_ULIB_TRY
  {
    // Unmarshalling JSON in model_in_span to decrypt_model_in.
    az_json_reader jr;
    cipher_1_decrypt_model_in decrypt_model_in = { 0 };
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_init(&jr, model_in_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    while (jr.token.kind != AZ_JSON_TOKEN_END_OBJECT)
    {
      if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR(CIPHER_1_DECRYPT_SRC_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        decrypt_model_in.src
            = az_span_create(az_span_ptr(jr.token.slice), az_span_size(jr.token.slice));
      }
      AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    }
    AZ_ULIB_THROW_IF_AZ_ERROR(AZ_ULIB_TRY_RESULT);

    // Create a temporary buffer to store the decrypt_model_out.
    char dest_buffer[200];
    az_span dest_span = AZ_SPAN_FROM_BUFFER(dest_buffer);
    cipher_1_decrypt_model_out decrypt_model_out = { .dest = &dest_span };

    // Call.
    AZ_ULIB_THROW_IF_AZ_ERROR(cipher_1_decrypt_concrete(
        (az_ulib_model_in)&decrypt_model_in, (az_ulib_model_out)&decrypt_model_out));

    // Marshalling decrypt_model_out to JSON in model_out_span.
    az_json_writer jw;
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_init(&jw, *model_out_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_begin_object(&jw));
    AZ_ULIB_THROW_IF_AZ_ERROR(
        az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR(CIPHER_1_DECRYPT_DEST_NAME)));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_string(&jw, *decrypt_model_out.dest));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_end_object(&jw));
    *model_out_span = az_json_writer_get_bytes_used_in_destination(&jw);
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
}

static const az_ulib_capability_descriptor CIPHER_1_CAPABILITIES[CIPHER_1_CAPABILITY_SIZE]
    = { AZ_ULIB_DESCRIPTOR_ADD_COMMAND(
            CIPHER_1_ENCRYPT_COMMAND_NAME,
            cipher_1_encrypt_concrete,
            cipher_1_encrypt_span_wrapper),
        AZ_ULIB_DESCRIPTOR_ADD_COMMAND(
            CIPHER_1_DECRYPT_COMMAND_NAME,
            cipher_1_decrypt_concrete,
            cipher_1_decrypt_span_wrapper) };

static const az_ulib_interface_descriptor CIPHER_1_DESCRIPTOR = AZ_ULIB_DESCRIPTOR_CREATE(
    CIPHER_1_INTERFACE_NAME,
    CIPHER_1_INTERFACE_VERSION,
    CIPHER_1_CAPABILITY_SIZE,
    CIPHER_1_CAPABILITIES);

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
