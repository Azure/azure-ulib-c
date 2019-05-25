// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_TESTS_INC_USTREAM_COMPLIANCE_UT_H_
#define AZURE_ULIB_C_TESTS_INC_USTREAM_COMPLIANCE_UT_H_

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
TEST_FUNCTION(uStreamDispose_complianceClonedInstanceDisposedFirstSucceed)
{
    ///arrange
    offset_t uStreamCloneCurrentPosition;
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamInstance);
        
    ///act
    ULIB_RESULT result = uStreamDispose(uStreamCloneInterface);
        
    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamInstance, &uStreamCloneCurrentPosition));
        
    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(uStreamDispose_complianceClonedInstanceDisposedSecondSucceed)
{
    ///arrange
    offset_t uStreamCloneCurrentPosition;
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamInstance);
        
    ///act
    ULIB_RESULT result = uStreamDispose(uStreamInstance);
        
    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamCloneInterface, &uStreamCloneCurrentPosition));
        
    ///cleanup
    (void)uStreamDispose(uStreamCloneInterface);
}

/* If there is no more instances of the buffer, the dispose shall release all allocated
 *          resources to control the buffer. */
TEST_FUNCTION(uStreamDispose_complianceSingleInstanceSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    
    ///act
    ULIB_RESULT result = uStreamDispose(uStreamInstance);
    
    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    
    ///cleanup
}

/* If the provided handle is NULL, the dispose shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(uStreamDispose_complianceNullBufferFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    
    ///act
    ULIB_RESULT result = uStreamInstance->api->dispose(NULL);
    
    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    
    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided handle is not the implemented buffer type, the dispose shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(uStreamDispose_complianceBufferIsNotTypeOfBufferFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    
    ///act
    ULIB_RESULT result = uStreamInstance->api->dispose(uStreamMockCreate());
    
    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    
    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* The clone shall return a buffer with the same content of the original buffer. */
/* The clone shall start the buffer on the current position of the original buffer. */
/* The clone shall move the current position to the start position. */
/* The clone shall move the released position to the one immediately before the current position. */
/* The cloned buffer shall not interfere in the instance of the original buffer and vice versa. */
TEST_FUNCTION(uStreamClone_complianceNewBufferClonedWithZeroOffsetSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);

    ///act
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 0);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);

    /* current position was moved to the start of the buffer. */
    offset_t uStreamCloneCurrentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamCloneInterface, &uStreamCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 0, uStreamCloneCurrentPosition);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_reset(uStreamCloneInterface));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamCloneInterface, &uStreamCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 0, uStreamCloneCurrentPosition);

    /* Full content was copied. */
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(
            uStreamCloneInterface, 
            bufResult, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult));
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        bufResult, 
        sizeResult);

    /* Keep original buffer instance */
    checkBuffer(
        uStreamInstance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
    (void)uStreamDispose(uStreamCloneInterface);
}

TEST_FUNCTION(uStreamClone_complianceNewBufferClonedWithOffsetSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);

    ///act
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 10000);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);

    /* current position was moved to the start of the buffer with the offset. */
    offset_t uStreamCloneCurrentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamCloneInterface, &uStreamCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 10000, uStreamCloneCurrentPosition);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_reset(uStreamCloneInterface));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamCloneInterface, &uStreamCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 10000, uStreamCloneCurrentPosition);

    /* Full content was copied. */
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(
            uStreamCloneInterface, 
            bufResult, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult));
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        bufResult, 
        sizeResult);

    /* Keep original buffer instance */
    checkBuffer(
        uStreamInstance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
    (void)uStreamDispose(uStreamCloneInterface);
}

