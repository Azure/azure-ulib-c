// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_CTEST_AUX_H
#define AZ_ULIB_CTEST_AUX_H

#include <stdint.h>

#include "az_ulib_ustream_base.h"

#include "cmocka.h"

/**
 * Check buffer
 */
static void check_buffer(
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

#endif /* AZ_ULIB_CTEST_AUX_H */
