// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_test_helpers.h"

#include "cmocka.h"

void check_buffer(
    az_ulib_ustream* ustream_instance,
    uint8_t offset,
    const uint8_t* const expected_content,
    uint8_t expected_content_length)
{
  uint8_t buf_result[256];
  size_t size_result;

  if (offset < expected_content_length)
  {
    assert_int_equal(az_ulib_ustream_read(ustream_instance, buf_result, 256, &size_result), AZ_OK);

    assert_int_equal(size_result, expected_content_length - offset);
    assert_memory_equal((const uint8_t* const)(expected_content + offset), buf_result, size_result);
  }

  size_result = 10;
  assert_int_equal(
      az_ulib_ustream_read(ustream_instance, buf_result, 256, &size_result), AZ_ULIB_EOF);
  assert_int_equal(size_result, 0);
}

void check_ustream_forward_buffer(
    az_ulib_ustream_forward* ustream_forward,
    uint8_t offset,
    const uint8_t* const expected_content,
    uint8_t expected_content_length)
{
  uint8_t buf_result[256];
  size_t size_result;

  if (offset < expected_content_length)
  {
    assert_int_equal(az_ulib_ustream_forward_read(ustream_forward, buf_result, 256, &size_result), AZ_OK);

    assert_int_equal(size_result, expected_content_length - offset);
    assert_memory_equal((const uint8_t* const)(expected_content + offset), buf_result, size_result);
  }

  size_result = 10;
  assert_int_equal(
      az_ulib_ustream_forward_read(ustream_forward, buf_result, 256, &size_result), AZ_ULIB_EOF);
  assert_int_equal(size_result, 0);
}