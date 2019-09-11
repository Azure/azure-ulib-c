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

#define ENABLE_MOCKS

#include "ulib_heap.h"

#undef ENABLE_MOCKS

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
    AZIOT_USTREAM_INNER_BUFFER* default_multi_inner_buffer1 = (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    ASSERT_IS_NOT_NULL(default_multi_inner_buffer1);
    memset(default_multi_inner_buffer1, 0, sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_USTREAM_MULTI_DATA* default_multi_data1 = (AZIOT_USTREAM_MULTI_DATA*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_MULTI_DATA));
    ASSERT_IS_NOT_NULL(default_multi_data1);
    memset(default_multi_data1, 0, sizeof(AZIOT_USTREAM_MULTI_DATA));

    //Set up required structs for second multi
    AZIOT_USTREAM_INNER_BUFFER* default_multi_inner_buffer2 = (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    ASSERT_IS_NOT_NULL(default_multi_inner_buffer2);
    memset(default_multi_inner_buffer2, 0, sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_USTREAM_MULTI_DATA* default_multi_data2 = (AZIOT_USTREAM_MULTI_DATA*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_MULTI_DATA));
    ASSERT_IS_NOT_NULL(default_multi_data2);
    memset(default_multi_data2, 0, sizeof(AZIOT_USTREAM_MULTI_DATA));

    //Set up first ustream
    AZIOT_USTREAM_INNER_BUFFER* ustream_inner_buffer1 = (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    memset(ustream_inner_buffer1, 0, sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_ULIB_RESULT result = aziot_ustream_init(&test_multi,
            ustream_inner_buffer1,
            aziot_ulib_free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result);

    //Set up second ustream
    AZIOT_USTREAM default_buffer2;
    AZIOT_USTREAM_INNER_BUFFER* ustream_inner_buffer2 = (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    memset(ustream_inner_buffer2, 0, sizeof(AZIOT_USTREAM_INNER_BUFFER));
    result = aziot_ustream_init(&default_buffer2,
                ustream_inner_buffer2,
                aziot_ulib_free,
                USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
                strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2),
                NULL);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result);

    //Set up third ustream
    AZIOT_USTREAM default_buffer3;
    AZIOT_USTREAM_INNER_BUFFER* ustream_inner_buffer3 = (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    memset(ustream_inner_buffer3, 0, sizeof(AZIOT_USTREAM_INNER_BUFFER));
    result = aziot_ustream_init(&default_buffer3,
                ustream_inner_buffer3,
                aziot_ulib_free,
                USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
                strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3), NULL);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result);

    //Concat buffers together
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, aziot_ustream_concat(&test_multi, &default_buffer2, 
                        default_multi_inner_buffer1, aziot_ulib_free, default_multi_data1, aziot_ulib_free));
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, aziot_ustream_concat(&test_multi, &default_buffer3,
                        default_multi_inner_buffer2, aziot_ulib_free, default_multi_data2, aziot_ulib_free));

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

    REGISTER_UMOCK_ALIAS_TYPE(AZIOT_USTREAM, void*);

    REGISTER_GLOBAL_MOCK_HOOK(aziot_ulib_malloc, malloc);
    REGISTER_GLOBAL_MOCK_HOOK(aziot_ulib_free, free);
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
    memset(&test_multi, 0, sizeof(AZIOT_USTREAM));
    TEST_MUTEX_RELEASE(g_test_by_test);
}

