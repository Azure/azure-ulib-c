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

static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1 =
        (const uint8_t* const)"0123456789";
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2 =
        (const uint8_t* const)"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3 =
        (const uint8_t* const)"abcdefghijklmnopqrstuvwxyz";

static USTREAM* create_test_default_multibuffer()
{
    USTREAM* default_multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(default_multibuffer);

    USTREAM* default_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(default_multibuffer, default_buffer1));

    USTREAM* default_buffer2 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2),
            NULL);
    ASSERT_IS_NOT_NULL(default_buffer2);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(default_multibuffer, default_buffer2));

    USTREAM* default_buffer3 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3), NULL);
    ASSERT_IS_NOT_NULL(default_buffer3);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(default_multibuffer, default_buffer3));

    (void)ustream_dispose(default_buffer1);
    (void)ustream_dispose(default_buffer2);
    (void)ustream_dispose(default_buffer3);

    return default_multibuffer;
}

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT =
        (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;
#define USTREAM_COMPLIANCE_TARGET_FACTORY          create_test_default_multibuffer()

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%s", MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
}

/**
 * Beginning of the UT for ustream_multi.c module.
 */
BEGIN_TEST_SUITE(ustream_multi_ut)

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

/* The ustream_multi_create shall create an instance of the multi-buffer and initialize the interface. */
TEST_FUNCTION(ustream_multi_create_succeed)
{
    ///arrange
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));

    ///act
    USTREAM* multibuffer = ustream_multi_create();

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(multibuffer);
    ASSERT_IS_NOT_NULL(multibuffer->api);

    ///cleanup
    (void)ustream_dispose(multibuffer);
}

/* The ustream_multi_create shall return NULL if there is no memory to create the multi-buffer interface. */
TEST_FUNCTION(ustream_multi_create_no_memory_to_create_interface_failed)
{
    ///arrange
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM))).SetReturn(NULL);

    ///act
    USTREAM* multibuffer = ustream_multi_create();

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(multibuffer);

    ///cleanup
}

TEST_FUNCTION(ustream_multi_create_no_memory_to_create_instance_failed)
{
    ///arrange
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    USTREAM* multibuffer = ustream_multi_create();

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(multibuffer);

    ///cleanup
}

/* The ustream_multi_append shall add the provided buffer to the multibuffer list. */
TEST_FUNCTION(ustream_multi_append_new_multibuffer_succeed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* test_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(test_buffer1);
    USTREAM* test_buffer2 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2), NULL);
    ASSERT_IS_NOT_NULL(test_buffer2);
    USTREAM* test_buffer3 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3), NULL);
    ASSERT_IS_NOT_NULL(test_buffer3);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));

    ///act
    ULIB_RESULT result1 = ustream_multi_append(multibuffer, test_buffer1);
    ULIB_RESULT result2 = ustream_multi_append(multibuffer, test_buffer2);
    ULIB_RESULT result3 = ustream_multi_append(multibuffer, test_buffer3);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result2);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result3);

    size_t size;
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_get_remaining_size(multibuffer, &size));
    ASSERT_ARE_EQUAL(
	    int, 
	    strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 
	    strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2) + 
	    strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3), size);

    ///cleanup
    (void)ustream_dispose(test_buffer1);
    (void)ustream_dispose(test_buffer2);
    (void)ustream_dispose(test_buffer3);
    (void)ustream_dispose(multibuffer);
}

TEST_FUNCTION(ustream_multi_append_partial_released_multibuffer_succeed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* test_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(test_buffer1);
    USTREAM* test_buffer2 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2), NULL);
    ASSERT_IS_NOT_NULL(test_buffer2);
    USTREAM* test_buffer3 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3), NULL);
    ASSERT_IS_NOT_NULL(test_buffer3);

    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, test_buffer1));
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, test_buffer2));
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, test_buffer3));
    
    // set_position to half of the buffer
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(multibuffer, (USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH / 2)));
    
    // release current - 1
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_release(multibuffer, (USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH / 2) - 1));

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));

    ///act
    ULIB_RESULT result1 = ustream_multi_append(multibuffer, test_buffer1);
    ULIB_RESULT result2 = ustream_multi_append(multibuffer, test_buffer2);
    ULIB_RESULT result3 = ustream_multi_append(multibuffer, test_buffer3);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result2);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result3);

    size_t size;
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_get_remaining_size(multibuffer, &size));
    ASSERT_ARE_EQUAL(int, 
            (strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) +
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2) +
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3) +
            (USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH / 2)), 
        size);

    ///cleanup
    (void)ustream_dispose(test_buffer1);
    (void)ustream_dispose(test_buffer2);
    (void)ustream_dispose(test_buffer3);
    (void)ustream_dispose(multibuffer);
}

