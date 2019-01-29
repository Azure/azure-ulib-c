// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ustreambuffer_mock_buffer.h"
#include "ustreambuffer_base.h"

static USTREAMBUFFER_RESULT _concreteSeekResult = USTREAMBUFFER_SUCCESS;
static USTREAMBUFFER_RESULT _concreteResetResult = USTREAMBUFFER_SUCCESS;
static USTREAMBUFFER_RESULT _concreteGetNextResult = USTREAMBUFFER_SUCCESS;
static USTREAMBUFFER_RESULT _concreteGetRemainingSizeResult = USTREAMBUFFER_SUCCESS;
static USTREAMBUFFER_RESULT _concreteGetCurrentPositionResult = USTREAMBUFFER_SUCCESS;
static USTREAMBUFFER_RESULT _concreteReleaseResult = USTREAMBUFFER_SUCCESS;
static USTREAMBUFFER_RESULT _concreteCloneResult = USTREAMBUFFER_SUCCESS;
static USTREAMBUFFER_RESULT _concreteDisposeResult = USTREAMBUFFER_SUCCESS;

static offset_t currentPosition = 0;

static USTREAMBUFFER_RESULT concreteSeek(
        USTREAMBUFFER_INTERFACE uStreamBufferInterface, 
        offset_t position)
{
    (void)uStreamBufferInterface;

    currentPosition = position;

    USTREAMBUFFER_RESULT resutl = _concreteSeekResult;
    _concreteSeekResult = USTREAMBUFFER_SUCCESS;
    return resutl;
}

static USTREAMBUFFER_RESULT concreteReset(
        USTREAMBUFFER_INTERFACE uStreamBufferInterface)
{
    (void)uStreamBufferInterface;

    USTREAMBUFFER_RESULT resutl = _concreteResetResult;
    _concreteResetResult = USTREAMBUFFER_SUCCESS;
    return resutl;
}

static USTREAMBUFFER_RESULT concreteGetNext(
        USTREAMBUFFER_INTERFACE uStreamBufferInterface, 
        uint8_t* const buffer, 
        size_t bufferLength, 
        size_t* const size)
{
    (void)uStreamBufferInterface;
    (void)buffer;
    (void)size;

    currentPosition += bufferLength;

    USTREAMBUFFER_RESULT resutl = _concreteGetNextResult;
    _concreteGetNextResult = USTREAMBUFFER_SUCCESS;
    return resutl;
}

static USTREAMBUFFER_RESULT concreteGetRemainingSize(
        USTREAMBUFFER_INTERFACE uStreamBufferInterface, 
        size_t* const size)
{
    (void)uStreamBufferInterface;

    *size = 10;

    USTREAMBUFFER_RESULT resutl = _concreteGetRemainingSizeResult;
    _concreteGetRemainingSizeResult = USTREAMBUFFER_SUCCESS;
    return resutl;
}

static USTREAMBUFFER_RESULT concreteGetCurrentPosition(
        USTREAMBUFFER_INTERFACE uStreamBufferInterface, 
        offset_t* const position)
{
    (void)uStreamBufferInterface;
    
    *position = currentPosition;

    USTREAMBUFFER_RESULT resutl = _concreteGetCurrentPositionResult;
    _concreteGetCurrentPositionResult = USTREAMBUFFER_SUCCESS;
    return resutl;
}

static USTREAMBUFFER_RESULT concreteRelease(
        USTREAMBUFFER_INTERFACE uStreamBufferInterface, 
        offset_t position)
{
    (void)uStreamBufferInterface;
    (void)position;

    USTREAMBUFFER_RESULT resutl = _concreteReleaseResult;
    _concreteReleaseResult = USTREAMBUFFER_SUCCESS;
    return resutl;
}

static USTREAMBUFFER_INTERFACE concreteClone(
        USTREAMBUFFER_INTERFACE uStreamBufferInterface, 
        offset_t offset)
{
    currentPosition = offset;

    USTREAMBUFFER_INTERFACE interfaceResult;
    if (_concreteCloneResult == USTREAMBUFFER_SUCCESS)
    {
        interfaceResult = uStreamBufferInterface;
    }
    else
    {
        interfaceResult = NULL;
        _concreteCloneResult = USTREAMBUFFER_SUCCESS;
    }
    return interfaceResult;
}

static USTREAMBUFFER_RESULT concreteDispose(
        USTREAMBUFFER_INTERFACE uStreamBufferInterface)
{
    (void)uStreamBufferInterface;

    USTREAMBUFFER_RESULT resutl = _concreteDisposeResult;
    _concreteDisposeResult = USTREAMBUFFER_SUCCESS;
    return resutl;
}

static const USTREAMBUFFER_API _api =
{
        concreteSeek,
        concreteReset,
        concreteGetNext,
        concreteGetRemainingSize,
        concreteGetCurrentPosition,
        concreteRelease,
        concreteClone,
        concreteDispose
};

static const int TEST_DATA = 1;

static const USTREAMBUFFER USTREAMBUFFER_COMPLIANCE_MOCK_BUFFER =
{
    (const USTREAMBUFFER_API*)&_api,
    (void*)&TEST_DATA
};

USTREAMBUFFER_INTERFACE uStreamBufferMockCreate(void)
{
    _concreteSeekResult = USTREAMBUFFER_SUCCESS;
    _concreteResetResult = USTREAMBUFFER_SUCCESS;
    _concreteGetNextResult = USTREAMBUFFER_SUCCESS;
    _concreteGetRemainingSizeResult = USTREAMBUFFER_SUCCESS;
    _concreteGetCurrentPositionResult = USTREAMBUFFER_SUCCESS;
    _concreteReleaseResult = USTREAMBUFFER_SUCCESS;
    _concreteCloneResult = USTREAMBUFFER_SUCCESS;
    _concreteDisposeResult = USTREAMBUFFER_SUCCESS;

    return (USTREAMBUFFER_INTERFACE)&USTREAMBUFFER_COMPLIANCE_MOCK_BUFFER;
}

void setSeekResult(USTREAMBUFFER_RESULT result)
{
    _concreteSeekResult = result;
}

void setResetResult(USTREAMBUFFER_RESULT result)
{
    _concreteResetResult = result;
}

void setGetNextResult(USTREAMBUFFER_RESULT result)
{
    _concreteGetNextResult = result;
}

void setGetRemainingSizeResult(USTREAMBUFFER_RESULT result)
{
    _concreteGetRemainingSizeResult = result;
}

void setGetCurrentPositionResult(USTREAMBUFFER_RESULT result)
{
    _concreteGetCurrentPositionResult = result;
}

void setReleaseResult(USTREAMBUFFER_RESULT result)
{
    _concreteReleaseResult = result;
}

void setCloneResult(USTREAMBUFFER_RESULT result)
{
    _concreteCloneResult = result;
}

void setDisposeResult(USTREAMBUFFER_RESULT result)
{
    _concreteDisposeResult = result;
}