TEST_FUNCTION(uStreamClone_complianceEmptyBufferSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamRelease(uStreamInstance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1));

    ///act
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 0);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);

    /* current position was moved to the start of the buffer. */
    offset_t uStreamCloneCurrentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamCloneInterface, &uStreamCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 0, uStreamCloneCurrentPosition);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_reset(uStreamCloneInterface));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamCloneInterface, &uStreamCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 0, uStreamCloneCurrentPosition);

    /* Full content was copied. */
    checkBuffer(
        uStreamCloneInterface, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    /* Keep original buffer instance */
    checkBuffer(
        uStreamInstance, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
    (void)uStreamDispose(uStreamCloneInterface);
}

TEST_FUNCTION(uStreamClone_complianceNewBufferWithNonZeroCurrentAndReleasedPositionsClonedWithOffsetSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamInstance, buf, USTREAM_COMPLIANCE_LENGTH_2, &size));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamRelease(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1));

    ///act
    USTREAM* uStreamCloneInterface =
        uStreamClone(uStreamInstance, 100);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);

    /* current position was moved to the start of the buffer with the offset. */
    offset_t uStreamCloneCurrentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamCloneInterface, &uStreamCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 100, uStreamCloneCurrentPosition);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_reset(uStreamCloneInterface));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamCloneInterface, &uStreamCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 100, uStreamCloneCurrentPosition);

    /* Full content was copied. */
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(
            uStreamCloneInterface, 
            bufResult, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2, 
        sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        (uint8_t*)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_2), 
        bufResult, 
        sizeResult);

    /* Keep original buffer instance */
    checkBuffer(
        uStreamInstance, 
        USTREAM_COMPLIANCE_LENGTH_2, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
    (void)uStreamDispose(uStreamCloneInterface);
}

TEST_FUNCTION(uStreamClone_complianceNewBufferWithNonZeroCurrentAndReleasedPositionsClonedWithNegativeOffsetSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamInstance, buf, USTREAM_COMPLIANCE_LENGTH_2, &size));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamRelease(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1));

    ///act
    USTREAM* uStreamCloneInterface =
        uStreamClone(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);

    /* current position was moved to the start of the buffer with the offset. */
    offset_t uStreamCloneCurrentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamCloneInterface, &uStreamCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_LENGTH_1, uStreamCloneCurrentPosition);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_reset(uStreamCloneInterface));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamCloneInterface, &uStreamCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_LENGTH_1, uStreamCloneCurrentPosition);

    /* Full content was copied. */
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(
            uStreamCloneInterface, 
            bufResult, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2, 
        sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        (uint8_t*)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_2), 
        bufResult, 
        sizeResult);

    /* Keep original buffer instance */
    checkBuffer(
        uStreamInstance, 
        USTREAM_COMPLIANCE_LENGTH_2, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
    (void)uStreamDispose(uStreamCloneInterface);
}

