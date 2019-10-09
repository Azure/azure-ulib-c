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

#include "testrunnerswitcher.h"
#include "azure_macro_utils/macro_utils.h"
#include "ustream_ctest_aux.h"
#include "ustream_mock_buffer.h"

static TEST_MUTEX_HANDLE g_test_by_test;

#include "ustream_base.h"
#include "ustream.h"
/*TODO:<insert your header file here for testing>*/

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT        /*TODO:<insert your test content here (must be at least 20 uint8_t long)>*/
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH /*TODO:<insert the length of your test content here>*/


static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT = (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;
static void ustream_factory(AZ_USTREAM* ustream)
{
    /*
        TODO:<Insert your code here which initializes the ustream parameter with your implementation>
    */
}
#define USTREAM_COMPLIANCE_TARGET_FACTORY(ustream)         ustream_factory(ustream)

#define TEST_CONST_BUFFER_LENGTH    (USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 2)
#define TEST_CONST_MAX_BUFFER_SIZE  (TEST_CONST_BUFFER_LENGTH - 1)


/**
 * Beginning of the UT for your ustream implementation.
 * Feel free to change the name of the test in BEGIN_TEST_SUITE
 *  and END_TEST_SUITE. Just be sure to change the passed parameter
 *  in the main.c as well.
 */
BEGIN_TEST_SUITE(ustream_e2e)

TEST_SUITE_INITIALIZE(suite_init)
{
    g_test_by_test = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_test_by_test);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    TEST_MUTEX_DESTROY(g_test_by_test);
}

TEST_FUNCTION_INITIALIZE(test_method_initialize)
{
    if (TEST_MUTEX_ACQUIRE(g_test_by_test))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }
}

TEST_FUNCTION_CLEANUP(test_method_cleanup)
{
    TEST_MUTEX_RELEASE(g_test_by_test);
}

//This include will add the tests to validate your ustream implementation.
#include "ustream_compliance_e2e.h"

END_TEST_SUITE(ustream_e2e)
