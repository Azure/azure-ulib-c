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
#include "ustream_test_thread_aux.h"

static TEST_MUTEX_HANDLE g_test_by_test;

#include "ustream.h"

static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1 =
        (const uint8_t* const)"0123456789";
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2 =
        (const uint8_t* const)"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3 =
        (const uint8_t* const)"abcdefghijklmnopqrstuvwxyz";


static AZ_USTREAM* create_test_default_multibuffer()
{
    AZ_USTREAM* test_multi = (AZ_USTREAM*)malloc(sizeof(AZ_USTREAM));
    //Set up required structs for first multi
    AZ_USTREAM_MULTI_DATA_CB* default_multi_data1 = (AZ_USTREAM_MULTI_DATA_CB*)malloc(sizeof(AZ_USTREAM_MULTI_DATA_CB));
    ASSERT_IS_NOT_NULL(default_multi_data1);
    memset(default_multi_data1, 0, sizeof(AZ_USTREAM_MULTI_DATA_CB));

    //Set up required structs for second multi
    AZ_USTREAM_MULTI_DATA_CB* default_multi_data2 = (AZ_USTREAM_MULTI_DATA_CB*)malloc(sizeof(AZ_USTREAM_MULTI_DATA_CB));
    ASSERT_IS_NOT_NULL(default_multi_data2);
    memset(default_multi_data2, 0, sizeof(AZ_USTREAM_MULTI_DATA_CB));

    //Set up first ustream
    AZ_USTREAM_DATA_CB* ustream_control_block1 = (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    memset(ustream_control_block1, 0, sizeof(AZ_USTREAM_DATA_CB));
    AZ_ULIB_RESULT result = az_ustream_init(test_multi,
            ustream_control_block1,
            free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);

    //Set up second ustream
    AZ_USTREAM default_buffer2;
    AZ_USTREAM_DATA_CB* ustream_control_block2 = (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    memset(ustream_control_block2, 0, sizeof(AZ_USTREAM_DATA_CB));
    result = az_ustream_init(&default_buffer2,
                ustream_control_block2,
                free,
                USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
                strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2),
                NULL);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);

    //Set up third ustream
    AZ_USTREAM default_buffer3;
    AZ_USTREAM_DATA_CB* ustream_control_block3 = (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    memset(ustream_control_block3, 0, sizeof(AZ_USTREAM_DATA_CB));
    result = az_ustream_init(&default_buffer3,
                ustream_control_block3,
                free,
                USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
                strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3), NULL);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);

    //Concat buffers together
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ustream_concat(test_multi, &default_buffer2, 
                        default_multi_data1, free));
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ustream_concat(test_multi, &default_buffer3,
                        default_multi_data2, free));

    (void)az_ustream_dispose(&default_buffer2);
    (void)az_ustream_dispose(&default_buffer3);

    return test_multi;
}

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT =
        (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;
#define USTREAM_COMPLIANCE_TARGET_FACTORY           create_test_default_multibuffer()

#define TEST_POSITION 10
#define TEST_SIZE 10
static AZ_USTREAM* thread_one_ustream;
static AZ_USTREAM* thread_two_ustream;

static int thread_one_func(void* arg)
{
    uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH] = { 0 };
    size_t size_result;
    offset_t cur_pos;
    az_ustream_read(
        thread_one_ustream,
        buf_result,
        TEST_SIZE,
        &size_result);
    az_ustream_get_position(thread_one_ustream, &cur_pos);
    ASSERT_ARE_EQUAL(int, TEST_SIZE, cur_pos);

    return 0;
}

static int thread_two_func(void* arg)
{
    uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH] = { 0 };
    size_t size_result;
    offset_t cur_pos;
    size_t position = strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1);
    az_ustream_set_position(thread_two_ustream, TEST_POSITION);
    az_ustream_read(
        thread_two_ustream,
        buf_result,
        TEST_SIZE,
        &size_result);
    az_ustream_get_position(thread_two_ustream, &cur_pos);
    ASSERT_ARE_EQUAL(int, TEST_SIZE + TEST_POSITION, cur_pos);

    return 0;
}

/**
 * Beginning of the E2E for ustream_aux.c module.
 */
BEGIN_TEST_SUITE(ustream_aux_e2e)

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

TEST_FUNCTION(az_ustream_multi_read_concurrent_succeed)
{
    ///arrange
    AZ_USTREAM* multi_ustream = ustream_mock_create();

    AZ_USTREAM concat_ustream;
    AZ_USTREAM_DATA_CB* control_block1 = (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    AZ_ULIB_RESULT result = az_ustream_init(&concat_ustream, control_block1, free,
                                    (const uint8_t*)USTREAM_COMPLIANCE_EXPECTED_CONTENT,
                                    strlen((const char*)USTREAM_COMPLIANCE_EXPECTED_CONTENT), NULL);
    ASSERT_ARE_EQUAL(int, result, AZ_ULIB_SUCCESS);

    AZ_USTREAM_MULTI_DATA_CB* multi_data1 =
        (AZ_USTREAM_MULTI_DATA_CB*)malloc(sizeof(AZ_USTREAM_MULTI_DATA_CB));

    result = az_ustream_concat(multi_ustream, &concat_ustream, multi_data1, free);
    ASSERT_ARE_EQUAL(int, result, AZ_ULIB_SUCCESS);

    az_ustream_dispose(&concat_ustream);

    //Clone the multistream
    AZ_USTREAM multibuffer_clone;
    result = az_ustream_clone(&multibuffer_clone, multi_ustream, 0);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);

    thread_one_ustream = multi_ustream;
    thread_two_ustream = &multibuffer_clone;

    set_concurrency_ustream();

    ///act
    THREAD_HANDLE test_thread_one;
    THREAD_HANDLE test_thread_two;
    (void)test_thread_create(&test_thread_one, &thread_one_func, NULL);
    (void)test_thread_create(&test_thread_two, &thread_two_func, NULL);

    ///assert
    int res1;
    int res2;
    test_thread_join(test_thread_one, &res1);
    test_thread_join(test_thread_two, &res2);

    ///cleanup
    (void)az_ustream_dispose(multi_ustream);
    (void)az_ustream_dispose(&multibuffer_clone);
}

#include "ustream_compliance_e2e.h"

END_TEST_SUITE(ustream_aux_e2e)