TEST_FUNCTION(uStreamClone_complianceClonedBufferWithNonZeroCurrentAndReleasedPositionsClonedWithOffsetSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamInstance, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamRelease(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1 - 1));
    USTREAM* uStreamClone1 = uStreamClone(uStreamInstance, 100);
    ASSERT_IS_NOT_NULL(uStreamClone1);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamClone1, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamRelease(uStreamClone1, 100 + USTREAM_COMPLIANCE_LENGTH_1 - 1));

    ///act
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamClone1, 0);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);

    /* current position was moved to the start of the buffer with the offset. */
    offset_t uStreamCloneCurrentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamCloneInterface, &uStreamCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 0, uStreamCloneCurrentPosition);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_reset(uStreamCloneInterface));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamCloneInterface, &uStreamCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 0, uStreamCloneCurrentPosition);

    /* Full content was copied. */
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(
            uStreamCloneInterface, 
            bufResult, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2, 
        sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        (uint8_t*)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_2), 
        bufResult, 
        sizeResult);

    /* Keep original buffer instance */
    checkBuffer(
        uStreamInstance, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    checkBuffer(
        uStreamClone1, 
        USTREAM_COMPLIANCE_LENGTH_2, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
    (void)uStreamDispose(uStreamCloneInterface);
    (void)uStreamDispose(uStreamClone1);
}

/* If the provided handle is NULL, the clone shall return NULL. */
TEST_FUNCTION(uStreamClone_complianceNullBufferFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);

    ///act
    USTREAM* uStreamCloneInterface = uStreamInstance->api->clone(NULL, 0);

    ///assert
    ASSERT_IS_NULL(uStreamCloneInterface);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided handle is not the implemented buffer type, the clone shall return NULL. */
TEST_FUNCTION(uStreamClone_complianceBufferIsNotTypeOfBufferFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);

    ///act
    USTREAM* uStreamCloneInterface =
        uStreamInstance->api->clone(uStreamMockCreate(), 0);

    ///assert
    ASSERT_IS_NULL(uStreamCloneInterface);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the offset plus the buffer length bypass UINT32_MAX, the clone shall return NULL. */
TEST_FUNCTION(uStreamClone_complianceOffsetExceedSizeFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();

    ///act
    USTREAM* uStreamCloneInterface =
        uStreamClone(uStreamInstance, UINT32_MAX - 2);

    ///assert
    ASSERT_IS_NULL(uStreamCloneInterface);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* The get_remaining_size shall return the number of bytes between the current position and the end of the buffer. */
TEST_FUNCTION(uStreamGetRemainingSize_complianceNewBufferSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    size_t size;

    ///act
    ULIB_RESULT result = ustream_get_remaining_size(uStreamInstance, &size);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, size);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(uStreamGetRemainingSize_complianceNewBufferWithNonZeroCurrentPositionSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamInstance, buf, USTREAM_COMPLIANCE_LENGTH_2, &size));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamRelease(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1));

    ///act
    ULIB_RESULT result = ustream_get_remaining_size(uStreamInstance, &size);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2, 
        size);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(uStreamGetRemainingSize_complianceClonedBufferWithNonZeroCurrentPositionSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamInstance, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamRelease(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1 - 1));
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 100);
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamCloneInterface, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamRelease(uStreamCloneInterface, 100 + USTREAM_COMPLIANCE_LENGTH_1 - 1));

    ///act
    ULIB_RESULT result = ustream_get_remaining_size(uStreamCloneInterface, &size);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2, 
        size);

    ///cleanup
    (void)uStreamDispose(uStreamCloneInterface);
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided handle is NULL, the get_remaining_size shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(uStreamGetRemainingSize_complianceNullBufferFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    size_t size;

    ///act
    ULIB_RESULT result = uStreamInstance->api->get_remaining_size(NULL, &size);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided handle is not the implemented buffer type, the get_remaining_size shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(uStreamGetRemainingSize_complianceBufferIsNotTypeOfBufferFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    size_t size;

    ///act
    ULIB_RESULT result = 
        uStreamInstance->api->get_remaining_size(uStreamMockCreate(), &size);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided size is NULL, the get_remaining_size shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(uStreamGetRemainingSize_complianceNullSizeFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    ULIB_RESULT result = ustream_get_remaining_size(uStreamInstance, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* The getCurrentPosition shall return the logical current position of the buffer. */
TEST_FUNCTION(uStreamGetCurrentPosition_complianceNewBufferSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    offset_t position;

    ///act
    ULIB_RESULT result = uStreamGetCurrentPosition(uStreamInstance, &position);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, 0, position);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(uStreamGetCurrentPosition_complianceNewBufferWithNonZeroCurrentPositionSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    offset_t position;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamInstance, buf, USTREAM_COMPLIANCE_LENGTH_2, &size));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamRelease(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1));

    ///act
    ULIB_RESULT result = uStreamGetCurrentPosition(uStreamInstance, &position);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_LENGTH_2, position);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(uStreamGetCurrentPosition_complianceClonedBufferWithNonZeroCurrentPositionSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    offset_t position;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamInstance, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamRelease(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1 - 1));
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 100);
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamCloneInterface, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamRelease(uStreamCloneInterface, 100 + USTREAM_COMPLIANCE_LENGTH_1 - 1));

    ///act
    ULIB_RESULT result = uStreamGetCurrentPosition(uStreamCloneInterface, &position);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, 100 + USTREAM_COMPLIANCE_LENGTH_1, position);

    ///cleanup
    (void)uStreamDispose(uStreamCloneInterface);
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided interface is NULL, the getCurrentPosition shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(uStreamGetCurrentPosition_complianceNullBufferFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    offset_t position;

    ///act
    ULIB_RESULT result = uStreamInstance->api->getCurrentPosition(NULL, &position);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided interface is not the implemented buffer type, the getCurrentPosition shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(uStreamGetCurrentPosition_complianceBufferIsNotTypeOfBufferFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    offset_t position;

    ///act
    ULIB_RESULT result = 
        uStreamInstance->api->getCurrentPosition(uStreamMockCreate(), &position);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided position is NULL, the getCurrentPosition shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(uStreamGetCurrentPosition_complianceNullPositionFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    ULIB_RESULT result = uStreamGetCurrentPosition(uStreamInstance, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* [1]The read shall copy the content in the provided buffer and return the number of valid {@code uint8_t} values in the local buffer in the provided `size`. */
/* [2]If the length of the content is bigger than the `bufferLength`, the read shall limit the copy size to the bufferLength.*/
/* [3]If there is no more content to return, the read shall return ULIB_NO_SUCH_ELEMENT_ERROR, size shall receive 0, and do not change the content of the local buffer. */
TEST_FUNCTION(ustream_read_complianceGetFromOriginalBufferSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult1[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    uint8_t bufResult2[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    uint8_t bufResult3[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult1;
    size_t sizeResult2;
    size_t sizeResult3;

    ///act
    ULIB_RESULT result1 = 
        ustream_read(
            uStreamInstance, 
            bufResult1, 
            USTREAM_COMPLIANCE_LENGTH_1, 
            &sizeResult1);
    ULIB_RESULT result2 = 
        ustream_read(
            uStreamInstance, 
            bufResult2, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult2);
    ULIB_RESULT result3 = 
        ustream_read(
            uStreamInstance, 
            bufResult3, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult3);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_LENGTH_1, sizeResult1);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        bufResult1, 
        sizeResult1);

    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result2);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1, 
        sizeResult2);
    ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr,
            (const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_1),
            bufResult2,
            sizeResult2);

    ASSERT_ARE_EQUAL(int, ULIB_EOF, result3);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(ustream_read_complianceGetFromClonedBufferSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1));
    USTREAM* uStreamCloneInterface = 
        uStreamClone(uStreamInstance, 100);
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);
    (void)uStreamDispose(uStreamInstance);

    uint8_t bufResult1[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    uint8_t bufResult2[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    uint8_t bufResult3[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult1;
    size_t sizeResult2;
    size_t sizeResult3;

    ///act
    ULIB_RESULT result1 = 
        ustream_read(
            uStreamCloneInterface, 
            bufResult1, 
            USTREAM_COMPLIANCE_LENGTH_1, 
            &sizeResult1);
    ULIB_RESULT result2 = 
        ustream_read(
            uStreamCloneInterface, 
            bufResult2, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult2);
    ULIB_RESULT result3 = 
        ustream_read(
            uStreamCloneInterface, 
            bufResult3, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult3);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_LENGTH_1, sizeResult1);
    ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr, 
            (const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_1),
            bufResult1, 
            sizeResult1);

    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result2);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_2, 
        sizeResult2);
    ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr,
            (const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_2),
            bufResult2,
            sizeResult2);

    ASSERT_ARE_EQUAL(int, ULIB_EOF, result3);

    ///cleanup
    (void)uStreamDispose(uStreamCloneInterface);
}

