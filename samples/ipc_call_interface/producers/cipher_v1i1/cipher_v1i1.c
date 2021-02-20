// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_result.h"
#include "interfaces/cipher_v1i1_interface.h"
#include <stdio.h>

#define NUMBER_OF_KEYS 1
#define KEY_SIZE 20
static const char key[NUMBER_OF_KEYS][KEY_SIZE] = { "12345678912345678901" };

void cipher_v1i1_create(void)
{
  az_result result;

  (void)printf("Create producer for cipher v1i1...\r\n");

  if ((result = publish_cipher_v1i1_interface()) != AZ_OK)
  {
    (void)printf("Publish interface cipher 1 failed with error %d\r\n", result);
  }
  else
  {
    (void)printf("Interface cipher 1 published with success\r\n");
  }
}

void cipher_v1i1_destroy(void)
{
  (void)printf("Destroy producer for cipher 1.\r\n");

  unpublish_cipher_v1i1_interface();
}

az_result cipher_v1i1_encrypt(
    uint32_t context,
    const char* const src,
    uint32_t src_size,
    uint32_t dst_buffer_size,
    char* dst,
    uint32_t* dst_size)
{
  AZ_ULIB_TRY
  {
    AZ_ULIB_THROW_IF_ERROR((context < NUMBER_OF_KEYS), AZ_ERROR_NOT_SUPPORTED);
    AZ_ULIB_THROW_IF_ERROR((src_size + 1 <= dst_buffer_size), AZ_ERROR_NOT_ENOUGH_SPACE);

    uint32_t key_pos = 0;

    dst[0] = context + '0';

    for (uint32_t i = 0; i < src_size; i++)
    {
      dst[i + 1] = src[i] ^ key[context][key_pos];
      key_pos++;
      if (key_pos == KEY_SIZE)
        key_pos = 0;
    }

    *dst_size = src_size + 1;
  }
  AZ_ULIB_CATCH(...) { return AZ_ULIB_TRY_RESULT; }

  return AZ_OK;
}

az_result cipher_v1i1_decrypt(
    const char* const src,
    uint32_t src_size,
    uint32_t dst_buffer_size,
    char* dst,
    uint32_t* dst_size)
{
  AZ_ULIB_TRY
  {
    AZ_ULIB_THROW_IF_ERROR((src_size > 1), AZ_ERROR_ARG);
    uint32_t context = src[0] - '0';
    AZ_ULIB_THROW_IF_ERROR((context < NUMBER_OF_KEYS), AZ_ERROR_NOT_SUPPORTED);
    AZ_ULIB_THROW_IF_ERROR((context >= 0), AZ_ERROR_ARG);
    AZ_ULIB_THROW_IF_ERROR((src_size <= dst_buffer_size), AZ_ERROR_NOT_ENOUGH_SPACE);

    uint32_t key_pos = 0;
    for (uint32_t i = 0; i < src_size - 1; i++)
    {
      dst[i] = src[i + 1] ^ key[context][key_pos];
      key_pos++;
      if (key_pos == KEY_SIZE)
        key_pos = 0;
    }

    *dst_size = src_size - 1;
  }
  AZ_ULIB_CATCH(...) { return AZ_ULIB_TRY_RESULT; }

  return AZ_OK;
}