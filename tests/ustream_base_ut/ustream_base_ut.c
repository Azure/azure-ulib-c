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

#ifdef __cplusplus
extern "C"
{
#endif

static int g_memory_counter;
void* my_malloc(size_t size)
{
    void* new_memo = malloc(size);
    if(new_memo != NULL)
    {
        g_memory_counter++;
    }
    return new_memo;
}

void my_free(void* ptr)
{
    if(ptr != NULL)
    {
        g_memory_counter--;
    }
    free(ptr);
}

#ifdef __cplusplus
}
#endif

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

    REGISTER_UMOCK_ALIAS_TYPE(USTREAM, void*);

    REGISTER_GLOBAL_MOCK_HOOK(ulib_malloc, my_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(ulib_free, my_free);
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

    g_memory_counter = 0;

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(test_method_cleanup)
{
    ASSERT_ARE_EQUAL(int, 0, g_memory_counter, "Memory issue");

    TEST_MUTEX_RELEASE(g_test_by_test);
}

/* ustream_create shall create an instance of the buffer and initialize the interface. */
TEST_FUNCTION(ustream_create_const_succeed)
{
    ///arrange
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));

    ///act
    USTREAM* buffer_interface = ustream_create(
        USTREAM_LOCAL_EXPECTED_CONTENT_1,
        sizeof(USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(buffer_interface);
    ASSERT_IS_NOT_NULL(buffer_interface->api);

    ///cleanup
    (void)ustream_dispose(buffer_interface);
}

/* ustream_create shall return NULL if there is not enough memory to create the buffer. */
TEST_FUNCTION(ustream_create_const_no_memory_to_create_interface_failed)
{
    ///arrange
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM))).SetReturn(NULL);
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    USTREAM* buffer_interface = ustream_create(
        USTREAM_LOCAL_EXPECTED_CONTENT_1,
        sizeof(USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
}

/* ustream_create shall return NULL if there is not enough memory to create the instance */
TEST_FUNCTION(ustream_create_const_no_memory_to_create_instance_failed)
{
    ///arrange
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    USTREAM* buffer_interface = ustream_create(
        USTREAM_LOCAL_EXPECTED_CONTENT_1,
        sizeof(USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
}

/* ustream_create shall return NULL if there is not enough memory to create the inner buffer */
TEST_FUNCTION(ustream_create_const_no_memory_to_create_inner_buffer_failed)
{
    ///arrange
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);

    ///act
    USTREAM* buffer_interface = ustream_create(
        USTREAM_LOCAL_EXPECTED_CONTENT_1,
        sizeof(USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
}

/* ustream_create shall return NULL if the provided constant buffer is NULL */
TEST_FUNCTION(ustream_create_const_null_buffer_failed)
{
    ///arrange

    ///act
    USTREAM* buffer_interface = ustream_create(NULL, sizeof(USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
}

/* ustream_create shall return NULL ff the provided buffer length is zero */
TEST_FUNCTION(ustream_create_const_zero_length_failed)
{
    ///arrange

    ///act
    USTREAM* buffer_interface = ustream_create(USTREAM_LOCAL_EXPECTED_CONTENT_1, 0, NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
}


/* ustream_append shall append the second provided buffer at the end of the first one. */
/* ustream_append shall convert the first provided buffer to a multibuffer if the first buffer is not a multibuffer */
TEST_FUNCTION(ustream_append_start_from_empty_multibuffer_succeed)
{
    ///arrange
    USTREAM* default_multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(default_multibuffer);

    USTREAM* default_buffer1 =
        ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);

    USTREAM* default_buffer2 =
        ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_2),
            NULL);
    ASSERT_IS_NOT_NULL(default_buffer2);

    USTREAM* default_buffer3 =
        ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_3,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_3), NULL);
    ASSERT_IS_NOT_NULL(default_buffer3);

    ///act
    ULIB_RESULT result1 = ustream_append(default_multibuffer, default_buffer1);
    ULIB_RESULT result2 = ustream_append(default_multibuffer, default_buffer2);
    ULIB_RESULT result3 = ustream_append(default_multibuffer, default_buffer3);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result2);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result3);
    ustream_dispose(default_buffer1);
    ustream_dispose(default_buffer2);
    ustream_dispose(default_buffer3);
    check_buffer(
        default_multibuffer,
        0, 
        USTREAM_LOCAL_EXPECTED_CONTENT, 
        USTREAM_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    ustream_dispose(default_multibuffer);
}

/* ustream_append shall return ULIB_SUCCESS if the uStreams were appended succesfully */
TEST_FUNCTION(ustream_append_append_multiple_buffers_succeed)
{
    ///arrange
    USTREAM* default_buffer1 =
        ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);

    USTREAM* default_buffer2 =
        ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_2),
            NULL);
    ASSERT_IS_NOT_NULL(default_buffer2);

    USTREAM* default_buffer3 =
        ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_3,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_3), NULL);
    ASSERT_IS_NOT_NULL(default_buffer3);

    ///act
    ULIB_RESULT result1 = ustream_append(default_buffer1, default_buffer2);
    ULIB_RESULT result2 = ustream_append(default_buffer1, default_buffer3);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result2);
    ustream_dispose(default_buffer2);
    ustream_dispose(default_buffer3);
    check_buffer(
        default_buffer1,
        0,
        USTREAM_LOCAL_EXPECTED_CONTENT,
        USTREAM_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    ustream_dispose(default_buffer1);
}