/* aziot_ustream_concat shall return AZIOT_ULIB_SUCCESS if the ustreams were concatenated succesfully */
TEST_FUNCTION(aziot_ustream_concat_multiple_buffers_succeed)
{
    ///arrange
    AZIOT_USTREAM_INNER_BUFFER* inner_buffer1 =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    ASSERT_IS_NOT_NULL(inner_buffer1);
    AZIOT_USTREAM test_buffer1;
    AZIOT_ULIB_RESULT result1 =
        aziot_ustream_init(
            &test_buffer1,
            inner_buffer1,
            aziot_ulib_free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
            NULL);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result1);

    AZIOT_USTREAM_INNER_BUFFER* inner_buffer2 =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    ASSERT_IS_NOT_NULL(inner_buffer2);
    AZIOT_USTREAM test_buffer2;
    AZIOT_ULIB_RESULT result2 =
        aziot_ustream_init(
            &test_buffer2,
            inner_buffer2,
            aziot_ulib_free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2),
            NULL);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result2);

    AZIOT_USTREAM_INNER_BUFFER* inner_buffer3 =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    ASSERT_IS_NOT_NULL(inner_buffer3);
    AZIOT_USTREAM test_buffer3;
    AZIOT_ULIB_RESULT result3 =
        aziot_ustream_init(
            &test_buffer3,
            inner_buffer3,
            aziot_ulib_free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3),
            NULL);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result3);

    AZIOT_USTREAM_INNER_BUFFER* multi_inner_buffer1 =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_USTREAM_INNER_BUFFER* multi_inner_buffer2 =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_USTREAM_MULTI_DATA* multi_data1 =
        (AZIOT_USTREAM_MULTI_DATA*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_MULTI_DATA));
    AZIOT_USTREAM_MULTI_DATA* multi_data2 =
        (AZIOT_USTREAM_MULTI_DATA*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_MULTI_DATA));

    ///act
    result1 = aziot_ustream_concat(&test_buffer1, &test_buffer2, multi_inner_buffer1, aziot_ulib_free, multi_data1, aziot_ulib_free);
    result2 = aziot_ustream_concat(&test_buffer1, &test_buffer3, multi_inner_buffer2, aziot_ulib_free, multi_data2, aziot_ulib_free);

    ///assert
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result2);
    aziot_ustream_dispose(&test_buffer2);
    aziot_ustream_dispose(&test_buffer3);
    check_buffer(
        &test_buffer1,
        0,
        (const uint8_t*)USTREAM_COMPLIANCE_EXPECTED_CONTENT,
        (uint8_t)strlen(USTREAM_COMPLIANCE_EXPECTED_CONTENT));

    ///cleanup
    aziot_ustream_dispose(&test_buffer1);
}

/* aziot_ustream_read shall read from all the concatenated ustreams and return AZIOT_ULIB_SUCCESS */
TEST_FUNCTION(aziot_ustream_concat_read_from_multiple_buffers_succeed)
{
    ///arrange
    AZIOT_USTREAM_INNER_BUFFER* inner_buffer1 =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    ASSERT_IS_NOT_NULL(inner_buffer1);
    AZIOT_USTREAM test_buffer1;
    AZIOT_ULIB_RESULT result1 =
        aziot_ustream_init(
            &test_buffer1,
            inner_buffer1,
            aziot_ulib_free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
            NULL);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result1);

    AZIOT_USTREAM_INNER_BUFFER* inner_buffer2 =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    ASSERT_IS_NOT_NULL(inner_buffer2);
    AZIOT_USTREAM test_buffer2;
    AZIOT_ULIB_RESULT result2 =
        aziot_ustream_init(
            &test_buffer2,
            inner_buffer2,
            aziot_ulib_free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2),
            NULL);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result2);

    AZIOT_USTREAM_INNER_BUFFER* inner_buffer3 =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    ASSERT_IS_NOT_NULL(inner_buffer3);
    AZIOT_USTREAM test_buffer3;
    AZIOT_ULIB_RESULT result3 =
        aziot_ustream_init(
            &test_buffer3,
            inner_buffer3,
            aziot_ulib_free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3),
            NULL);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result3);

    AZIOT_USTREAM_INNER_BUFFER* multi_inner_buffer1 =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_USTREAM_INNER_BUFFER* multi_inner_buffer2 =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_USTREAM_MULTI_DATA* multi_data1 =
        (AZIOT_USTREAM_MULTI_DATA*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_MULTI_DATA));
    AZIOT_USTREAM_MULTI_DATA* multi_data2 =
        (AZIOT_USTREAM_MULTI_DATA*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_MULTI_DATA));

    result1 = aziot_ustream_concat(&test_buffer1, &test_buffer2, multi_inner_buffer1, aziot_ulib_free, multi_data1, aziot_ulib_free);
    result2 = aziot_ustream_concat(&test_buffer1, &test_buffer3, multi_inner_buffer2, aziot_ulib_free, multi_data2, aziot_ulib_free);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result2);

    uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
    size_t size_result;

    ///act
    AZIOT_ULIB_RESULT result =
        aziot_ustream_read(
            &test_buffer1,
            buf_result,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_EXPECTED_CONTENT, buf_result, size_result);

    ///cleanup
    aziot_ustream_dispose(&test_buffer2);
    aziot_ustream_dispose(&test_buffer3);
    aziot_ustream_dispose(&test_buffer1);
}

