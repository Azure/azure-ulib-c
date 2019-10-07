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
#include "ustream_ctest_aux.h"
#include "ustream_mock_buffer.h"

static TEST_MUTEX_HANDLE g_test_by_test;

#include "ustream.h"

static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1 =
        (const uint8_t* const)"0123456789";
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2 =
        (const uint8_t* const)"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3 =
        (const uint8_t* const)"abcdefghijklmnopqrstuvwxyz";
    
static AZ_USTREAM test_multi;

static AZ_USTREAM* create_test_default_multibuffer()
{
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
    AZ_ULIB_RESULT result = az_ustream_init(&test_multi,
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
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ustream_concat(&test_multi, &default_buffer2, 
                        default_multi_data1, free));
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, az_ustream_concat(&test_multi, &default_buffer3,
                        default_multi_data2, free));

    (void)az_ustream_dispose(&default_buffer2);
    (void)az_ustream_dispose(&default_buffer3);

    return &test_multi;
}

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT =
        (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;
#define USTREAM_COMPLIANCE_TARGET_FACTORY           create_test_default_multibuffer()

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%s", MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
}

/**
 * Beginning of the UT for ustream_aux.c module.
 */
BEGIN_TEST_SUITE(ustream_aux_ut)

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

    REGISTER_UMOCK_ALIAS_TYPE(AZ_USTREAM, void*);
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

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(test_method_cleanup)
{
    memset(&test_multi, 0, sizeof(AZ_USTREAM));
    TEST_MUTEX_RELEASE(g_test_by_test);
}

/* az_ustream_concat shall return AZ_ULIB_SUCCESS if the ustreams were concatenated succesfully */
TEST_FUNCTION(az_ustream_concat_multiple_buffers_succeed)
{
    ///arrange
    AZ_USTREAM_DATA_CB* control_block1 =
        (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    ASSERT_IS_NOT_NULL(control_block1);
    AZ_USTREAM test_buffer1;
    AZ_ULIB_RESULT result1 =
        az_ustream_init(
            &test_buffer1,
            control_block1,
            free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
            NULL);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result1);

    AZ_USTREAM_DATA_CB* control_block2 =
        (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    ASSERT_IS_NOT_NULL(control_block2);
    AZ_USTREAM test_buffer2;
    AZ_ULIB_RESULT result2 =
        az_ustream_init(
            &test_buffer2,
            control_block2,
            free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2),
            NULL);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result2);

    AZ_USTREAM_DATA_CB* control_block3 =
        (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    ASSERT_IS_NOT_NULL(control_block3);
    AZ_USTREAM test_buffer3;
    AZ_ULIB_RESULT result3 =
        az_ustream_init(
            &test_buffer3,
            control_block3,
            free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3),
            NULL);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result3);

    AZ_USTREAM_MULTI_DATA_CB* multi_data1 =
        (AZ_USTREAM_MULTI_DATA_CB*)malloc(sizeof(AZ_USTREAM_MULTI_DATA_CB));
    AZ_USTREAM_MULTI_DATA_CB* multi_data2 =
        (AZ_USTREAM_MULTI_DATA_CB*)malloc(sizeof(AZ_USTREAM_MULTI_DATA_CB));

    ///act
    result1 = az_ustream_concat(&test_buffer1, &test_buffer2, multi_data1, free);
    result2 = az_ustream_concat(&test_buffer1, &test_buffer3, multi_data2, free);

    ///assert
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result2);
    az_ustream_dispose(&test_buffer2);
    az_ustream_dispose(&test_buffer3);
    check_buffer(
        &test_buffer1,
        0,
        (const uint8_t*)USTREAM_COMPLIANCE_EXPECTED_CONTENT,
        (uint8_t)strlen(USTREAM_COMPLIANCE_EXPECTED_CONTENT));

    ///cleanup
    az_ustream_dispose(&test_buffer1);
}

/* az_ustream_read shall read from all the concatenated ustreams and return AZ_ULIB_SUCCESS */
TEST_FUNCTION(az_ustream_concat_read_from_multiple_buffers_succeed)
{
    ///arrange
    AZ_USTREAM_DATA_CB* control_block1 =
        (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    ASSERT_IS_NOT_NULL(control_block1);
    AZ_USTREAM test_buffer1;
    AZ_ULIB_RESULT result1 =
        az_ustream_init(
            &test_buffer1,
            control_block1,
            free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
            NULL);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result1);

    AZ_USTREAM_DATA_CB* control_block2 =
        (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    ASSERT_IS_NOT_NULL(control_block2);
    AZ_USTREAM test_buffer2;
    AZ_ULIB_RESULT result2 =
        az_ustream_init(
            &test_buffer2,
            control_block2,
            free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2),
            NULL);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result2);

    AZ_USTREAM_DATA_CB* control_block3 =
        (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    ASSERT_IS_NOT_NULL(control_block3);
    AZ_USTREAM test_buffer3;
    AZ_ULIB_RESULT result3 =
        az_ustream_init(
            &test_buffer3,
            control_block3,
            free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3),
            NULL);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result3);

    AZ_USTREAM_MULTI_DATA_CB* multi_data1 =
        (AZ_USTREAM_MULTI_DATA_CB*)malloc(sizeof(AZ_USTREAM_MULTI_DATA_CB));
    AZ_USTREAM_MULTI_DATA_CB* multi_data2 =
        (AZ_USTREAM_MULTI_DATA_CB*)malloc(sizeof(AZ_USTREAM_MULTI_DATA_CB));

    result1 = az_ustream_concat(&test_buffer1, &test_buffer2, multi_data1, free);
    result2 = az_ustream_concat(&test_buffer1, &test_buffer3, multi_data2, free);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result2);

    uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
    size_t size_result;

    ///act
    AZ_ULIB_RESULT result =
        az_ustream_read(
            &test_buffer1,
            buf_result,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_EXPECTED_CONTENT, buf_result, size_result);

    ///cleanup
    az_ustream_dispose(&test_buffer2);
    az_ustream_dispose(&test_buffer3);
    az_ustream_dispose(&test_buffer1);
}

