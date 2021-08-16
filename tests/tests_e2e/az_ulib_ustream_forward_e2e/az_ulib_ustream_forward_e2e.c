// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "az_ulib_ustream_forward.h"
#include "az_ulib_ustream_forward_e2e.h"

#include "az_ulib_ustream_forward_mock_buffer.h"

#include "cmocka.h"

/* define constants for the compliance test */
#define USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT \
  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62

static const uint8_t* const USTREAM_FORWARD_COMPLIANCE_LOCAL_EXPECTED_CONTENT
    = (const uint8_t* const)USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT;

static void ustream_forward_factory(az_ulib_ustream_forward** ustream_forward)
{
  *ustream_forward
      = (az_ulib_ustream_forward*)malloc(sizeof(az_ulib_ustream_forward));
  uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t) * USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  assert_non_null(buf);
  (void)memcpy(
      buf, USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT, USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
  assert_int_equal(
      az_ulib_ustream_forward_init(
          *ustream_forward,
          free,
          buf,
          USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
          free),
      AZ_OK);
}
#define USTREAM_FORWARD_COMPLIANCE_TARGET_FACTORY(ustream_forward) ustream_forward_factory(ustream_forward)

#define TEST_CONST_BUFFER_LENGTH (USTREAM_FORWARD_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 2)
#define TEST_CONST_MAX_BUFFER_SIZE (TEST_CONST_BUFFER_LENGTH - 1)

/**
 * Beginning of the e2e for ustream_forward.c on ownership model.
 */

// Run e2e compliance tests for ustream_forward
#include "az_ulib_ustream_forward_compliance_e2e.h"

int az_ulib_ustream_forward_e2e()
{
  const struct CMUnitTest tests[] = { AZ_ULIB_USTREAM_FORWARD_COMPLIANCE_E2E_LIST };

  return cmocka_run_group_tests_name("az_ulib_ustream_forward_e2e", tests, NULL, NULL);
}
