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

#include "umock_c.h"
#include "testrunnerswitcher.h"
#include "umocktypes_charptr.h"
#include "umocktypes_bool.h"
#include "umocktypes_stdint.h"
#include "umock_c_negative_tests.h"
#include "macro_utils.h"
#include "ustreambuffer_ctest_aux.h"
#include "ustreambuffer_mock_buffer.h"

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

#include "ustreambuffer.h"

static const uint8_t* const USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1 =
        (const uint8_t* const)"0123456789";
static const uint8_t* const USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2 =
        (const uint8_t* const)"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const uint8_t* const USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3 =
        (const uint8_t* const)"abcdefghijklmnopqrstuvwxyz";

static USTREAMBUFFER_INTERFACE createTestDefaultMultibuffer()
{
    USTREAMBUFFER_INTERFACE defaultMultibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(defaultMultibuffer);

    USTREAMBUFFER_INTERFACE defaultBuffer1 =
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(defaultMultibuffer, defaultBuffer1));

    USTREAMBUFFER_INTERFACE defaultBuffer2 = 
        uStreamBufferCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2), 
            false);
    ASSERT_IS_NOT_NULL(defaultBuffer2);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(defaultMultibuffer, defaultBuffer2));
    
    USTREAMBUFFER_INTERFACE defaultBuffer3 = 
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3));
    ASSERT_IS_NOT_NULL(defaultBuffer3);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(defaultMultibuffer, defaultBuffer3));

    (void)uStreamBufferDispose(defaultBuffer1);
    (void)uStreamBufferDispose(defaultBuffer2);
    (void)uStreamBufferDispose(defaultBuffer3);

    return defaultMultibuffer;
}

/* define constants for the compliance test */
#define USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62
static const uint8_t* const USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT =
        (const uint8_t* const)USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT;
#define USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY          createTestDefaultMultibuffer()

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
}

/**
 * Beginning of the UT for ustreambuffer_multi.c module.
 */
BEGIN_TEST_SUITE(ustreambuffer_multi_ut)

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

    REGISTER_UMOCK_ALIAS_TYPE(USTREAMBUFFER_INTERFACE, void*);
    REGISTER_UMOCK_ALIAS_TYPE(USTREAMBUFFER_HANDLE, void*);

    REGISTER_GLOBAL_MOCK_HOOK(uLibMalloc, myMalloc);
    REGISTER_GLOBAL_MOCK_HOOK(uLibFree, myFree);
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

/* The uStreamBufferMultiCreate shall create an instance of the multi-buffer and initialize the interface. */
TEST_FUNCTION(uStreamBufferMultiCreate_succeed)
{
    ///arrange
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));

    ///act
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(multibuffer);
    ASSERT_IS_NOT_NULL(multibuffer->api);

    ///cleanup
    (void)uStreamBufferDispose(multibuffer);
}

/* The uStreamBufferMultiCreate shall return NULL if there is no memory to create the multi-buffer interface. */
TEST_FUNCTION(uStreamBufferMultiCreate_noMemoryToCreateInterfaceFailed)
{
    ///arrange
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER))).SetReturn(NULL);

    ///act
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(multibuffer);

    ///cleanup
}

TEST_FUNCTION(uStreamBufferMultiCreate_noMemoryToCreateInstanceFailed)
{
    ///arrange
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));

    ///act
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(multibuffer);

    ///cleanup
}

/* The uStreamBufferMultiAppend shall add the provided buffer to the multibuffer list. */
TEST_FUNCTION(uStreamBufferMultiAppend_newMultibufferSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAMBUFFER_INTERFACE testBuffer1 = 
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(testBuffer1);
    USTREAMBUFFER_INTERFACE testBuffer2 = 
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2));
    ASSERT_IS_NOT_NULL(testBuffer2);
    USTREAMBUFFER_INTERFACE testBuffer3 = 
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3));
    ASSERT_IS_NOT_NULL(testBuffer3);

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));

    ///act
    USTREAMBUFFER_RESULT result1 = uStreamBufferMultiAppend(multibuffer, testBuffer1);
    USTREAMBUFFER_RESULT result2 = uStreamBufferMultiAppend(multibuffer, testBuffer2);
    USTREAMBUFFER_RESULT result3 = uStreamBufferMultiAppend(multibuffer, testBuffer3);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result2);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result3);

    size_t size;
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferGetRemainingSize(multibuffer, &size));
    ASSERT_ARE_EQUAL(
	    int, 
	    strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 
	    strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2) + 
	    strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3), size);

    ///cleanup
    (void)uStreamBufferDispose(testBuffer1);
    (void)uStreamBufferDispose(testBuffer2);
    (void)uStreamBufferDispose(testBuffer3);
    (void)uStreamBufferDispose(multibuffer);
}

