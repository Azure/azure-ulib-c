// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ustream_mock_buffer.h"
#include "ustream_base.h"

static USTREAM_RESULT _concreteSeekResult = USTREAM_SUCCESS;
static USTREAM_RESULT _concreteResetResult = USTREAM_SUCCESS;
static USTREAM_RESULT _concreteGetNextResult = USTREAM_SUCCESS;
static USTREAM_RESULT _concreteGetRemainingSizeResult = USTREAM_SUCCESS;
static USTREAM_RESULT _concreteGetCurrentPositionResult = USTREAM_SUCCESS;
static USTREAM_RESULT _concreteReleaseResult = USTREAM_SUCCESS;
static USTREAM_RESULT _concreteCloneResult = USTREAM_SUCCESS;
static USTREAM_RESULT _concreteDisposeResult = USTREAM_SUCCESS;

static offset_t currentPosition = 0;

static USTREAM_RESULT concreteSeek(
        USTREAM* uStreamInterface, 
        offset_t position)
{
    (void)uStreamInterface;

    currentPosition = position;

    USTREAM_RESULT resutl = _concreteSeekResult;
    _concreteSeekResult = USTREAM_SUCCESS;
    return resutl;
}

static USTREAM_RESULT concreteReset(
        USTREAM* uStreamInterface)
{
    (void)uStreamInterface;

    USTREAM_RESULT resutl = _concreteResetResult;
    _concreteResetResult = USTREAM_SUCCESS;
    return resutl;
}

static USTREAM_RESULT concreteGetNext(
        USTREAM* uStreamInterface, 
        uint8_t* const buffer, 
        size_t bufferLength, 
        size_t* const size)
{
    (void)uStreamInterface;
    (void)buffer;
    (void)size;

    currentPosition += bufferLength;

    USTREAM_RESULT resutl = _concreteGetNextResult;
    _concreteGetNextResult = USTREAM_SUCCESS;
    return resutl;
}

static USTREAM_RESULT concreteGetRemainingSize(
        USTREAM* uStreamInterface, 
        size_t* const size)
{
    (void)uStreamInterface;

    *size = 10;

    USTREAM_RESULT resutl = _concreteGetRemainingSizeResult;
    _concreteGetRemainingSizeResult = USTREAM_SUCCESS;
    return resutl;
}

static USTREAM_RESULT concreteGetCurrentPosition(
        USTREAM* uStreamInterface, 
        offset_t* const position)
{
    (void)uStreamInterface;
    
    *position = currentPosition;

    USTREAM_RESULT resutl = _concreteGetCurrentPositionResult;
    _concreteGetCurrentPositionResult = USTREAM_SUCCESS;
    return resutl;
}

static USTREAM_RESULT concreteRelease(
        USTREAM* uStreamInterface, 
        offset_t position)
{
    (void)uStreamInterface;
    (void)position;

    USTREAM_RESULT resutl = _concreteReleaseResult;
    _concreteReleaseResult = USTREAM_SUCCESS;
    return resutl;
}

static USTREAM* concreteClone(
        USTREAM* uStreamInterface, 
        offset_t offset)
{
    currentPosition = offset;

    USTREAM* interfaceResult;
    if (_concreteCloneResult == USTREAM_SUCCESS)
    {
        interfaceResult = uStreamInterface;
    }
    else
    {
        interfaceResult = NULL;
        _concreteCloneResult = USTREAM_SUCCESS;
    }
    return interfaceResult;
}

static USTREAM_RESULT concreteDispose(
        USTREAM* uStreamInterface)
{
    (void)uStreamInterface;

    USTREAM_RESULT resutl = _concreteDisposeResult;
    _concreteDisposeResult = USTREAM_SUCCESS;
    return resutl;
}

static const USTREAM_INTERFACE _api =
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

static const USTREAM USTREAM_COMPLIANCE_MOCK_BUFFER =
{
    (const USTREAM_INTERFACE*)&_api,
    (void*)&TEST_DATA
};

USTREAM* uStreamMockCreate(void)
{
    _concreteSeekResult = USTREAM_SUCCESS;
    _concreteResetResult = USTREAM_SUCCESS;
    _concreteGetNextResult = USTREAM_SUCCESS;
    _concreteGetRemainingSizeResult = USTREAM_SUCCESS;
    _concreteGetCurrentPositionResult = USTREAM_SUCCESS;
    _concreteReleaseResult = USTREAM_SUCCESS;
    _concreteCloneResult = USTREAM_SUCCESS;
    _concreteDisposeResult = USTREAM_SUCCESS;

    return (USTREAM*)&USTREAM_COMPLIANCE_MOCK_BUFFER;
}

void setSeekResult(USTREAM_RESULT result)
{
    _concreteSeekResult = result;
}

void setResetResult(USTREAM_RESULT result)
{
    _concreteResetResult = result;
}

void setGetNextResult(USTREAM_RESULT result)
{
    _concreteGetNextResult = result;
}

void setGetRemainingSizeResult(USTREAM_RESULT result)
{
    _concreteGetRemainingSizeResult = result;
}

void setGetCurrentPositionResult(USTREAM_RESULT result)
{
    _concreteGetCurrentPositionResult = result;
}

void setReleaseResult(USTREAM_RESULT result)
{
    _concreteReleaseResult = result;
}

void setCloneResult(USTREAM_RESULT result)
{
    _concreteCloneResult = result;
}

void setDisposeResult(USTREAM_RESULT result)
{
    _concreteDisposeResult = result;
}