/* ustream_append shall return ULIB_ILLEGAL_ARGUMENT_ERROR if the provided interface is NULL */
TEST_FUNCTION(ustream_append_null_interface_failed)
{
    ///arrange
    USTREAM* default_buffer =
        ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer);

    ///act
    ULIB_RESULT result = ustream_append(NULL, default_buffer);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    ustream_dispose(default_buffer);
}

/* ustream_append shall return ULIB_ILLEGAL_ARGUMENT_ERROR if the provided buffer to add is NULL */
TEST_FUNCTION(ustream_append_null_buffer_to_add_failed)
{
    ///arrange
    USTREAM* default_buffer =
        ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer);

    ///act
    ULIB_RESULT result = ustream_append(default_buffer, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    ustream_dispose(default_buffer);
}

/*  ustream_append shall return ULIB_OUT_OF_MEMORY_ERROR if there is not enough memory to append the buffer */
TEST_FUNCTION(ustream_append_starting_from_multibuffer_with_not_enough_memory_failed)
{
    ///arrange
    USTREAM* default_multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(default_multibuffer);

    USTREAM* default_buffer =
        ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);

    ///act
    ULIB_RESULT result = ustream_append(default_multibuffer, default_buffer);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, ULIB_OUT_OF_MEMORY_ERROR, result);

    ///cleanup
    ustream_dispose(default_buffer);
    ustream_dispose(default_multibuffer);
}

/* ustream_append shall return ULIB_OUT_OF_MEMORY_ERROR if there is not enough memory to create the multibuffer */
TEST_FUNCTION(ustream_append_not_enough_memory_to_create_multibuffer_failed)
{
    ///arrange
    USTREAM* default_buffer1 =
        ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);

    USTREAM* default_buffer2 =
        ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_2),
            NULL);
    ASSERT_IS_NOT_NULL(default_buffer2);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM))).SetReturn(NULL);

    ///act
    ULIB_RESULT result = ustream_append(default_buffer1, default_buffer2);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, ULIB_OUT_OF_MEMORY_ERROR, result);
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
    ustream_dispose(default_buffer1);
    ustream_dispose(default_buffer2);
}

/* ustream_append shall return ULIB_OUT_OF_MEMORY_ERROR if there is not enough memory to append the first uStream */
TEST_FUNCTION(ustream_append_not_enough_memory_to_append_first_buffer_failed)
{
    ///arrange
    USTREAM* default_buffer1 =
        ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);

    USTREAM* default_buffer2 =
        ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_2),
            NULL);
    ASSERT_IS_NOT_NULL(default_buffer2);

    umock_c_reset_all_calls();
    /* Create multibuffer */
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    /* Append first buffer */
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    /* Release multibuffer */
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    ULIB_RESULT result = ustream_append(default_buffer1, default_buffer2);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, ULIB_OUT_OF_MEMORY_ERROR, result);
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
    ustream_dispose(default_buffer1);
    ustream_dispose(default_buffer2);
}

/* ustream_append shall return ULIB_OUT_OF_MEMORY_ERROR if there is not enough memory to append the second uStream */
TEST_FUNCTION(ustream_append_not_enough_memory_to_append_second_buffer_failed)
{
    ///arrange
    USTREAM* default_buffer1 =
        ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);

    USTREAM* default_buffer2 =
        ustream_create(
            USTREAM_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_LOCAL_EXPECTED_CONTENT_2),
            NULL);
    ASSERT_IS_NOT_NULL(default_buffer2);

    umock_c_reset_all_calls();
    /* Create multibuffer */
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    /* Append first buffer */
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    /* Append second buffer */
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    /* Release multibuffer */
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    ULIB_RESULT result = ustream_append(default_buffer1, default_buffer2);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, ULIB_OUT_OF_MEMORY_ERROR, result);
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
    ustream_dispose(default_buffer1);
    ustream_dispose(default_buffer2);
}

END_TEST_SUITE(ustream_base_ut)