/* aziot_ustream_concat shall return AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR if the provided ustream is NULL */
TEST_FUNCTION(aziot_ustream_concat_null_interface_failed)
{
    ///arrange
    AZIOT_USTREAM_INNER_BUFFER* inner_buffer =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    ASSERT_IS_NOT_NULL(inner_buffer);
    AZIOT_USTREAM test_buffer;
    AZIOT_ULIB_RESULT result1 =
        aziot_ustream_init(
            &test_buffer,
            inner_buffer,
            aziot_ulib_free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
            NULL);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result1);

    ///act
    AZIOT_ULIB_RESULT result = aziot_ustream_concat(NULL, &test_buffer, NULL, aziot_ulib_free, NULL, aziot_ulib_free);

    ///assert
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    aziot_ustream_dispose(&test_buffer);
}

/* aziot_ustream_concat shall return AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR if the provided ustream to add is NULL */
TEST_FUNCTION(aziot_ustream_concat_null_buffer_to_add_failed)
{
    ///arrange
    AZIOT_USTREAM_INNER_BUFFER* inner_buffer =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    ASSERT_IS_NOT_NULL(inner_buffer);
    AZIOT_USTREAM default_buffer;
    AZIOT_ULIB_RESULT result1 =
        aziot_ustream_init(
            &default_buffer,
            inner_buffer,
            aziot_ulib_free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
            NULL);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result1);

    AZIOT_USTREAM_INNER_BUFFER multi_inner;
    AZIOT_USTREAM_MULTI_DATA multi_data;

    ///act
    AZIOT_ULIB_RESULT result = aziot_ustream_concat(&default_buffer, NULL, &multi_inner, NULL, &multi_data, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    aziot_ustream_dispose(&default_buffer);
}

/* aziot_ustream_concat shall return AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR if the provided inner buffer to add is NULL */
TEST_FUNCTION(aziot_ustream_concat_null_inner_buffer_failed)
{
    ///arrange
    AZIOT_USTREAM_INNER_BUFFER* inner_buffer =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    ASSERT_IS_NOT_NULL(inner_buffer);
    AZIOT_USTREAM default_buffer;
    AZIOT_ULIB_RESULT result = aziot_ustream_init(
                &default_buffer,
                inner_buffer,
                aziot_ulib_free,
                USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
                strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
                NULL);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result);

    AZIOT_USTREAM_INNER_BUFFER* inner_buffer2 =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    ASSERT_IS_NOT_NULL(inner_buffer2);
    AZIOT_USTREAM test_buffer2;
    result = aziot_ustream_init(
                &test_buffer2,
                inner_buffer2,
                aziot_ulib_free,
                USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
                strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
                NULL);

    AZIOT_USTREAM_MULTI_DATA multi_data;

    ///act
    result = aziot_ustream_concat(&default_buffer, &test_buffer2, NULL, NULL, &multi_data, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    aziot_ustream_dispose(&test_buffer2);
    aziot_ustream_dispose(&default_buffer);
}

/* aziot_ustream_concat shall return AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR if the provided multi data to add is NULL */
TEST_FUNCTION(aziot_ustream_concat_null_multi_data_failed)
{
    ///arrange
    AZIOT_USTREAM_INNER_BUFFER* inner_buffer =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    ASSERT_IS_NOT_NULL(inner_buffer);
    AZIOT_USTREAM default_buffer;
    AZIOT_ULIB_RESULT result1 =
        aziot_ustream_init(
            &default_buffer,
            inner_buffer,
            aziot_ulib_free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
            NULL);
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result1);

    AZIOT_USTREAM_INNER_BUFFER* inner_buffer2 =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    ASSERT_IS_NOT_NULL(inner_buffer2);
    AZIOT_USTREAM test_buffer2;
    AZIOT_ULIB_RESULT result2 =
        aziot_ustream_init(
            &test_buffer2,
            inner_buffer2,
            aziot_ulib_free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
            NULL);

    AZIOT_USTREAM_INNER_BUFFER multi_inner;

    ///act
    AZIOT_ULIB_RESULT result = aziot_ustream_concat(&default_buffer, &test_buffer2, &multi_inner, NULL, NULL, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    aziot_ustream_dispose(&test_buffer2);
    aziot_ustream_dispose(&default_buffer);
}

