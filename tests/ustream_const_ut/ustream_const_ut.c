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

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT = (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;
#define USTREAM_COMPLIANCE_TARGET_FACTORY \
        ustream_const_create( \
            USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, \
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH)

#define TEST_CONST_BUFFER_LENGTH    (USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 2)
#define TEST_CONST_MAX_BUFFER_SIZE  (TEST_CONST_BUFFER_LENGTH - 1)

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%s", MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
}

/**
 * Beginning of the UT for ustream.c on constant module.
 */
BEGIN_TEST_SUITE(ustream_const_ut)

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

/* The ustream_const_create shall create an instance of the buffer and initialize the interface. */
TEST_FUNCTION(uStreamConstCreate_succeed)
{
    ///arrange
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));

    ///act
    USTREAM* bufferInterface = ustream_const_create(
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(bufferInterface);
    ASSERT_IS_NOT_NULL(bufferInterface->api);

    ///cleanup
    (void)ustream_dispose(bufferInterface);
}

/* The ustream_const_create shall return NULL if there is no memory to create the buffer. */
TEST_FUNCTION(uStreamConstCreate_noMemoryToCreateInterfaceFailed)
{
    ///arrange
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAM))).SetReturn(NULL);
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));

    ///act
    USTREAM* bufferInterface = ustream_const_create(
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(bufferInterface);

    ///cleanup
}

TEST_FUNCTION(uStreamConstCreate_noMemoryToCreateInstanceFailed)
{
    ///arrange
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(uLibMalloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));

    ///act
    USTREAM* bufferInterface = ustream_const_create(
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(bufferInterface);

    ///cleanup
}

TEST_FUNCTION(uStreamConstCreate_noMemoryToCreateInnerBufferFailed)
{
    ///arrange
    STRICT_EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG)).SetReturn(NULL);

    ///act
    USTREAM* bufferInterface = ustream_const_create(
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(bufferInterface);

    ///cleanup
}

/* If the provided constant buffer is NULL, the ustream_const_create shall return NULL. */
TEST_FUNCTION(uStreamConstCreate_NULLBufferFailed)
{
    ///arrange

    ///act
    USTREAM* bufferInterface = ustream_const_create(NULL, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(bufferInterface);

    ///cleanup
}

/* If the provided buffer length is zero, the ustream_const_create shall return NULL. */
TEST_FUNCTION(uStreamConstCreate_zeroLengthFailed)
{
    ///arrange

    ///act
    USTREAM* bufferInterface = ustream_const_create(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 0);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(bufferInterface);

    ///cleanup
}

/*  The clone shall return NULL if there is not enough memory to control the new buffer. */
TEST_FUNCTION(uStreamClone_noMemoryToCreateInterfaceFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();
    EXPECTED_CALL(uLibMalloc(sizeof(USTREAM))).SetReturn(NULL);

    ///act
    USTREAM* uStreamCloneInterface = ustream_clone(uStreamInstance, 0);

    ///assert
    ASSERT_IS_NULL(uStreamCloneInterface);

    ///cleanup
    (void)ustream_dispose(uStreamInstance);
}

TEST_FUNCTION(uStreamClone_noMemoryToCreateInstanceFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();
    EXPECTED_CALL(uLibMalloc(sizeof(USTREAM)));
    EXPECTED_CALL(uLibMalloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(uLibFree(IGNORED_PTR_ARG));

    ///act
    USTREAM* uStreamCloneInterface = ustream_clone(uStreamInstance, 0);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(uStreamCloneInterface);

    ///cleanup
    (void)ustream_dispose(uStreamInstance);
}

#include "ustream_compliance_ut.h"

END_TEST_SUITE(ustream_const_ut)
