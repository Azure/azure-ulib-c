// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef USTREAM_CTEST_AUX_H
#define USTREAM_CTEST_AUX_H

#include <stdint.h>
#include "azure_macro_utils/macro_utils.h"
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

static int uint8_t_ptr_compare(
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

static void uint8_t_ptr_to_string(
        char* string, 
        size_t buffer_size, 
        uint8_t_ptr buf, 
        size_t size)
{
    size_t final_size = ((buffer_size - 1) < size) ? buffer_size - 1 : size;
    size_t i;

    for(i = 0; i < final_size; i++)
    {
        string[i] = IS_PRINTABLE((char)buf[i]) ? (char)buf[i] : '.';
    }
    string[i] = '\0';
}

#define ASSERT_BUFFER_ARE_EQUAL(type, A, B, size, ...) \
do { \
    const type A_value = (const type)(A); \
    const type B_value = (const type)(B); \
    char expected_string[256]; \
    char actual_string[256]; \
    MU_C2(type,_to_string)(expected_string, sizeof(expected_string), A_value, size); /*one evaluation per argument*/ \
    MU_C2(type,_to_string)(actual_string, sizeof(actual_string), B_value, size);/*one evaluation per argument*/ \
    if (MU_C2(type,_compare)(A_value, B_value, size)) \
    { \
        char temp_str[1024]; \
        (void)snprintf(temp_str, sizeof(temp_str), "  Assert failed: Expected: %s, Actual: %s\n", expected_string, actual_string); \
        ASSERT_FAIL(temp_str); \
    } \
} while (0)

/**
 * Check buffer
 */
static void check_buffer(
        AZ_USTREAM* ustream_interface, 
        uint8_t offset, 
        const uint8_t* const expected_content, 
        uint8_t expected_content_length)
{
    uint8_t buf_result[256];
    size_t size_result;

    if (offset < expected_content_length)
    {
        ASSERT_ARE_EQUAL(int, AZ_ULIB_SUCCESS,
            az_ustream_read(ustream_interface, buf_result, 256, &size_result));

        ASSERT_ARE_EQUAL(int, expected_content_length - offset, size_result);
        ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr,
            (const uint8_t* const)(expected_content + offset),
            buf_result,
            size_result);
    }

    size_result = 10;
    ASSERT_ARE_EQUAL(
        int,
        AZ_ULIB_EOF,
        az_ustream_read(ustream_interface, buf_result, 256, &size_result));
    ASSERT_ARE_EQUAL(int, 0, size_result);
}

#endif /* USTREAM_CTEST_AUX_H */
