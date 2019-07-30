// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_TESTS_INC_USTREAM_COMPLIANCE_UT_H
#define AZURE_ULIB_C_TESTS_INC_USTREAM_COMPLIANCE_UT_H

#include "ustream_mock_buffer.h"
#include "ustream_ctest_aux.h"

/* check for test artifacts. */
#ifndef USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH
#error "USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH not defined"
#endif

#ifndef USTREAM_COMPLIANCE_TARGET_FACTORY
#error "USTREAM_COMPLIANCE_TARGET_FACTORY not defined"
#endif

/* split the content in 4 parts. */
#define USTREAM_COMPLIANCE_LENGTH_1           (USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH >> 2)
#define USTREAM_COMPLIANCE_LENGTH_2           (USTREAM_COMPLIANCE_LENGTH_1 + USTREAM_COMPLIANCE_LENGTH_1)
#define USTREAM_COMPLIANCE_LENGTH_3           (USTREAM_COMPLIANCE_LENGTH_2 + USTREAM_COMPLIANCE_LENGTH_1)

/* create local buffer with enough size to handle the full content. */
#define USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH (USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 2)

/*
 * Start compliance tests:
 */

/* The dispose shall free all allocated resources for the instance of the buffer. */
TEST_FUNCTION(azulib_ustream_dispose_compliance_cloned_instance_disposed_first_succeed)
{
    ///arrange
    offset_t azulib_ustream_clone_current_position;
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 0);
    ASSERT_IS_NOT_NULL(ustream_instance);
        
    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_dispose(azulib_ustream_clone_interface);
        
    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(ustream_instance, &azulib_ustream_clone_current_position));
        
    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_dispose_compliance_cloned_instance_disposed_second_succeed)
{
    ///arrange
    offset_t azulib_ustream_clone_current_position;
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 0);
    ASSERT_IS_NOT_NULL(ustream_instance);
        
    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_dispose(ustream_instance);
        
    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(azulib_ustream_clone_interface, &azulib_ustream_clone_current_position));
        
    ///cleanup
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

/* If there is no more instances of the buffer, the dispose shall release all allocated
 *          resources to control the buffer. */
TEST_FUNCTION(azulib_ustream_dispose_compliance_single_instance_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    
    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_dispose(ustream_instance);
    
    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    
    ///cleanup
}

/* If the provided handle is NULL, the dispose shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(azulib_ustream_dispose_compliance_null_buffer_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    
    ///act
    AZULIB_ULIB_RESULT result = ustream_instance->api->dispose(NULL);
    
    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    
    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided handle is not the implemented buffer type, the dispose shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(azulib_ustream_dispose_compliance_buffer_is_not_type_of_buffer_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    
    ///act
    AZULIB_ULIB_RESULT result = ustream_instance->api->dispose(ustream_mock_create());
    
    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    
    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* The clone shall return a buffer with the same content of the original buffer. */
