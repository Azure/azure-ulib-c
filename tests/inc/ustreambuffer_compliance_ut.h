// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_TESTS_INC_USTREAMBUFFER_COMPLIANCE_UT_H_
#define AZURE_ULIB_C_TESTS_INC_USTREAMBUFFER_COMPLIANCE_UT_H_

#include "ustreambuffer_mock_buffer.h"
#include "ustreambuffer_ctest_aux.h"

/* check for test artifacts. */
#ifndef USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH
#error "USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH not defined"
#endif

#ifndef USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY
#error "USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY not defined"
#endif

/* split the content in 4 parts. */
#define USTREAMBUFFER_COMPLIANCE_LENGTH_1           (USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH >> 2)
#define USTREAMBUFFER_COMPLIANCE_LENGTH_2           (USTREAMBUFFER_COMPLIANCE_LENGTH_1 + USTREAMBUFFER_COMPLIANCE_LENGTH_1)
#define USTREAMBUFFER_COMPLIANCE_LENGTH_3           (USTREAMBUFFER_COMPLIANCE_LENGTH_2 + USTREAMBUFFER_COMPLIANCE_LENGTH_1)

/* create local buffer with enough size to handle the full content. */
#define USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH (USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 2)

/*
 * Start compliance tests:
 */

/* The dispose shall free all allocated resources for the instance of the buffer. */
TEST_FUNCTION(uStreamBufferDispose_complianceClonedInstanceDisposedFirstSucceed)
{
    ///arrange
    uint32_t uStreamBufferCloneCurrentPosition;
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
        
    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferDispose(uStreamBufferCloneInterface);
        
    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferInstance, &uStreamBufferCloneCurrentPosition));
        
    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferDispose_complianceClonedInstanceDisposedSecondSucceed)
{
    ///arrange
    uint32_t uStreamBufferCloneCurrentPosition;
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
        
    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferDispose(uStreamBufferInstance);
        
    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferCloneInterface, &uStreamBufferCloneCurrentPosition));
        
    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

/* If there is no more instances of the buffer, the dispose shall release all allocated
 *          resources to control the buffer. */
TEST_FUNCTION(uStreamBufferDispose_complianceSingleInstanceSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    
    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferDispose(uStreamBufferInstance);
    
    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    
    ///cleanup
}

/* If the provided handle is NULL, the dispose shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferDispose_complianceNullBufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    
    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferInstance->api->dispose(NULL);
    
    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);
    
    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided handle is not the implemented buffer type, the dispose shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferDispose_complianceBufferIsNotTypeOfBufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    
    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferInstance->api->dispose(uStreamBufferMockCreate());
    
    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);
    
    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* The clone shall return a buffer with the same content of the original buffer. */
