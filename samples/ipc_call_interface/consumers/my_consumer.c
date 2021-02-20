// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_result.h"
#include "wrappers/cipher_1_wrapper.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

static az_ulib_ipc_interface_handle _cipher_1;
#define BUFFER_SIZE 100

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
      (void)printf("Get cypher.1 interface failed with code %d\r\n", result);
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

void my_consumer_do_cipher(uint32_t context)
{
  (void)printf("My consumer try use cipher.1 interface... \r\n");

  get_handle_if_need();

  if (_cipher_1 != NULL)
  {
    AZ_ULIB_TRY
    {
      const char src[] = "Welcome to Azure IoT Hackathon 2021!";
      const uint32_t src_size = sizeof(src) - 1;
      char encrypted_dst[BUFFER_SIZE];
      uint32_t encrypted_dst_size = 0;
      char dst[BUFFER_SIZE];
      uint32_t dst_size = 0;

      AZ_ULIB_THROW_IF_AZ_ERROR(cipher_1_encrypt(
          _cipher_1, context, src, src_size, BUFFER_SIZE, encrypted_dst, &encrypted_dst_size));
      (void)printf("cipher.1 encrypted \"%s\" with context %d.\"\r\n", src, context);

      AZ_ULIB_THROW_IF_AZ_ERROR(cipher_1_decrypt(
          _cipher_1, encrypted_dst, encrypted_dst_size, BUFFER_SIZE, dst, &dst_size));
      dst[dst_size] = '\0';
      (void)printf("cipher.1 decrypted \"%s\" with context %d.\r\n", dst, context);
    }
    AZ_ULIB_CATCH(...)
    {
      if (AZ_ULIB_TRY_RESULT == AZ_ERROR_ITEM_NOT_FOUND)
        (void)printf("cipher.1 was uninstalled.\r\n");
      else if (AZ_ULIB_TRY_RESULT == AZ_ERROR_NOT_SUPPORTED)
        (void)printf("cipher.1 does not support context %d.\r\n", context);
      else
        (void)printf("cipher.1.sum failed with error %d\r\n", AZ_ULIB_TRY_RESULT);

      (void)printf("Release the handle.\r\n");
      cipher_1_destroy(_cipher_1);
      _cipher_1 = NULL;
    }
  }
}
