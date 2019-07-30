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

static TEST_MUTEX_HANDLE g_test_by_test;
static TEST_MUTEX_HANDLE g_dll_by_dll;

#define ENABLE_MOCKS

#include "ulib_heap.h"

#undef ENABLE_MOCKS

#include "ustream.h"

static const uint8_t* const USTREAM_LOCAL_EXPECTED_CONTENT_1 = (const uint8_t* const)"0123456789";
static const uint8_t* const USTREAM_LOCAL_EXPECTED_CONTENT_2 = (const uint8_t* const)"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const uint8_t* const USTREAM_LOCAL_EXPECTED_CONTENT_3 = (const uint8_t* const)"abcdefghijklmnopqrstuvwxyz";

/* define constants for the compliance test */
#define USTREAM_EXPECTED_CONTENT        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAM_EXPECTED_CONTENT_LENGTH 62
static const uint8_t* const USTREAM_LOCAL_EXPECTED_CONTENT = (const uint8_t* const)USTREAM_EXPECTED_CONTENT;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%s", MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
}

/**
 * Beginning of the UT for ustream_base.c module.
 */
BEGIN_TEST_SUITE(ustream_base_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    int result;

    TEST_INITIALIZE_MEMORY_DEBUG(g_dll_by_dll);
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

    REGISTER_UMOCK_ALIAS_TYPE(AZULIB_USTREAM, void*);

    REGISTER_GLOBAL_MOCK_HOOK(azulib_ulib_malloc, malloc);
    REGISTER_GLOBAL_MOCK_HOOK(azulib_ulib_free, free);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_test_by_test);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dll_by_dll);
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
    TEST_MUTEX_RELEASE(g_test_by_test);
}

/* azulib_ustream_append shall append the second provided uStream at the end of the first one. */
/* azulib_ustream_append shall convert the first provided uStream to a multibuffer if the first uStream is not a multibuffer */
TEST_FUNCTION(azulib_ustream_append_start_from_empty_multibuffer_succeed)
{
    ///arrange
    AZULIB_USTREAM* default_multibuffer = azulib_ustream_multi_create();
    ASSERT_IS_NOT_NULL(default_multibuffer);

    AZULIB_USTREAM* default_buffer1 =
        azulib_ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);

    AZULIB_USTREAM* default_buffer2 =
        azulib_ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_2),
            NULL);
    ASSERT_IS_NOT_NULL(default_buffer2);

    AZULIB_USTREAM* default_buffer3 =
        azulib_ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_3,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_3), NULL);
    ASSERT_IS_NOT_NULL(default_buffer3);

    ///act
    AZULIB_RESULT result1 = azulib_ustream_append(default_multibuffer, default_buffer1);
    AZULIB_RESULT result2 = azulib_ustream_append(default_multibuffer, default_buffer2);
    AZULIB_RESULT result3 = azulib_ustream_append(default_multibuffer, default_buffer3);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, AZULIB_SUCCESS, result2);
    ASSERT_ARE_EQUAL(int, AZULIB_SUCCESS, result3);
    azulib_ustream_dispose(default_buffer1);
    azulib_ustream_dispose(default_buffer2);
    azulib_ustream_dispose(default_buffer3);
    check_buffer(
        default_multibuffer,
        0, 
        USTREAM_LOCAL_EXPECTED_CONTENT, 
        USTREAM_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    azulib_ustream_dispose(default_multibuffer);
}

/* azulib_ustream_append shall return AZULIB_SUCCESS if the uStreams were appended succesfully */
TEST_FUNCTION(azulib_ustream_append_append_multiple_buffers_succeed)
{
    ///arrange
    AZULIB_USTREAM* default_buffer1 =
        azulib_ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);

    AZULIB_USTREAM* default_buffer2 =
        azulib_ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_2),
            NULL);
    ASSERT_IS_NOT_NULL(default_buffer2);

    AZULIB_USTREAM* default_buffer3 =
        azulib_ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_3,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_3), NULL);
    ASSERT_IS_NOT_NULL(default_buffer3);

    ///act
    AZULIB_RESULT result1 = azulib_ustream_append(default_buffer1, default_buffer2);
    AZULIB_RESULT result2 = azulib_ustream_append(default_buffer1, default_buffer3);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, AZULIB_SUCCESS, result2);
    azulib_ustream_dispose(default_buffer2);
    azulib_ustream_dispose(default_buffer3);
    check_buffer(
        default_buffer1,
        0,
        USTREAM_LOCAL_EXPECTED_CONTENT,
        USTREAM_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    azulib_ustream_dispose(default_buffer1);
}

/* azulib_ustream_append shall return AZULIB_ILLEGAL_ARGUMENT_ERROR if the provided uStream is NULL */
TEST_FUNCTION(azulib_ustream_append_null_interface_failed)
{
    ///arrange
    AZULIB_USTREAM* default_buffer =
        azulib_ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer);

    ///act
    AZULIB_RESULT result = azulib_ustream_append(NULL, default_buffer);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    azulib_ustream_dispose(default_buffer);
}

/* azulib_ustream_append shall return AZULIB_ILLEGAL_ARGUMENT_ERROR if the provided uStream to add is NULL */
TEST_FUNCTION(azulib_ustream_append_null_buffer_to_add_failed)
{
    ///arrange
    AZULIB_USTREAM* default_buffer =
        azulib_ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer);

    ///act
    AZULIB_RESULT result = azulib_ustream_append(default_buffer, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    azulib_ustream_dispose(default_buffer);
}

