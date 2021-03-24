// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "az_ulib_ustream.h"
#include "az_ulib_ustream_base.h"
/*TODO:<insert your header file here for testing>*/

#include "az_ulib_ustream_mock_buffer.h"

#include "cmocka.h"

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT /*TODO:<insert your test content here (must be at \
                                               least 20 uint8_t long)>*/
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH /*TODO:<insert the length of your test content \
                                                      here>*/

static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT
    = (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;
static void ustream_factory(az_ulib_ustream* ustream)
{
  /*
      TODO:<Insert your code here which initializes the ustream parameter with your implementation>
  */
}
#define USTREAM_COMPLIANCE_TARGET_FACTORY(ustream) ustream_factory(ustream)

#define TEST_CONST_BUFFER_LENGTH (USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 2)
#define TEST_CONST_MAX_BUFFER_SIZE (TEST_CONST_BUFFER_LENGTH - 1)

/**
 * Beginning of the E2E for your ustream implementation.
 * TODO:<Add your own set of tests here.>
 */
static void my_ustream_e2e_test1_succeed(void** state)
{
  /// arrange

  /// act

  /// assert

  /// cleanup
}

static void my_ustream_e2e_test2_succeed(void** state)
{
  /// arrange

  /// act

  /// assert

  /// cleanup
}

/**
 * This include, together with #AZ_ULIB_USTREAM_COMPLIANCE_E2E_LIST, will add the tests to validate
 * your ustream implementation.
 */
#include "az_ulib_ustream_compliance_e2e.h"

/**
 * This is the function that will run all tests, feel free to change the function name. Just be sure
 * to change the call in the main.c as well.
 */
int my_ustream_e2e()
{
  const struct CMUnitTest tests[] = { cmocka_unit_test(my_ustream_e2e_test1_succeed),
                                      cmocka_unit_test(my_ustream_e2e_test2_succeed),
                                      AZ_ULIB_USTREAM_COMPLIANCE_E2E_LIST };

  return cmocka_run_group_tests_name("my_ustream_e2e", tests, NULL, NULL);
}
