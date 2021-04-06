// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "my_consumer.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"
#include "wrappers/cipher_1_wrapper.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

static az_ulib_ipc_interface_handle _cipher_1;
#define BUFFER_SIZE 200

#define CLOSE_STRING_IN_SPAN(span)                                                           \
  do                                                                                         \
  {                                                                                          \
    az_span_ptr(span)[(az_span_size(span) < BUFFER_SIZE) ? az_span_size(span) : BUFFER_SIZE] \
        = '\0';                                                                              \
  } while (0)

static void get_handle_if_need(void)
{
  az_result result;
  if (_cipher_1 == NULL)
  {
    if ((result = cipher_1_create(&_cipher_1)) == AZ_OK)
    {
      (void)printf("My consumer got cipher.1 interface with success.\r\n");
    }
    else if (result == AZ_ERROR_ITEM_NOT_FOUND)
    {
      (void)printf("cypher.1 is not available.\r\n");
    }
    else
    {
      (void)printf("Get cypher.1 interface failed with code %" PRIi32 "\r\n", result);
    }
  }
}

void my_consumer_create(void)
{
  (void)printf("Create my consumer...\r\n");

  _cipher_1 = NULL;
}

void my_consumer_destroy(void)
{
  (void)printf("Destroy my consumer\r\n");
  if (_cipher_1 != NULL)
  {
    cipher_1_destroy(_cipher_1);
  }
}

static az_result call_wrappers(uint32_t context)
{
  AZ_ULIB_TRY
  {
    const az_span original_span_to_encrypt
        = AZ_SPAN_FROM_STR("Welcome to Azure IoT Hackathon 2021!");
    char buffer_1[BUFFER_SIZE];
    char buffer_2[BUFFER_SIZE];

    az_span encrypted_span = AZ_SPAN_FROM_BUFFER(buffer_1);
    AZ_ULIB_THROW_IF_AZ_ERROR(
        cipher_1_encrypt(_cipher_1, context, original_span_to_encrypt, &encrypted_span));
    CLOSE_STRING_IN_SPAN(encrypted_span);
    (void)printf(
        "cipher.1 encrypted \"%s\" to \"%s\" with context %" PRIu32 ".\r\n",
        az_span_ptr(original_span_to_encrypt),
        az_span_ptr(encrypted_span),
        context);

    az_span decrypted_span = AZ_SPAN_FROM_BUFFER(buffer_2);
    AZ_ULIB_THROW_IF_AZ_ERROR(cipher_1_decrypt(_cipher_1, encrypted_span, &decrypted_span));
    CLOSE_STRING_IN_SPAN(decrypted_span);
    (void)printf(
        "cipher.1 decrypted \"%s\" to \"%s\" with context %" PRIu32 ".\r\n",
        az_span_ptr(encrypted_span),
        az_span_ptr(decrypted_span),
        context);
  }
  AZ_ULIB_CATCH(...) { return AZ_ULIB_TRY_RESULT; }

  return AZ_OK;
}

static az_result call_w_str(uint32_t context)
{
  AZ_ULIB_TRY
  {
    char buffer_1[BUFFER_SIZE];
    char buffer_2[BUFFER_SIZE];

    // Encrypt string.
    const az_span encrypt_command_name = AZ_SPAN_FROM_STR("encrypt");
    az_json_writer jw;
    az_span encrypt_model_in_json = AZ_SPAN_FROM_BUFFER(buffer_1);
    az_span encrypt_model_out_json = AZ_SPAN_FROM_BUFFER(buffer_2);

    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_init(&jw, encrypt_model_in_json, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_begin_object(&jw));
    AZ_ULIB_THROW_IF_AZ_ERROR(
        az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR("context")));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_int32(&jw, (int32_t)context));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR("src")));
    AZ_ULIB_THROW_IF_AZ_ERROR(
        az_json_writer_append_string(&jw, AZ_SPAN_FROM_STR("Welcome to Azure IoT!")));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_end_object(&jw));
    encrypt_model_in_json = az_json_writer_get_bytes_used_in_destination(&jw);
    CLOSE_STRING_IN_SPAN(encrypt_model_in_json);

    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_call_w_str(
        _cipher_1, encrypt_command_name, encrypt_model_in_json, &encrypt_model_out_json));
    CLOSE_STRING_IN_SPAN(encrypt_model_out_json);
    (void)printf(
        "cipher.1 encrypted \"%s\" to \"%s\".\r\n",
        az_span_ptr(encrypt_model_in_json),
        az_span_ptr(encrypt_model_out_json));

    // Get encrypted string from JSON.
    az_json_reader jr;
    az_span encrypted_str = AZ_SPAN_FROM_BUFFER(buffer_1);
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_init(&jr, encrypt_model_out_json, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    while (jr.token.kind != AZ_JSON_TOKEN_END_OBJECT)
    {
      if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR("dest")))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        encrypted_str = az_span_create(az_span_ptr(jr.token.slice), az_span_size(jr.token.slice));
        break;
      }
      AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    }
    AZ_ULIB_THROW_IF_AZ_ERROR(AZ_ULIB_TRY_RESULT);

    // Decrypt encrypted string.
    az_span decrypt_model_in_json = AZ_SPAN_FROM_BUFFER(buffer_2);
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_init(&jw, decrypt_model_in_json, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_begin_object(&jw));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR("src")));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_string(&jw, encrypted_str));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_end_object(&jw));
    decrypt_model_in_json = az_json_writer_get_bytes_used_in_destination(&jw);
    CLOSE_STRING_IN_SPAN(decrypt_model_in_json);

    az_span decrypt_model_out_json = AZ_SPAN_FROM_BUFFER(buffer_1);
    const az_span decrypt_command_name = AZ_SPAN_FROM_STR("decrypt");
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_call_w_str(
        _cipher_1, decrypt_command_name, decrypt_model_in_json, &decrypt_model_out_json));
    CLOSE_STRING_IN_SPAN(decrypt_model_out_json);
    (void)printf(
        "cipher.1 decrypted \"%s\" to \"%s\".\r\n",
        az_span_ptr(decrypt_model_in_json),
        az_span_ptr(decrypt_model_out_json));
  }
  AZ_ULIB_CATCH(...) { return AZ_ULIB_TRY_RESULT; }

  return AZ_OK;
}

void my_consumer_do_cipher(uint32_t context)
{
  (void)printf("My consumer try use cipher.1 interface... \r\n");

  get_handle_if_need();

  if (_cipher_1 != NULL)
  {
    AZ_ULIB_TRY
    {
      AZ_ULIB_THROW_IF_AZ_ERROR(call_wrappers(context));
      AZ_ULIB_THROW_IF_AZ_ERROR(call_w_str(context));
    }
    AZ_ULIB_CATCH(...)
    {
      switch (AZ_ULIB_TRY_RESULT)
      {
        case AZ_ERROR_NOT_SUPPORTED:
          (void)printf("cipher.1 does not support context %" PRIu32 ".\r\n", context);
          break;
        case AZ_ERROR_ITEM_NOT_FOUND:
          (void)printf("cipher.1 was uninstalled. Release the handle.\r\n");
          cipher_1_destroy(_cipher_1);
          _cipher_1 = NULL;
          break;
        default:
          (void)printf(
              "cipher.1 failed with error 0x%" PRIx32 ". Release the handle.\r\n",
              AZ_ULIB_TRY_RESULT);
          cipher_1_destroy(_cipher_1);
          _cipher_1 = NULL;
          break;
      }
    }
  }
}