TEST_FUNCTION(uStreamBufferMultiAppend_partialReleasedMultibufferSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAMBUFFER_INTERFACE testBuffer1 = 
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(testBuffer1);
    USTREAMBUFFER_INTERFACE testBuffer2 = 
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2));
    ASSERT_IS_NOT_NULL(testBuffer2);
    USTREAMBUFFER_INTERFACE testBuffer3 = 
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3));
    ASSERT_IS_NOT_NULL(testBuffer3);

    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, testBuffer1));
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, testBuffer2));
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, testBuffer3));
    
    // seek to half of the buffer
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(multibuffer, (USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH / 2)));
    
    // release current - 1
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(multibuffer, (USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH / 2) - 1));

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));

    ///act
    USTREAMBUFFER_RESULT result1 = uStreamBufferMultiAppend(multibuffer, testBuffer1);
    USTREAMBUFFER_RESULT result2 = uStreamBufferMultiAppend(multibuffer, testBuffer2);
    USTREAMBUFFER_RESULT result3 = uStreamBufferMultiAppend(multibuffer, testBuffer3);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result2);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result3);

    size_t size;
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferGetRemainingSize(multibuffer, &size));
    ASSERT_ARE_EQUAL(int, 
            (strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) +
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2) +
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3) +
            (USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH / 2)), 
        size);

    ///cleanup
    (void)uStreamBufferDispose(testBuffer1);
    (void)uStreamBufferDispose(testBuffer2);
    (void)uStreamBufferDispose(testBuffer3);
    (void)uStreamBufferDispose(multibuffer);
}

TEST_FUNCTION(uStreamBufferMultiAppend_fullyReleasedMultibufferSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAMBUFFER_INTERFACE testBuffer1 = 
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(testBuffer1);
    USTREAMBUFFER_INTERFACE testBuffer2 = 
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2));
    ASSERT_IS_NOT_NULL(testBuffer2);
    USTREAMBUFFER_INTERFACE testBuffer3 = 
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3));
    ASSERT_IS_NOT_NULL(testBuffer3);

    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, testBuffer1));
    
    // seek to end of the buffer
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(multibuffer, strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1)));
    
    // release all
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(multibuffer, strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) - 1));

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));

    ///act
    USTREAMBUFFER_RESULT result1 = uStreamBufferMultiAppend(multibuffer, testBuffer1);
    USTREAMBUFFER_RESULT result2 = uStreamBufferMultiAppend(multibuffer, testBuffer2);
    USTREAMBUFFER_RESULT result3 = uStreamBufferMultiAppend(multibuffer, testBuffer3);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result2);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result3);

    size_t size;
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferGetRemainingSize(multibuffer, &size));
    ASSERT_ARE_EQUAL(
	    int, 
	    strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 
	    strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2) + 
	    strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_3), size);

    ///cleanup
    (void)uStreamBufferDispose(testBuffer1);
    (void)uStreamBufferDispose(testBuffer2);
    (void)uStreamBufferDispose(testBuffer3);
    (void)uStreamBufferDispose(multibuffer);
}