/* The clone shall start the buffer on the current position of the original buffer. */
/* The clone shall move the current position to the start position. */
/* The clone shall move the released position to the one immediately before the current position. */
/* The cloned buffer shall not interfere in the instance of the original buffer and vice versa. */
TEST_FUNCTION(uStreamBufferClone_complianceNewBufferClonedWithZeroOffsetSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);

    ///act
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 0);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);

    /* current position was moved to the start of the buffer. */
    uint32_t uStreamBufferCloneCurrentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferCloneInterface, &uStreamBufferCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 0, uStreamBufferCloneCurrentPosition);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferReset(uStreamBufferCloneInterface));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferCloneInterface, &uStreamBufferCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 0, uStreamBufferCloneCurrentPosition);

    /* Full content was copied. */
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(
            uStreamBufferCloneInterface, 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult));
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        bufResult, 
        sizeResult);

    /* Keep original buffer instance */
    checkBuffer(
        uStreamBufferInstance, 
        0, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

TEST_FUNCTION(uStreamBufferClone_complianceNewBufferClonedWithOffsetSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);

    ///act
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 10000);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);

    /* current position was moved to the start of the buffer with the offset. */
    uint32_t uStreamBufferCloneCurrentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferCloneInterface, &uStreamBufferCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 10000, uStreamBufferCloneCurrentPosition);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferReset(uStreamBufferCloneInterface));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferCloneInterface, &uStreamBufferCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 10000, uStreamBufferCloneCurrentPosition);

    /* Full content was copied. */
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(
            uStreamBufferCloneInterface, 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult));
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        bufResult, 
        sizeResult);

    /* Keep original buffer instance */
    checkBuffer(
        uStreamBufferInstance, 
        0, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

TEST_FUNCTION(uStreamBufferClone_complianceEmptyBufferSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1));

    ///act
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 0);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);

    /* current position was moved to the start of the buffer. */
    uint32_t uStreamBufferCloneCurrentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferCloneInterface, &uStreamBufferCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 0, uStreamBufferCloneCurrentPosition);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferReset(uStreamBufferCloneInterface));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferCloneInterface, &uStreamBufferCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 0, uStreamBufferCloneCurrentPosition);

    /* Full content was copied. */
    checkBuffer(
        uStreamBufferCloneInterface, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    /* Keep original buffer instance */
    checkBuffer(
        uStreamBufferInstance, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

TEST_FUNCTION(uStreamBufferClone_complianceNewBufferWithNonZeroCurrentAndReleasedPositionsClonedWithOffsetSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    uint8_t buf[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferInstance, buf, USTREAMBUFFER_COMPLIANCE_LENGTH_2, &size));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1));

    ///act
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = 
        uStreamBufferClone(uStreamBufferInstance, 100);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);

    /* current position was moved to the start of the buffer with the offset. */
    uint32_t uStreamBufferCloneCurrentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferCloneInterface, &uStreamBufferCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 100, uStreamBufferCloneCurrentPosition);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferReset(uStreamBufferCloneInterface));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferCloneInterface, &uStreamBufferCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 100, uStreamBufferCloneCurrentPosition);

    /* Full content was copied. */
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(
            uStreamBufferCloneInterface, 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAMBUFFER_COMPLIANCE_LENGTH_2, 
        sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        (uint8_t*)(USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAMBUFFER_COMPLIANCE_LENGTH_2), 
        bufResult, 
        sizeResult);

    /* Keep original buffer instance */
    checkBuffer(
        uStreamBufferInstance, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_2, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

TEST_FUNCTION(uStreamBufferClone_complianceNewBufferWithNonZeroCurrentAndReleasedPositionsClonedWithNegativeOffsetSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    uint8_t buf[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferInstance, buf, USTREAMBUFFER_COMPLIANCE_LENGTH_2, &size));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1));

    ///act
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = 
        uStreamBufferClone(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);

    /* current position was moved to the start of the buffer with the offset. */
    uint32_t uStreamBufferCloneCurrentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferCloneInterface, &uStreamBufferCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_COMPLIANCE_LENGTH_1, uStreamBufferCloneCurrentPosition);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferReset(uStreamBufferCloneInterface));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferCloneInterface, &uStreamBufferCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_COMPLIANCE_LENGTH_1, uStreamBufferCloneCurrentPosition);

    /* Full content was copied. */
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(
            uStreamBufferCloneInterface, 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAMBUFFER_COMPLIANCE_LENGTH_2, 
        sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        (uint8_t*)(USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAMBUFFER_COMPLIANCE_LENGTH_2), 
        bufResult, 
        sizeResult);

    /* Keep original buffer instance */
    checkBuffer(
        uStreamBufferInstance, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_2, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

TEST_FUNCTION(uStreamBufferClone_complianceClonedBufferWithNonZeroCurrentAndReleasedPositionsClonedWithOffsetSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    uint8_t buf[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferInstance, buf, USTREAMBUFFER_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1 - 1));
    USTREAMBUFFER_INTERFACE uStreamBufferClone1 = uStreamBufferClone(uStreamBufferInstance, 100);
    ASSERT_IS_NOT_NULL(uStreamBufferClone1);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferClone1, buf, USTREAMBUFFER_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(uStreamBufferClone1, 100 + USTREAMBUFFER_COMPLIANCE_LENGTH_1 - 1));

    ///act
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferClone1, 0);

    ///assert
    /* clone succeed */
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);

    /* current position was moved to the start of the buffer with the offset. */
    uint32_t uStreamBufferCloneCurrentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferCloneInterface, &uStreamBufferCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 0, uStreamBufferCloneCurrentPosition);

    /* release position was moved to the start menus one. */
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferReset(uStreamBufferCloneInterface));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferCloneInterface, &uStreamBufferCloneCurrentPosition));
    ASSERT_ARE_EQUAL(int, 0, uStreamBufferCloneCurrentPosition);

    /* Full content was copied. */
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(
            uStreamBufferCloneInterface, 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAMBUFFER_COMPLIANCE_LENGTH_2, 
        sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        (uint8_t*)(USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAMBUFFER_COMPLIANCE_LENGTH_2), 
        bufResult, 
        sizeResult);

    /* Keep original buffer instance */
    checkBuffer(
        uStreamBufferInstance, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_1, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    checkBuffer(
        uStreamBufferClone1, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_2, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
    (void)uStreamBufferDispose(uStreamBufferClone1);
}

/* If the provided handle is NULL, the clone shall return NULL. */
TEST_FUNCTION(uStreamBufferClone_complianceNullBufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);

    ///act
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferInstance->api->clone(NULL, 0);

    ///assert
    ASSERT_IS_NULL(uStreamBufferCloneInterface);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided handle is not the implemented buffer type, the clone shall return NULL. */
TEST_FUNCTION(uStreamBufferClone_complianceBufferIsNotTypeOfBufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);

    ///act
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = 
        uStreamBufferInstance->api->clone(uStreamBufferMockCreate(), 0);

    ///assert
    ASSERT_IS_NULL(uStreamBufferCloneInterface);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the offset plus the buffer length bypass UINT32_MAX, the clone shall return NULL. */
TEST_FUNCTION(uStreamBufferClone_complianceOffsetExceedSizeFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    umock_c_reset_all_calls();

    ///act
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = 
        uStreamBufferClone(uStreamBufferInstance, UINT32_MAX - 2);

    ///assert
    ASSERT_IS_NULL(uStreamBufferCloneInterface);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* The getRemainingSize shall return the number of bytes between the current position and the end of the buffer. */
TEST_FUNCTION(uStreamBufferGetRemainingSize_complianceNewBufferSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    size_t size;

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferGetRemainingSize(uStreamBufferInstance, &size);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, size);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferGetRemainingSize_complianceNewBufferWithNonZeroCurrentPositionSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    uint8_t buf[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferInstance, buf, USTREAMBUFFER_COMPLIANCE_LENGTH_2, &size));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferGetRemainingSize(uStreamBufferInstance, &size);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAMBUFFER_COMPLIANCE_LENGTH_2, 
        size);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferGetRemainingSize_complianceClonedBufferWithNonZeroCurrentPositionSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    uint8_t buf[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferInstance, buf, USTREAMBUFFER_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1 - 1));
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 100);
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferCloneInterface, buf, USTREAMBUFFER_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(uStreamBufferCloneInterface, 100 + USTREAMBUFFER_COMPLIANCE_LENGTH_1 - 1));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferGetRemainingSize(uStreamBufferCloneInterface, &size);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAMBUFFER_COMPLIANCE_LENGTH_2, 
        size);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided handle is NULL, the getRemainingSize shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferGetRemainingSize_complianceNullBufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    size_t size;

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferInstance->api->getRemainingSize(NULL, &size);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided handle is not the implemented buffer type, the getRemainingSize shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferGetRemainingSize_complianceBufferIsNotTypeOfBufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    size_t size;

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferInstance->api->getRemainingSize(uStreamBufferMockCreate(), &size);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided size is NULL, the getRemainingSize shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferGetRemainingSize_complianceNullSizeFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferGetRemainingSize(uStreamBufferInstance, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* The getCurrentPosition shall return the logical current position of the buffer. */
TEST_FUNCTION(uStreamBufferGetCurrentPosition_complianceNewBufferSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    uint32_t position;

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferGetCurrentPosition(uStreamBufferInstance, &position);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, 0, position);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferGetCurrentPosition_complianceNewBufferWithNonZeroCurrentPositionSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    uint8_t buf[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    uint32_t position;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferInstance, buf, USTREAMBUFFER_COMPLIANCE_LENGTH_2, &size));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferGetCurrentPosition(uStreamBufferInstance, &position);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_COMPLIANCE_LENGTH_2, position);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferGetCurrentPosition_complianceClonedBufferWithNonZeroCurrentPositionSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    uint8_t buf[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    uint32_t position;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferInstance, buf, USTREAMBUFFER_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1 - 1));
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 100);
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferCloneInterface, buf, USTREAMBUFFER_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(uStreamBufferCloneInterface, 100 + USTREAMBUFFER_COMPLIANCE_LENGTH_1 - 1));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferGetCurrentPosition(uStreamBufferCloneInterface, &position);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, 100 + USTREAMBUFFER_COMPLIANCE_LENGTH_1, position);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided interface is NULL, the getCurrentPosition shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferGetCurrentPosition_complianceNullBufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint32_t position;

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferInstance->api->getCurrentPosition(NULL, &position);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided interface is not the implemented buffer type, the getCurrentPosition shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferGetCurrentPosition_complianceBufferIsNotTypeOfBufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint32_t position;

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferInstance->api->getCurrentPosition(uStreamBufferMockCreate(), &position);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided position is NULL, the getCurrentPosition shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferGetCurrentPosition_complianceNullPositionFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferGetCurrentPosition(uStreamBufferInstance, NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* [1]The getNext shall copy the content in the provided buffer and return the number of valid {@code uint8_t} values in the local buffer in the provided `size`. */
/* [2]If the length of the content is bigger than the `bufferLength`, the getNext shall limit the copy size to the bufferLength.*/
/* [3]If there is no more content to return, the getNext shall return AZ_RUNTIME_NO_SUCH_ELEMENT_EXCEPTION, size shall receive 0, and do not change the content of the local buffer. */
TEST_FUNCTION(uStreamBufferGetNext_complianceGetFromOriginalBufferSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult1[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    uint8_t bufResult2[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    uint8_t bufResult3[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult1;
    size_t sizeResult2;
    size_t sizeResult3;

    ///act
    USTREAMBUFFER_RESULT result1 = 
        uStreamBufferGetNext(
            uStreamBufferInstance, 
            bufResult1, 
            USTREAMBUFFER_COMPLIANCE_LENGTH_1, 
            &sizeResult1);
    USTREAMBUFFER_RESULT result2 = 
        uStreamBufferGetNext(
            uStreamBufferInstance, 
            bufResult2, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult2);
    USTREAMBUFFER_RESULT result3 = 
        uStreamBufferGetNext(
            uStreamBufferInstance, 
            bufResult3, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult3);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_COMPLIANCE_LENGTH_1, sizeResult1);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        bufResult1, 
        sizeResult1);

    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result2);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAMBUFFER_COMPLIANCE_LENGTH_1, 
        sizeResult2);
    ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr,
            (const uint8_t* const)(USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAMBUFFER_COMPLIANCE_LENGTH_1),
            bufResult2,
            sizeResult2);

    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION, result3);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferGetNext_complianceGetFromClonedBufferSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1));
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = 
        uStreamBufferClone(uStreamBufferInstance, 100);
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);
    (void)uStreamBufferDispose(uStreamBufferInstance);

    uint8_t bufResult1[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    uint8_t bufResult2[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    uint8_t bufResult3[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult1;
    size_t sizeResult2;
    size_t sizeResult3;

    ///act
    USTREAMBUFFER_RESULT result1 = 
        uStreamBufferGetNext(
            uStreamBufferCloneInterface, 
            bufResult1, 
            USTREAMBUFFER_COMPLIANCE_LENGTH_1, 
            &sizeResult1);
    USTREAMBUFFER_RESULT result2 = 
        uStreamBufferGetNext(
            uStreamBufferCloneInterface, 
            bufResult2, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult2);
    USTREAMBUFFER_RESULT result3 = 
        uStreamBufferGetNext(
            uStreamBufferCloneInterface, 
            bufResult3, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult3);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result1);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_COMPLIANCE_LENGTH_1, sizeResult1);
    ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr, 
            (const uint8_t* const)(USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAMBUFFER_COMPLIANCE_LENGTH_1),
            bufResult1, 
            sizeResult1);

    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result2);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAMBUFFER_COMPLIANCE_LENGTH_2, 
        sizeResult2);
    ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr,
            (const uint8_t* const)(USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAMBUFFER_COMPLIANCE_LENGTH_2),
            bufResult2,
            sizeResult2);

    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION, result3);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