TEST_FUNCTION(ustream_read_complianceSingleBufferSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    ULIB_RESULT result = 
        ustream_read(
            uStreamInstance, 
            bufResult, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, bufResult, sizeResult);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(ustream_read_complianceRightBoundaryConditionSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    ULIB_RESULT result = 
        ustream_read(
            uStreamInstance, 
            bufResult, 
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1, 
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        bufResult, 
        sizeResult);

    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(
            uStreamInstance, 
            bufResult, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult));
    ASSERT_ARE_EQUAL(int, 1, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr,
            (const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1),
            bufResult,
            sizeResult);

    ASSERT_ARE_EQUAL(
            int,
            ULIB_EOF,
            ustream_read(
                uStreamInstance, 
                bufResult, 
                USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
                &sizeResult));

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(ustream_read_complianceClonedBufferRightBoundaryConditionSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1));
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);
    (void)uStreamDispose(uStreamInstance);

    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    ULIB_RESULT result = 
        ustream_read(
            uStreamCloneInterface, 
            bufResult, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, 1, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr, 
            (const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1),
            bufResult, 
            sizeResult);

    ASSERT_ARE_EQUAL(
            int, 
            ULIB_EOF, 
            ustream_read(
                uStreamCloneInterface, 
                bufResult, 
                USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
                &sizeResult));

    ///cleanup
    (void)uStreamDispose(uStreamCloneInterface);
}