/* If the provided handle is NULL, the uStreamBufferMultiAppend shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferMultiAppend_nullMultibufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE testBuffer1 = 
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferMultiAppend(NULL, testBuffer1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(testBuffer1);
}

/* If the provided handle is not the implemented buffer type, the uStreamBufferMultiAppend shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferMultiAppend_bufferIsNotTypeOfBufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE testBuffer1 = 
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    USTREAMBUFFER_INTERFACE testBuffer2 = 
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_2));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferMultiAppend(testBuffer1, testBuffer2);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(testBuffer1);
    (void)uStreamBufferDispose(testBuffer2);
}

/* If the provided buffer to add is NULL, the uStreamBufferMultiAppend shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferMultiAppend_nullBufferToAddFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);
    USTREAMBUFFER_INTERFACE testBuffer1 = 
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    uStreamBufferMultiAppend(multibuffer, testBuffer1);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferMultiAppend(multibuffer, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(testBuffer1);
    (void)uStreamBufferDispose(multibuffer);
}

/* If there is no memory to control the new buffer, the uStreamBufferMultiAppend shall return USTREAMBUFFER_OUT_OF_MEMORY_EXCEPTION. */
TEST_FUNCTION(uStreamBufferMultiAppend_notEnoughMemoryFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);
    USTREAMBUFFER_INTERFACE testBuffer1 = 
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG)).SetReturn(NULL);

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferMultiAppend(multibuffer, testBuffer1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_OUT_OF_MEMORY_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(testBuffer1);
    (void)uStreamBufferDispose(multibuffer);
}

/* If the uStreamBufferMultiAppend failed to copy the buffer, it shall return USTREAMBUFFER_OUT_OF_MEMORY_EXCEPTION. */
TEST_FUNCTION(uStreamBufferMultiAppend_notEnoughMemoryToCloneTheBufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);
    USTREAMBUFFER_INTERFACE testBuffer1 = 
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferMultiAppend(multibuffer, testBuffer1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_OUT_OF_MEMORY_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(testBuffer1);
    (void)uStreamBufferDispose(multibuffer);
}

/* If the uStreamBufferMultiAppend failed to copy the buffer, it shall return USTREAMBUFFER_OUT_OF_MEMORY_EXCEPTION. */
TEST_FUNCTION(uStreamBufferMultiAppend_newInnerBufferFailedOnGetRemainingSizeFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAMBUFFER_INTERFACE defaultBuffer1 =
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer1));

    USTREAMBUFFER_INTERFACE defaultBuffer2 = uStreamBufferMockCreate();

    setGetRemainingSizeResult(USTREAMBUFFER_SYSTEM_EXCEPTION);

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferMultiAppend(multibuffer, defaultBuffer2);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SYSTEM_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(defaultBuffer1);
    (void)uStreamBufferDispose(defaultBuffer2);
    (void)uStreamBufferDispose(multibuffer);
}

/* The dispose shall uLibFree all allocated resources. */
TEST_FUNCTION(uStreamBufferMultiDispose_multibufferWithoutBuffersFreeAllResourcesSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferDispose(multibuffer);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);

    ///cleanup
}

/* If the multibuffer contains appended buffers, the dispose shall release all buffers on its list. */
TEST_FUNCTION(uStreamBufferMultiDispose_multibufferWithBuffersFreeAllResourcesSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;

    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferDispose(multibuffer);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);

    ///cleanup
}

/* The Seek shall bypass the exception if the Inner uStreamBuffer return not success for one of the needed operations. */
TEST_FUNCTION(uStreamBufferMultiSeek_innerBufferFailedInGetCurrentPositionFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAMBUFFER_INTERFACE defaultBuffer1 =
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer1));

    USTREAMBUFFER_INTERFACE defaultBuffer2 = uStreamBufferMockCreate();
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer2));
    setGetCurrentPositionResult(USTREAMBUFFER_SYSTEM_EXCEPTION);

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferSeek(
            multibuffer, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SYSTEM_EXCEPTION, result);
    offset_t pos;
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferGetCurrentPosition(multibuffer, &pos));
    ASSERT_ARE_EQUAL(int, 0, pos);

    ///cleanup
    (void)uStreamBufferDispose(multibuffer);
    (void)uStreamBufferDispose(defaultBuffer1);
    (void)uStreamBufferDispose(defaultBuffer2);
}