/* The clone shall start the buffer on the current position of the original buffer. */
/* The clone shall move the current position to the start position. */
/* The clone shall move the released position to the one immediately before the current position. */
/* The cloned buffer shall not interfere in the instance of the original buffer and vice versa. */
TEST_FUNCTION(azulib_ustream_clone_compliance_new_buffer_cloned_with_zero_offset_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);

    ///act
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 0);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);

    /* current position was moved to the start of the buffer. */
    offset_t azulib_ustream_clone_current_position;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(azulib_ustream_clone_interface, &azulib_ustream_clone_current_position));
    ASSERT_ARE_EQUAL(int, 0, azulib_ustream_clone_current_position);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_reset(azulib_ustream_clone_interface));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(azulib_ustream_clone_interface, &azulib_ustream_clone_current_position));
    ASSERT_ARE_EQUAL(int, 0, azulib_ustream_clone_current_position);

    /* Full content was copied. */
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(
            azulib_ustream_clone_interface, 
            buf_result, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result));
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, size_result);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        buf_result, 
        size_result);

    /* Keep original buffer instance */
    check_buffer(
        ustream_instance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

TEST_FUNCTION(azulib_ustream_clone_compliance_new_buffer_cloned_with_offset_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);

    ///act
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 10000);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);

    /* current position was moved to the start of the buffer with the offset. */
    offset_t azulib_ustream_clone_current_position;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(azulib_ustream_clone_interface, &azulib_ustream_clone_current_position));
    ASSERT_ARE_EQUAL(int, 10000, azulib_ustream_clone_current_position);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_reset(azulib_ustream_clone_interface));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(azulib_ustream_clone_interface, &azulib_ustream_clone_current_position));
    ASSERT_ARE_EQUAL(int, 10000, azulib_ustream_clone_current_position);

    /* Full content was copied. */
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(
            azulib_ustream_clone_interface, 
            buf_result, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result));
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, size_result);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        buf_result, 
        size_result);

    /* Keep original buffer instance */
    check_buffer(
        ustream_instance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

TEST_FUNCTION(azulib_ustream_clone_compliance_empty_buffer_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_release(ustream_instance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1));

    ///act
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 0);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);

    /* current position was moved to the start of the buffer. */
    offset_t azulib_ustream_clone_current_position;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(azulib_ustream_clone_interface, &azulib_ustream_clone_current_position));
    ASSERT_ARE_EQUAL(int, 0, azulib_ustream_clone_current_position);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_reset(azulib_ustream_clone_interface));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(azulib_ustream_clone_interface, &azulib_ustream_clone_current_position));
    ASSERT_ARE_EQUAL(int, 0, azulib_ustream_clone_current_position);

    /* Full content was copied. */
    check_buffer(
        azulib_ustream_clone_interface, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    /* Keep original buffer instance */
    check_buffer(
        ustream_instance, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

TEST_FUNCTION(azulib_ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_2, &size));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_release(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1));

    ///act
    AZULIB_USTREAM* azulib_ustream_clone_interface =
        azulib_ustream_clone(ustream_instance, 100);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);

    /* current position was moved to the start of the buffer with the offset. */
    offset_t azulib_ustream_clone_current_position;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(azulib_ustream_clone_interface, &azulib_ustream_clone_current_position));
    ASSERT_ARE_EQUAL(int, 100, azulib_ustream_clone_current_position);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_reset(azulib_ustream_clone_interface));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(azulib_ustream_clone_interface, &azulib_ustream_clone_current_position));
    ASSERT_ARE_EQUAL(int, 100, azulib_ustream_clone_current_position);

    /* Full content was copied. */
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(
            azulib_ustream_clone_interface, 
            buf_result, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2, 
        size_result);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        (uint8_t*)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_2), 
        buf_result, 
        size_result);

    /* Keep original buffer instance */
    check_buffer(
        ustream_instance, 
        USTREAM_COMPLIANCE_LENGTH_2, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

TEST_FUNCTION(azulib_ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_negative_offset_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_2, &size));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_release(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1));

    ///act
    AZULIB_USTREAM* azulib_ustream_clone_interface =
        azulib_ustream_clone(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);

    /* current position was moved to the start of the buffer with the offset. */
    offset_t azulib_ustream_clone_current_position;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(azulib_ustream_clone_interface, &azulib_ustream_clone_current_position));
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_LENGTH_1, azulib_ustream_clone_current_position);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_reset(azulib_ustream_clone_interface));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(azulib_ustream_clone_interface, &azulib_ustream_clone_current_position));
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_LENGTH_1, azulib_ustream_clone_current_position);

    /* Full content was copied. */
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(
            azulib_ustream_clone_interface, 
            buf_result, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2, 
        size_result);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        (uint8_t*)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_2), 
        buf_result, 
        size_result);

    /* Keep original buffer instance */
    check_buffer(
        ustream_instance, 
        USTREAM_COMPLIANCE_LENGTH_2, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

TEST_FUNCTION(azulib_ustream_clone_compliance_cloned_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_release(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1 - 1));
    AZULIB_USTREAM* azulib_ustream_clone1 = azulib_ustream_clone(ustream_instance, 100);
    ASSERT_IS_NOT_NULL(azulib_ustream_clone1);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(azulib_ustream_clone1, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_release(azulib_ustream_clone1, 100 + USTREAM_COMPLIANCE_LENGTH_1 - 1));

    ///act
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(azulib_ustream_clone1, 0);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);

    /* current position was moved to the start of the buffer with the offset. */
    offset_t azulib_ustream_clone_current_position;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(azulib_ustream_clone_interface, &azulib_ustream_clone_current_position));
    ASSERT_ARE_EQUAL(int, 0, azulib_ustream_clone_current_position);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_reset(azulib_ustream_clone_interface));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(azulib_ustream_clone_interface, &azulib_ustream_clone_current_position));
    ASSERT_ARE_EQUAL(int, 0, azulib_ustream_clone_current_position);

    /* Full content was copied. */
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(
            azulib_ustream_clone_interface, 
            buf_result, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2, 
        size_result);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        (uint8_t*)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_2), 
        buf_result, 
        size_result);

    /* Keep original buffer instance */
    check_buffer(
        ustream_instance, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    check_buffer(
        azulib_ustream_clone1, 
        USTREAM_COMPLIANCE_LENGTH_2, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
    (void)azulib_ustream_dispose(azulib_ustream_clone1);
}

/* If the provided handle is NULL, the clone shall return NULL. */
TEST_FUNCTION(azulib_ustream_clone_compliance_null_buffer_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);

    ///act
    AZULIB_USTREAM* azulib_ustream_clone_interface = ustream_instance->api->clone(NULL, 0);

    ///assert
    ASSERT_IS_NULL(azulib_ustream_clone_interface);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided handle is not the implemented buffer type, the clone shall return NULL. */
TEST_FUNCTION(azulib_ustream_clone_compliance_buffer_is_not_type_of_buffer_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);

    ///act
    AZULIB_USTREAM* azulib_ustream_clone_interface =
        ustream_instance->api->clone(ustream_mock_create(), 0);

    ///assert
    ASSERT_IS_NULL(azulib_ustream_clone_interface);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the offset plus the buffer length bypass UINT32_MAX, the clone shall return NULL. */
TEST_FUNCTION(azulib_ustream_clone_compliance_offset_exceed_size_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();

    ///act
    AZULIB_USTREAM* azulib_ustream_clone_interface =
        azulib_ustream_clone(ustream_instance, UINT32_MAX - 2);

    ///assert
    ASSERT_IS_NULL(azulib_ustream_clone_interface);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* The get_remaining_size shall return the number of bytes between the current position and the end of the buffer. */
TEST_FUNCTION(azulib_ustream_get_remaining_size_compliance_new_buffer_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    size_t size;

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_get_remaining_size(ustream_instance, &size);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, size);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_get_remaining_size_compliance_new_buffer_with_non_zero_current_position_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_2, &size));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_release(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1));

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_get_remaining_size(ustream_instance, &size);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2, 
        size);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_get_remaining_size_compliance_cloned_buffer_with_non_zero_current_position_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_release(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1 - 1));
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 100);
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(azulib_ustream_clone_interface, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_release(azulib_ustream_clone_interface, 100 + USTREAM_COMPLIANCE_LENGTH_1 - 1));

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_get_remaining_size(azulib_ustream_clone_interface, &size);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2, 
        size);

    ///cleanup
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided handle is NULL, the get_remaining_size shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(azulib_ustream_get_remaining_size_compliance_null_buffer_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    size_t size;

    ///act
    AZULIB_ULIB_RESULT result = ustream_instance->api->get_remaining_size(NULL, &size);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided handle is not the implemented buffer type, the get_remaining_size shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(azulib_ustream_get_remaining_size_compliance_buffer_is_not_type_of_buffer_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    size_t size;

    ///act
    AZULIB_ULIB_RESULT result = 
        ustream_instance->api->get_remaining_size(ustream_mock_create(), &size);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided size is NULL, the get_remaining_size shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(azulib_ustream_get_remaining_size_compliance_null_size_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_get_remaining_size(ustream_instance, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* The get_position shall return the logical current position of the buffer. */
TEST_FUNCTION(ustream_get_current_position_compliance_new_buffer_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    offset_t position;

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_get_position(ustream_instance, &position);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, 0, position);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(ustream_get_current_position_compliance_new_buffer_with_non_zero_current_position_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    offset_t position;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_2, &size));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_release(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1));

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_get_position(ustream_instance, &position);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_LENGTH_2, position);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(ustream_get_current_position_compliance_cloned_buffer_with_non_zero_current_position_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    offset_t position;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_release(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1 - 1));
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 100);
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(azulib_ustream_clone_interface, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_release(azulib_ustream_clone_interface, 100 + USTREAM_COMPLIANCE_LENGTH_1 - 1));

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_get_position(azulib_ustream_clone_interface, &position);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, 100 + USTREAM_COMPLIANCE_LENGTH_1, position);

    ///cleanup
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided interface is NULL, the get_position shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(ustream_get_current_position_compliance_null_buffer_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    offset_t position;

    ///act
    AZULIB_ULIB_RESULT result = ustream_instance->api->get_position(NULL, &position);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided interface is not the implemented buffer type, the get_position shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(ustream_get_current_position_compliance_buffer_is_not_type_of_buffer_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    offset_t position;

    ///act
    AZULIB_ULIB_RESULT result = 
        ustream_instance->api->get_position(ustream_mock_create(), &position);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided position is NULL, the get_position shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(ustream_get_current_position_compliance_null_position_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_get_position(ustream_instance, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* [1]The read shall copy the content in the provided buffer and return the number of valid <tt>uint8_t</tt> values in the local buffer in the provided `size`. */
/* [2]If the length of the content is bigger than the `buffer_length`, the read shall limit the copy size to the buffer_length.*/
/* [3]If there is no more content to return, the read shall return AZULIB_ULIB_NO_SUCH_ELEMENT_ERROR, size shall receive 0, and do not change the content of the local buffer. */
TEST_FUNCTION(azulib_ustream_read_compliance_get_from_original_buffer_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result1[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    uint8_t buf_result2[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    uint8_t buf_result3[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result1;
    size_t size_result2;
    size_t size_result3;

    ///act
    AZULIB_ULIB_RESULT result1 = 
        azulib_ustream_read(
            ustream_instance, 
            buf_result1, 
            USTREAM_COMPLIANCE_LENGTH_1, 
            &size_result1);
    AZULIB_ULIB_RESULT result2 = 
        azulib_ustream_read(
            ustream_instance, 
            buf_result2, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result2);
    AZULIB_ULIB_RESULT result3 = 
        azulib_ustream_read(
            ustream_instance, 
            buf_result3, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result3);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_LENGTH_1, size_result1);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        buf_result1, 
        size_result1);

    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result2);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1, 
        size_result2);
    ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr,
            (const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_1),
            buf_result2,
            size_result2);

    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_EOF, result3);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_read_compliance_get_from_cloned_buffer_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1));
    AZULIB_USTREAM* azulib_ustream_clone_interface = 
        azulib_ustream_clone(ustream_instance, 100);
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);
    (void)azulib_ustream_dispose(ustream_instance);

    uint8_t buf_result1[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    uint8_t buf_result2[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    uint8_t buf_result3[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result1;
    size_t size_result2;
    size_t size_result3;

    ///act
    AZULIB_ULIB_RESULT result1 = 
        azulib_ustream_read(
            azulib_ustream_clone_interface, 
            buf_result1, 
            USTREAM_COMPLIANCE_LENGTH_1, 
            &size_result1);
    AZULIB_ULIB_RESULT result2 = 
        azulib_ustream_read(
            azulib_ustream_clone_interface, 
            buf_result2, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result2);
    AZULIB_ULIB_RESULT result3 = 
        azulib_ustream_read(
            azulib_ustream_clone_interface, 
            buf_result3, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result3);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_LENGTH_1, size_result1);
    ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr, 
            (const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_1),
            buf_result1, 
            size_result1);

    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result2);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2, 
        size_result2);
    ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr,
            (const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_2),
            buf_result2,
            size_result2);

    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_EOF, result3);

    ///cleanup
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