/* aziot_ustream_multi_concat shall return an error if the internal call to aziot_ustream_get_remaining_size failed
    when concatenating two ustreams */
TEST_FUNCTION(aziot_ustream_concat_new_inner_buffer_failed_on_get_remaining_size_failed)
{
    ///arrange
    AZIOT_USTREAM multibuffer;
    AZIOT_USTREAM_INNER_BUFFER* inner_buffer1 = (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_ULIB_RESULT result =
        aziot_ustream_init(&multibuffer, inner_buffer1, aziot_ulib_free,
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_ARE_EQUAL(int, result, AZIOT_ULIB_SUCCESS);

    AZIOT_USTREAM* test_buffer2 = ustream_mock_create();

    AZIOT_USTREAM_INNER_BUFFER* multi_inner_buffer1 =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_USTREAM_MULTI_DATA* multi_data1 =
        (AZIOT_USTREAM_MULTI_DATA*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_MULTI_DATA));

    set_get_remaining_size_result(AZIOT_ULIB_SYSTEM_ERROR);

    ///act
    result = aziot_ustream_concat(&multibuffer, test_buffer2, multi_inner_buffer1, aziot_ulib_free, multi_data1, aziot_ulib_free);

    ///assert
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SYSTEM_ERROR, result);

    ///cleanup
    (void)aziot_ustream_dispose(test_buffer2);
    (void)aziot_ustream_dispose(&multibuffer);
}

