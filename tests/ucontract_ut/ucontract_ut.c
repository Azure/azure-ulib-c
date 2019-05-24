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
#include "ucontract.h"

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    ASSERT_FAIL("umock_c reported error :%s", MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
}

/**
 * Beginning of the UT for ucontract module.
 */
BEGIN_TEST_SUITE(ucontract_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    ASSERT_ARE_EQUAL(int, 0, umock_c_init(on_umock_c_error));
    ASSERT_ARE_EQUAL(int, 0, umocktypes_charptr_register_types());
    ASSERT_ARE_EQUAL(int, 0, umocktypes_stdint_register_types());
    ASSERT_ARE_EQUAL(int, 0, umocktypes_bool_register_types());
}

static int require(bool expression)
{
    UCONTRACT_REQUIRE(expression, -1, "Require invoked");
    return 0;
}

static int requireEquals(bool expression)
{
    UCONTRACT_REQUIRE_EQUALS(expression, true, -1);
    return 0;
}

static int requireNotEquals(bool expression)
{
    UCONTRACT_REQUIRE_NOT_EQUALS(expression, false, -1);
    return 0;
}

static int requireNotNull(void* ptr)
{
    UCONTRACT_REQUIRE_NOT_NULL(ptr, -1);
    return 0;
}

static int uContractWithOneArg(bool expression)
{
    UCONTRACT(UCONTRACT_REQUIRE(expression, -1, "Require invoked"));
    return 0;
}

static int uContractWithTwoArg(void* ptr, bool expression)
{
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(ptr, -1),
        UCONTRACT_REQUIRE(expression, -2, "Require invoked"));
    return 0;
}

static int uContractWithThreeArg(void* ptr, bool expression, bool valEquals)
{
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(ptr, -1),
        UCONTRACT_REQUIRE(expression, -2, "Require invoked"),
        UCONTRACT_REQUIRE_EQUALS(valEquals, true, -3));
    return 0;
}

static int uContractWithFourArg(void* ptr, bool expression, bool valEquals, int num)
{
    UCONTRACT(
        UCONTRACT_REQUIRE_NOT_NULL(ptr, -1),
        UCONTRACT_REQUIRE(expression, -2, "Require invoked"),
        UCONTRACT_REQUIRE_EQUALS(valEquals, true, -3),
        UCONTRACT_REQUIRE_NOT_EQUALS(num, 0, -4));
    return 0;
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

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* The UCONTRACT_REQUIRE shall do nothing if the required expression returns true. */
TEST_FUNCTION(UCONTRACT_REQUIRE_Succeed)
{
    ///arrange

    ///act
    int result = require(true);

    ///assert
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
}

TEST_FUNCTION(UCONTRACT_REQUIRE_EQUALS_Succeed)
{
    ///arrange

    ///act
    int result = requireEquals(true);

    ///assert
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
}

TEST_FUNCTION(UCONTRACT_REQUIRE_NOT_EQUALS_Succeed)
{
    ///arrange

    ///act
    int result = requireNotEquals(true);

    ///assert
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
}

TEST_FUNCTION(UCONTRACT_REQUIRE_NOT_NULL_Succeed)
{
    ///arrange
    int i = 0;

    ///act
    int result = requireNotNull(&i);

    ///assert
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
}

/* The UCONTRACT_REQUIRE shall return `result` if the required expression returns false. */
TEST_FUNCTION(UCONTRACT_REQUIRE_Failed)
{
    ///arrange

    ///act
    int result = require(false);

    ///assert
    ASSERT_ARE_EQUAL(int, -1, result);

    ///cleanup
}

TEST_FUNCTION(UCONTRACT_REQUIRE_EQUALS_Failed)
{
    ///arrange

    ///act
    int result = requireEquals(false);

    ///assert
    ASSERT_ARE_EQUAL(int, -1, result);

    ///cleanup
}

TEST_FUNCTION(UCONTRACT_REQUIRE_NOT_EQUALS_Failed)
{
    ///arrange

    ///act
    int result = requireNotEquals(false);

    ///assert
    ASSERT_ARE_EQUAL(int, -1, result);

    ///cleanup
}

TEST_FUNCTION(UCONTRACT_REQUIRE_NOT_NULL_Failed)
{
    ///arrange

    ///act
    int result = requireNotNull(NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, -1, result);

    ///cleanup
}

/* The UCONTRACT shall do nothing if the required expressions return true. */
TEST_FUNCTION(UCONTRACT_uContractWithOneArgSucceed)
{
    ///arrange

    ///act
    int result = uContractWithOneArg(true);

    ///assert
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
}

TEST_FUNCTION(UCONTRACT_uContractWithTwoArgSucceed)
{
    ///arrange
    int i = 0;

    ///act
    int result = uContractWithTwoArg(&i, true);

    ///assert
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
}

TEST_FUNCTION(UCONTRACT_uContractWithThreeArgSucceed)
{
    ///arrange
    int i = 0;

    ///act
    int result = uContractWithThreeArg(&i, true, true);

    ///assert
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
}

TEST_FUNCTION(UCONTRACT_uContractWithFourArgSucceed)
{
    ///arrange
    int i = 0;

    ///act
    int result = uContractWithFourArg(&i, true, true, 5);

    ///assert
    ASSERT_ARE_EQUAL(int, 0, result);

    ///cleanup
}

/* The UCONTRACT_REQUIRE shall return `result` if at least one of the required expressions return false. */
TEST_FUNCTION(UCONTRACT_REQUIRE_uContractWithOneArgFailed)
{
    ///arrange

    ///act
    int result = uContractWithOneArg(false);

    ///assert
    ASSERT_ARE_EQUAL(int, -1, result);

    ///cleanup
}

TEST_FUNCTION(UCONTRACT_uContractWithTwoArgFirstFailed)
{
    ///arrange

    ///act
    int result = uContractWithTwoArg(NULL, true);

    ///assert
    ASSERT_ARE_EQUAL(int, -1, result);

    ///cleanup
}

TEST_FUNCTION(UCONTRACT_uContractWithTwoArgSecondFailed)
{
    ///arrange
    int i = 0;

    ///act
    int result = uContractWithTwoArg(&i, false);

    ///assert
    ASSERT_ARE_EQUAL(int, -2, result);

    ///cleanup
}

TEST_FUNCTION(UCONTRACT_uContractWithThreeArgFailed)
{
    ///arrange
    int i = 0;

    ///act
    int result = uContractWithThreeArg(&i, true, false);

    ///assert
    ASSERT_ARE_EQUAL(int, -3, result);

    ///cleanup
}

TEST_FUNCTION(UCONTRACT_uContractWithFourArgFailed)
{
    ///arrange
    int i = 0;

    ///act
    int result = uContractWithFourArg(&i, true, true, 0);

    ///assert
    ASSERT_ARE_EQUAL(int, -4, result);

    ///cleanup
}

END_TEST_SUITE(ucontract_ut)
