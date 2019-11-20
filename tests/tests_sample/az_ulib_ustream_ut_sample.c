// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef __cplusplus
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <cstring>
#else
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#endif

#include "umock_c/umock_c.h"
#include "testrunnerswitcher.h"
#include "umock_c/umocktypes_charptr.h"
#include "umock_c/umocktypes_bool.h"
#include "umock_c/umocktypes_stdint.h"
#include "umock_c/umock_c_negative_tests.h"
#include "azure_macro_utils/macro_utils.h"
#include "az_ulib_ctest_aux.h"
#include "az_ulib_ustream_mock_buffer.h"

static TEST_MUTEX_HANDLE g_test_by_test;

#include "az_ulib_ustream_base.h"
/*TODO:<insert your header file here for testing>*/

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT        /*TODO:<insert your test content here (must be at least 20 uint8_t long)>*/
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH /*TODO:<insert the length of your test content here>*/

static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT = (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;

//Factory to create your implementation of a ustream.
static void ustream_factory(az_ulib_ustream* ustream)
{
    /*TODO:<Insert your code here which initializes the ustream parameter>*/
}

#define USTREAM_COMPLIANCE_TARGET_FACTORY(ustream)         ustream_factory(ustream)

#define TEST_CONST_BUFFER_LENGTH    (USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 2)
#define TEST_CONST_MAX_BUFFER_SIZE  (TEST_CONST_BUFFER_LENGTH - 1)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%i", error_code);
}

/**
 * Beginning of the UT for your ustream implementation.
 * Feel free to change the name of the test in BEGIN_TEST_SUITE
 *  and END_TEST_SUITE. Just be sure to change the passed parameter
 *  in the main.c as well.
 */
BEGIN_TEST_SUITE(ustream_ut_sample)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    g_test_by_test = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_test_by_test);

    result = umock_c_init(on_umock_c_error);
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_bool_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_UMOCK_ALIAS_TYPE(az_ulib_ustream, void*);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_test_by_test);
}

TEST_FUNCTION_INITIALIZE(test_method_initialize)
{
    if (TEST_MUTEX_ACQUIRE(g_test_by_test))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    memset(&test_ustream_instance, 0, sizeof(az_ulib_ustream));

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(test_method_cleanup)
{
    TEST_MUTEX_RELEASE(g_test_by_test);
}

//This include will add the tests to validate your ustream implementation.
#include "az_ulib_ustream_compliance_ut.h"

END_TEST_SUITE(ustream_ut_sample)