TEST_FUNCTION(azulib_ustream_read_compliance_single_buffer_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;

    ///act
    AZULIB_ULIB_RESULT result = 
        azulib_ustream_read(
            ustream_instance, 
            buf_result, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, size_result);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_read_compliance_right_boundary_condition_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;

    ///act
    AZULIB_ULIB_RESULT result = 
        azulib_ustream_read(
            ustream_instance, 
            buf_result, 
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1, 
            &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1, size_result);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        buf_result, 
        size_result);

    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(
            ustream_instance, 
            buf_result, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result));
    ASSERT_ARE_EQUAL(int, 1, size_result);
    ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr,
            (const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1),
            buf_result,
            size_result);

    ASSERT_ARE_EQUAL(
            int,
            AZULIB_ULIB_EOF,
            azulib_ustream_read(
                ustream_instance, 
                buf_result, 
                USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
                &size_result));

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_read_compliance_cloned_buffer_right_boundary_condition_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1));
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 0);
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);
    (void)azulib_ustream_dispose(ustream_instance);

    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;

    ///act
    AZULIB_ULIB_RESULT result = 
        azulib_ustream_read(
            azulib_ustream_clone_interface, 
            buf_result, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, 1, size_result);
    ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr, 
            (const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1),
            buf_result, 
            size_result);

    ASSERT_ARE_EQUAL(
            int, 
            AZULIB_ULIB_EOF, 
            azulib_ustream_read(
                azulib_ustream_clone_interface, 
                buf_result, 
                USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
                &size_result));

    ///cleanup
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

