// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_USTREAM_FORWARD_COMPLIANCE_E2E_H
#define AZ_ULIB_USTREAM_FORWARD_COMPLIANCE_E2E_H

#include "az_ulib_ustream_mock_buffer.h"
#include "azure/core/az_span.h"

#include "cmocka.h"

#ifdef __cplusplus
#include <cstdio>
extern "C"
{
#else
#include <stdio.h>
#endif /* __cplusplus */

/* check for test artifacts. */
#ifndef USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH
#error "USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH not defined"
#endif

#ifndef USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY
#error "USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY not defined"
#endif

#if (USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH / 20 == 0)
#error "USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH must be at least 20 uint8_t long"
#endif

/* split the content in 2 parts. */
#define USTREAM_FORWARD_COMPLIANCE_LENGTH_1 (USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH / 2)
#define USTREAM_FORWARD_COMPLIANCE_LENGTH_2 \
  (USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_FORWARD_COMPLIANCE_LENGTH_1)

/* create local buffer with enough size to handle the full content. */
#define USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH \
  (USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH / 2)

  static const uint8_t* const compliance_expected_content
      = (const uint8_t*)USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT;

  /**
   * Start compliance tests.
   */

  /**
   * In the event that read does not completely traverse the buffer, flush shall pick up where read
   * left off and successfully hand off the remaining buffer to the flush callback.
   */
  static void az_ulib_ustream_forward_e2e_compliance_read_flush_succeed(void** state)
  {
    /// arrange
    (void)state;
    az_ulib_ustream_forward* ustream_forward;
    USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(&ustream_forward);
    uint8_t buf_result[USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH];
    az_span span_read_copy = AZ_SPAN_FROM_BUFFER(buf_result);
    size_t size_result_read_copy;
    az_span span_read_direct = AZ_SPAN_EMPTY;
    size_t size_result_read_direct;

    /// act
    az_result result_read_copy
        = az_ulib_ustream_forward_read(ustream_forward, &span_read_copy, &size_result_read_copy);
    az_result result_read_direct = az_ulib_ustream_forward_read(
        ustream_forward, &span_read_direct, &size_result_read_direct);

    /// assert
    assert_int_equal(result_read_copy, AZ_OK);
    assert_int_equal(size_result_read_copy, USTREAM_FORWARD_COMPLIANCE_LENGTH_1);
    assert_memory_equal(
        USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
        az_span_ptr(span_read_copy),
        size_result_read_copy);

    assert_int_equal(result_read_direct, AZ_OK);
    assert_int_equal(size_result_read_direct, USTREAM_FORWARD_COMPLIANCE_LENGTH_2);
    assert_memory_equal(
        (const uint8_t* const)(
            USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT
            + USTREAM_FORWARD_COMPLIANCE_LENGTH_1),
        az_span_ptr(span_read_direct),
        size_result_read_direct);

    /// cleanup
    az_result result = az_ulib_ustream_forward_dispose(ustream_forward);
    (void)result;
  }

#define AZ_ULIB_USTREAM_FORWARD_COMPLIANCE_E2E_LIST \
  cmocka_unit_test(az_ulib_ustream_forward_e2e_compliance_read_flush_succeed)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZ_ULIB_USTREAM_FORWARD_COMPLIANCE_E2E_H */