TEST_FUNCTION(uStreamBufferGetNext_complianceSingleBufferSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferGetNext(
            uStreamBufferInstance, 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, bufResult, sizeResult);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferGetNext_complianceRightBoundaryConditionSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferGetNext(
            uStreamBufferInstance, 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1, 
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(
        uint8_t_ptr, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        bufResult, 
        sizeResult);

    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(
            uStreamBufferInstance, 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult));
    ASSERT_ARE_EQUAL(int, 1, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr,
            (const uint8_t* const)(USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1),
            bufResult,
            sizeResult);

    ASSERT_ARE_EQUAL(
            int,
            USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION,
            uStreamBufferGetNext(
                uStreamBufferInstance, 
                bufResult, 
                USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
                &sizeResult));

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferGetNext_complianceClonedBufferRightBoundaryConditionSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1));
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);
    (void)uStreamBufferDispose(uStreamBufferInstance);

    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferGetNext(
            uStreamBufferCloneInterface, 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, 1, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(
            uint8_t_ptr, 
            (const uint8_t* const)(USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1),
            bufResult, 
            sizeResult);

    ASSERT_ARE_EQUAL(
            int, 
            USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION, 
            uStreamBufferGetNext(
                uStreamBufferCloneInterface, 
                bufResult, 
                USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
                &sizeResult));

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

TEST_FUNCTION(uStreamBufferGetNext_complianceBoundaryConditionSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferGetNext(
            uStreamBufferInstance, 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, bufResult, sizeResult);

    ASSERT_ARE_EQUAL(
            int,
            USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION,
            uStreamBufferGetNext(
                uStreamBufferInstance, 
                bufResult, 
                USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
                &sizeResult));

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferGetNext_complianceLeftBoundaryConditionSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferGetNext(
            uStreamBufferInstance, 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 1, 
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, bufResult, sizeResult);

    ASSERT_ARE_EQUAL(
            int,
            USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION,
            uStreamBufferGetNext(
                uStreamBufferInstance, 
                bufResult, 
                USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
                &sizeResult));

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferGetNext_complianceSingleByteSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferGetNext(uStreamBufferInstance, bufResult, 1, &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, 1, sizeResult);
    ASSERT_BUFFER_ARE_EQUAL(uint8_t_ptr, USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, bufResult, sizeResult);

    checkBuffer(
        uStreamBufferInstance, 
        1, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided bufferLength is zero, the getNext shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferGetNext_complianceBufferWithZeroSizeFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferGetNext(uStreamBufferInstance, bufResult, 0, &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);
    checkBuffer(
        uStreamBufferInstance, 
        0, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided handle is NULL, the getNext shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferGetNext_complianceNullBufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferInstance->api->getNext(
            NULL, 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);
    checkBuffer(
        uStreamBufferInstance, 
        0, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided handle is not the implemented buffer type, the getNext shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferGetNext_complianceNonTypeOfBufferAPIFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferInstance->api->getNext(
            uStreamBufferMockCreate(), 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);
    checkBuffer(
        uStreamBufferInstance, 
        0, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided buffer is NULL, the getNext shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferGetNext_complianceNullReturnBufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    size_t sizeResult;

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferGetNext(
            uStreamBufferInstance, 
            NULL, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);
    checkBuffer(
        uStreamBufferInstance, 
        0, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided return size pointer is NULL, the getNext shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferGetNext_complianceNullReturnSizeFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferGetNext(
            uStreamBufferInstance, 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);
    checkBuffer(
        uStreamBufferInstance, 
        0, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* The seek shall change the current position of the buffer. */
TEST_FUNCTION(uStreamBufferSeek_complianceBackToBeginningSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(
            uStreamBufferInstance, 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_LENGTH_1, 
            &sizeResult));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferSeek(uStreamBufferInstance, 0);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    checkBuffer(
        uStreamBufferInstance, 
        0, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferSeek_complianceBackPositionSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(
            uStreamBufferInstance, 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult));

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    checkBuffer(
        uStreamBufferInstance, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_1, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferSeek_complianceForwardPositionSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferInstance, bufResult, USTREAMBUFFER_COMPLIANCE_LENGTH_1, &sizeResult));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_2);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    checkBuffer(
        uStreamBufferInstance, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_2, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferSeek_complianceForwardToTheEndPositionSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    checkBuffer(
        uStreamBufferInstance, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferSeek_complianceRunFullBufferByteByByteSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[1];
    size_t sizeResult;

    for(uint32_t i = 0; i < USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH; i++)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferSeek(uStreamBufferInstance, i));
        
        ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS,
                uStreamBufferGetNext(uStreamBufferInstance, bufResult, 1, &sizeResult));

        ASSERT_ARE_EQUAL(int, 1, sizeResult);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT + i),
                *bufResult);
    }
    checkBuffer(
        uStreamBufferInstance, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferSeek_complianceRunFullBufferByteByByteReverseOrderSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[1];
    size_t sizeResult;

    for(uint32_t i = USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1; i > 0; i--)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferSeek(uStreamBufferInstance, i));
        
        ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS,
                uStreamBufferGetNext(uStreamBufferInstance, bufResult, 1, &sizeResult));

        ASSERT_ARE_EQUAL(int, 1, sizeResult);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT + i),
                *bufResult);
    }
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferSeek(uStreamBufferInstance, 0));
    checkBuffer(
        uStreamBufferInstance, 
        0, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferSeek_complianceClonedBufferBackToBeginningSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1));
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);
    (void)uStreamBufferDispose(uStreamBufferInstance);
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferCloneInterface, bufResult, USTREAMBUFFER_COMPLIANCE_LENGTH_1, &sizeResult));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferSeek(uStreamBufferCloneInterface, 0);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    checkBuffer(
        uStreamBufferCloneInterface, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_1, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

TEST_FUNCTION(uStreamBufferSeek_complianceClonedBufferBackPositionSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1));
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);
    (void)uStreamBufferDispose(uStreamBufferInstance);
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(
            uStreamBufferCloneInterface, 
            bufResult, 
            USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, 
            &sizeResult));

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferSeek(uStreamBufferCloneInterface, USTREAMBUFFER_COMPLIANCE_LENGTH_1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    checkBuffer(
        uStreamBufferCloneInterface, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_2, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

TEST_FUNCTION(uStreamBufferSeek_complianceClonedBufferForwardPositionSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1));
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);
    (void)uStreamBufferDispose(uStreamBufferInstance);
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferCloneInterface, bufResult, USTREAMBUFFER_COMPLIANCE_LENGTH_1, &sizeResult));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferSeek(uStreamBufferCloneInterface, USTREAMBUFFER_COMPLIANCE_LENGTH_2);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    checkBuffer(
        uStreamBufferCloneInterface, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_3, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

TEST_FUNCTION(uStreamBufferSeek_complianceClonedBufferForwardToTheEndPositionSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1));
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 100);
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);
    (void)uStreamBufferDispose(uStreamBufferInstance);

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferSeek(
            uStreamBufferCloneInterface, 
            USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAMBUFFER_COMPLIANCE_LENGTH_1 + 100);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    checkBuffer(
        uStreamBufferCloneInterface, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

TEST_FUNCTION(uStreamBufferSeek_complianceClonedBufferRunFullBufferByteByByteSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1));
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);
    (void)uStreamBufferDispose(uStreamBufferInstance);
    uint8_t bufResult[1];
    size_t sizeResult;

    for(uint32_t i = 0; i < USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAMBUFFER_COMPLIANCE_LENGTH_1; i++)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferSeek(uStreamBufferCloneInterface, i));
        
        ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS,
                uStreamBufferGetNext(uStreamBufferCloneInterface, bufResult, 1, &sizeResult));

        ASSERT_ARE_EQUAL(int, 1, sizeResult);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAMBUFFER_COMPLIANCE_LENGTH_1 + i),
                *bufResult);
    }
    checkBuffer(
        uStreamBufferCloneInterface, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

TEST_FUNCTION(uStreamBufferSeek_complianceClonedBufferRunFullBufferByteByByteReverseOrderSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1));
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);
    (void)uStreamBufferDispose(uStreamBufferInstance);
    uint8_t bufResult[1];
    size_t sizeResult;

    for(uint32_t i = USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAMBUFFER_COMPLIANCE_LENGTH_1 - 1; i > 0; i--)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferSeek(uStreamBufferCloneInterface, i));
        
        ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS,
                uStreamBufferGetNext(uStreamBufferCloneInterface, bufResult, 1, &sizeResult));

        ASSERT_ARE_EQUAL(int, 1, sizeResult);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAMBUFFER_COMPLIANCE_LENGTH_1 + i),
                *bufResult);
    }
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(uStreamBufferCloneInterface, 0));
    checkBuffer(
        uStreamBufferCloneInterface, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_1, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

/* If the provided position is out of the range of the buffer, the seek shall return AZ_RUNTIME_NO_SUCH_ELEMENT_EXCEPTION, and do not change the current position. */
TEST_FUNCTION(uStreamBufferSeek_complianceForwardOutOfTheBufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH + 1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION, result);
    checkBuffer(
        uStreamBufferInstance, 
        0, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided position is before the first valid position, the seek shall return USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION, and do not change the current position. */
TEST_FUNCTION(uStreamBufferSeek_complianceBackBeforeFirstValidPositionFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    uint8_t buf[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferInstance, buf, USTREAMBUFFER_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1 - 1));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferSeek(uStreamBufferInstance, 0);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION, result);
    checkBuffer(
        uStreamBufferInstance, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_1, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided handle is NULL, the seek shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferSeek_complianceNullBufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferInstance->api->seek(NULL, 1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided handle is not the implemented buffer type, the seek shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferSeek_complianceNonTypeOfBufferAPIFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferInstance->api->seek(uStreamBufferMockCreate(), 1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* The release shall do nothing for uStreamBufferInstance. */
TEST_FUNCTION(uStreamBufferRelease_complianceSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferInstance, bufResult, USTREAMBUFFER_COMPLIANCE_LENGTH_1, &sizeResult));
    uint32_t currentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferInstance, &currentPosition));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferRelease(uStreamBufferInstance, currentPosition - 1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    checkBuffer(
        uStreamBufferInstance, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_1, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferRelease_complianceReleaseAllSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH));

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferRelease(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferReset(uStreamBufferInstance));
    checkBuffer(
        uStreamBufferInstance, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferRelease_complianceRunFullBufferByteByByteSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[1];
    size_t sizeResult;

    for(uint32_t i = 1; i < USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1; i++)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferSeek(uStreamBufferInstance, i + 1));
    
        ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferRelease(uStreamBufferInstance, i - 1));
        
        ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferReset(uStreamBufferInstance));
    
        ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS,
                uStreamBufferGetNext(uStreamBufferInstance, bufResult, 1, &sizeResult));

        ASSERT_ARE_EQUAL(int, 1, sizeResult);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT + i),
                *bufResult);
    }
    checkBuffer(
        uStreamBufferInstance, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferRelease_complianceClonedBufferSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1));
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);
    (void)uStreamBufferDispose(uStreamBufferInstance);
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferCloneInterface, bufResult, USTREAMBUFFER_COMPLIANCE_LENGTH_1, &sizeResult));
    uint32_t currentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferCloneInterface, &currentPosition));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferRelease(uStreamBufferCloneInterface, currentPosition - 1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    checkBuffer(
        uStreamBufferCloneInterface, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_2, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

TEST_FUNCTION(uStreamBufferRelease_complianceClonedBufferReleaseAllSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1));
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 1000);
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);
    (void)uStreamBufferDispose(uStreamBufferInstance);
    ASSERT_ARE_EQUAL(
            int, 
            USTREAMBUFFER_SUCCESS, 
            uStreamBufferSeek(
                uStreamBufferCloneInterface, 
                USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAMBUFFER_COMPLIANCE_LENGTH_1 + 1000));

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferRelease(
            uStreamBufferCloneInterface, 
            USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAMBUFFER_COMPLIANCE_LENGTH_1 + 999);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferReset(uStreamBufferCloneInterface));
    checkBuffer(
        uStreamBufferCloneInterface, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

TEST_FUNCTION(uStreamBufferRelease_complianceClonedBufferRunFullBufferByteByByteSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferSeek(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1));
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 0);
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);
    (void)uStreamBufferDispose(uStreamBufferInstance);
    uint8_t bufResult[1];
    size_t sizeResult;

    for(uint32_t i = 1; i < USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - USTREAMBUFFER_COMPLIANCE_LENGTH_1 - 1; i++)
    {
        ///act - assert
        ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferSeek(uStreamBufferCloneInterface, i + 1));
    
        ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferRelease(uStreamBufferCloneInterface, i - 1));
        
        ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferReset(uStreamBufferCloneInterface));
    
        ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS,
                uStreamBufferGetNext(uStreamBufferCloneInterface, bufResult, 1, &sizeResult));

        ASSERT_ARE_EQUAL(int, 1, sizeResult);
        ASSERT_ARE_EQUAL(
                uint8_t,
                *(const uint8_t* const)(USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT + USTREAMBUFFER_COMPLIANCE_LENGTH_1 + i),
                *bufResult);
    }
    checkBuffer(
        uStreamBufferCloneInterface, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH - 1, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

/* If the release position is after the current position, the release shall return AZ_RUNTIME_ILLEGAL_ARGUMENT_EXCEPTION, and do not release any resource. */
TEST_FUNCTION(uStreamBufferRelease_complianceReleaseAfterCurrentFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferInstance, bufResult, USTREAMBUFFER_COMPLIANCE_LENGTH_1, &sizeResult));
    uint32_t currentPosition;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetCurrentPosition(uStreamBufferInstance, &currentPosition));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferRelease(uStreamBufferInstance, currentPosition);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);
    checkBuffer(
        uStreamBufferInstance, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_1, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the release position is already released, the release shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, and do not release any resource. */
TEST_FUNCTION(uStreamBufferRelease_complianceReleasePositionAlreayReleasedFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferInstance, bufResult, USTREAMBUFFER_COMPLIANCE_LENGTH_2, &sizeResult));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_2 - 1));

    ///act
    USTREAMBUFFER_RESULT result = 
        uStreamBufferRelease(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, uStreamBufferReset(uStreamBufferInstance));
    checkBuffer(
        uStreamBufferInstance, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_2, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided handle is NULL, the release shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferRelease_complianceNullBufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;

    ///act
    USTREAMBUFFER_RESULT result1 = uStreamBufferInstance->api->release(NULL, 0);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result1);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided handle is not the implemented buffer type, the release shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferRelease_complianceNonTypeOfBufferAPIFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;

    ///act
    USTREAMBUFFER_RESULT result1 = 
        uStreamBufferInstance->api->release(uStreamBufferMockCreate(), 0);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result1);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* The seek shall change the current position of the buffer. */