TEST_FUNCTION(ustream_read_complianceBoundaryConditionSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    ULIB_RESULT result = 
        ustream_read(
            uStreamInstance, 
            bufResult, 
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, bufResult, sizeResult);

    ASSERT_ARE_EQUAL(
            int,
            ULIB_EOF,
            ustream_read(
                uStreamInstance, 
                bufResult, 
                USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
                &sizeResult));

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(ustream_read_complianceLeftBoundaryConditionSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    ULIB_RESULT result = 
        ustream_read(
            uStreamInstance, 
            bufResult, 
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 1, 
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, bufResult, sizeResult);

    ASSERT_ARE_EQUAL(
            int,
            ULIB_EOF,
            ustream_read(
                uStreamInstance, 
                bufResult, 
                USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
                &sizeResult));

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(ustream_read_complianceSingleByteSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    ULIB_RESULT result = ustream_read(uStreamInstance, bufResult, 1, &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, 1, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, bufResult, sizeResult);

    checkBuffer(
        uStreamInstance, 
        1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided bufferLength is zero, the read shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(ustream_read_complianceBufferWithZeroSizeFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    ULIB_RESULT result = ustream_read(uStreamInstance, bufResult, 0, &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    checkBuffer(
        uStreamInstance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided handle is NULL, the read shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(ustream_read_complianceNullBufferFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    ULIB_RESULT result = 
        uStreamInstance->api->read(
            NULL, 
            bufResult, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    checkBuffer(
        uStreamInstance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided handle is not the implemented buffer type, the read shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(ustream_read_complianceNonTypeOfBufferAPIFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    ULIB_RESULT result = 
        uStreamInstance->api->read(
            uStreamMockCreate(), 
            bufResult, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    checkBuffer(
        uStreamInstance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided buffer is NULL, the read shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(ustream_read_complianceNullReturnBufferFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    size_t sizeResult;

    ///act
    ULIB_RESULT result = 
        ustream_read(
            uStreamInstance, 
            NULL, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    checkBuffer(
        uStreamInstance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided return size pointer is NULL, the read shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(ustream_read_complianceNullReturnSizeFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];

    ///act
    ULIB_RESULT result = 
        ustream_read(
            uStreamInstance, 
            bufResult, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    checkBuffer(
        uStreamInstance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* The set_position shall change the current position of the buffer. */
TEST_FUNCTION(ustream_set_position_complianceBackToBeginningSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(
            uStreamInstance, 
            bufResult, 
            USTREAM_COMPLIANCE_LENGTH_1, 
            &sizeResult));

    ///act
    ULIB_RESULT result = ustream_set_position(uStreamInstance, 0);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    checkBuffer(
        uStreamInstance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(ustream_set_position_complianceBackPositionSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(
            uStreamInstance, 
            bufResult, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult));

    ///act
    ULIB_RESULT result = 
        ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    checkBuffer(
        uStreamInstance, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(ustream_set_position_complianceForwardPositionSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamInstance, bufResult, USTREAM_COMPLIANCE_LENGTH_1, &sizeResult));

    ///act
    ULIB_RESULT result = ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_2);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    checkBuffer(
        uStreamInstance, 
        USTREAM_COMPLIANCE_LENGTH_2, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(ustream_set_position_complianceForwardToTheEndPositionSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    ULIB_RESULT result = 
        ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    checkBuffer(
        uStreamInstance, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(ustream_set_position_complianceRunFullBufferByteByByteSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[1];
    size_t sizeResult;

    for(offset_t i = 0; i < USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH; i++)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_set_position(uStreamInstance, i));
        
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS,
                ustream_read(uStreamInstance, bufResult, 1, &sizeResult));

        ASSERT_ARE_EQUAL(int, 1, sizeResult);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + i),
                *bufResult);
    }
    checkBuffer(
        uStreamInstance, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(ustream_set_position_complianceRunFullBufferByteByByteReverseOrderSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[1];
    size_t sizeResult;

    for(offset_t i = USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1; i > 0; i--)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_set_position(uStreamInstance, i));
        
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS,
                ustream_read(uStreamInstance, bufResult, 1, &sizeResult));

        ASSERT_ARE_EQUAL(int, 1, sizeResult);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + i),
                *bufResult);
    }
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_set_position(uStreamInstance, 0));
    checkBuffer(
        uStreamInstance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(ustream_set_position_complianceClonedBufferBackToBeginningSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1));
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);
    (void)uStreamDispose(uStreamInstance);
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamCloneInterface, bufResult, USTREAM_COMPLIANCE_LENGTH_1, &sizeResult));

    ///act
    ULIB_RESULT result = ustream_set_position(uStreamCloneInterface, 0);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    checkBuffer(
        uStreamCloneInterface, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamCloneInterface);
}

TEST_FUNCTION(ustream_set_position_complianceClonedBufferBackPositionSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1));
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);
    (void)uStreamDispose(uStreamInstance);
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(
            uStreamCloneInterface, 
            bufResult, 
            USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult));

    ///act
    ULIB_RESULT result = 
        ustream_set_position(uStreamCloneInterface, USTREAM_COMPLIANCE_LENGTH_1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    checkBuffer(
        uStreamCloneInterface, 
        USTREAM_COMPLIANCE_LENGTH_2, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamCloneInterface);
}

TEST_FUNCTION(ustream_set_position_complianceClonedBufferForwardPositionSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1));
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);
    (void)uStreamDispose(uStreamInstance);
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamCloneInterface, bufResult, USTREAM_COMPLIANCE_LENGTH_1, &sizeResult));

    ///act
    ULIB_RESULT result = ustream_set_position(uStreamCloneInterface, USTREAM_COMPLIANCE_LENGTH_2);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    checkBuffer(
        uStreamCloneInterface, 
        USTREAM_COMPLIANCE_LENGTH_3, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamCloneInterface);
}

TEST_FUNCTION(ustream_set_position_complianceClonedBufferForwardToTheEndPositionSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1));
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 100);
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);
    (void)uStreamDispose(uStreamInstance);

    ///act
    ULIB_RESULT result = 
        ustream_set_position(
            uStreamCloneInterface, 
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1 + 100);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    checkBuffer(
        uStreamCloneInterface, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamCloneInterface);
}

