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

/* define constants for the compliance test */
#define USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62
static const uint8_t* const USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT = (const uint8_t* const)USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT;
#define USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY \
        uStreamBufferCreate( \
            USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, \
            USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, \
            false)

#define TEST_CONST_BUFFER_LENGTH    (USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 2)
#define TEST_CONST_MAX_BUFFER_SIZE  (TEST_CONST_BUFFER_LENGTH - 1)

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
}

/**
 * Beginning of the UT for ustreambuffer.c on no ownership module.
 */
BEGIN_TEST_SUITE(ustreambuffer_no_ownership_ut)

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

/* The uStreamBufferCreate shall create an instance of the buffer and initialize the interface. */
TEST_FUNCTION(uStreamBufferCreate_succeed)
{
    ///arrange
    STRICT_EXPECTED_CALL(uLibMalloc(USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));

    ///act
    USTREAMBUFFER_INTERFACE bufferInterface = uStreamBufferCreate(
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
        false);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(bufferInterface);
    ASSERT_IS_NOT_NULL(bufferInterface->api);

    ///cleanup
    (void)uStreamBufferDispose(bufferInterface);
}

TEST_FUNCTION(uStreamBufferCreate_protectedImmutableBufferSucceed)
{
    USTREAMBUFFER_INTERFACE bufferInterface;
    {
        ///arrange
        uint8_t inBuffer[11] = "0123456789";

        ///act
        bufferInterface = uStreamBufferCreate((const uint8_t* const)inBuffer, 10, false);

        ///assert
        inBuffer[0] = '\0';
    }
    uint8_t outBuffer[TEST_CONST_BUFFER_LENGTH];
    size_t resultSize;
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferGetNext(bufferInterface, outBuffer, TEST_CONST_MAX_BUFFER_SIZE, &resultSize));
    outBuffer[resultSize] = '\0';
    ASSERT_ARE_EQUAL(char_ptr, "0123456789", outBuffer);

    ///cleanup
    (void)uStreamBufferDispose(bufferInterface);
}

/* The uStreamBufferCreate shall return NULL if there is no memory to create the buffer. */
TEST_FUNCTION(uStreamBufferCreate_noMemoryToCreateInterfaceFailed)
{
    ///arrange
    STRICT_EXPECTED_CALL(uLibMalloc(USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER))).SetReturn(NULL);
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));

    ///act
    USTREAMBUFFER_INTERFACE bufferInterface = uStreamBufferCreate(
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
        false);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(bufferInterface);

    ///cleanup
}

TEST_FUNCTION(uStreamBufferCreate_noMemoryToCreateInstanceFailed)
{
    ///arrange
    STRICT_EXPECTED_CALL(uLibMalloc(USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAMBUFFER)));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));

    ///act
    USTREAMBUFFER_INTERFACE bufferInterface = uStreamBufferCreate(
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
        false);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(bufferInterface);

    ///cleanup
}

TEST_FUNCTION(uStreamBufferCreate_noMemoryToCreateInnerBufferFailed)
{
    ///arrange
    STRICT_EXPECTED_CALL(uLibMalloc(USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));

    ///act
    USTREAMBUFFER_INTERFACE bufferInterface = uStreamBufferCreate(
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
        false);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(bufferInterface);

    ///cleanup
}

TEST_FUNCTION(uStreamBufferCreate_noMemoryToCopyDataInTheInnerBufferFailed)
{
    ///arrange
    STRICT_EXPECTED_CALL(uLibMalloc(USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH)).SetReturn(NULL);

    ///act
    USTREAMBUFFER_INTERFACE bufferInterface = uStreamBufferCreate(
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
        false);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(bufferInterface);

    ///cleanup
}

/* If the provided constant buffer is NULL, the uStreamBufferCreate shall return NULL. */
TEST_FUNCTION(uStreamBufferCreate_NULLBufferFailed)
{
    ///arrange

    ///act
    USTREAMBUFFER_INTERFACE bufferInterface = uStreamBufferCreate(NULL, USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, false);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(bufferInterface);

    ///cleanup
}

/* If the provided buffer length is zero, the uStreamBufferCreate shall return NULL. */
TEST_FUNCTION(uStreamBufferCreate_zeroLengthFailed)
{
    ///arrange

    ///act
    USTREAMBUFFER_INTERFACE bufferInterface = uStreamBufferCreate(USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 0, false);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(bufferInterface);

    ///cleanup
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

#include "ustreambuffer_compliance_ut.h"

END_TEST_SUITE(ustreambuffer_no_ownership_ut)
