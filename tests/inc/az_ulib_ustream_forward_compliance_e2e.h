// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_USTREAM_FORWARD_COMPLIANCE_E2E_H
#define AZ_ULIB_USTREAM_FORWARD_COMPLIANCE_E2E_H

#include "az_ulib_ustream_mock_buffer.h"

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
 * @brief E2e test flush callback.
 */
typedef struct ustream_forward_basic_context
{
  offset_t offset;
  char buffer[100];
} consumer_context;

static az_result flush_callback(
    const uint8_t* const buffer,
    size_t size,
    az_ulib_callback_context flush_callback_context)
{
  // handle buffer
  consumer_context* flush_context = (consumer_context*)flush_callback_context;
  (void)snprintf(
      flush_context->buffer + flush_context->offset,
      sizeof(flush_context->buffer) / sizeof(char),
      "%s",
      buffer);

  // adjust offset
  flush_context->offset += size;

  return AZ_OK;
}
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
  size_t size_result;
  consumer_context test_consumer_context = { 0 };

  /// act
  az_result result_read = az_ulib_ustream_forward_read(
      ustream_forward, buf_result, USTREAM_FORWARD_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result);
  az_result result_flush
      = az_ulib_ustream_forward_flush(ustream_forward, flush_callback, &test_consumer_context);

  /// assert
  assert_int_equal(result_read, AZ_OK);
  assert_int_equal(size_result, USTREAM_FORWARD_COMPLIANCE_LENGTH_1);
  assert_memory_equal(USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

  assert_int_equal(result_flush, AZ_OK);
  assert_int_equal(test_consumer_context.offset, USTREAM_FORWARD_COMPLIANCE_LENGTH_2);
  assert_memory_equal(
      (const uint8_t* const)(
          USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_FORWARD_COMPLIANCE_LENGTH_1),
      test_consumer_context.buffer,
      size_result);

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