TEST_FUNCTION(uStreamBufferReset_complianceBackToBeginningSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferInstance, bufResult, 5, &sizeResult));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferReset(uStreamBufferInstance);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    checkBuffer(
        uStreamBufferInstance, 
        0, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferReset_complianceBackPositionSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    uint8_t bufResult[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t sizeResult;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferInstance, bufResult, USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH, &sizeResult));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1 - 1));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferReset(uStreamBufferInstance);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    checkBuffer(
        uStreamBufferInstance, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_1, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

TEST_FUNCTION(uStreamBufferReset_complianceClonedBufferSucceed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;
    ASSERT_IS_NOT_NULL(uStreamBufferInstance);
    uint8_t buf[USTREAMBUFFER_COMPLIANCE_TEMP_BUFFER_LENGTH];
    size_t size;
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferInstance, buf, USTREAMBUFFER_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(uStreamBufferInstance, USTREAMBUFFER_COMPLIANCE_LENGTH_1 - 1));
    USTREAMBUFFER_INTERFACE uStreamBufferCloneInterface = uStreamBufferClone(uStreamBufferInstance, 100);
    ASSERT_IS_NOT_NULL(uStreamBufferCloneInterface);
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferGetNext(uStreamBufferCloneInterface, buf, USTREAMBUFFER_COMPLIANCE_LENGTH_1, &size));
    ASSERT_ARE_EQUAL(
        int, 
        USTREAMBUFFER_SUCCESS, 
        uStreamBufferRelease(uStreamBufferCloneInterface, 100 + USTREAMBUFFER_COMPLIANCE_LENGTH_1 - 1));

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferReset(uStreamBufferCloneInterface);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_SUCCESS, result);
    checkBuffer(
        uStreamBufferCloneInterface, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_2, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);
    checkBuffer(
        uStreamBufferInstance, 
        USTREAMBUFFER_COMPLIANCE_LENGTH_1, 
        USTREAMBUFFER_COMPLIANCE_LOCAL_EXPECTED_CONTENT, 
        USTREAMBUFFER_COMPLIANCE_EXPECTED_CONTENT_LENGTH);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
    (void)uStreamBufferDispose(uStreamBufferCloneInterface);
}

/* If the provided handle is NULL, the seek shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferReset_complianceNullBufferFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferInstance->api->reset(NULL);

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

/* If the provided handle is not the implemented buffer type, the seek shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION. */
TEST_FUNCTION(uStreamBufferReset_complianceNonTypeOfBufferAPIFailed)
{
    ///arrange
    USTREAMBUFFER_INTERFACE uStreamBufferInstance = USTREAMBUFFER_COMPLIANCE_TARGET_FACTORY;

    ///act
    USTREAMBUFFER_RESULT result = uStreamBufferInstance->api->reset(uStreamBufferMockCreate());

    ///assert
    ASSERT_ARE_EQUAL(int, USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, result);

    ///cleanup
    (void)uStreamBufferDispose(uStreamBufferInstance);
}

#endif /* AZURE_ULIB_C_TESTS_INC_USTREAMBUFFER_COMPLIANCE_UT_H_ */