TEST_FUNCTION(ustream_set_position_complianceClonedBufferRunFullBufferByteByByteSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1));
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);
    (void)uStreamDispose(uStreamInstance);
    uint8_t bufResult[1];
    size_t sizeResult;

    for(offset_t i = 0; i < USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1; i++)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_set_position(uStreamCloneInterface, i));
        
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS,
                ustream_read(uStreamCloneInterface, bufResult, 1, &sizeResult));

        ASSERT_ARE_EQUAL(int, 1, sizeResult);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_1 + i),
                *bufResult);
    }
    checkBuffer(
        uStreamCloneInterface, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamCloneInterface);
}

TEST_FUNCTION(ustream_set_position_complianceClonedBufferRunFullBufferByteByByteReverseOrderSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1));
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);
    (void)uStreamDispose(uStreamInstance);
    uint8_t bufResult[1];
    size_t sizeResult;

    for(offset_t i = USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1 - 1; i > 0; i--)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_set_position(uStreamCloneInterface, i));
        
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS,
                ustream_read(uStreamCloneInterface, bufResult, 1, &sizeResult));

        ASSERT_ARE_EQUAL(int, 1, sizeResult);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_1 + i),
                *bufResult);
    }
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(uStreamCloneInterface, 0));
    checkBuffer(
        uStreamCloneInterface, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamCloneInterface);
}

