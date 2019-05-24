// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_TESTS_INC_USTREAM_CTEST_AUX_H_
#define AZURE_ULIB_C_TESTS_INC_USTREAM_CTEST_AUX_H_

#include <stdint.h>
#include "ctest_macros.h"
#include "testrunnerswitcher.h"
#include "ustream_base.h"

/**
 * Missing CTEST function to compare arrays.
 */
#define IS_PRINTABLE(c) (                               \
        (c == '!') ||                                   \
        (c == '(') || (c == ')') || (c == '*') ||       \
        (c == '-') || (c == '.') ||                     \
        ((c >= '0') && (c <= '9')) ||                   \
        ((c >= 'A') && (c <= 'Z')) ||                   \
        (c == '_') ||                                   \
        ((c >= 'a') && (c <= 'z'))                      \
)

typedef const uint8_t* uint8_t_ptr;

static int uint8_t_ptr_Compare(
        const uint8_t_ptr left, 
        const uint8_t_ptr right, 
        size_t size)
{
    int result;
    if (((left == NULL) &&
        (right == NULL)) ||
            (size == 0))
    {
        result = 0;
    }
    else if (left == NULL)
    {
        result = -1;
    }
    else if (right == NULL)
    {
        result = 1;
    }
    else
    {
        result = 0;
        for(size_t i = 0; i < size; i++)
        {
            if((result = left[i] - right[i]) != 0)
            {
                break;
            }
        }
    }
    return result;
}

static void uint8_t_ptr_ToString(
        char* string, 
        size_t bufferSize, 
        uint8_t_ptr buf, 
        size_t size)
{
    size_t finalSize = ((bufferSize - 1) < size) ? bufferSize - 1 : size;
	size_t i;

    for(i = 0; i < finalSize; i++)
    {
        string[i] = IS_PRINTABLE((char)buf[i]) ? (char)buf[i] : '.';
    }
    string[i] = '\0';
}

#define ASSERT_BUFFER_ARE_EQUAL(type, A, B, size, ...) \
do { \
    const type A_value = (const type)(A); \
    const type B_value = (const type)(B); \
    char expectedString[256]; \
    char actualString[256]; \
    C2(type,_ToString)(expectedString, sizeof(expectedString), A_value, size); /*one evaluation per argument*/ \
    C2(type,_ToString)(actualString, sizeof(actualString), B_value, size);/*one evaluation per argument*/ \
    if (C2(type,_Compare)(A_value, B_value, size)) \
    { \
        char temp_str[1024]; \
        (void)snprintf(temp_str, sizeof(temp_str), "  Assert failed: Expected: %s, Actual: %s\n", expectedString, actualString); \
        ASSERT_FAIL(temp_str); \
    } \
} while (0)

/**
 * Check buffer
 */
static void checkBuffer(
        USTREAM* azBuffer, 
        uint8_t offset, 
        const uint8_t* const expectedContent, 
        uint8_t expectedContentLength)
{
    uint8_t bufResult[256];
    size_t sizeResult;

    if (offset < expectedContentLength)
    {
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS,
            uStreamGetNext(azBuffer, bufResult, 256, &sizeResult));

        ASSERT_ARE_EQUAL(int, expectedContentLength - offset, sizeResult);
        ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr,
            (const uint8_t* const)(expectedContent + offset),
            bufResult,
            sizeResult);
    }

    sizeResult = 10;
    ASSERT_ARE_EQUAL(
        int,
        ULIB_EOF,
        uStreamGetNext(azBuffer, bufResult, 256, &sizeResult));
    ASSERT_ARE_EQUAL(int, 0, sizeResult);
}

#endif /* AZURE_ULIB_C_TESTS_INC_USTREAM_CTEST_AUX_H_ */
