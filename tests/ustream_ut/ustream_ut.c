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

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT = (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;
static USTREAM* ustream_factory()
{
    uint8_t* buf = (uint8_t*)ulib_malloc(sizeof(uint8_t)*USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    (void)memcpy(buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    return ustream_create(buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, my_free);
}
#define USTREAM_COMPLIANCE_TARGET_FACTORY         ustream_factory()

#define TEST_CONST_BUFFER_LENGTH    (USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 2)
#define TEST_CONST_MAX_BUFFER_SIZE  (TEST_CONST_BUFFER_LENGTH - 1)

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%s", MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
}

/**
 * Beginning of the UT for ustream.c on ownership model.
 */
BEGIN_TEST_SUITE(ustream_ut)

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

/* The ustream_create shall create an instance of the buffer and initialize the interface. */
TEST_FUNCTION(ustream_create_succeed)
{
    ///arrange
    uint8_t* buf = (uint8_t*)ulib_malloc(sizeof(uint8_t)*USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    (void)memcpy(buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));

    ///act
    USTREAM* buffer_interface = 
        ustream_create(
            buf, 
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
            my_free);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(buffer_interface);
    ASSERT_IS_NOT_NULL(buffer_interface->api);

    ///cleanup
    (void)ustream_dispose(buffer_interface);
}

/* The ustream_create shall return NULL if there is no memory to create the buffer. */
TEST_FUNCTION(ustream_create_no_memory_to_create_interface_failed)
{
    ///arrange
    uint8_t* buf = (uint8_t*)ulib_malloc(sizeof(uint8_t)*USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    (void)memcpy(buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM))).SetReturn(NULL);
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    USTREAM* buffer_interface =
        ustream_create(
            buf,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            my_free);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
}

/* The ustream_create shall return NULL if there is no memory to create the instance */
TEST_FUNCTION(ustream_create_no_memory_to_create_instance_failed)
{
    ///arrange
    uint8_t* buf = (uint8_t*)ulib_malloc(sizeof(uint8_t)*USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    (void)memcpy(buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(ulib_malloc(sizeof(USTREAM)));
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(ulib_free(IGNORED_PTR_ARG));

    ///act
    USTREAM* buffer_interface =
        ustream_create(
            buf,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            my_free);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
}

/* The ustream_create shall return NULL if there is no memory to create the inner buffer */
TEST_FUNCTION(ustream_create_no_memory_to_create_inner_buffer_failed)
{
    ///arrange
    uint8_t* buf = (uint8_t*)ulib_malloc(sizeof(uint8_t)*USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    (void)memcpy(buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(ulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);

    ///act
    USTREAM* buffer_interface =
        ustream_create(
            buf,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            my_free);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
    ulib_free(buf);
}

/* If the provided constant buffer is NULL, the ustream_create shall return NULL. */
TEST_FUNCTION(ustream_create_null_buffer_failed)
{
    ///arrange

    ///act
    USTREAM* buffer_interface = ustream_create(NULL, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, my_free);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
}

/* If the provided buffer length is zero, the ustream_create shall return NULL. */
TEST_FUNCTION(ustream_create_zero_length_failed)
{
    ///arrange

    ///act
    USTREAM* buffer_interface = ustream_create(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 0, my_free);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
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

/* ustream_clone shall return null if there is not enough memory to create the instance */
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

#include "ustream_compliance_ut.h"

END_TEST_SUITE(ustream_ut)
