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
#include "az_ulib_ctest_aux.h"
#include "az_ulib_ustream_mock_buffer.h"

static TEST_MUTEX_HANDLE g_test_by_test;

#include "az_ulib_ustream_base.h"
#include "az_ulib_ustream.h"

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62

static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT = (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;
static void ustream_factory(AZ_USTREAM* ustream)
{
    AZ_USTREAM_DATA_CB* ustream_control_block = (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    uint8_t* buf = (uint8_t*)malloc(sizeof(uint8_t)*USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    (void)memcpy(buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    az_ustream_init(ustream, ustream_control_block, free, buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, free);
}
#define USTREAM_COMPLIANCE_TARGET_FACTORY(ustream)         ustream_factory(ustream)

#define TEST_CONST_BUFFER_LENGTH    (USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 2)
#define TEST_CONST_MAX_BUFFER_SIZE  (TEST_CONST_BUFFER_LENGTH - 1)


/**
 * Beginning of the UT for ustream.c on ownership model.
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

//Run e2e compliance tests for ustream
#include "az_ulib_ustream_compliance_e2e.h"

END_TEST_SUITE(ustream_e2e)
