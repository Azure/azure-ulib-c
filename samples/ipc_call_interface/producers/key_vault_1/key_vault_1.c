// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "key_vault_1.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"
#include "cipher_1_capabilities.h"
#include "cipher_1_model.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#define NUMBER_OF_KEYS 1
#define KEY_SIZE 21

#define KEY_VAULT_PACKAGE_NAME "key_vault"
#define KEY_VAULT_PACKAGE_VERSION 1

static const az_ulib_interface_descriptor CIPHER_1_DESCRIPTOR = AZ_ULIB_DESCRIPTOR_CREATE(
    KEY_VAULT_PACKAGE_NAME,
    KEY_VAULT_PACKAGE_VERSION,
    CIPHER_1_INTERFACE_NAME,
    CIPHER_1_INTERFACE_VERSION,
    CIPHER_1_CAPABILITIES);

static const char key[NUMBER_OF_KEYS][KEY_SIZE] = { "12345678912345678901" };

#define splitInt(intVal, bytePos) (char)((intVal >> (bytePos << 3)) & 0xFF)
#define joinChars(a, b, c, d) \
  (uint32_t)((uint32_t)a + ((uint32_t)b << 8) + ((uint32_t)c << 16) + ((uint32_t)d << 24))

typedef struct
{
  int8_t alpha;
  uint32_t delta;
} key_vault_cb;

static key_vault_cb cb = { 0 };

static inline uint32_t next_key_pos(uint32_t cur)
{
  uint32_t next = cur + 1;
  if (next == KEY_SIZE)
  {
    next = 0;
  }
  return next;
}

static int32_t encoded_len(az_span src)
{
  int32_t size = az_span_size(src);
  // Base 64 uses 4 characters to represent each set of 3 bytes,
  //   so the final size is 4/3 of the original size.
  // Round it up and add '=' at the end to make the final
  //   size divisible by 4.
  return (size == 0) ? (0) : (((((size - 1) / 3) + 1) * 4) + 1) + 1;
}

static int32_t decoded_len(const char* encodedString, int32_t size)
{
  int32_t result;
  int32_t sourceLength = size;

  if (sourceLength == 0)
  {
    result = 0;
  }
  else
  {
    result = sourceLength / 4 * 3;
    if (encodedString[sourceLength - 1] == '=')
    {
      if (encodedString[sourceLength - 2] == '=')
      {
        result--;
      }
      result--;
    }
  }
  return result;
}

static char base64char(unsigned char val)
{
  char result;

  if (val < 26)
  {
    result = (char)(val + 'A');
  }
  else if (val < 52)
  {
    result = (char)(val - 26 + 'a');
  }
  else if (val < 62)
  {
    result = (char)(val - 52 + '0');
  }
  else if (val == 62)
  {
    result = (char)('+');
  }
  else
  {
    result = (char)('/');
  }

  return result;
}

static char base64b16(unsigned char val)
{
  const uint32_t base64b16values[4] = { joinChars('A', 'E', 'I', 'M'),
                                        joinChars('Q', 'U', 'Y', 'c'),
                                        joinChars('g', 'k', 'o', 's'),
                                        joinChars('w', '0', '4', '8') };
  return splitInt(base64b16values[val >> 2], (val & 0x03));
}

static char base64b8(unsigned char val)
{
  const uint32_t base64b8values = joinChars('A', 'Q', 'g', 'w');
  return splitInt(base64b8values, val);
}

static int base64toValue(char base64character, unsigned char* value)
{
  int result = 0;
  if (('A' <= base64character) && (base64character <= 'Z'))
  {
    *value = (unsigned char)(base64character - 'A');
  }
  else if (('a' <= base64character) && (base64character <= 'z'))
  {
    *value = (unsigned char)(('Z' - 'A') + 1 + (base64character - 'a'));
  }
  else if (('0' <= base64character) && (base64character <= '9'))
  {
    *value = (unsigned char)(('Z' - 'A') + 1 + ('z' - 'a') + 1 + (base64character - '0'));
  }
  else if ('+' == base64character)
  {
    *value = 62;
  }
  else if ('/' == base64character)
  {
    *value = 63;
  }
  else
  {
    *value = 0;
    result = -1;
  }
  return result;
}

static size_t numberOfBase64Characters(const char* encodedString)
{
  size_t length = 0;
  unsigned char junkChar;
  while (base64toValue(encodedString[length], &junkChar) != -1)
  {
    length++;
  }
  return length;
}