/*  azulib_ustream_append shall return AZULIB_OUT_OF_MEMORY_ERROR if there is not enough memory to append the uStream */
TEST_FUNCTION(azulib_ustream_append_starting_from_multibuffer_with_not_enough_memory_failed)
{
    ///arrange
    AZULIB_USTREAM* default_multibuffer = azulib_ustream_multi_create();
    ASSERT_IS_NOT_NULL(default_multibuffer);

    AZULIB_USTREAM* default_buffer =
        azulib_ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(azulib_ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);

    ///act
    AZULIB_RESULT result = azulib_ustream_append(default_multibuffer, default_buffer);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, AZULIB_OUT_OF_MEMORY_ERROR, result);

    ///cleanup
    azulib_ustream_dispose(default_buffer);
    azulib_ustream_dispose(default_multibuffer);
}

/* azulib_ustream_append shall return AZULIB_OUT_OF_MEMORY_ERROR if there is not enough memory to create the multibuffer */
TEST_FUNCTION(azulib_ustream_append_not_enough_memory_to_create_multibuffer_failed)
{
    ///arrange
    AZULIB_USTREAM* default_buffer1 =
        azulib_ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);

    AZULIB_USTREAM* default_buffer2 =
        azulib_ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_2),
            NULL);
    ASSERT_IS_NOT_NULL(default_buffer2);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(azulib_ulib_malloc(sizeof(AZULIB_USTREAM))).SetReturn(NULL);

    ///act
    AZULIB_RESULT result = azulib_ustream_append(default_buffer1, default_buffer2);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, AZULIB_OUT_OF_MEMORY_ERROR, result);
    check_buffer(
        default_buffer1,
        0,
        USTREAM_LOCAL_EXPECTED_CONTENT_1,
        (uint8_t)strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1));
    check_buffer(
        default_buffer2,
        0,
        USTREAM_LOCAL_EXPECTED_CONTENT_2,
        (uint8_t)strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_2));

    ///cleanup
    azulib_ustream_dispose(default_buffer1);
    azulib_ustream_dispose(default_buffer2);
}

/* azulib_ustream_append shall return AZULIB_OUT_OF_MEMORY_ERROR if there is not enough memory to append the first uStream */
TEST_FUNCTION(azulib_ustream_append_not_enough_memory_to_append_first_buffer_failed)
{
    ///arrange
    AZULIB_USTREAM* default_buffer1 =
        azulib_ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);

    AZULIB_USTREAM* default_buffer2 =
        azulib_ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_2),
            NULL);
    ASSERT_IS_NOT_NULL(default_buffer2);

    umock_c_reset_all_calls();
    /* Create multibuffer */
    STRICT_EXPECTED_CALL(azulib_ulib_malloc(sizeof(AZULIB_USTREAM)));
    STRICT_EXPECTED_CALL(azulib_ulib_malloc(IGNORED_NUM_ARG));
    /* Append first buffer */
    STRICT_EXPECTED_CALL(azulib_ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    /* Release multibuffer */
    STRICT_EXPECTED_CALL(azulib_ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(azulib_ulib_free(IGNORED_PTR_ARG));

    ///act
    AZULIB_RESULT result = azulib_ustream_append(default_buffer1, default_buffer2);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, AZULIB_OUT_OF_MEMORY_ERROR, result);
    check_buffer(
        default_buffer1,
        0,
        USTREAM_LOCAL_EXPECTED_CONTENT_1,
        (uint8_t)strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1));
    check_buffer(
        default_buffer2,
        0,
        USTREAM_LOCAL_EXPECTED_CONTENT_2,
        (uint8_t)strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_2));

    ///cleanup
    azulib_ustream_dispose(default_buffer1);
    azulib_ustream_dispose(default_buffer2);
}

/* azulib_ustream_append shall return AZULIB_OUT_OF_MEMORY_ERROR if there is not enough memory to append the second uStream */
TEST_FUNCTION(azulib_ustream_append_not_enough_memory_to_append_second_buffer_failed)
{
    ///arrange
    AZULIB_USTREAM* default_buffer1 =
        azulib_ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);

    AZULIB_USTREAM* default_buffer2 =
        azulib_ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_2),
            NULL);
    ASSERT_IS_NOT_NULL(default_buffer2);

    umock_c_reset_all_calls();
    /* Create multibuffer */
    STRICT_EXPECTED_CALL(azulib_ulib_malloc(sizeof(AZULIB_USTREAM)));
    STRICT_EXPECTED_CALL(azulib_ulib_malloc(IGNORED_NUM_ARG));
    /* Append first buffer */
    STRICT_EXPECTED_CALL(azulib_ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(azulib_ulib_malloc(sizeof(AZULIB_USTREAM)));
    STRICT_EXPECTED_CALL(azulib_ulib_malloc(IGNORED_NUM_ARG));
    /* Append second buffer */
    STRICT_EXPECTED_CALL(azulib_ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    /* Release multibuffer */
    STRICT_EXPECTED_CALL(azulib_ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(azulib_ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(azulib_ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(azulib_ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(azulib_ulib_free(IGNORED_PTR_ARG));

    ///act
    AZULIB_RESULT result = azulib_ustream_append(default_buffer1, default_buffer2);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, AZULIB_OUT_OF_MEMORY_ERROR, result);
    check_buffer(
        default_buffer1,
        0,
        USTREAM_LOCAL_EXPECTED_CONTENT_1,
        (uint8_t)strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1));
    check_buffer(
        default_buffer2,
        0,
        USTREAM_LOCAL_EXPECTED_CONTENT_2,
        (uint8_t)strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_2));

    ///cleanup
    azulib_ustream_dispose(default_buffer1);
    azulib_ustream_dispose(default_buffer2);
}

END_TEST_SUITE(ustream_base_ut)