/* If the provided position is out of the range of the buffer, the set_position shall return ULIB_NO_SUCH_ELEMENT_ERROR, and do not change the current position. */
TEST_FUNCTION(ustream_set_position_complianceForwardOutOfTheBufferFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    ULIB_RESULT result = 
        ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_NO_SUCH_ELEMENT_ERROR, result);
    checkBuffer(
        uStreamInstance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided position is before the first valid position, the set_position shall return ULIB_NO_SUCH_ELEMENT_ERROR, and do not change the current position. */
TEST_FUNCTION(ustream_set_position_complianceBackBeforeFirstValidPositionFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamInstance, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamRelease(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1 - 1));

    ///act
    ULIB_RESULT result = ustream_set_position(uStreamInstance, 0);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_NO_SUCH_ELEMENT_ERROR, result);
    checkBuffer(
        uStreamInstance, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided handle is NULL, the set_position shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(ustream_set_position_complianceNullBufferFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    ULIB_RESULT result = uStreamInstance->api->set_position(NULL, 1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided handle is not the implemented buffer type, the set_position shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(ustream_set_position_complianceNonTypeOfBufferAPIFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    ULIB_RESULT result = 
        uStreamInstance->api->set_position(uStreamMockCreate(), 1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* The release shall do nothing for uStreamInstance. */
TEST_FUNCTION(uStreamRelease_complianceSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamInstance, bufResult, USTREAM_COMPLIANCE_LENGTH_1, &sizeResult));
    offset_t currentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamInstance, &currentPosition));

    ///act
    ULIB_RESULT result = uStreamRelease(uStreamInstance, currentPosition - 1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    checkBuffer(
        uStreamInstance, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(uStreamRelease_complianceReleaseAllSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH));

    ///act
    ULIB_RESULT result = 
        uStreamRelease(uStreamInstance, USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_reset(uStreamInstance));
    checkBuffer(
        uStreamInstance, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(uStreamRelease_complianceRunFullBufferByteByByteSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[1];
    size_t sizeResult;

    for(offset_t i = 1; i < USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1; i++)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_set_position(uStreamInstance, i + 1));
    
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, uStreamRelease(uStreamInstance, i - 1));
        
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_reset(uStreamInstance));
    
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS,
                ustream_read(uStreamInstance, bufResult, 1, &sizeResult));

        ASSERT_ARE_EQUAL(int, 1, sizeResult);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + i),
                *bufResult);
    }
    checkBuffer(
        uStreamInstance, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(uStreamRelease_complianceClonedBufferSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1));
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);
    (void)uStreamDispose(uStreamInstance);
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamCloneInterface, bufResult, USTREAM_COMPLIANCE_LENGTH_1, &sizeResult));
    offset_t currentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamCloneInterface, &currentPosition));

    ///act
    ULIB_RESULT result = uStreamRelease(uStreamCloneInterface, currentPosition - 1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    checkBuffer(
        uStreamCloneInterface, 
        USTREAM_COMPLIANCE_LENGTH_2, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamCloneInterface);
}

TEST_FUNCTION(uStreamRelease_complianceClonedBufferReleaseAllSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1));
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 1000);
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);
    (void)uStreamDispose(uStreamInstance);
    ASSERT_ARE_EQUAL(
            int, 
            ULIB_SUCCESS, 
            ustream_set_position(
                uStreamCloneInterface, 
                USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1 + 1000));

    ///act
    ULIB_RESULT result = 
        uStreamRelease(
            uStreamCloneInterface, 
            USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1 + 999);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_reset(uStreamCloneInterface));
    checkBuffer(
        uStreamCloneInterface, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamCloneInterface);
}

TEST_FUNCTION(uStreamRelease_complianceClonedBufferRunFullBufferByteByByteSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_set_position(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1));
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);
    (void)uStreamDispose(uStreamInstance);
    uint8_t bufResult[1];
    size_t sizeResult;

    for(offset_t i = 1; i < USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAM_COMPLIANCE_LENGTH_1 - 1; i++)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_set_position(uStreamCloneInterface, i + 1));
    
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, uStreamRelease(uStreamCloneInterface, i - 1));
        
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_reset(uStreamCloneInterface));
    
        ASSERT_ARE_EQUAL(int, ULIB_SUCCESS,
                ustream_read(uStreamCloneInterface, bufResult, 1, &sizeResult));

        ASSERT_ARE_EQUAL(int, 1, sizeResult);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAM_COMPLIANCE_LENGTH_1 + i),
                *bufResult);
    }
    checkBuffer(
        uStreamCloneInterface, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamCloneInterface);
}