TEST_FUNCTION(ustream_multi_append_fully_released_multibuffer_succeed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* test_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(test_buffer1);
    USTREAM* test_buffer2 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2), NULL);
    ASSERT_IS_NOT_NULL(test_buffer2);
    USTREAM* test_buffer3 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3), NULL);
    ASSERT_IS_NOT_NULL(test_buffer3);

    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, test_buffer1));
    
    // set_position to end of the buffer
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(multibuffer, strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1)));
    
    // release all
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_release(multibuffer, strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) - 1));

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));

    ///act
    ULIB_RESULT result1 = ustream_multi_append(multibuffer, test_buffer1);
    ULIB_RESULT result2 = ustream_multi_append(multibuffer, test_buffer2);
    ULIB_RESULT result3 = ustream_multi_append(multibuffer, test_buffer3);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result2);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result3);

    size_t size;
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_get_remaining_size(multibuffer, &size));
    ASSERT_ARE_EQUAL(
	    int, 
	    strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 
	    strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2) + 
	    strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3), size);

    ///cleanup
    (void)ustream_dispose(test_buffer1);
    (void)ustream_dispose(test_buffer2);
    (void)ustream_dispose(test_buffer3);
    (void)ustream_dispose(multibuffer);
}

/* If the provided handle is NULL, the ustream_multi_append shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(ustream_multi_append_null_multibuffer_failed)
{
    ///arrange
    USTREAM* test_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);

    ///act
    ULIB_RESULT result = ustream_multi_append(NULL, test_buffer1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)ustream_dispose(test_buffer1);
}

/* If the provided handle is not the implemented buffer type, the ustream_multi_append shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(ustream_multi_append_buffer_is_not_type_of_buffer_failed)
{
    ///arrange
    USTREAM* test_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    USTREAM* test_buffer2 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2), NULL);

    ///act
    ULIB_RESULT result = ustream_multi_append(test_buffer1, test_buffer2);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)ustream_dispose(test_buffer1);
    (void)ustream_dispose(test_buffer2);
}

/* If the provided buffer to add is NULL, the ustream_multi_append shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(ustream_multi_append_null_buffer_to_add_failed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);
    USTREAM* test_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ustream_multi_append(multibuffer, test_buffer1);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));

    ///act
    ULIB_RESULT result = ustream_multi_append(multibuffer, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)ustream_dispose(test_buffer1);
    (void)ustream_dispose(multibuffer);
}

/* If there is no memory to control the new buffer, the ustream_multi_append shall return ULIB_OUT_OF_MEMORY_ERROR. */
TEST_FUNCTION(ustream_multi_append_not_enough_memory_failed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);
    USTREAM* test_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);

    ///act
    ULIB_RESULT result = ustream_multi_append(multibuffer, test_buffer1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_OUT_OF_MEMORY_ERROR, result);

    ///cleanup
    (void)ustream_dispose(test_buffer1);
    (void)ustream_dispose(multibuffer);
}

/* If the ustream_multi_append failed to copy the buffer, it shall return ULIB_OUT_OF_MEMORY_ERROR. */
TEST_FUNCTION(ustream_multi_append_not_enough_memory_to_clone_the_buffer_failed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);
    USTREAM* test_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    ULIB_RESULT result = ustream_multi_append(multibuffer, test_buffer1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_OUT_OF_MEMORY_ERROR, result);

    ///cleanup
    (void)ustream_dispose(test_buffer1);
    (void)ustream_dispose(multibuffer);
}

/* If the ustream_multi_append failed to copy the buffer, it shall return ULIB_OUT_OF_MEMORY_ERROR. */
TEST_FUNCTION(ustream_multi_append_new_inner_buffer_failed_on_get_remaining_size_failed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* default_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer1));

    USTREAM* default_buffer2 = ustream_mock_create();

    set_get_remaining_size_result(ULIB_SYSTEM_ERROR);

    ///act
    ULIB_RESULT result = ustream_multi_append(multibuffer, default_buffer2);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SYSTEM_ERROR, result);

    ///cleanup
    (void)ustream_dispose(default_buffer1);
    (void)ustream_dispose(default_buffer2);
    (void)ustream_dispose(multibuffer);
}

