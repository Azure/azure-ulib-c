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
static TEST_MUTEX_HANDLE g_dll_by_dll;

#define ENABLE_MOCKS

#include "ulib_heap.h"

#undef ENABLE_MOCKS

#include "ustream.h"

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT = (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;
static AZULIB_USTREAM* ustream_factory()
{
    uint8_t* buf = (uint8_t*)azulib_malloc(sizeof(uint8_t)*USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    (void)memcpy(buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    return azulib_ustream_create(buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, free);
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

    REGISTER_UMOCK_ALIAS_TYPE(AZULIB_USTREAM, void*);

    REGISTER_GLOBAL_MOCK_HOOK(azulib_malloc, malloc);
    REGISTER_GLOBAL_MOCK_HOOK(azulib_free, free);
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

/* azulib_ustream_create shall create an instance of the uStream and initialize the interface. */
TEST_FUNCTION(azulib_ustream_create_const_succeed)
{
    ///arrange
    STRICT_EXPECTED_CALL(azulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(azulib_malloc(sizeof(AZULIB_USTREAM)));
    STRICT_EXPECTED_CALL(azulib_malloc(IGNORED_NUM_ARG));

    ///act
    AZULIB_USTREAM* buffer_interface = azulib_ustream_create(
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(buffer_interface);
    ASSERT_IS_NOT_NULL(buffer_interface->api);

    ///cleanup
    (void)azulib_ustream_dispose(buffer_interface);
}

/* azulib_ustream_create shall return NULL if there is not enough memory to create the uStream. */
TEST_FUNCTION(azulib_ustream_create_const_no_memory_to_create_interface_failed)
{
    ///arrange
    STRICT_EXPECTED_CALL(azulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(azulib_malloc(sizeof(AZULIB_USTREAM))).SetReturn(NULL);
    STRICT_EXPECTED_CALL(azulib_free(IGNORED_PTR_ARG));

    ///act
    AZULIB_USTREAM* buffer_interface = azulib_ustream_create(
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
}

/* azulib_ustream_create shall return NULL if there is not enough memory to create the instance */
TEST_FUNCTION(azulib_ustream_create_const_no_memory_to_create_instance_failed)
{
    ///arrange
    STRICT_EXPECTED_CALL(azulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(azulib_malloc(sizeof(AZULIB_USTREAM)));
    STRICT_EXPECTED_CALL(azulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(azulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(azulib_free(IGNORED_PTR_ARG));

    ///act
    AZULIB_USTREAM* buffer_interface = azulib_ustream_create(
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
}

/* azulib_ustream_create shall return NULL if there is not enough memory to create the inner buffer */
TEST_FUNCTION(azulib_ustream_create_const_no_memory_to_create_inner_buffer_failed)
{
    ///arrange
    STRICT_EXPECTED_CALL(azulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);

    ///act
    AZULIB_USTREAM* buffer_interface = azulib_ustream_create(
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
}

/* azulib_ustream_create shall return NULL if the provided constant buffer is NULL */
TEST_FUNCTION(azulib_ustream_create_const_null_buffer_failed)
{
    ///arrange

    ///act
    AZULIB_USTREAM* buffer_interface = azulib_ustream_create(NULL, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
}

/* azulib_ustream_create shall return NULL ff the provided buffer length is zero */
TEST_FUNCTION(azulib_ustream_create_const_zero_length_failed)
{
    ///arrange

    ///act
    AZULIB_USTREAM* buffer_interface = azulib_ustream_create(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 0, NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
}

/* azulib_ustream_create shall create an instance of the uStream and initialize the interface. */
TEST_FUNCTION(azulib_ustream_create_succeed)
{
    ///arrange
    uint8_t* buf = (uint8_t*)azulib_malloc(sizeof(uint8_t)*USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    (void)memcpy(buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(azulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(azulib_malloc(sizeof(AZULIB_USTREAM)));
    STRICT_EXPECTED_CALL(azulib_malloc(IGNORED_NUM_ARG));

    ///act
    AZULIB_USTREAM* buffer_interface = 
        azulib_ustream_create(
            buf, 
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
            free);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NOT_NULL(buffer_interface);
    ASSERT_IS_NOT_NULL(buffer_interface->api);

    ///cleanup
    (void)azulib_ustream_dispose(buffer_interface);
}

/* azulib_ustream_create shall return NULL if there is not enough memory to create the uStream. */
TEST_FUNCTION(azulib_ustream_create_no_memory_to_create_interface_failed)
{
    ///arrange
    uint8_t* buf = (uint8_t*)azulib_malloc(sizeof(uint8_t)*USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    (void)memcpy(buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(azulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(azulib_malloc(sizeof(AZULIB_USTREAM))).SetReturn(NULL);
    STRICT_EXPECTED_CALL(azulib_free(IGNORED_PTR_ARG));

    ///act
    AZULIB_USTREAM* buffer_interface =
        azulib_ustream_create(
            buf,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            free);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
    azulib_free(buf);
}

/* azulib_ustream_create shall return NULL if there is not enough memory to create the instance */
TEST_FUNCTION(azulib_ustream_create_no_memory_to_create_instance_failed)
{
    ///arrange
    uint8_t* buf = (uint8_t*)azulib_malloc(sizeof(uint8_t)*USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    (void)memcpy(buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(azulib_malloc(IGNORED_NUM_ARG));
    STRICT_EXPECTED_CALL(azulib_malloc(sizeof(AZULIB_USTREAM)));
    STRICT_EXPECTED_CALL(azulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(azulib_free(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(azulib_free(IGNORED_PTR_ARG));

    ///act
    AZULIB_USTREAM* buffer_interface =
        azulib_ustream_create(
            buf,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            free);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
    azulib_free(buf);
}

/* azulib_ustream_create shall return NULL if there is not enough memory to create the inner buffer */
TEST_FUNCTION(azulib_ustream_create_no_memory_to_create_inner_buffer_failed)
{
    ///arrange
    uint8_t* buf = (uint8_t*)azulib_malloc(sizeof(uint8_t)*USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    (void)memcpy(buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(azulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);

    ///act
    AZULIB_USTREAM* buffer_interface =
        azulib_ustream_create(
            buf,
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            free);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
    azulib_free(buf);
}

/* azulib_ustream_create shall return NULL if the provided constant buffer is NULL */
TEST_FUNCTION(azulib_ustream_create_null_buffer_failed)
{
    ///arrange

    ///act
    AZULIB_USTREAM* buffer_interface = azulib_ustream_create(NULL, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, free);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
}

/* azulib_ustream_create shall return NULL if the provided buffer length is zero */
TEST_FUNCTION(azulib_ustream_create_zero_length_failed)
{
    ///arrange

    ///act
    AZULIB_USTREAM* buffer_interface = azulib_ustream_create(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 0, free);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(buffer_interface);

    ///cleanup
}

/*  azulib_ustream_clone shall return NULL if there is not enough memory to control the new uStream. */
TEST_FUNCTION(azulib_ustream_clone_no_memory_to_create_interface_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();
    EXPECTED_CALL(azulib_malloc(sizeof(AZULIB_USTREAM))).SetReturn(NULL);

    ///act
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 0);

    ///assert
    ASSERT_IS_NULL(azulib_ustream_clone_interface);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* azulib_ustream_clone shall return NULL if there is not enough memory to create the instance */
TEST_FUNCTION(azulib_ustream_clone_no_memory_to_create_instance_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();
    EXPECTED_CALL(azulib_malloc(sizeof(AZULIB_USTREAM)));
    EXPECTED_CALL(azulib_malloc(IGNORED_NUM_ARG)).SetReturn(NULL);
    STRICT_EXPECTED_CALL(azulib_free(IGNORED_PTR_ARG));

    ///act
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 0);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(azulib_ustream_clone_interface);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

#include "ustream_compliance_ut.h"

END_TEST_SUITE(ustream_ut)