/* aziot_ustream_dispose shall release all buffers in its list if the multibuffer contains concatenated buffers */
TEST_FUNCTION(ustream_multi_dispose_multibuffer_with_buffers_free_all_resources_succeed)
{
    ///arrange
    AZIOT_USTREAM* multibuffer = USTREAM_COMPLIANCE_TARGET_FACTORY;

    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(aziot_ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(aziot_ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(aziot_ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(aziot_ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(aziot_ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(aziot_ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(aziot_ulib_free(IGNORED_PTR_ARG));

    ///act
    AZIOT_ULIB_RESULT result = aziot_ustream_dispose(multibuffer);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result);

    ///cleanup
}


/* aziot_ustream_read shall return partial result if one of the internal buffers failed. */
TEST_FUNCTION(ustream_multi_read_inner_buffer_failed_in_read_with_some_valid_content_succeed)
{
    ///arrange
    AZIOT_USTREAM multibuffer;
    AZIOT_USTREAM_INNER_BUFFER* inner_buffer1 = (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_ULIB_RESULT result =
        aziot_ustream_init(&multibuffer, inner_buffer1, aziot_ulib_free,
                           USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
                           strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_ARE_EQUAL(int, result, AZIOT_ULIB_SUCCESS);

    AZIOT_USTREAM* test_buffer2 = ustream_mock_create();

    AZIOT_USTREAM_INNER_BUFFER* multi_inner_buffer1 =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_USTREAM_MULTI_DATA* multi_data1 =
        (AZIOT_USTREAM_MULTI_DATA*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_MULTI_DATA));

    result = aziot_ustream_concat(&multibuffer, test_buffer2, multi_inner_buffer1, aziot_ulib_free, multi_data1, aziot_ulib_free);
    set_read_result(AZIOT_ULIB_SYSTEM_ERROR);

    uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
    size_t size_result;


    ///act
    result =
        aziot_ustream_read(
            &multibuffer,
            buf_result,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, buf_result, size_result);

    ///cleanup
    (void)aziot_ustream_dispose(&multibuffer);
    (void)aziot_ustream_dispose(test_buffer2);
}

/* aziot_ustream_read shall return AZIOT_ULIB_SYSTEM_ERROR if it failed to read the requested bytes */
TEST_FUNCTION(ustream_multi_read_inner_buffer_failed_in_read_failed)
{
    ///arrange
    AZIOT_USTREAM multibuffer;
    AZIOT_USTREAM_INNER_BUFFER* inner_buffer1 = (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_ULIB_RESULT result =
        aziot_ustream_init(&multibuffer, inner_buffer1, aziot_ulib_free,
                           USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
                           strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_ARE_EQUAL(int, result, AZIOT_ULIB_SUCCESS);

    AZIOT_USTREAM* test_buffer2 = ustream_mock_create();

    AZIOT_USTREAM_INNER_BUFFER* multi_inner_buffer1 =
        (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_USTREAM_MULTI_DATA* multi_data1 =
        (AZIOT_USTREAM_MULTI_DATA*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_MULTI_DATA));

    result = aziot_ustream_concat(&multibuffer, test_buffer2, multi_inner_buffer1, aziot_ulib_free, multi_data1, aziot_ulib_free);
    set_read_result(AZIOT_ULIB_SYSTEM_ERROR);

    uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
    size_t size_result;

    ASSERT_ARE_EQUAL(
        int, 
        AZIOT_ULIB_SUCCESS, 
        aziot_ustream_read(
            &multibuffer,
            buf_result,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &size_result));
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, buf_result, size_result);

    set_read_result(AZIOT_ULIB_SYSTEM_ERROR);

    ///act
    result =
        aziot_ustream_read(
            &multibuffer,
            buf_result,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SYSTEM_ERROR, result);

    ///cleanup
    (void)aziot_ustream_dispose(&multibuffer);
    (void)aziot_ustream_dispose(test_buffer2);
}

/* aziot_ustream_read shall set the position to an instance's position before reading and return AZIOT_USTREAM_SUCCESS */
TEST_FUNCTION(ustream_multi_read_clone_and_original_in_parallel_succeed)
{
    ///arrange
    AZIOT_USTREAM* multibuffer = USTREAM_COMPLIANCE_TARGET_FACTORY;

    AZIOT_USTREAM multibuffer_clone;

    aziot_ustream_clone(&multibuffer_clone, multibuffer, 0);

    uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
    size_t size_result;

    ASSERT_ARE_EQUAL(
        int,
        AZIOT_ULIB_SUCCESS,
        aziot_ustream_read(
            multibuffer,
            buf_result,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
            &size_result));

    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, buf_result, size_result);

    memset(buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ASSERT_ARE_EQUAL(
        int,
        AZIOT_ULIB_SUCCESS,
        aziot_ustream_read(
            &multibuffer_clone,
            buf_result,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1),
            &size_result));
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, buf_result, size_result);

    memset(buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS,
                     aziot_ustream_reset(multibuffer));

    ///act
    AZIOT_ULIB_RESULT result =
        aziot_ustream_read(
            multibuffer,
            buf_result,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &size_result);

    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result);

    ///assert
    result =
        aziot_ustream_read(
            multibuffer,
            buf_result,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &size_result);

    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_EOF, result);

    (void)aziot_ustream_dispose(multibuffer);

    memset(buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ASSERT_ARE_EQUAL(
        int,
        AZIOT_ULIB_SUCCESS,
        aziot_ustream_read(
            &multibuffer_clone,
            buf_result,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2),
            &size_result));
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2, buf_result, size_result);

    memset(buf_result, 0, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ASSERT_ARE_EQUAL(
        int,
        AZIOT_ULIB_SUCCESS,
        aziot_ustream_read(
            &multibuffer_clone,
            buf_result,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3),
            &size_result));
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3, buf_result, size_result);

    ASSERT_ARE_EQUAL(
        int,
        AZIOT_ULIB_EOF,
        aziot_ustream_read(
            &multibuffer_clone,
            buf_result,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3),
            &size_result));

    ///cleanup
    (void)aziot_ustream_dispose(&multibuffer_clone);
}

#include "ustream_compliance_ut.h"

END_TEST_SUITE(ustream_aux_ut)
