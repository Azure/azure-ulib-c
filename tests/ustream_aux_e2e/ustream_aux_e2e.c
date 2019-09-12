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
#include "test_thread.h"

static TEST_MUTEX_HANDLE g_test_by_test;

#include "ustream.h"

static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1 =
        (const uint8_t* const)"0123456789";
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2 =
        (const uint8_t* const)"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3 =
        (const uint8_t* const)"abcdefghijklmnopqrstuvwxyz";
    
static AZIOT_USTREAM test_multi;

static AZIOT_USTREAM* create_test_default_multibuffer()
{
    //Set up required structs for first multi
    AZIOT_USTREAM_INNER_BUFFER* default_multi_inner_buffer1 = (AZIOT_USTREAM_INNER_BUFFER*)malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    ASSERT_IS_NOT_NULL(default_multi_inner_buffer1);
    memset(default_multi_inner_buffer1, 0, sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_USTREAM_MULTI_DATA* default_multi_data1 = (AZIOT_USTREAM_MULTI_DATA*)malloc(sizeof(AZIOT_USTREAM_MULTI_DATA));
    ASSERT_IS_NOT_NULL(default_multi_data1);
    memset(default_multi_data1, 0, sizeof(AZIOT_USTREAM_MULTI_DATA));

    //Set up required structs for second multi
    AZIOT_USTREAM_INNER_BUFFER* default_multi_inner_buffer2 = (AZIOT_USTREAM_INNER_BUFFER*)malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    ASSERT_IS_NOT_NULL(default_multi_inner_buffer2);
    memset(default_multi_inner_buffer2, 0, sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_USTREAM_MULTI_DATA* default_multi_data2 = (AZIOT_USTREAM_MULTI_DATA*)malloc(sizeof(AZIOT_USTREAM_MULTI_DATA));
    ASSERT_IS_NOT_NULL(default_multi_data2);
    memset(default_multi_data2, 0, sizeof(AZIOT_USTREAM_MULTI_DATA));

    //Set up first ustream
    AZIOT_USTREAM_INNER_BUFFER* ustream_inner_buffer1 = (AZIOT_USTREAM_INNER_BUFFER*)malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    memset(ustream_inner_buffer1, 0, sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_ULIB_RESULT result = aziot_ustream_init(&test_multi,
            ustream_inner_buffer1,
            free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result);

    //Set up second ustream
    AZIOT_USTREAM default_buffer2;
    AZIOT_USTREAM_INNER_BUFFER* ustream_inner_buffer2 = (AZIOT_USTREAM_INNER_BUFFER*)malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    memset(ustream_inner_buffer2, 0, sizeof(AZIOT_USTREAM_INNER_BUFFER));
    result = aziot_ustream_init(&default_buffer2,
                ustream_inner_buffer2,
                free,
                USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
                strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2),
                NULL);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result);

    //Set up third ustream
    AZIOT_USTREAM default_buffer3;
    AZIOT_USTREAM_INNER_BUFFER* ustream_inner_buffer3 = (AZIOT_USTREAM_INNER_BUFFER*)malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    memset(ustream_inner_buffer3, 0, sizeof(AZIOT_USTREAM_INNER_BUFFER));
    result = aziot_ustream_init(&default_buffer3,
                ustream_inner_buffer3,
                free,
                USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
                strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3), NULL);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result);

    //Concat buffers together
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, aziot_ustream_concat(&test_multi, &default_buffer2, 
                        default_multi_inner_buffer1, free, default_multi_data1, free));
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, aziot_ustream_concat(&test_multi, &default_buffer3,
                        default_multi_inner_buffer2, free, default_multi_data2, free));

    (void)aziot_ustream_dispose(&default_buffer2);
    (void)aziot_ustream_dispose(&default_buffer3);

    return &test_multi;
}

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT =
        (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;
#define USTREAM_COMPLIANCE_TARGET_FACTORY           create_test_default_multibuffer()
#define USTREAM_COMPLIANCE_TARGET_INSTANCE          AZIOT_USTREAM

#define TEST_POSITION 10
#define TEST_SIZE 10
static AZIOT_USTREAM* thread_one_ustream;
static AZIOT_USTREAM* thread_two_ustream;

static int thread_one_func(void* arg)
{
    uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH] = { 0 };
    size_t size_result;
    offset_t cur_pos;
    aziot_ustream_read(
        thread_one_ustream,
        buf_result,
        TEST_SIZE,
        &size_result);
    aziot_ustream_get_position(thread_one_ustream, &cur_pos);
    ASSERT_ARE_EQUAL(int, TEST_POSITION, cur_pos);

    return 0;
}

static int thread_two_func(void* arg)
{
    uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH] = { 0 };
    size_t size_result;
    offset_t cur_pos;
    size_t position = strlen(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1);
    aziot_ustream_set_position(thread_two_ustream, TEST_POSITION);
    aziot_ustream_read(
        thread_two_ustream,
        buf_result,
        TEST_SIZE,
        &size_result);
    aziot_ustream_get_position(thread_two_ustream, &cur_pos);
    ASSERT_ARE_EQUAL(int, TEST_POSITION + TEST_SIZE, cur_pos);

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
    memset(&test_multi, 0, sizeof(AZIOT_USTREAM));
    TEST_MUTEX_RELEASE(g_test_by_test);
}

TEST_FUNCTION(ustream_multi_read_concurrent_succeed)
{
    ///arrange
    AZIOT_USTREAM* multibuffer = ustream_mock_create();

    AZIOT_USTREAM test_buffer2;
    AZIOT_USTREAM_INNER_BUFFER* inner_buffer1 = (AZIOT_USTREAM_INNER_BUFFER*)malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_ULIB_RESULT result = aziot_ustream_init(&test_buffer2, inner_buffer1, free,
                                    USTREAM_COMPLIANCE_EXPECTED_CONTENT,
                                    strlen((const char*)USTREAM_COMPLIANCE_EXPECTED_CONTENT), NULL);
    ASSERT_ARE_EQUAL(int, result, AZIOT_ULIB_SUCCESS);

    AZIOT_USTREAM_INNER_BUFFER* multi_inner_buffer1 =
        (AZIOT_USTREAM_INNER_BUFFER*)malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_USTREAM_MULTI_DATA* multi_data1 =
        (AZIOT_USTREAM_MULTI_DATA*)malloc(sizeof(AZIOT_USTREAM_MULTI_DATA));

    result = aziot_ustream_concat(multibuffer, &test_buffer2, multi_inner_buffer1, free, multi_data1, free);
    ASSERT_ARE_EQUAL(int, result, AZIOT_ULIB_SUCCESS);

    aziot_ustream_dispose(&test_buffer2);

    //Clone the multistream
    AZIOT_USTREAM multibuffer_clone;
    result = aziot_ustream_clone(&multibuffer_clone, multibuffer, 0);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result);

    thread_one_ustream = multibuffer;
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
    (void)aziot_ustream_dispose(multibuffer);
    (void)aziot_ustream_dispose(&multibuffer_clone);
}

END_TEST_SUITE(ustream_aux_e2e)
