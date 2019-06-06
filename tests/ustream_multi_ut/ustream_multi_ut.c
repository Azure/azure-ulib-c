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

static int g_memoryCounter;
void* myMalloc(size_t size)
{
    void* newMemo = malloc(size);
    if(newMemo != NULL)
    {
        g_memoryCounter++;
    }
    return newMemo;
}

void myFree(void* ptr)
{
    if(ptr != NULL)
    {
        g_memoryCounter--;
    }
    free(ptr);
}

#ifdef __cplusplus
}
#endif

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

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

static USTREAM* createTestDefaultMultibuffer()
{
    USTREAM* defaultMultibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(defaultMultibuffer);

    USTREAM* defaultBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(defaultMultibuffer, defaultBuffer1));

    USTREAM* defaultBuffer2 =
        ustream_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2),
            false);
    ASSERT_IS_NOT_NULL(defaultBuffer2);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(defaultMultibuffer, defaultBuffer2));

    USTREAM* defaultBuffer3 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3));
    ASSERT_IS_NOT_NULL(defaultBuffer3);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(defaultMultibuffer, defaultBuffer3));

    (void)ustream_dispose(defaultBuffer1);
    (void)ustream_dispose(defaultBuffer2);
    (void)ustream_dispose(defaultBuffer3);

    return defaultMultibuffer;
}

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT =
        (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;
#define USTREAM_COMPLIANCE_TARGET_FACTORY          createTestDefaultMultibuffer()

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

    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    result = umock_c_init(on_umock_c_error);
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_charptr_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_stdint_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);
    result = umocktypes_bool_register_types();
    ASSERT_ARE_EQUAL(int, 0, result);

    REGISTER_UMOCK_ALIAS_TYPE(USTREAM, void*);

    REGISTER_GLOBAL_MOCK_HOOK(ulib_malloc, myMalloc);
    REGISTER_GLOBAL_MOCK_HOOK(ulib_free, myFree);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    g_memoryCounter = 0;

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    ASSERT_ARE_EQUAL(int, 0, g_memoryCounter, "Memory issue");

    TEST_MUTEX_RELEASE(g_testByTest);
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
TEST_FUNCTION(ustream_multi_create_noMemoryToCreateInterfaceFailed)
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

TEST_FUNCTION(ustream_multi_create_noMemoryTocreate_instanceFailed)
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
TEST_FUNCTION(ustream_multi_append_newMultibufferSucceed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* testBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(testBuffer1);
    USTREAM* testBuffer2 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2));
    ASSERT_IS_NOT_NULL(testBuffer2);
    USTREAM* testBuffer3 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3));
    ASSERT_IS_NOT_NULL(testBuffer3);

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
    ULIB_RESULT result1 = ustream_multi_append(multibuffer, testBuffer1);
    ULIB_RESULT result2 = ustream_multi_append(multibuffer, testBuffer2);
    ULIB_RESULT result3 = ustream_multi_append(multibuffer, testBuffer3);

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
    (void)ustream_dispose(testBuffer1);
    (void)ustream_dispose(testBuffer2);
    (void)ustream_dispose(testBuffer3);
    (void)ustream_dispose(multibuffer);
}

TEST_FUNCTION(ustream_multi_append_partialReleasedMultibufferSucceed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* testBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(testBuffer1);
    USTREAM* testBuffer2 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2));
    ASSERT_IS_NOT_NULL(testBuffer2);
    USTREAM* testBuffer3 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3));
    ASSERT_IS_NOT_NULL(testBuffer3);

    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, testBuffer1));
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, testBuffer2));
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, testBuffer3));
    
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
    ULIB_RESULT result1 = ustream_multi_append(multibuffer, testBuffer1);
    ULIB_RESULT result2 = ustream_multi_append(multibuffer, testBuffer2);
    ULIB_RESULT result3 = ustream_multi_append(multibuffer, testBuffer3);

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
    (void)ustream_dispose(testBuffer1);
    (void)ustream_dispose(testBuffer2);
    (void)ustream_dispose(testBuffer3);
    (void)ustream_dispose(multibuffer);
}

TEST_FUNCTION(ustream_multi_append_fullyReleasedMultibufferSucceed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* testBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(testBuffer1);
    USTREAM* testBuffer2 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2));
    ASSERT_IS_NOT_NULL(testBuffer2);
    USTREAM* testBuffer3 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3));
    ASSERT_IS_NOT_NULL(testBuffer3);

    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, testBuffer1));
    
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
    ULIB_RESULT result1 = ustream_multi_append(multibuffer, testBuffer1);
    ULIB_RESULT result2 = ustream_multi_append(multibuffer, testBuffer2);
    ULIB_RESULT result3 = ustream_multi_append(multibuffer, testBuffer3);

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
    (void)ustream_dispose(testBuffer1);
    (void)ustream_dispose(testBuffer2);
    (void)ustream_dispose(testBuffer3);
    (void)ustream_dispose(multibuffer);
}

