// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "my_consumer.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_query_1_model.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"
#include "cipher_1_model.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#define BUFFER_SIZE 200 // Should be big enough to fit JSON input and output.
#define KEY_VAULT_PACKAGE_NAME "key_vault"

#define CLOSE_STRING_IN_SPAN(span)                                                           \
  do                                                                                         \
  {                                                                                          \
    az_span_ptr(span)[(az_span_size(span) < BUFFER_SIZE) ? az_span_size(span) : BUFFER_SIZE] \
        = '\0';                                                                              \
  } while (0)

void my_consumer_change_default(void)
{
  (void)printf("Set cipher.2 as default.\r\n");

  AZ_ULIB_TRY
  {
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_set_default(
        AZ_SPAN_FROM_STR(KEY_VAULT_PACKAGE_NAME),
        2,
        AZ_SPAN_FROM_STR(CIPHER_1_INTERFACE_NAME),
        CIPHER_1_INTERFACE_VERSION));
  }
  AZ_ULIB_CATCH(...)
  {
    (void)printf("Set default interface failed with code %" PRIi32 "\r\n", AZ_ULIB_TRY_RESULT);
  }
}

void my_consumer_query_interfaces(void)
{
  static az_ulib_ipc_interface_handle _ipc_handle = { 0 };
  AZ_ULIB_TRY
  {
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_try_get_interface(
        AZ_SPAN_EMPTY,
        AZ_SPAN_FROM_STR(IPC_1_PACKAGE_NAME),
        AZ_ULIB_VERSION_DEFAULT,
        AZ_SPAN_FROM_STR(QUERY_1_INTERFACE_NAME),
        QUERY_1_INTERFACE_VERSION,
        &_ipc_handle));

    uint8_t buf[BUFFER_SIZE];
    az_span interface_list = AZ_SPAN_FROM_BUFFER(buf);
    az_span query = AZ_SPAN_FROM_STR("");
    query_1_query_model_out out = { .result = &interface_list, .continuation_token = 0 };
    az_result result = az_ulib_ipc_call(_ipc_handle, QUERY_1_QUERY_COMMAND, &query, &out);

    while (result == AZ_OK)
    {
      uint32_t continuation_token = 0;
      char* name_str = (char*)az_span_ptr(interface_list);
      name_str[az_span_size(interface_list)] = '\0';
      (void)printf("Published interfaces [ %s ]\r\n", name_str);
      interface_list = AZ_SPAN_FROM_BUFFER(buf);
      result = az_ulib_ipc_call(_ipc_handle, QUERY_1_NEXT_COMMAND, &continuation_token, &out);
    }

    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_release_interface(_ipc_handle));
  }
  AZ_ULIB_CATCH(...)
  {
    (void)printf(
        "Get ipc.1.query.1 interface failed with code %" PRIi32 "\r\n", AZ_ULIB_TRY_RESULT);
  }
}

void my_consumer_create(void) { (void)printf("Create my consumer...\r\n"); }

void my_consumer_destroy(void) { (void)printf("Destroy my consumer\r\n"); }

static az_result call_wrappers(az_ulib_ipc_interface_handle handle, uint32_t algorithm)
{
  AZ_ULIB_TRY
  {
    const az_span original_span_to_encrypt = AZ_SPAN_FROM_STR("Welcome to Azure IoT with DCF!");
    char buffer_1[BUFFER_SIZE];
    char buffer_2[BUFFER_SIZE];

    cipher_1_alpha_model alpha = 0;
    cipher_1_delta_model delta = 0;
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_call(handle, CIPHER_1_ALPHA_PROPERTY, NULL, &alpha));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_call(handle, CIPHER_1_DELTA_PROPERTY, NULL, &delta));
    alpha = (cipher_1_alpha_model)(alpha - 5);
    delta += 10;
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_call(handle, CIPHER_1_ALPHA_PROPERTY, &alpha, &alpha));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_call(handle, CIPHER_1_DELTA_PROPERTY, &delta, &delta));
    (void)printf("cipher.1 alpha:%" PRId8 " and delta:%" PRIu32 ".\r\n", alpha, delta);

    az_span encrypt_out = AZ_SPAN_FROM_BUFFER(buffer_1);
    cipher_1_encrypt_model_in encrypt_in
        = { .algorithm = algorithm, .src = original_span_to_encrypt };
    AZ_ULIB_THROW_IF_AZ_ERROR(
        az_ulib_ipc_call(handle, CIPHER_1_ENCRYPT_COMMAND, &encrypt_in, &encrypt_out));

    CLOSE_STRING_IN_SPAN(encrypt_out);
    (void)printf(
        "cipher.1 encrypted \"%s\" to \"%s\" with algorithm %" PRIu32 ".\r\n",
        az_span_ptr(original_span_to_encrypt),
        az_span_ptr(encrypt_out),
        algorithm);

    az_span decrypt_out = AZ_SPAN_FROM_BUFFER(buffer_2);
    AZ_ULIB_THROW_IF_AZ_ERROR(
        az_ulib_ipc_call(handle, CIPHER_1_DECRYPT_COMMAND, &encrypt_out, &decrypt_out));

    CLOSE_STRING_IN_SPAN(decrypt_out);
    (void)printf(
        "cipher.1 decrypted \"%s\" to \"%s\" with algorithm %" PRIu32 ".\r\n",
        az_span_ptr(encrypt_out),
        az_span_ptr(decrypt_out),
        algorithm);
  }
  AZ_ULIB_CATCH(...) { return AZ_ULIB_TRY_RESULT; }

  return AZ_OK;
}