/* az_ustream_concat shall return AZ_ULIB_ILLEGAL_ARGUMENT_ERROR if the provided ustream is NULL */
TEST_FUNCTION(az_ustream_concat_null_instance_failed)
{
    ///arrange
    AZ_USTREAM_DATA_CB* control_block =
        (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    ASSERT_IS_NOT_NULL(control_block);
    AZ_USTREAM test_buffer;
    AZ_ULIB_RESULT result1 =
        az_ustream_init(
            &test_buffer,
            control_block,
            free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
            NULL);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result1);

    ///act
    AZ_ULIB_RESULT result = az_ustream_concat(NULL, &test_buffer, NULL, free);

    ///assert
    ASSERT_ARE_EQUAL(int, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    az_ustream_dispose(&test_buffer);
}

/* az_ustream_concat shall return AZ_ULIB_ILLEGAL_ARGUMENT_ERROR if the provided ustream to add is NULL */
TEST_FUNCTION(az_ustream_concat_null_buffer_to_add_failed)
{
    ///arrange
    AZ_USTREAM_DATA_CB* control_block =
        (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    ASSERT_IS_NOT_NULL(control_block);
    AZ_USTREAM default_buffer;
    AZ_ULIB_RESULT result1 =
        az_ustream_init(
            &default_buffer,
            control_block,
            free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
            NULL);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result1);

    AZ_USTREAM_MULTI_DATA_CB multi_data;

    ///act
    AZ_ULIB_RESULT result = az_ustream_concat(&default_buffer, NULL, &multi_data, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    az_ustream_dispose(&default_buffer);
}


/* az_ustream_concat shall return AZ_ULIB_ILLEGAL_ARGUMENT_ERROR if the provided multi data to add is NULL */
TEST_FUNCTION(az_ustream_concat_null_multi_data_failed)
{
    ///arrange
    AZ_USTREAM_DATA_CB* control_block =
        (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    ASSERT_IS_NOT_NULL(control_block);
    AZ_USTREAM default_buffer;
    AZ_ULIB_RESULT result1 =
        az_ustream_init(
            &default_buffer,
            control_block,
            free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
            NULL);
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result1);

    AZ_USTREAM_DATA_CB* control_block2 =
        (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    ASSERT_IS_NOT_NULL(control_block2);
    AZ_USTREAM test_buffer2;
    AZ_ULIB_RESULT result2 =
        az_ustream_init(
            &test_buffer2,
            control_block2,
            free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
            NULL);

    ///act
    AZ_ULIB_RESULT result = az_ustream_concat(&default_buffer, &test_buffer2, NULL, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    az_ustream_dispose(&test_buffer2);
    az_ustream_dispose(&default_buffer);
}

/* az_ustream_multi_concat shall return an error if the internal call to az_ustream_get_remaining_size failed
    when concatenating two ustreams */
TEST_FUNCTION(az_ustream_concat_new_control_block_failed_on_get_remaining_size_failed)
{
    ///arrange
    AZ_USTREAM multibuffer;
    AZ_USTREAM_DATA_CB* control_block1 = (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    AZ_ULIB_RESULT result =
        az_ustream_init(&multibuffer, control_block1, free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_ARE_EQUAL(int, result, AZ_ULIB_SUCCESS);

    AZ_USTREAM* test_buffer2 = ustream_mock_create();

    AZ_USTREAM_MULTI_DATA_CB* multi_data1 =
        (AZ_USTREAM_MULTI_DATA_CB*)malloc(sizeof(AZ_USTREAM_MULTI_DATA_CB));

    set_get_remaining_size_result(AZ_ULIB_SYSTEM_ERROR);

    ///act
    result = az_ustream_concat(&multibuffer, test_buffer2, multi_data1, free);

    ///assert
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SYSTEM_ERROR, result);

    ///cleanup
    (void)az_ustream_dispose(test_buffer2);
    (void)az_ustream_dispose(&multibuffer);
}

/* az_ustream_read shall return partial result if one of the internal buffers failed. */
TEST_FUNCTION(az_ustream_multi_read_control_block_failed_in_read_with_some_valid_content_succeed)
{
    ///arrange
    AZ_USTREAM multibuffer;
    AZ_USTREAM_DATA_CB* control_block1 = (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    AZ_ULIB_RESULT result =
        az_ustream_init(&multibuffer, control_block1, free,
                           USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
                           strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_ARE_EQUAL(int, result, AZ_ULIB_SUCCESS);

    AZ_USTREAM* test_buffer2 = ustream_mock_create();

    AZ_USTREAM_MULTI_DATA_CB* multi_data1 =
        (AZ_USTREAM_MULTI_DATA_CB*)malloc(sizeof(AZ_USTREAM_MULTI_DATA_CB));

    result = az_ustream_concat(&multibuffer, test_buffer2, multi_data1, free);
    set_read_result(AZ_ULIB_SYSTEM_ERROR);

    uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
    size_t size_result;


    ///act
    result =
        az_ustream_read(
            &multibuffer,
            buf_result,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, buf_result, size_result);

    ///cleanup
    (void)az_ustream_dispose(&multibuffer);
    (void)az_ustream_dispose(test_buffer2);
}

/* az_ustream_read shall return AZ_ULIB_SYSTEM_ERROR if it failed to read the requested bytes */
TEST_FUNCTION(az_ustream_multi_read_control_block_failed_in_read_failed)
{
    ///arrange
    AZ_USTREAM multibuffer;
    AZ_USTREAM_DATA_CB* control_block1 = (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    AZ_ULIB_RESULT result =
        az_ustream_init(&multibuffer, control_block1, free,
                           USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
                           strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_ARE_EQUAL(int, result, AZ_ULIB_SUCCESS);

    AZ_USTREAM* test_buffer2 = ustream_mock_create();

    AZ_USTREAM_MULTI_DATA_CB* multi_data1 =
        (AZ_USTREAM_MULTI_DATA_CB*)malloc(sizeof(AZ_USTREAM_MULTI_DATA_CB));

    result = az_ustream_concat(&multibuffer, test_buffer2, multi_data1, free);
    set_read_result(AZ_ULIB_SYSTEM_ERROR);

    uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
    size_t size_result;

    ASSERT_ARE_EQUAL(
        int, 
        AZ_ULIB_SUCCESS, 
        az_ustream_read(
            &multibuffer,
            buf_result,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &size_result));
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, buf_result, size_result);

    set_read_result(AZ_ULIB_SYSTEM_ERROR);

    ///act
    result =
        az_ustream_read(
            &multibuffer,
            buf_result,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, AZ_ULIB_SYSTEM_ERROR, result);

    ///cleanup
    (void)az_ustream_dispose(&multibuffer);
    (void)az_ustream_dispose(test_buffer2);
}

/* az_ustream_read shall set the position to an instance's position before reading and return AZ_ULIB_SUCCESS */
TEST_FUNCTION(az_ustream_multi_read_clone_and_original_in_parallel_succeed)
{
    ///arrange
    AZ_USTREAM* multibuffer = USTREAM_COMPLIANCE_TARGET_FACTORY;

    AZ_USTREAM multibuffer_clone;

    az_ustream_clone(&multibuffer_clone, multibuffer, 0);

    uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
    size_t size_result;

    ASSERT_ARE_EQUAL(
        int,
        AZ_ULIB_SUCCESS,
        az_ustream_read(
            multibuffer,
            buf_result,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
            &size_result));

    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, buf_result, size_result);

    memset(buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ASSERT_ARE_EQUAL(
        int,
        AZ_ULIB_SUCCESS,
        az_ustream_read(
            &multibuffer_clone,
            buf_result,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
            &size_result));
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, buf_result, size_result);

    memset(buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS,
                     az_ustream_reset(multibuffer));

    ///act
    AZ_ULIB_RESULT result =
        az_ustream_read(
            multibuffer,
            buf_result,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &size_result);

    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

    ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS, result);

    ///assert
    result =
        az_ustream_read(
            multibuffer,
            buf_result,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &size_result);

    ASSERT_ARE_EQUAL(int, AZ_ULIB_EOF, result);

    (void)az_ustream_dispose(multibuffer);

    memset(buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ASSERT_ARE_EQUAL(
        int,
        AZ_ULIB_SUCCESS,
        az_ustream_read(
            &multibuffer_clone,
            buf_result,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2),
            &size_result));
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2, buf_result, size_result);

    memset(buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ASSERT_ARE_EQUAL(
        int,
        AZ_ULIB_SUCCESS,
        az_ustream_read(
            &multibuffer_clone,
            buf_result,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3),
            &size_result));
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3, buf_result, size_result);

    ASSERT_ARE_EQUAL(
        int,
        AZ_ULIB_EOF,
        az_ustream_read(
            &multibuffer_clone,
            buf_result,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3),
            &size_result));

    ///cleanup
    (void)az_ustream_dispose(&multibuffer_clone);
}

#include "ustream_compliance_ut.h"

END_TEST_SUITE(ustream_aux_ut)
