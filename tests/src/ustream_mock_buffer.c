// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ustream_mock_buffer.h"
#include "ustream_base.h"

static ULIB_RESULT _concrete_set_positionResult = ULIB_SUCCESS;
static ULIB_RESULT _concrete_resetResult = ULIB_SUCCESS;
static ULIB_RESULT _concrete_readResult = ULIB_SUCCESS;
static ULIB_RESULT _concrete_get_remaining_sizeResult = ULIB_SUCCESS;
static ULIB_RESULT _concrete_get_positionResult = ULIB_SUCCESS;
static ULIB_RESULT _concrete_releaseResult = ULIB_SUCCESS;
static ULIB_RESULT _concrete_cloneResult = ULIB_SUCCESS;
static ULIB_RESULT _concrete_disposeResult = ULIB_SUCCESS;

static offset_t currentPosition = 0;

static ULIB_RESULT concrete_set_position(
        USTREAM* ustream_interface, 
        offset_t position)
{
    (void)ustream_interface;

    currentPosition = position;

    ULIB_RESULT resutl = _concrete_set_positionResult;
    _concrete_set_positionResult = ULIB_SUCCESS;
    return resutl;
}

static ULIB_RESULT concrete_reset(
        USTREAM* ustream_interface)
{
    (void)ustream_interface;

    ULIB_RESULT resutl = _concrete_resetResult;
    _concrete_resetResult = ULIB_SUCCESS;
    return resutl;
}

static ULIB_RESULT concrete_read(
        USTREAM* ustream_interface, 
        uint8_t* const buffer, 
        size_t buffer_length, 
        size_t* const size)
{
    (void)ustream_interface;
    (void)buffer;
    (void)size;

    currentPosition += buffer_length;

    ULIB_RESULT resutl = _concrete_readResult;
    _concrete_readResult = ULIB_SUCCESS;
    return resutl;
}

static ULIB_RESULT concrete_get_remaining_size(
        USTREAM* ustream_interface, 
        size_t* const size)
{
    (void)ustream_interface;

    *size = 10;

    ULIB_RESULT resutl = _concrete_get_remaining_sizeResult;
    _concrete_get_remaining_sizeResult = ULIB_SUCCESS;
    return resutl;
}

static ULIB_RESULT concrete_get_position(
        USTREAM* ustream_interface, 
        offset_t* const position)
{
    (void)ustream_interface;
    
    *position = currentPosition;

    ULIB_RESULT resutl = _concrete_get_positionResult;
    _concrete_get_positionResult = ULIB_SUCCESS;
    return resutl;
}

static ULIB_RESULT concrete_release(
        USTREAM* ustream_interface, 
        offset_t position)
{
    (void)ustream_interface;
    (void)position;

    ULIB_RESULT resutl = _concrete_releaseResult;
    _concrete_releaseResult = ULIB_SUCCESS;
    return resutl;
}

static USTREAM* concrete_clone(
        USTREAM* ustream_interface, 
        offset_t offset)
{
    currentPosition = offset;

    USTREAM* interfaceResult;
    if (_concrete_cloneResult == ULIB_SUCCESS)
    {
        interfaceResult = ustream_interface;
    }
    else
    {
        interfaceResult = NULL;
        _concrete_cloneResult = ULIB_SUCCESS;
    }
    return interfaceResult;
}

static ULIB_RESULT concrete_dispose(
        USTREAM* ustream_interface)
{
    (void)ustream_interface;

    ULIB_RESULT resutl = _concrete_disposeResult;
    _concrete_disposeResult = ULIB_SUCCESS;
    return resutl;
}

static const USTREAM_INTERFACE _api =
{
        concrete_set_position,
        concrete_reset,
        concrete_read,
        concrete_get_remaining_size,
        concrete_get_position,
        concrete_release,
        concrete_clone,
        concrete_dispose
};

static const int TEST_DATA = 1;

static const USTREAM USTREAM_COMPLIANCE_MOCK_BUFFER =
{
    (const USTREAM_INTERFACE*)&_api,
    (void*)&TEST_DATA
};

USTREAM* uStreamMockCreate(void)
{
    _concrete_set_positionResult = ULIB_SUCCESS;
    _concrete_resetResult = ULIB_SUCCESS;
    _concrete_readResult = ULIB_SUCCESS;
    _concrete_get_remaining_sizeResult = ULIB_SUCCESS;
    _concrete_get_positionResult = ULIB_SUCCESS;
    _concrete_releaseResult = ULIB_SUCCESS;
    _concrete_cloneResult = ULIB_SUCCESS;
    _concrete_disposeResult = ULIB_SUCCESS;

    return (USTREAM*)&USTREAM_COMPLIANCE_MOCK_BUFFER;
}

void setSeekResult(ULIB_RESULT result)
{
    _concrete_set_positionResult = result;
}

void setResetResult(ULIB_RESULT result)
{
    _concrete_resetResult = result;
}

void setGetNextResult(ULIB_RESULT result)
{
    _concrete_readResult = result;
}

void setGetRemainingSizeResult(ULIB_RESULT result)
{
    _concrete_get_remaining_sizeResult = result;
}

void setGetCurrentPositionResult(ULIB_RESULT result)
{
    _concrete_get_positionResult = result;
}

void setReleaseResult(ULIB_RESULT result)
{
    _concrete_releaseResult = result;
}

void setCloneResult(ULIB_RESULT result)
{
    _concrete_cloneResult = result;
}

void setDisposeResult(ULIB_RESULT result)
{
    _concrete_disposeResult = result;
}
