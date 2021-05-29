// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "cipher_v2i1.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"
#include "interfaces/cipher_v2i1_interface.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#define NUMBER_OF_KEYS 2
#define KEY_SIZE 21
static const uint8_t key[NUMBER_OF_KEYS][KEY_SIZE]
    = { "12345678912345678901", "h948kfd--fsd{jfh}l2D" };

#define splitInt(intVal, bytePos) (char)((intVal >> (bytePos << 3)) & 0xFF)
#define joinChars(a, b, c, d) \
  (uint32_t)((uint32_t)a + ((uint32_t)b << 8) + ((uint32_t)c << 16) + ((uint32_t)d << 24))

static int8_t alpha = -3;
int8_t cipher_v2i1_alpha_get(void) { return alpha; }
void cipher_v2i1_alpha_set(const int8_t val) { alpha = val; }

static uint32_t delta = 8;
uint32_t cipher_v2i1_delta_get(void) { return delta; }
void cipher_v2i1_delta_set(const uint32_t val)
{
  delta = val;
  while (delta >= KEY_SIZE)
  {
    delta -= KEY_SIZE;
  }
}

void cipher_v2i1_create(void)
{
  az_result result;

  (void)printf("Create producer for cipher v2i1...\r\n");

  if ((result = publish_cipher_v2i1_interface()) != AZ_OK)
  {
    (void)printf("Publish interface cipher 1 failed with error %" PRIi32 "\r\n", result);
  }
  else
  {
    (void)printf("Interface cipher 1 published with success\r\n");
  }
}

void cipher_v2i1_destroy(void)
{
  (void)printf("Destroy producer for cipher 1.\r\n");

  unpublish_cipher_v2i1_interface();
}

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

static char base64char(uint8_t val)
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

static char base64b16(uint8_t val)
{
  const uint32_t base64b16values[4] = { joinChars('A', 'E', 'I', 'M'),
                                        joinChars('Q', 'U', 'Y', 'c'),
                                        joinChars('g', 'k', 'o', 's'),
                                        joinChars('w', '0', '4', '8') };
  return splitInt(base64b16values[val >> 2], (val & 0x03));
}

static char base64b8(uint8_t val)
{
  const uint32_t base64b8values = joinChars('A', 'Q', 'g', 'w');
  return splitInt(base64b8values, val);
}