/* The dispose shall ulib_free all allocated resources. */
TEST_FUNCTION(ustream_multi_dispose_multibuffer_without_buffers_free_all_resources_succeed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    ULIB_RESULT result = ustream_dispose(multibuffer);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);

    ///cleanup
}

/* If the multibuffer contains appended buffers, the dispose shall release all buffers on its list. */
TEST_FUNCTION(ustream_multi_dispose_multibuffer_with_buffers_free_all_resources_succeed)
{
    ///arrange
    USTREAM* multibuffer = USTREAM_COMPLIANCE_TARGET_FACTORY;

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    ULIB_RESULT result = ustream_dispose(multibuffer);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);

    ///cleanup
}

/* The set_position shall bypass the error if the Inner ustream return not success for one of the needed operations. */
TEST_FUNCTION(ustream_multi_seek_inner_buffer_failed_in_get_current_position_failed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* default_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer1));

    USTREAM* default_buffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer2));
    set_get_position_result(ULIB_SYSTEM_ERROR);

    ///act
    ULIB_RESULT result = 
        ustream_set_position(
            multibuffer, 
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SYSTEM_ERROR, result);
    offset_t pos;
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_get_position(multibuffer, &pos));
    ASSERT_ARE_EQUAL(int, 0, pos);

    ///cleanup
    (void)ustream_dispose(multibuffer);
    (void)ustream_dispose(default_buffer1);
    (void)ustream_dispose(default_buffer2);
}

TEST_FUNCTION(ustream_multi_seek_inner_buffer_failed_in_get_remaining_size_failed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* default_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer1));

    USTREAM* default_buffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer2));
    set_get_remaining_size_result(ULIB_SYSTEM_ERROR);

    ///act
    ULIB_RESULT result = 
        ustream_set_position(
            multibuffer, 
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SYSTEM_ERROR, result);
    offset_t pos;
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_get_position(multibuffer, &pos));
    ASSERT_ARE_EQUAL(int, 0, pos);

    ///cleanup
    (void)ustream_dispose(multibuffer);
    (void)ustream_dispose(default_buffer1);
    (void)ustream_dispose(default_buffer2);
}

TEST_FUNCTION(ustream_multi_seek_inner_buffer_failed_in_seek_failed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* default_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer1));

    USTREAM* default_buffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer2));
    set_set_position_result(ULIB_SYSTEM_ERROR);

    ///act
    ULIB_RESULT result = 
        ustream_set_position(
            multibuffer, 
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SYSTEM_ERROR, result);
    offset_t pos;
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_get_position(multibuffer, &pos));
    ASSERT_ARE_EQUAL(int, 0, pos);

    ///cleanup
    (void)ustream_dispose(multibuffer);
    (void)ustream_dispose(default_buffer1);
    (void)ustream_dispose(default_buffer2);
}

/* The Read shall return partial result if one of the internal buffers failed. */
TEST_FUNCTION(ustream_multi_read_inner_buffer_failed_in_read_with_some_valid_content_succeed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* default_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer1));

    USTREAM* default_buffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer2));
    set_read_result(ULIB_SYSTEM_ERROR);

    uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
    size_t size_result;


    ///act
    ULIB_RESULT result =
        ustream_read(
            multibuffer,
            buf_result,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, buf_result, size_result);

    ///cleanup
    (void)ustream_dispose(multibuffer);
    (void)ustream_dispose(default_buffer1);
    (void)ustream_dispose(default_buffer2);
}

TEST_FUNCTION(ustream_multi_read_inner_buffer_failed_in_read_failed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* default_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer1));

    USTREAM* default_buffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer2));
    set_read_result(ULIB_SYSTEM_ERROR);

    uint8_t buf_result[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
    size_t size_result;

    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(
            multibuffer,
            buf_result,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &size_result));
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, buf_result, size_result);

    set_read_result(ULIB_SYSTEM_ERROR);

    ///act
    ULIB_RESULT result =
        ustream_read(
            multibuffer,
            buf_result,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SYSTEM_ERROR, result);

    ///cleanup
    (void)ustream_dispose(multibuffer);
    (void)ustream_dispose(default_buffer1);
    (void)ustream_dispose(default_buffer2);
}

