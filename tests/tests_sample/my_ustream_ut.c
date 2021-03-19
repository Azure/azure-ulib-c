// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "az_ulib_ustream_base.h"
/*TODO:<insert your header file here for testing>*/

#include "az_ulib_ctest_aux.h"
#include "az_ulib_test_precondition.h"
#include "az_ulib_ustream_mock_buffer.h"
#include "azure/core/az_precondition.h"

#include "cmocka.h"

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT /*TODO:<insert your test content here (must be at \
                                               least 20 uint8_t long)>*/
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH /*TODO:<insert the length of your test content \
                                                      here>*/

static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT
    = (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;

// Factory to create your implementation of a ustream.
static void ustream_factory(az_ulib_ustream* ustream)
{
  /*TODO:<Insert your code here which initializes the ustream parameter>*/
}

#define USTREAM_COMPLIANCE_TARGET_FACTORY(ustream) ustream_factory(ustream)

#define TEST_CONST_BUFFER_LENGTH (USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 2)
#define TEST_CONST_MAX_BUFFER_SIZE (TEST_CONST_BUFFER_LENGTH - 1)

#ifndef AZ_NO_PRECONDITION_CHECKING
AZ_ULIB_ENABLE_PRECONDITION_CHECK_TESTS()
#endif // AZ_NO_PRECONDITION_CHECKING

static int setup(void** state)
{
  (void)state;

  memset(&test_ustream_instance, 0, sizeof(az_ulib_ustream));

  return 0;
}

static int teardown(void** state)
{
  (void)state;

  reset_mock_buffer();

  return 0;
}

/**
 * Beginning of the E2E for your ustream implementation.
 * TODO:<Add your own set of tests here.>
 */
#ifndef AZ_NO_PRECONDITION_CHECKING
static void my_ustream_precondition_test1_failed(void** state)
{
  /// arrange

  /// act

  /// assert
  // TODO: All precondition tests shall use the macro AZ_ULIB_ASSERT_PRECONDITION_CHECKED here.

  /// cleanup
}
#endif // AZ_NO_PRECONDITION_CHECKING

static void my_ustream_test1_succeed(void** state)
{
  /// arrange

  /// act

  /// assert

  /// cleanup
}

static void my_ustream_test2_succeed(void** state)
{
  /// arrange

  /// act

  /// assert

  /// cleanup
}

/**
 * This include, together with #AZ_ULIB_USTREAM_PRECONDITION_COMPLIANCE_UT_LIST and
 * AZ_ULIB_USTREAM_COMPLIANCE_UT_LIST, will add the tests to validate your ustream implementation.
 */
#include "az_ulib_ustream_compliance_ut.h"

/**
 * This is the function that will run all tests, feel free to change the function name. Just be sure
 * to change the call in the main.c as well.
 */
int my_ustream_ut()
{
#ifndef AZ_NO_PRECONDITION_CHECKING
  AZ_ULIB_SETUP_PRECONDITION_CHECK_TESTS();
#endif // AZ_NO_PRECONDITION_CHECKING

  const struct CMUnitTest tests[] = {
#ifndef AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test(my_ustream_precondition_test1_failed),
#endif // AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test_setup_teardown(my_ustream_test1_succeed, setup, teardown),
    cmocka_unit_test_setup_teardown(my_ustream_test2_succeed, setup, teardown),
#ifndef AZ_NO_PRECONDITION_CHECKING
    AZ_ULIB_USTREAM_PRECONDITION_COMPLIANCE_UT_LIST
#endif // AZ_NO_PRECONDITION_CHECKING
        AZ_ULIB_USTREAM_COMPLIANCE_UT_LIST
  };

  return cmocka_run_group_tests_name("my_ustream_ut", tests, NULL, NULL);
}
