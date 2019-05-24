// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ustream_mock_buffer.h"
#include "ustream_base.h"

static ULIB_RESULT _concreteSeekResult = ULIB_SUCCESS;
static ULIB_RESULT _concreteResetResult = ULIB_SUCCESS;
static ULIB_RESULT _concreteGetNextResult = ULIB_SUCCESS;
static ULIB_RESULT _concreteGetRemainingSizeResult = ULIB_SUCCESS;
static ULIB_RESULT _concreteGetCurrentPositionResult = ULIB_SUCCESS;
static ULIB_RESULT _concreteReleaseResult = ULIB_SUCCESS;
static ULIB_RESULT _concreteCloneResult = ULIB_SUCCESS;
static ULIB_RESULT _concreteDisposeResult = ULIB_SUCCESS;

static offset_t currentPosition = 0;

static ULIB_RESULT concreteSeek(
        USTREAM* uStreamInterface, 
        offset_t position)
{
    (void)uStreamInterface;

    currentPosition = position;

    ULIB_RESULT resutl = _concreteSeekResult;
    _concreteSeekResult = ULIB_SUCCESS;
    return resutl;
}

static ULIB_RESULT concreteReset(
        USTREAM* uStreamInterface)
{
    (void)uStreamInterface;

    ULIB_RESULT resutl = _concreteResetResult;
    _concreteResetResult = ULIB_SUCCESS;
    return resutl;
}

static ULIB_RESULT concreteGetNext(
        USTREAM* uStreamInterface, 
        uint8_t* const buffer, 
        size_t bufferLength, 
        size_t* const size)
{
    (void)uStreamInterface;
    (void)buffer;
    (void)size;

    currentPosition += bufferLength;

    ULIB_RESULT resutl = _concreteGetNextResult;
    _concreteGetNextResult = ULIB_SUCCESS;
    return resutl;
}

static ULIB_RESULT concreteGetRemainingSize(
        USTREAM* uStreamInterface, 
        size_t* const size)
{
    (void)uStreamInterface;

    *size = 10;

    ULIB_RESULT resutl = _concreteGetRemainingSizeResult;
    _concreteGetRemainingSizeResult = ULIB_SUCCESS;
    return resutl;
}

static ULIB_RESULT concreteGetCurrentPosition(
        USTREAM* uStreamInterface, 
        offset_t* const position)
{
    (void)uStreamInterface;
    
    *position = currentPosition;

    ULIB_RESULT resutl = _concreteGetCurrentPositionResult;
    _concreteGetCurrentPositionResult = ULIB_SUCCESS;
    return resutl;
}

static ULIB_RESULT concreteRelease(
        USTREAM* uStreamInterface, 
        offset_t position)
{
    (void)uStreamInterface;
    (void)position;

    ULIB_RESULT resutl = _concreteReleaseResult;
    _concreteReleaseResult = ULIB_SUCCESS;
    return resutl;
}

static USTREAM* concreteClone(
        USTREAM* uStreamInterface, 
        offset_t offset)
{
    currentPosition = offset;

    USTREAM* interfaceResult;
    if (_concreteCloneResult == ULIB_SUCCESS)
    {
        interfaceResult = uStreamInterface;
    }
    else
    {
        interfaceResult = NULL;
        _concreteCloneResult = ULIB_SUCCESS;
    }
    return interfaceResult;
}

static ULIB_RESULT concreteDispose(
        USTREAM* uStreamInterface)
{
    (void)uStreamInterface;

    ULIB_RESULT resutl = _concreteDisposeResult;
    _concreteDisposeResult = ULIB_SUCCESS;
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
    _concreteSeekResult = ULIB_SUCCESS;
    _concreteResetResult = ULIB_SUCCESS;
    _concreteGetNextResult = ULIB_SUCCESS;
    _concreteGetRemainingSizeResult = ULIB_SUCCESS;
    _concreteGetCurrentPositionResult = ULIB_SUCCESS;
    _concreteReleaseResult = ULIB_SUCCESS;
    _concreteCloneResult = ULIB_SUCCESS;
    _concreteDisposeResult = ULIB_SUCCESS;

    return (USTREAM*)&USTREAM_COMPLIANCE_MOCK_BUFFER;
}

void setSeekResult(ULIB_RESULT result)
{
    _concreteSeekResult = result;
}

void setResetResult(ULIB_RESULT result)
{
    _concreteResetResult = result;
}

void setGetNextResult(ULIB_RESULT result)
{
    _concreteGetNextResult = result;
}

void setGetRemainingSizeResult(ULIB_RESULT result)
{
    _concreteGetRemainingSizeResult = result;
}

void setGetCurrentPositionResult(ULIB_RESULT result)
{
    _concreteGetCurrentPositionResult = result;
}

void setReleaseResult(ULIB_RESULT result)
{
    _concreteReleaseResult = result;
}

void setCloneResult(ULIB_RESULT result)
{
    _concreteCloneResult = result;
}

void setDisposeResult(ULIB_RESULT result)
{
    _concreteDisposeResult = result;
}