/*
 * This is a simple encrypt algorithm that use an Exclusive or of the data with
 * a key and encode the result in base-64 so we can send over IoTHub.
 *
 * The first char in the encrypted data represent the algorithm, which is, at the end
 * the key used in the encryption process.
 */
static az_result cipher_1_encrypt_concrete(
    const cipher_1_encrypt_model_in* const in,
    cipher_1_encrypt_model_out* out)
{
  AZ_ULIB_TRY
  {
    AZ_ULIB_THROW_IF_ERROR((in->algorithm < NUMBER_OF_KEYS), AZ_ERROR_NOT_SUPPORTED);
    AZ_ULIB_THROW_IF_ERROR((encoded_len(in->src) <= az_span_size(*out)), AZ_ERROR_NOT_ENOUGH_SPACE);

    uint32_t key_pos = 0;
    char* dest_str = (char*)az_span_ptr(*out);

    int32_t src_size = az_span_size(in->src);
    char* src_str = (char*)az_span_ptr(in->src);

    dest_str[0] = (char)(in->algorithm + '0');

    int32_t destinationPosition = 1;
    int32_t currentPosition = 0;
    char src_char[3];
    while (src_size - currentPosition >= 3)
    {
      src_char[0] = src_str[currentPosition] ^ key[in->algorithm][key_pos];
      key_pos = next_key_pos(key_pos);
      src_char[1] = src_str[currentPosition + 1] ^ key[in->algorithm][key_pos];
      key_pos = next_key_pos(key_pos);
      src_char[2] = src_str[currentPosition + 2] ^ key[in->algorithm][key_pos];
      key_pos = next_key_pos(key_pos);

      char c1 = base64char((unsigned char)(src_char[0] >> 2));
      char c2 = base64char((unsigned char)(((src_char[0] & 3) << 4) | (src_char[1] >> 4)));
      char c3 = base64char((unsigned char)(((src_char[1] & 0x0F) << 2) | ((src_char[2] >> 6) & 3)));
      char c4 = base64char((unsigned char)(src_char[2] & 0x3F));
      currentPosition += 3;
      dest_str[destinationPosition++] = c1;
      dest_str[destinationPosition++] = c2;
      dest_str[destinationPosition++] = c3;
      dest_str[destinationPosition++] = c4;
    }
    if (src_size - currentPosition == 2)
    {
      src_char[0] = src_str[currentPosition] ^ key[in->algorithm][key_pos];
      key_pos = next_key_pos(key_pos);
      src_char[1] = src_str[currentPosition + 1] ^ key[in->algorithm][key_pos];
      char c1 = base64char((unsigned char)(src_char[0] >> 2));
      char c2 = base64char((unsigned char)(((src_char[0] & 0x03) << 4) | (src_char[1] >> 4)));
      char c3 = base64b16(src_char[1] & 0x0F);
      dest_str[destinationPosition++] = c1;
      dest_str[destinationPosition++] = c2;
      dest_str[destinationPosition++] = c3;
      dest_str[destinationPosition++] = '=';
    }
    else if (src_size - currentPosition == 1)
    {
      src_char[0] = src_str[currentPosition] ^ key[in->algorithm][key_pos];
      char c1 = base64char((unsigned char)(src_char[0] >> 2));
      char c2 = base64b8(src_char[0] & 0x03);
      dest_str[destinationPosition++] = c1;
      dest_str[destinationPosition++] = c2;
      dest_str[destinationPosition++] = '=';
      dest_str[destinationPosition++] = '=';
    }

    dest_str[destinationPosition] = '\0';

    *out = az_span_create((uint8_t*)dest_str, destinationPosition);
  }
  AZ_ULIB_CATCH(...) { return AZ_ULIB_TRY_RESULT; }

  return AZ_OK;
}