/* If the provided handle is NULL, the ustream_multi_append shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(ustream_multi_append_nullMultibufferFailed)
{
    ///arrange
    USTREAM* testBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));

    ///act
    ULIB_RESULT result = ustream_multi_append(NULL, testBuffer1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)ustream_dispose(testBuffer1);
}

/* If the provided handle is not the implemented buffer type, the ustream_multi_append shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(ustream_multi_append_bufferIsNotTypeOfBufferFailed)
{
    ///arrange
    USTREAM* testBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    USTREAM* testBuffer2 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2));

    ///act
    ULIB_RESULT result = ustream_multi_append(testBuffer1, testBuffer2);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)ustream_dispose(testBuffer1);
    (void)ustream_dispose(testBuffer2);
}

/* If the provided buffer to add is NULL, the ustream_multi_append shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(ustream_multi_append_nullBufferToAddFailed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);
    USTREAM* testBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ustream_multi_append(multibuffer, testBuffer1);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));

    ///act
    ULIB_RESULT result = ustream_multi_append(multibuffer, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)ustream_dispose(testBuffer1);
    (void)ustream_dispose(multibuffer);
}

/* If there is no memory to control the new buffer, the ustream_multi_append shall return ULIB_OUT_OF_MEMORY_ERROR. */
TEST_FUNCTION(ustream_multi_append_notEnoughMemoryFailed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);
    USTREAM* testBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);

    ///act
    ULIB_RESULT result = ustream_multi_append(multibuffer, testBuffer1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_OUT_OF_MEMORY_ERROR, result);

    ///cleanup
    (void)ustream_dispose(testBuffer1);
    (void)ustream_dispose(multibuffer);
}

/* If the ustream_multi_append failed to copy the buffer, it shall return ULIB_OUT_OF_MEMORY_ERROR. */
TEST_FUNCTION(ustream_multi_append_notEnoughMemoryToCloneTheBufferFailed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);
    USTREAM* testBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    ULIB_RESULT result = ustream_multi_append(multibuffer, testBuffer1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_OUT_OF_MEMORY_ERROR, result);

    ///cleanup
    (void)ustream_dispose(testBuffer1);
    (void)ustream_dispose(multibuffer);
}

/* If the ustream_multi_append failed to copy the buffer, it shall return ULIB_OUT_OF_MEMORY_ERROR. */
TEST_FUNCTION(ustream_multi_append_newInnerBufferFailedOnGetRemainingSizeFailed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* defaultBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer1));

    USTREAM* defaultBuffer2 = ustream_mock_create();

    set_get_remaining_size_result(ULIB_SYSTEM_ERROR);

    ///act
    ULIB_RESULT result = ustream_multi_append(multibuffer, defaultBuffer2);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SYSTEM_ERROR, result);

    ///cleanup
    (void)ustream_dispose(defaultBuffer1);
    (void)ustream_dispose(defaultBuffer2);
    (void)ustream_dispose(multibuffer);
}

/* The dispose shall ulib_free all allocated resources. */
TEST_FUNCTION(ustream_multi_dispose_multibufferWithoutBuffersFreeAllResourcesSucceed)
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
TEST_FUNCTION(ustream_multi_dispose_multibufferWithBuffersFreeAllResourcesSucceed)
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
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    ULIB_RESULT result = ustream_dispose(multibuffer);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);

    ///cleanup
}

/* The set_position shall bypass the error if the Inner uStream return not success for one of the needed operations. */
TEST_FUNCTION(ustream_multi_seek_innerBufferFailedInGetCurrentPositionFailed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* defaultBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer1));

    USTREAM* defaultBuffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer2));
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
    (void)ustream_dispose(defaultBuffer1);
    (void)ustream_dispose(defaultBuffer2);
}

TEST_FUNCTION(ustream_multi_seek_innerBufferFailedInGetRemainingSizeFailed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* defaultBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer1));

    USTREAM* defaultBuffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer2));
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
    (void)ustream_dispose(defaultBuffer1);
    (void)ustream_dispose(defaultBuffer2);
}

TEST_FUNCTION(ustream_multi_seek_innerBufferFailedInSeekFailed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* defaultBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer1));

    USTREAM* defaultBuffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer2));
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
    (void)ustream_dispose(defaultBuffer1);
    (void)ustream_dispose(defaultBuffer2);
}

/* The Read shall return partial result if one of the internal buffers failed. */
TEST_FUNCTION(uStreamMultiRead_innerBufferFailedInReadWithSomeValidContentSucceed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* defaultBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer1));

    USTREAM* defaultBuffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer2));
    set_read_result(ULIB_SYSTEM_ERROR);

    uint8_t bufResult[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
    size_t sizeResult;


    ///act
    ULIB_RESULT result =
        ustream_read(
            multibuffer,
            bufResult,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, bufResult, sizeResult);

    ///cleanup
    (void)ustream_dispose(multibuffer);
    (void)ustream_dispose(defaultBuffer1);
    (void)ustream_dispose(defaultBuffer2);
}