static az_result call_w_str(az_ulib_ipc_interface_handle handle, uint32_t algorithm)
{
  AZ_ULIB_TRY
  {
    char buffer_1[BUFFER_SIZE];
    char buffer_2[BUFFER_SIZE];

    // Encrypt string.
    az_json_writer jw;
    az_span encrypt_model_in_json = AZ_SPAN_FROM_BUFFER(buffer_1);
    az_span encrypt_model_out_json = AZ_SPAN_FROM_BUFFER(buffer_2);

    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_init(&jw, encrypt_model_in_json, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_begin_object(&jw));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_property_name(
        &jw, AZ_SPAN_FROM_STR(CIPHER_1_ENCRYPT_ALGORITHM_NAME)));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_int32(&jw, (int32_t)algorithm));
    AZ_ULIB_THROW_IF_AZ_ERROR(
        az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR(CIPHER_1_ENCRYPT_SRC_NAME)));
    AZ_ULIB_THROW_IF_AZ_ERROR(
        az_json_writer_append_string(&jw, AZ_SPAN_FROM_STR("Welcome to Azure IoT!")));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_end_object(&jw));
    encrypt_model_in_json = az_json_writer_get_bytes_used_in_destination(&jw);
    CLOSE_STRING_IN_SPAN(encrypt_model_in_json);

    az_ulib_capability_index capability_index;
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_try_get_capability(
        handle, AZ_SPAN_FROM_STR(CIPHER_1_ENCRYPT_COMMAND_NAME), &capability_index));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_call_with_str(
        handle, capability_index, encrypt_model_in_json, &encrypt_model_out_json));
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
      if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR(CIPHER_1_ENCRYPT_DEST_NAME)))
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
    AZ_ULIB_THROW_IF_AZ_ERROR(
        az_json_writer_append_property_name(&jw, AZ_SPAN_FROM_STR(CIPHER_1_DECRYPT_SRC_NAME)));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_string(&jw, encrypted_str));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_writer_append_end_object(&jw));
    decrypt_model_in_json = az_json_writer_get_bytes_used_in_destination(&jw);
    CLOSE_STRING_IN_SPAN(decrypt_model_in_json);

    az_span decrypt_model_out_json = AZ_SPAN_FROM_BUFFER(buffer_1);
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_try_get_capability(
        handle, AZ_SPAN_FROM_STR(CIPHER_1_DECRYPT_COMMAND_NAME), &capability_index));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_call_with_str(
        handle, capability_index, decrypt_model_in_json, &decrypt_model_out_json));
    CLOSE_STRING_IN_SPAN(decrypt_model_out_json);
    (void)printf(
        "cipher.1 decrypted \"%s\" to \"%s\".\r\n",
        az_span_ptr(decrypt_model_in_json),
        az_span_ptr(decrypt_model_out_json));
  }
  AZ_ULIB_CATCH(...) { return AZ_ULIB_TRY_RESULT; }

  return AZ_OK;
}

void my_consumer_do_cipher(uint32_t algorithm)
{
  (void)printf("My consumer try use cipher.1 interface... \r\n");

  static az_ulib_ipc_interface_handle _cipher_handle = { 0 };

  az_result result = az_ulib_ipc_try_get_interface(
      AZ_SPAN_EMPTY,
      AZ_SPAN_FROM_STR(KEY_VAULT_PACKAGE_NAME),
      AZ_ULIB_VERSION_DEFAULT,
      AZ_SPAN_FROM_STR(CIPHER_1_INTERFACE_NAME),
      CIPHER_1_INTERFACE_VERSION,
      &_cipher_handle);

  if (!AZ_ULIB_IS_AZ_ERROR(result))
  {
    AZ_ULIB_TRY
    {
      AZ_ULIB_THROW_IF_AZ_ERROR(call_wrappers(_cipher_handle, algorithm));
      AZ_ULIB_THROW_IF_AZ_ERROR(call_w_str(_cipher_handle, algorithm));
    }
    AZ_ULIB_CATCH(...)
    {
      if (AZ_ULIB_TRY_RESULT == AZ_ERROR_NOT_SUPPORTED)
      {
        (void)printf("cipher.1 does not support algorithm %" PRIu32 ".\r\n", algorithm);
      }
      else
      {
        (void)printf("cipher.1 failed with error 0x%" PRIx32 ".\r\n", AZ_ULIB_TRY_RESULT);
      }
    }

    result = az_ulib_ipc_release_interface(_cipher_handle);
    (void)result;
  }
  else if (result == AZ_ERROR_ITEM_NOT_FOUND)
  {
    (void)printf("cipher.1 is not available.\r\n");
  }
  else
  {
    (void)printf("Get cipher.1 interface failed with code %" PRIi32 "\r\n", result);
  }
}