/* If the release position is after the current position, the release shall return ULIB_NO_SUCH_ELEMENT_ERROR, and do not release any resource. */
TEST_FUNCTION(uStreamRelease_complianceReleaseAfterCurrentFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamInstance, bufResult, USTREAM_COMPLIANCE_LENGTH_1, &sizeResult));
    offset_t currentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamGetCurrentPosition(uStreamInstance, &currentPosition));

    ///act
    ULIB_RESULT result = uStreamRelease(uStreamInstance, currentPosition);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    checkBuffer(
        uStreamInstance, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the release position is already released, the release shall return ULIB_ILLEGAL_ARGUMENT_ERROR, and do not release any resource. */
TEST_FUNCTION(uStreamRelease_complianceReleasePositionAlreayReleasedFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamInstance, bufResult, USTREAM_COMPLIANCE_LENGTH_2, &sizeResult));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamRelease(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_2 - 1));

    ///act
    ULIB_RESULT result = 
        uStreamRelease(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, ustream_reset(uStreamInstance));
    checkBuffer(
        uStreamInstance, 
        USTREAM_COMPLIANCE_LENGTH_2, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided handle is NULL, the release shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(uStreamRelease_complianceNullBufferFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    ULIB_RESULT result1 = uStreamInstance->api->release(NULL, 0);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result1);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided handle is not the implemented buffer type, the release shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(uStreamRelease_complianceNonTypeOfBufferAPIFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    ULIB_RESULT result1 = 
        uStreamInstance->api->release(uStreamMockCreate(), 0);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result1);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* The set_position shall change the current position of the buffer. */
TEST_FUNCTION(uStreamReset_complianceBackToBeginningSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamInstance, bufResult, 5, &sizeResult));

    ///act
    ULIB_RESULT result = ustream_reset(uStreamInstance);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    checkBuffer(
        uStreamInstance, 
        0, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(uStreamReset_complianceBackPositionSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamInstance, bufResult, USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH, &sizeResult));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamRelease(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1 - 1));

    ///act
    ULIB_RESULT result = ustream_reset(uStreamInstance);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    checkBuffer(
        uStreamInstance, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

TEST_FUNCTION(uStreamReset_complianceClonedBufferSucceed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamInstance);
    uint8_t buf[USTREAM_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamInstance, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamRelease(uStreamInstance, USTREAM_COMPLIANCE_LENGTH_1 - 1));
    USTREAM* uStreamCloneInterface = uStreamClone(uStreamInstance, 100);
    ASSERT_IS_NOT_NULL(uStreamCloneInterface);
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        ustream_read(uStreamCloneInterface, buf, USTREAM_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        ULIB_SUCCESS, 
        uStreamRelease(uStreamCloneInterface, 100 + USTREAM_COMPLIANCE_LENGTH_1 - 1));

    ///act
    ULIB_RESULT result = ustream_reset(uStreamCloneInterface);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_SUCCESS, result);
    checkBuffer(
        uStreamCloneInterface, 
        USTREAM_COMPLIANCE_LENGTH_2, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    checkBuffer(
        uStreamInstance, 
        USTREAM_COMPLIANCE_LENGTH_1, 
        USTREAM_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAM_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
    (void)uStreamDispose(uStreamCloneInterface);
}

/* If the provided handle is NULL, the set_position shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(uStreamReset_complianceNullBufferFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    ULIB_RESULT result = uStreamInstance->api->reset(NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

/* If the provided handle is not the implemented buffer type, the set_position shall return ULIB_ILLEGAL_ARGUMENT_ERROR. */
TEST_FUNCTION(uStreamReset_complianceNonTypeOfBufferAPIFailed)
{
    ///arrange
    USTREAM* uStreamInstance = USTREAM_COMPLIANCE_TARGET_FACTORY;

    ///act
    ULIB_RESULT result = uStreamInstance->api->reset(uStreamMockCreate());

    ///assert
    ASSERT_ARE_EQUAL(int, ULIB_ILLEGAL_ARGUMENT_ERROR, result);

    ///cleanup
    (void)uStreamDispose(uStreamInstance);
}

#endif /* AZURE_ULIB_C_TESTS_INC_USTREAM_COMPLIANCE_UT_H_ */