static az_result cipher_1_decrypt_concrete(
    const cipher_1_decrypt_model_in* const in,
    cipher_1_decrypt_model_out* out)
{
  AZ_ULIB_TRY
  {
    char* src_str = (char*)az_span_ptr(*in);
    int32_t src_size = az_span_size(*in);

    char* dest_str = (char*)az_span_ptr(*out);
    int32_t dest_size = az_span_size(*out);

    AZ_ULIB_THROW_IF_ERROR((src_size > 1), AZ_ERROR_ARG);

    uint32_t algorithm = (uint32_t)(src_str[0] - '0');
    AZ_ULIB_THROW_IF_ERROR((algorithm < NUMBER_OF_KEYS), AZ_ERROR_NOT_SUPPORTED);

    AZ_ULIB_THROW_IF_ERROR(
        (decoded_len(&src_str[1], src_size) <= dest_size), AZ_ERROR_NOT_ENOUGH_SPACE);

    size_t numberOfEncodedChars;
    size_t indexOfFirstEncodedChar;
    int32_t decodedIndex;

    numberOfEncodedChars = numberOfBase64Characters(&src_str[1]);
    indexOfFirstEncodedChar = 1;
    decodedIndex = 0;
    while (numberOfEncodedChars >= 4)
    {
      unsigned char c1;
      unsigned char c2;
      unsigned char c3;
      unsigned char c4;
      (void)base64toValue(src_str[indexOfFirstEncodedChar], &c1);
      (void)base64toValue(src_str[indexOfFirstEncodedChar + 1], &c2);
      (void)base64toValue(src_str[indexOfFirstEncodedChar + 2], &c3);
      (void)base64toValue(src_str[indexOfFirstEncodedChar + 3], &c4);
      dest_str[decodedIndex] = (char)((c1 << 2) | (c2 >> 4));
      decodedIndex++;
      dest_str[decodedIndex] = (char)(((c2 & 0x0f) << 4) | (c3 >> 2));
      decodedIndex++;
      dest_str[decodedIndex] = (char)(((c3 & 0x03) << 6) | c4);
      decodedIndex++;
      numberOfEncodedChars -= 4;
      indexOfFirstEncodedChar += 4;
    }
    if (numberOfEncodedChars == 2)
    {
      unsigned char c1;
      unsigned char c2;
      (void)base64toValue(src_str[indexOfFirstEncodedChar], &c1);
      (void)base64toValue(src_str[indexOfFirstEncodedChar + 1], &c2);
      dest_str[decodedIndex++] = (char)((c1 << 2) | (c2 >> 4));
    }
    else if (numberOfEncodedChars == 3)
    {
      unsigned char c1;
      unsigned char c2;
      unsigned char c3;
      (void)base64toValue(src_str[indexOfFirstEncodedChar], &c1);
      (void)base64toValue(src_str[indexOfFirstEncodedChar + 1], &c2);
      (void)base64toValue(src_str[indexOfFirstEncodedChar + 2], &c3);
      dest_str[decodedIndex++] = (char)((c1 << 2) | (c2 >> 4));
      dest_str[decodedIndex++] = (char)(((c2 & 0x0f) << 4) | (c3 >> 2));
    }

    uint32_t key_pos = 0;
    for (int32_t i = 0; i < decodedIndex; i++)
    {
      dest_str[i] ^= key[algorithm][key_pos];
      key_pos = next_key_pos(key_pos);
    }

    dest_str[decodedIndex] = '\0';

    *out = az_span_create((uint8_t*)dest_str, decodedIndex);
  }
  AZ_ULIB_CATCH(...) { return AZ_ULIB_TRY_RESULT; }

  return AZ_OK;
}

static az_result cipher_1_alpha_concrete(
    const cipher_1_alpha_model* const in,
    cipher_1_alpha_model* out)
{
  if (in != NULL)
  {
    cb.alpha = *in;
  }

  if (out != NULL)
  {
    *out = cb.alpha;
  }

  return AZ_OK;
}

static az_result cipher_1_delta_concrete(
    const cipher_1_delta_model* const in,
    cipher_1_delta_model* out)
{
  if (in != NULL)
  {
    cb.delta = *in;
  }

  if (out != NULL)
  {
    *out = cb.delta;
  }

  return AZ_OK;
}

void key_vault_1_create(void)
{
  az_result result;

  (void)printf("Create producer for key_vault.1...\r\n");

  if ((result = az_ulib_ipc_publish(&CIPHER_1_DESCRIPTOR)) != AZ_OK)
  {
    (void)printf("Publish interface cipher.1 failed with error %" PRIi32 "\r\n", result);
  }
  else
  {
    (void)printf("Interface cipher.1 published with success\r\n");
  }
}

void key_vault_1_destroy(void)
{
  az_result result;

  if ((result = az_ulib_ipc_unpublish(&CIPHER_1_DESCRIPTOR, AZ_ULIB_NO_WAIT)) != AZ_OK)
  {
    (void)printf(
        "Unpublish interface cipher.1 in key_vault.1 failed with error %" PRIi32 "\r\n", result);
  }
  else
  {
    (void)printf("Destroy producer for key_vault.1.\r\n");
  }
}