/* The Release shall bypass the error if the Inner ustream return not success for one of the needed operations. */
TEST_FUNCTION(ustream_multi_release_inner_buffer_failed_in_get_current_position_failed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* default_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer1));

    USTREAM* default_buffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer2));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(multibuffer, strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 2));

    set_get_position_result(ULIB_SYSTEM_ERROR);

    ///act
    ULIB_RESULT result =
        ustream_release(
            multibuffer,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SYSTEM_ERROR, result);
    offset_t pos;
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_get_position(multibuffer, &pos));
    ASSERT_ARE_EQUAL(int, strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 2, pos);

    ///cleanup
    (void)ustream_dispose(multibuffer);
    (void)ustream_dispose(default_buffer1);
    (void)ustream_dispose(default_buffer2);
}

TEST_FUNCTION(ustream_multi_release_inner_buffer_failed_in_get_remaining_size_failed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* default_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer1));

    USTREAM* default_buffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer2));
    ASSERT_ARE_EQUAL(
        int,
        ULIB_SUCCESS,
        ustream_set_position(multibuffer, strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 2));

    set_get_remaining_size_result(ULIB_SYSTEM_ERROR);

    ///act
    ULIB_RESULT result =
        ustream_release(
            multibuffer,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SYSTEM_ERROR, result);
    offset_t pos;
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_get_position(multibuffer, &pos));
    ASSERT_ARE_EQUAL(int, strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 2, pos);

    ///cleanup
    (void)ustream_dispose(multibuffer);
    (void)ustream_dispose(default_buffer1);
    (void)ustream_dispose(default_buffer2);
}

/* The Clone shall bypass the error if the Inner ustream return not success for one of the needed operations. */
TEST_FUNCTION(ustream_multi_clone_inner_buffer_failed_in_get_remaining_size_failed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* default_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer1));

    USTREAM* default_buffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer2));

    set_get_remaining_size_result(ULIB_SYSTEM_ERROR);

    ///act
    USTREAM* clone_result = ustream_clone(multibuffer, 0);

    ///assert
    ASSERT_IS_NULL(clone_result);

    ///cleanup
    (void)ustream_dispose(multibuffer);
    (void)ustream_dispose(default_buffer1);
    (void)ustream_dispose(default_buffer2);
}

/*  The clone shall return NULL if there is not enough memory to control the new buffer. */
TEST_FUNCTION(ustream_clone_no_memory_to_create_interface_failed)
{
    ///arrange
    USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();
    EXPECTED_CALL(ulib_malloc(sizeof(USTREAM))).SetReturn(NULL);

    ///act
    USTREAM* ustream_clone_interface = ustream_clone(ustream_instance, 0);

    ///assert
    ASSERT_IS_NULL(ustream_clone_interface);

    ///cleanup
    (void)ustream_dispose(ustream_instance);
}

TEST_FUNCTION(ustream_clone_no_memory_to_create_instance_failed)
{
    ///arrange
    USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();
    EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    USTREAM* ustream_clone_interface = ustream_clone(ustream_instance, 0);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(ustream_clone_interface);

    ///cleanup
    (void)ustream_dispose(ustream_instance);
}

TEST_FUNCTION(ustream_clone_no_memory_to_create_first_node_failed)
{
    ///arrange
    USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();
    EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(ulib_malloc(sizeof(USTREAM))).SetReturn(NULL);
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    USTREAM* ustream_clone_interface = ustream_clone(ustream_instance, 0);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(ustream_clone_interface);

    ///cleanup
    (void)ustream_dispose(ustream_instance);
}

TEST_FUNCTION(ustream_clone_no_memory_to_clone_first_node_failed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* default_buffer1 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1), NULL);
    ASSERT_IS_NOT_NULL(default_buffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer1));

    USTREAM* default_buffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, default_buffer2));

    set_clone_result(ULIB_SYSTEM_ERROR);

    ///act
    USTREAM* ustream_clone_interface = ustream_clone(multibuffer, 0);

    ///assert
    ASSERT_IS_NULL(ustream_clone_interface);

    ///cleanup
    (void)ustream_dispose(multibuffer);
    (void)ustream_dispose(default_buffer1);
    (void)ustream_dispose(default_buffer2);
}

TEST_FUNCTION(ustream_clone_no_memory_to_create_second_node_failed)
{
    ///arrange
    USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();
    EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(ulib_malloc(sizeof(USTREAM))).SetReturn(NULL);
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    USTREAM* ustream_clone_interface = ustream_clone(ustream_instance, 0);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(ustream_clone_interface);

    ///cleanup
    (void)ustream_dispose(ustream_instance);
}

#include "ustream_compliance_ut.h"

END_TEST_SUITE(ustream_multi_ut)