TEST_FUNCTION(uStreamBufferMultiSeek_innerBufferFailedInGetRemainingSizeFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAMBUFFER_INTERFACE defaultBuffer1 =
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer1));

    USTREAMBUFFER_INTERFACE defaultBuffer2 = uStreamBufferMockCreate();
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer2));
    setGetRemainingSizeResult(USTREAMBUFFER_SYSTEM_EXCEPTION);

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferSeek(
            multibuffer, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SYSTEM_EXCEPTION, result);
    offset_t pos;
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferGetCurrentPosition(multibuffer, &pos));
    ASSERT_ARE_EQUAL(int, 0, pos);

    ///cleanup
    (void)uStreamBufferDispose(multibuffer);
    (void)uStreamBufferDispose(defaultBuffer1);
    (void)uStreamBufferDispose(defaultBuffer2);
}

TEST_FUNCTION(uStreamBufferMultiSeek_innerBufferFailedInSeekFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAMBUFFER_INTERFACE defaultBuffer1 =
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer1));

    USTREAMBUFFER_INTERFACE defaultBuffer2 = uStreamBufferMockCreate();
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer2));
    setSeekResult(USTREAMBUFFER_SYSTEM_EXCEPTION);

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferSeek(
            multibuffer, 
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SYSTEM_EXCEPTION, result);
    offset_t pos;
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferGetCurrentPosition(multibuffer, &pos));
    ASSERT_ARE_EQUAL(int, 0, pos);

    ///cleanup
    (void)uStreamBufferDispose(multibuffer);
    (void)uStreamBufferDispose(defaultBuffer1);
    (void)uStreamBufferDispose(defaultBuffer2);
}

/* The GetNext shall return partial result if one of the internal buffers failed. */
TEST_FUNCTION(uStreamBufferMultiGetNext_innerBufferFailedInGetNextWithSomeValidContentSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAMBUFFER_INTERFACE defaultBuffer1 =
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer1));

    USTREAMBUFFER_INTERFACE defaultBuffer2 = uStreamBufferMockCreate();
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer2));
    setGetNextResult(USTREAMBUFFER_SYSTEM_EXCEPTION);

    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
    size_t sizeResult;


    ///act
    USTREAMBUFFER_RESULT result =
        uStreamBufferGetNext(
            multibuffer,
            bufResult,
            USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, bufResult, sizeResult);

    ///cleanup
    (void)uStreamBufferDispose(multibuffer);
    (void)uStreamBufferDispose(defaultBuffer1);
    (void)uStreamBufferDispose(defaultBuffer2);
}

TEST_FUNCTION(uStreamBufferMultiGetNext_innerBufferFailedInGetNextFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAMBUFFER_INTERFACE defaultBuffer1 =
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer1));

    USTREAMBUFFER_INTERFACE defaultBuffer2 = uStreamBufferMockCreate();
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer2));
    setGetNextResult(USTREAMBUFFER_SYSTEM_EXCEPTION);

    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH];
    size_t sizeResult;

    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(
            multibuffer,
            bufResult,
            USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &sizeResult));
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1, bufResult, sizeResult);

    setGetNextResult(USTREAMBUFFER_SYSTEM_EXCEPTION);

    ///act
    USTREAMBUFFER_RESULT result =
        uStreamBufferGetNext(
            multibuffer,
            bufResult,
            USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SYSTEM_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(multibuffer);
    (void)uStreamBufferDispose(defaultBuffer1);
    (void)uStreamBufferDispose(defaultBuffer2);
}

/* The Release shall bypass the exception if the Inner uStreamBuffer return not success for one of the needed operations. */
TEST_FUNCTION(uStreamBufferMultiRelease_innerBufferFailedInGetCurrentPositionFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAMBUFFER_INTERFACE defaultBuffer1 =
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer1));

    USTREAMBUFFER_INTERFACE defaultBuffer2 = uStreamBufferMockCreate();
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer2));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(multibuffer, strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 2));

    setGetCurrentPositionResult(USTREAMBUFFER_SYSTEM_EXCEPTION);

    ///act
    USTREAMBUFFER_RESULT result =
        uStreamBufferRelease(
            multibuffer,
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SYSTEM_EXCEPTION, result);
    offset_t pos;
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferGetCurrentPosition(multibuffer, &pos));
    ASSERT_ARE_EQUAL(int, strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 2, pos);

    ///cleanup
    (void)uStreamBufferDispose(multibuffer);
    (void)uStreamBufferDispose(defaultBuffer1);
    (void)uStreamBufferDispose(defaultBuffer2);
}