TEST_FUNCTION(uStreamMultiRead_innerBufferFailedInReadFailed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* defaultBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer1));

    USTREAM* defaultBuffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer2));
    set_read_result(ULIB_SYSTEM_ERROR);

    uint8_t bufResult[USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
    size_t sizeResult;

    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(
            multibuffer,
            bufResult,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &sizeResult));
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, bufResult, sizeResult);

    set_read_result(ULIB_SYSTEM_ERROR);

    ///act
    ULIB_RESULT result =
        ustream_read(
            multibuffer,
            bufResult,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SYSTEM_ERROR, result);

    ///cleanup
    (void)ustream_dispose(multibuffer);
    (void)ustream_dispose(defaultBuffer1);
    (void)ustream_dispose(defaultBuffer2);
}

/* The Release shall bypass the error if the Inner uStream return not success for one of the needed operations. */
TEST_FUNCTION(uStreamMultiRelease_innerBufferFailedInGetCurrentPositionFailed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* defaultBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer1));

    USTREAM* defaultBuffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer2));
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
    (void)ustream_dispose(defaultBuffer1);
    (void)ustream_dispose(defaultBuffer2);
}

TEST_FUNCTION(uStreamMultiRelease_innerBufferFailedInGetRemainingSizeFailed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* defaultBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer1));

    USTREAM* defaultBuffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer2));
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
    (void)ustream_dispose(defaultBuffer1);
    (void)ustream_dispose(defaultBuffer2);
}

/* The Clone shall bypass the error if the Inner uStream return not success for one of the needed operations. */
TEST_FUNCTION(uStreamMultiClone_innerBufferFailedInGetRemainingSizeFailed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* defaultBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer1));

    USTREAM* defaultBuffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer2));

    set_get_remaining_size_result(ULIB_SYSTEM_ERROR);

    ///act
    USTREAM* cloneResult = ustream_clone(multibuffer, 0);

    ///assert
    ASSERT_IS_NULL(cloneResult);

    ///cleanup
    (void)ustream_dispose(multibuffer);
    (void)ustream_dispose(defaultBuffer1);
    (void)ustream_dispose(defaultBuffer2);
}

/*  The clone shall return NULL if there is not enough memory to control the new buffer. */
TEST_FUNCTION(ustream_clone_noMemoryToCreateInterfaceFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();
    EXPECTED_CALL(ulib_malloc(sizeof(USTREAM))).SetReturn(NULL);

    ///act
    USTREAM* uStreamCloneInterface = ustream_clone(uStreamInstance, 0);

    ///assert
    ASSERT_IS_NULL(uStreamCloneInterface);

    ///cleanup
    (void)ustream_dispose(uStreamInstance);
}

TEST_FUNCTION(ustream_clone_noMemoryTocreate_instanceFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();
    EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    USTREAM* uStreamCloneInterface = ustream_clone(uStreamInstance, 0);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(uStreamCloneInterface);

    ///cleanup
    (void)ustream_dispose(uStreamInstance);
}

TEST_FUNCTION(ustream_clone_noMemoryToCreateFirstNodeFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();
    EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(ulib_malloc(sizeof(USTREAM))).SetReturn(NULL);
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    USTREAM* uStreamCloneInterface = ustream_clone(uStreamInstance, 0);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(uStreamCloneInterface);

    ///cleanup
    (void)ustream_dispose(uStreamInstance);
}

TEST_FUNCTION(ustream_clone_noMemoryToCloneFirstNodeFailed)
{
    ///arrange
    USTREAM* multibuffer = ustream_multi_create();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAM* defaultBuffer1 =
        ustream_const_create(
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer1));

    USTREAM* defaultBuffer2 = ustream_mock_create();
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_multi_append(multibuffer, defaultBuffer2));

    set_clone_result(ULIB_SYSTEM_ERROR);

    ///act
    USTREAM* uStreamCloneInterface = ustream_clone(multibuffer, 0);

    ///assert
    ASSERT_IS_NULL(uStreamCloneInterface);

    ///cleanup
    (void)ustream_dispose(multibuffer);
    (void)ustream_dispose(defaultBuffer1);
    (void)ustream_dispose(defaultBuffer2);
}

TEST_FUNCTION(ustream_clone_noMemoryToCreateSecondNodeFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
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
    USTREAM* uStreamCloneInterface = ustream_clone(uStreamInstance, 0);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(uStreamCloneInterface);

    ///cleanup
    (void)ustream_dispose(uStreamInstance);
}

#include "ustream_compliance_ut.h"

END_TEST_SUITE(ustream_multi_ut)
