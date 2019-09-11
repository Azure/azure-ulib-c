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

#include "ustream_base.h"
#include "ustream.h"

/* define constants for the compliance test */
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH 62
#define USTREAM_COMPLIANCE_TARGET_INSTANCE AZIOT_USTREAM
static const uint8_t* const USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT = (const uint8_t* const)USTREAM_COMPLIANCE_EXPECTED_CONTENT;
static AZIOT_USTREAM test_ustream_instance;
static AZIOT_USTREAM* ustream_factory()
{
    AZIOT_USTREAM_INNER_BUFFER* ustream_inner_buffer = (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    uint8_t* buf = (uint8_t*)aziot_ulib_malloc(sizeof(uint8_t)*USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    (void)memcpy(buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    aziot_ustream_init(&test_ustream_instance, ustream_inner_buffer, aziot_ulib_free, buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, aziot_ulib_free);
    return (AZIOT_USTREAM*)&test_ustream_instance;
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
    TEST_MUTEX_RELEASE(g_test_by_test);
}

/* aziot_ustream_init shall create an instance of the ustream and initialize the interface. */
TEST_FUNCTION(aziot_ustream_init_const_succeed)
{
    ///arrange
    AZIOT_USTREAM_INNER_BUFFER* inner_buffer = (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    umock_c_reset_all_calls();
    AZIOT_USTREAM ustream_instance;

    ///act
    AZIOT_ULIB_RESULT result = aziot_ustream_init(
        &ustream_instance,
        inner_buffer,
        aziot_ulib_free,
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT,
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
        NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result);

    ///cleanup
    (void)aziot_ustream_dispose((AZIOT_USTREAM*)&ustream_instance);
}

/* aziot_ustream_init shall create an instance of the ustream and initialize the interface. */
TEST_FUNCTION(aziot_ustream_init_succeed)
{
    ///arrange
    uint8_t* buf = (uint8_t*)aziot_ulib_malloc(sizeof(uint8_t)*USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    (void)memcpy(buf, USTREAM_COMPLIANCE_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    AZIOT_USTREAM_INNER_BUFFER* inner_buffer = (AZIOT_USTREAM_INNER_BUFFER*)aziot_ulib_malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    umock_c_reset_all_calls();
    AZIOT_USTREAM ustream_instance;

    ///act
    AZIOT_ULIB_RESULT result =
        aziot_ustream_init(
            &ustream_instance,
            inner_buffer,
            free,
            buf, 
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH,
            free);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_SUCCESS, result);

    ///cleanup
    (void)aziot_ustream_dispose((AZIOT_USTREAM*)&ustream_instance);
}

/* aziot_ustream_init shall return NULL if the provided constant buffer is NULL */
TEST_FUNCTION(aziot_ustream_init_null_buffer_failed)
{
    ///arrange
    AZIOT_USTREAM ustream_instance;
    AZIOT_USTREAM_INNER_BUFFER inner_buffer;

    ///act
    AZIOT_ULIB_RESULT result = aziot_ustream_init(&ustream_instance, &inner_buffer, NULL, NULL, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
}

/* aziot_ustream_init shall return NULL if the provided buffer length is zero */
TEST_FUNCTION(aziot_ustream_init_zero_length_failed)
{
    ///arrange
    AZIOT_USTREAM ustream_instance;
    AZIOT_USTREAM_INNER_BUFFER inner_buffer;

    ///act
    AZIOT_ULIB_RESULT result = aziot_ustream_init(&ustream_instance, &inner_buffer, NULL, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 0, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
}

/* aziot_ustream_init shall return NULL if the provided buffer length is zero */
TEST_FUNCTION(aziot_ustream_init_NULL_ustream_instance_failed)
{
    ///arrange
    AZIOT_USTREAM_INNER_BUFFER inner_buffer;

    ///act
    AZIOT_ULIB_RESULT result = aziot_ustream_init(NULL, &inner_buffer, NULL, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
}

/* aziot_ustream_init shall return NULL if the provided buffer length is zero */
TEST_FUNCTION(aziot_ustream_init_NULL_inner_buffer_failed)
{
    ///arrange
    AZIOT_USTREAM ustream_instance;

    ///act
    AZIOT_ULIB_RESULT result = aziot_ustream_init(&ustream_instance, NULL, NULL, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
}

#include "ustream_compliance_ut.h"

END_TEST_SUITE(ustream_ut)