TEST_FUNCTION(uStreamBufferMultiRelease_innerBufferFailedInGetRemainingSizeFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAMBUFFER_INTERFACE defaultBuffer1 =
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer1));

    USTREAMBUFFER_INTERFACE defaultBuffer2 = uStreamBufferMockCreate();
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer2));
    ASSERT_ARE_EQUAL(
        int,
        USTREAMBUFFER_SUCCESS,
        uStreamBufferSeek(multibuffer, strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 2));

    setGetRemainingSizeResult(USTREAMBUFFER_SYSTEM_EXCEPTION);

    ///act
    USTREAMBUFFER_RESULT result =
        uStreamBufferRelease(
            multibuffer,
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SYSTEM_EXCEPTION, result);
    offset_t pos;
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferGetCurrentPosition(multibuffer, &pos));
    ASSERT_ARE_EQUAL(int, strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1) + 2, pos);

    ///cleanup
    (void)uStreamBufferDispose(multibuffer);
    (void)uStreamBufferDispose(defaultBuffer1);
    (void)uStreamBufferDispose(defaultBuffer2);
}

/* The Clone shall bypass the exception if the Inner uStreamBuffer return not success for one of the needed operations. */
TEST_FUNCTION(uStreamBufferMultiClone_innerBufferFailedInGetRemainingSizeFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAMBUFFER_INTERFACE defaultBuffer1 =
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer1));

    USTREAMBUFFER_INTERFACE defaultBuffer2 = uStreamBufferMockCreate();
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer2));

    setGetRemainingSizeResult(USTREAMBUFFER_SYSTEM_EXCEPTION);

    ///act
    USTREAMBUFFER_INTERFACE cloneResult = uStreamBufferClone( multibuffer, 0);

    ///assert
    ASSERT_IS_NULL(cloneResult);

    ///cleanup
    (void)uStreamBufferDispose(multibuffer);
    (void)uStreamBufferDispose(defaultBuffer1);
    (void)uStreamBufferDispose(defaultBuffer2);
}

/*  The clone shall return NULL if there is not enough memory to control the new buffer. */
TEST_FUNCTION(uStreamBufferClone_noMemoryToCreateInterfaceFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();
    EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER))).SetReturn(NULL);

    ///act
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 0);

    ///assert
    ASSERT_IS_NULL(uStreamBufferCloneInterface);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferClone_noMemoryToCreateInstanceFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();
    EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));

    ///act
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 0);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(uStreamBufferCloneInterface);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferClone_noMemoryToCreateFirstNodeFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();
    EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER))).SetReturn(NULL);
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));

    ///act
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 0);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(uStreamBufferCloneInterface);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferClone_noMemoryToCloneFirstNodeFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE multibuffer = uStreamBufferMultiCreate();
    ASSERT_IS_NOT_NULL(multibuffer);

    USTREAMBUFFER_INTERFACE defaultBuffer1 =
        uStreamBufferConstCreate(
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1,
            strlen((const char*)USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT_1));
    ASSERT_IS_NOT_NULL(defaultBuffer1);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer1));

    USTREAMBUFFER_INTERFACE defaultBuffer2 = uStreamBufferMockCreate();
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferMultiAppend(multibuffer, defaultBuffer2));

    setCloneResult(USTREAMBUFFER_SYSTEM_EXCEPTION);

    ///act
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(multibuffer, 0);

    ///assert
    ASSERT_IS_NULL(uStreamBufferCloneInterface);

    ///cleanup
    (void)uStreamBufferDispose(multibuffer);
    (void)uStreamBufferDispose(defaultBuffer1);
    (void)uStreamBufferDispose(defaultBuffer2);
}

TEST_FUNCTION(uStreamBufferClone_noMemoryToCreateSecondNodeFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();
    EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER))).SetReturn(NULL);
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));

    ///act
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 0);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(uStreamBufferCloneInterface);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

#include "uStreamBuffer_compliance_ut.h"

END_TEST_SUITE(ustreambuffer_multi_ut)