TEST_FUNCTION(azulib_ustream_read_compliance_boundary_condition_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;

    ///act
    AZULIB_ULIB_RESULT result = 
        azulib_ustream_read(
            ustream_instance, 
            buf_result, 
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
            &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, size_result);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

    ASSERT_ARE_EQUAL(
            int,
            AZULIB_ULIB_EOF,
            azulib_ustream_read(
                ustream_instance, 
                buf_result, 
                USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
                &size_result));

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_read_compliance_left_boundary_condition_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;

    ///act
    AZULIB_ULIB_RESULT result = 
        azulib_ustream_read(
            ustream_instance, 
            buf_result, 
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 1, 
            &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, size_result);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

    ASSERT_ARE_EQUAL(
            int,
            AZULIB_ULIB_EOF,
            azulib_ustream_read(
                ustream_instance, 
                buf_result, 
                USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
                &size_result));

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_read_compliance_single_byte_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_read(ustream_instance, buf_result, 1, &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, 1, size_result);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, buf_result, size_result);

    check_buffer(
        ustream_instance, 
        1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided buffer_length is zero, the read shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(azulib_ustream_read_compliance_buffer_with_zero_size_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_read(ustream_instance, buf_result, 0, &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    check_buffer(
        ustream_instance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided handle is NULL, the read shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(azulib_ustream_read_compliance_null_buffer_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;

    ///act
    AZULIB_ULIB_RESULT result = 
        ustream_instance->api->read(
            NULL, 
            buf_result, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    check_buffer(
        ustream_instance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided handle is not the implemented buffer type, the read shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(azulib_ustream_read_compliance_non_type_of_buffer_api_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;

    ///act
    AZULIB_ULIB_RESULT result = 
        ustream_instance->api->read(
            ustream_mock_create(), 
            buf_result, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    check_buffer(
        ustream_instance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided buffer is NULL, the read shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(azulib_ustream_read_compliance_null_return_buffer_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    size_t size_result;

    ///act
    AZULIB_ULIB_RESULT result = 
        azulib_ustream_read(
            ustream_instance, 
            NULL, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    check_buffer(
        ustream_instance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided return size pointer is NULL, the read shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(azulib_ustream_read_compliance_null_return_size_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];

    ///act
    AZULIB_ULIB_RESULT result = 
        azulib_ustream_read(
            ustream_instance, 
            buf_result, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    check_buffer(
        ustream_instance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* The set_position shall change the current position of the buffer. */
TEST_FUNCTION(azulib_ustream_set_position_compliance_back_to_beginning_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(
            ustream_instance, 
            buf_result, 
            USTREAM_COMPLIANCE_LENGTH_1, 
            &size_result));

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_set_position(ustream_instance, 0);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    check_buffer(
        ustream_instance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_set_position_compliance_back_position_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(
            ustream_instance, 
            buf_result, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result));

    ///act
    AZULIB_ULIB_RESULT result = 
        azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    check_buffer(
        ustream_instance, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_set_position_compliance_forward_position_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(ustream_instance, buf_result, USTREAM_COMPLIANCE_LENGTH_1, &size_result));

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_LENGTH_2);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    check_buffer(
        ustream_instance, 
        USTREAM_COMPLIANCE_LENGTH_2, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_set_position_compliance_forward_to_the_end_position_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    AZULIB_ULIB_RESULT result = 
        azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    check_buffer(
        ustream_instance, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_set_position_compliance_run_full_buffer_byte_by_byte_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[1];
    size_t size_result;

    for(offset_t i = 0; i < USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH; i++)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_set_position(ustream_instance, i));
        
        ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS,
                azulib_ustream_read(ustream_instance, buf_result, 1, &size_result));

        ASSERT_ARE_EQUAL(int, 1, size_result);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + i),
                *buf_result);
    }
    check_buffer(
        ustream_instance, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_set_position_compliance_run_full_buffer_byte_by_byte_reverse_order_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[1];
    size_t size_result;

    for(offset_t i = USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1; i > 0; i--)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_set_position(ustream_instance, i));
        
        ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS,
                azulib_ustream_read(ustream_instance, buf_result, 1, &size_result));

        ASSERT_ARE_EQUAL(int, 1, size_result);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + i),
                *buf_result);
    }
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_set_position(ustream_instance, 0));
    check_buffer(
        ustream_instance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_set_position_compliance_cloned_buffer_back_to_beginning_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1));
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 0);
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);
    (void)azulib_ustream_dispose(ustream_instance);
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(azulib_ustream_clone_interface, buf_result, USTREAM_COMPLIANCE_LENGTH_1, &size_result));

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_set_position(azulib_ustream_clone_interface, 0);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    check_buffer(
        azulib_ustream_clone_interface, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

TEST_FUNCTION(azulib_ustream_set_position_compliance_cloned_buffer_back_position_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1));
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 0);
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);
    (void)azulib_ustream_dispose(ustream_instance);
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(
            azulib_ustream_clone_interface, 
            buf_result, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &size_result));

    ///act
    AZULIB_ULIB_RESULT result = 
        azulib_ustream_set_position(azulib_ustream_clone_interface, USTREAM_COMPLIANCE_LENGTH_1);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    check_buffer(
        azulib_ustream_clone_interface, 
        USTREAM_COMPLIANCE_LENGTH_2, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

TEST_FUNCTION(azulib_ustream_set_position_compliance_cloned_buffer_forward_position_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1));
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 0);
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);
    (void)azulib_ustream_dispose(ustream_instance);
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(azulib_ustream_clone_interface, buf_result, USTREAM_COMPLIANCE_LENGTH_1, &size_result));

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_set_position(azulib_ustream_clone_interface, USTREAM_COMPLIANCE_LENGTH_2);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    check_buffer(
        azulib_ustream_clone_interface, 
        USTREAM_COMPLIANCE_LENGTH_3, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

TEST_FUNCTION(azulib_ustream_set_position_compliance_cloned_buffer_forward_to_the_end_position_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1));
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 100);
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);
    (void)azulib_ustream_dispose(ustream_instance);

    ///act
    AZULIB_ULIB_RESULT result = 
        azulib_ustream_set_position(
            azulib_ustream_clone_interface, 
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1 + 100);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    check_buffer(
        azulib_ustream_clone_interface, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

TEST_FUNCTION(azulib_ustream_set_position_compliance_cloned_buffer_run_full_buffer_byte_by_byte_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1));
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 0);
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);
    (void)azulib_ustream_dispose(ustream_instance);
    uint8_t buf_result[1];
    size_t size_result;

    for(offset_t i = 0; i < USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1; i++)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_set_position(azulib_ustream_clone_interface, i));
        
        ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS,
                azulib_ustream_read(azulib_ustream_clone_interface, buf_result, 1, &size_result));

        ASSERT_ARE_EQUAL(int, 1, size_result);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_1 + i),
                *buf_result);
    }
    check_buffer(
        azulib_ustream_clone_interface, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

TEST_FUNCTION(azulib_ustream_set_position_compliance_cloned_buffer_run_full_buffer_byte_by_byte_reverse_order_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1));
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 0);
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);
    (void)azulib_ustream_dispose(ustream_instance);
    uint8_t buf_result[1];
    size_t size_result;

    for(offset_t i = USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1 - 1; i > 0; i--)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_set_position(azulib_ustream_clone_interface, i));
        
        ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS,
                azulib_ustream_read(azulib_ustream_clone_interface, buf_result, 1, &size_result));

        ASSERT_ARE_EQUAL(int, 1, size_result);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_1 + i),
                *buf_result);
    }
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_set_position(azulib_ustream_clone_interface, 0));
    check_buffer(
        azulib_ustream_clone_interface, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

/* If the provided position is out of the range of the buffer, the set_position shall return AZULIB_ULIB_NO_SUCH_ELEMENT_ERROR, and do not change the current position. */
TEST_FUNCTION(azulib_ustream_set_position_compliance_forward_out_of_the_buffer_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    AZULIB_ULIB_RESULT result = 
        azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 1);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_NO_SUCH_ELEMENT_ERROR, result);
    check_buffer(
        ustream_instance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided position is before the first valid position, the set_position shall return AZULIB_ULIB_NO_SUCH_ELEMENT_ERROR, and do not change the current position. */
TEST_FUNCTION(azulib_ustream_set_position_compliance_back_before_first_valid_position_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_release(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1 - 1));

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_set_position(ustream_instance, 0);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_NO_SUCH_ELEMENT_ERROR, result);
    check_buffer(
        ustream_instance, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided handle is NULL, the set_position shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(azulib_ustream_set_position_compliance_null_buffer_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    AZULIB_ULIB_RESULT result = ustream_instance->api->set_position(NULL, 1);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided handle is not the implemented buffer type, the set_position shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(azulib_ustream_set_position_compliance_non_type_of_buffer_api_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    AZULIB_ULIB_RESULT result = 
        ustream_instance->api->set_position(ustream_mock_create(), 1);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* The release shall do nothing for ustream_instance. */
TEST_FUNCTION(azulib_ustream_release_compliance_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(ustream_instance, buf_result, USTREAM_COMPLIANCE_LENGTH_1, &size_result));
    offset_t current_position;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(ustream_instance, &current_position));

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_release(ustream_instance, current_position - 1);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    check_buffer(
        ustream_instance, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_release_compliance_release_all_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH));

    ///act
    AZULIB_ULIB_RESULT result = 
        azulib_ustream_release(ustream_instance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_reset(ustream_instance));
    check_buffer(
        ustream_instance, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_release_compliance_run_full_buffer_byte_by_byte_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[1];
    size_t size_result;

    for(offset_t i = 1; i < USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1; i++)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_set_position(ustream_instance, i + 1));
    
        ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_release(ustream_instance, i - 1));
        
        ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_reset(ustream_instance));
    
        ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS,
                azulib_ustream_read(ustream_instance, buf_result, 1, &size_result));

        ASSERT_ARE_EQUAL(int, 1, size_result);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + i),
                *buf_result);
    }
    check_buffer(
        ustream_instance, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_release_compliance_cloned_buffer_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1));
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 0);
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);
    (void)azulib_ustream_dispose(ustream_instance);
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(azulib_ustream_clone_interface, buf_result, USTREAM_COMPLIANCE_LENGTH_1, &size_result));
    offset_t current_position;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(azulib_ustream_clone_interface, &current_position));

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_release(azulib_ustream_clone_interface, current_position - 1);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    check_buffer(
        azulib_ustream_clone_interface, 
        USTREAM_COMPLIANCE_LENGTH_2, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

TEST_FUNCTION(azulib_ustream_release_compliance_cloned_buffer_release_all_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1));
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 1000);
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);
    (void)azulib_ustream_dispose(ustream_instance);
    ASSERT_ARE_EQUAL(
            int, 
            AZULIB_ULIB_SUCCESS, 
            azulib_ustream_set_position(
                azulib_ustream_clone_interface, 
                USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1 + 1000));

    ///act
    AZULIB_ULIB_RESULT result = 
        azulib_ustream_release(
            azulib_ustream_clone_interface, 
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1 + 999);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_reset(azulib_ustream_clone_interface));
    check_buffer(
        azulib_ustream_clone_interface, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

TEST_FUNCTION(azulib_ustream_release_compliance_cloned_buffer_run_full_buffer_byte_by_byte_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_set_position(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1));
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 0);
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);
    (void)azulib_ustream_dispose(ustream_instance);
    uint8_t buf_result[1];
    size_t size_result;

    for(offset_t i = 1; i < USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1 - 1; i++)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_set_position(azulib_ustream_clone_interface, i + 1));
    
        ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_release(azulib_ustream_clone_interface, i - 1));
        
        ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_reset(azulib_ustream_clone_interface));
    
        ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS,
                azulib_ustream_read(azulib_ustream_clone_interface, buf_result, 1, &size_result));

        ASSERT_ARE_EQUAL(int, 1, size_result);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_1 + i),
                *buf_result);
    }
    check_buffer(
        azulib_ustream_clone_interface, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

/* If the release position is after the current position, the release shall return AZULIB_ULIB_NO_SUCH_ELEMENT_ERROR, and do not release any resource. */
TEST_FUNCTION(azulib_ustream_release_compliance_release_after_current_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(ustream_instance, buf_result, USTREAM_COMPLIANCE_LENGTH_1, &size_result));
    offset_t current_position;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_get_position(ustream_instance, &current_position));

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_release(ustream_instance, current_position);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    check_buffer(
        ustream_instance, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the release position is already released, the release shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, and do not release any resource. */
TEST_FUNCTION(azulib_ustream_release_compliance_release_position_already_released_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(ustream_instance, buf_result, USTREAM_COMPLIANCE_LENGTH_2, &size_result));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_release(ustream_instance, USTREAM_COMPLIANCE_LENGTH_2 - 1));

    ///act
    AZULIB_ULIB_RESULT result = 
        azulib_ustream_release(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, azulib_ustream_reset(ustream_instance));
    check_buffer(
        ustream_instance, 
        USTREAM_COMPLIANCE_LENGTH_2, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided handle is NULL, the release shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(azulib_ustream_release_compliance_null_buffer_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    AZULIB_ULIB_RESULT result1 = ustream_instance->api->release(NULL, 0);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result1);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided handle is not the implemented buffer type, the release shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(azulib_ustream_release_compliance_non_type_of_buffer_api_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    AZULIB_ULIB_RESULT result1 = 
        ustream_instance->api->release(ustream_mock_create(), 0);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result1);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* The reset shall change the current position of the buffer. */
TEST_FUNCTION(azulib_ustream_reset_compliance_back_to_beginning_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(ustream_instance, buf_result, 5, &size_result));

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_reset(ustream_instance);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    check_buffer(
        ustream_instance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_reset_compliance_back_position_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t buf_result[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size_result;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(ustream_instance, buf_result, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &size_result));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_release(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1 - 1));

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_reset(ustream_instance);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    check_buffer(
        ustream_instance, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

TEST_FUNCTION(azulib_ustream_reset_compliance_cloned_buffer_succeed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(ustream_instance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(ustream_instance, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_release(ustream_instance, USTREAM_COMPLIANCE_LENGTH_1 - 1));
    AZULIB_USTREAM* azulib_ustream_clone_interface = azulib_ustream_clone(ustream_instance, 100);
    ASSERT_IS_NOT_NULL(azulib_ustream_clone_interface);
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_read(azulib_ustream_clone_interface, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        AZULIB_ULIB_SUCCESS, 
        azulib_ustream_release(azulib_ustream_clone_interface, 100 + USTREAM_COMPLIANCE_LENGTH_1 - 1));

    ///act
    AZULIB_ULIB_RESULT result = azulib_ustream_reset(azulib_ustream_clone_interface);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_SUCCESS, result);
    check_buffer(
        azulib_ustream_clone_interface, 
        USTREAM_COMPLIANCE_LENGTH_2, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    check_buffer(
        ustream_instance, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
    (void)azulib_ustream_dispose(azulib_ustream_clone_interface);
}

/* If the provided handle is NULL, the reset shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(azulib_ustream_reset_compliance_null_buffer_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    AZULIB_ULIB_RESULT result = ustream_instance->api->reset(NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

/* If the provided handle is not the implemented buffer type, the reset shall return AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(azulib_ustream_reset_compliance_non_type_of_buffer_api_failed)
{
    ///arrange
    AZULIB_USTREAM* ustream_instance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    AZULIB_ULIB_RESULT result = ustream_instance->api->reset(ustream_mock_create());

    ///assert
    ASSERT_ARE_EQUAL(int, AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)azulib_ustream_dispose(ustream_instance);
}

#endif /* AZURE_ULIB_C_TESTS_INC_USTREAM_COMPLIANCE_UT_H */