static int base64toValue(char base64character, uint8_t* value)
{
  int result = 0;
  if (('A' <= base64character) && (base64character <= 'Z'))
  {
    *value = (uint8_t)(base64character - 'A');
  }
  else if (('a' <= base64character) && (base64character <= 'z'))
  {
    *value = (uint8_t)(('Z' - 'A') + 1 + (base64character - 'a'));
  }
  else if (('0' <= base64character) && (base64character <= '9'))
  {
    *value = (uint8_t)(('Z' - 'A') + 1 + ('z' - 'a') + 1 + (base64character - '0'));
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
  uint8_t junkChar;
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
az_result cipher_v2i1_encrypt(uint32_t algorithm, az_span src, az_span* dest)
{
  AZ_ULIB_TRY
  {
    AZ_ULIB_THROW_IF_ERROR((algorithm < NUMBER_OF_KEYS), AZ_ERROR_NOT_SUPPORTED);
    AZ_ULIB_THROW_IF_ERROR((encoded_len(src) <= az_span_size(*dest)), AZ_ERROR_NOT_ENOUGH_SPACE);

    int8_t a = 0;
    uint32_t key_pos = 0;
    if (algorithm != 0)
    {
      a = alpha;
      key_pos = delta;
    }

    char* dest_str = (char*)az_span_ptr(*dest);

    int32_t src_size = az_span_size(src);
    uint8_t* src_str = az_span_ptr(src);

    dest_str[0] = (char)(algorithm + '0');

    int32_t destinationPosition = 1;
    int32_t currentPosition = 0;
    uint8_t src_char[3];
    while (src_size - currentPosition >= 3)
    {
      src_char[0] = src_str[currentPosition] ^ (uint8_t)(key[algorithm][key_pos] + a);
      key_pos = next_key_pos(key_pos);
      src_char[1] = src_str[currentPosition + 1] ^ (uint8_t)(key[algorithm][key_pos] + a);
      key_pos = next_key_pos(key_pos);
      src_char[2] = src_str[currentPosition + 2] ^ (uint8_t)(key[algorithm][key_pos] + a);
      key_pos = next_key_pos(key_pos);

      dest_str[destinationPosition++] = base64char((uint8_t)(src_char[0] >> 2));
      dest_str[destinationPosition++]
          = base64char((uint8_t)(((src_char[0] & 3) << 4) | (src_char[1] >> 4)));
      dest_str[destinationPosition++]
          = base64char((uint8_t)(((src_char[1] & 0x0F) << 2) | ((src_char[2] >> 6) & 3)));
      dest_str[destinationPosition++] = base64char((uint8_t)(src_char[2] & 0x3F));
      currentPosition += 3;
    }
    if (src_size - currentPosition == 2)
    {
      src_char[0] = src_str[currentPosition] ^ (uint8_t)(key[algorithm][key_pos] + a);
      key_pos = next_key_pos(key_pos);
      src_char[1] = src_str[currentPosition + 1] ^ (uint8_t)(key[algorithm][key_pos] + a);
      dest_str[destinationPosition++] = base64char(src_char[0] >> 2);
      dest_str[destinationPosition++]
          = base64char((uint8_t)(((src_char[0] & 0x03) << 4) | (src_char[1] >> 4)));
      dest_str[destinationPosition++] = base64b16(src_char[1] & 0x0F);
      dest_str[destinationPosition++] = '=';
    }
    else if (src_size - currentPosition == 1)
    {
      src_char[0] = src_str[currentPosition] ^ (uint8_t)(key[algorithm][key_pos] + a);
      dest_str[destinationPosition++] = base64char(src_char[0] >> 2);
      dest_str[destinationPosition++] = base64b8(src_char[0] & 0x03);
      dest_str[destinationPosition++] = '=';
      dest_str[destinationPosition++] = '=';
    }

    dest_str[destinationPosition] = '\0';

    *dest = az_span_create((uint8_t*)dest_str, destinationPosition);
  }
  AZ_ULIB_CATCH(...) { return AZ_ULIB_TRY_RESULT; }

  return AZ_OK;
}

az_result cipher_v2i1_decrypt(az_span src, az_span* dest)
{
  AZ_ULIB_TRY
  {
    char* src_str = (char*)az_span_ptr(src);
    int32_t src_size = az_span_size(src);

    uint8_t* dest_str = az_span_ptr(*dest);
    int32_t dest_size = az_span_size(*dest);

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
      uint8_t c1;
      uint8_t c2;
      uint8_t c3;
      uint8_t c4;
      (void)base64toValue(src_str[indexOfFirstEncodedChar], &c1);
      (void)base64toValue(src_str[indexOfFirstEncodedChar + 1], &c2);
      (void)base64toValue(src_str[indexOfFirstEncodedChar + 2], &c3);
      (void)base64toValue(src_str[indexOfFirstEncodedChar + 3], &c4);
      dest_str[decodedIndex++] = (uint8_t)((c1 << 2) | (c2 >> 4));
      dest_str[decodedIndex++] = (uint8_t)(((c2 & 0x0f) << 4) | (c3 >> 2));
      dest_str[decodedIndex++] = (uint8_t)(((c3 & 0x03) << 6) | c4);
      numberOfEncodedChars -= 4;
      indexOfFirstEncodedChar += 4;
    }
    if (numberOfEncodedChars == 2)
    {
      uint8_t c1;
      uint8_t c2;
      (void)base64toValue(src_str[indexOfFirstEncodedChar], &c1);
      (void)base64toValue(src_str[indexOfFirstEncodedChar + 1], &c2);
      dest_str[decodedIndex++] = (uint8_t)((c1 << 2) | (c2 >> 4));
    }
    else if (numberOfEncodedChars == 3)
    {
      uint8_t c1;
      uint8_t c2;
      uint8_t c3;
      (void)base64toValue(src_str[indexOfFirstEncodedChar], &c1);
      (void)base64toValue(src_str[indexOfFirstEncodedChar + 1], &c2);
      (void)base64toValue(src_str[indexOfFirstEncodedChar + 2], &c3);
      dest_str[decodedIndex++] = (uint8_t)((c1 << 2) | (c2 >> 4));
      dest_str[decodedIndex++] = (uint8_t)(((c2 & 0x0f) << 4) | (c3 >> 2));
    }

    int8_t a = 0;
    uint32_t key_pos = 0;
    if (algorithm != 0)
    {
      a = alpha;
      key_pos = delta;
    }

    for (int32_t i = 0; i < decodedIndex; i++)
    {
      dest_str[i] ^= (uint8_t)(key[algorithm][key_pos] + a);
      key_pos = next_key_pos(key_pos);
    }

    dest_str[decodedIndex] = '\0';

    *dest = az_span_create(dest_str, decodedIndex);
  }
  AZ_ULIB_CATCH(...) { return AZ_ULIB_TRY_RESULT; }

  return AZ_OK;
}
