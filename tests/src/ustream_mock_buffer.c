// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ustream_mock_buffer.h"
#include "ustream_base.h"

static ULIB_RESULT _concrete_set_position_result = ULIB_SUCCESS;
static ULIB_RESULT _concrete_reset_result = ULIB_SUCCESS;
static ULIB_RESULT _concrete_read_result = ULIB_SUCCESS;
static ULIB_RESULT _concrete_get_remaining_size_result = ULIB_SUCCESS;
static ULIB_RESULT _concrete_get_position_result = ULIB_SUCCESS;
static ULIB_RESULT _concrete_release_result = ULIB_SUCCESS;
static ULIB_RESULT _concrete_clone_result = ULIB_SUCCESS;
static ULIB_RESULT _concrete_dispose_result = ULIB_SUCCESS;

static offset_t current_position = 0;

static ULIB_RESULT concrete_set_position(
        USTREAM* ustream_interface, 
        offset_t position)
{
    (void)ustream_interface;

    current_position = position;

    ULIB_RESULT result = _concrete_set_position_result;
    _concrete_set_position_result = ULIB_SUCCESS;
    return result;
}

static ULIB_RESULT concrete_reset(
        USTREAM* ustream_interface)
{
    (void)ustream_interface;

    ULIB_RESULT result = _concrete_reset_result;
    _concrete_reset_result = ULIB_SUCCESS;
    return result;
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

    current_position += buffer_length;

    ULIB_RESULT result = _concrete_read_result;
    _concrete_read_result = ULIB_SUCCESS;
    return result;
}

static ULIB_RESULT concrete_get_remaining_size(
        USTREAM* ustream_interface, 
        size_t* const size)
{
    (void)ustream_interface;

    *size = 10;

    ULIB_RESULT result = _concrete_get_remaining_size_result;
    _concrete_get_remaining_size_result = ULIB_SUCCESS;
    return result;
}

static ULIB_RESULT concrete_get_position(
        USTREAM* ustream_interface, 
        offset_t* const position)
{
    (void)ustream_interface;
    
    *position = current_position;

    ULIB_RESULT result = _concrete_get_position_result;
    _concrete_get_position_result = ULIB_SUCCESS;
    return result;
}

static ULIB_RESULT concrete_release(
        USTREAM* ustream_interface, 
        offset_t position)
{
    (void)ustream_interface;
    (void)position;

    ULIB_RESULT result = _concrete_release_result;
    _concrete_release_result = ULIB_SUCCESS;
    return result;
}

static USTREAM* concrete_clone(
        USTREAM* ustream_interface, 
        offset_t offset)
{
    current_position = offset;

    USTREAM* interface_result;
    if (_concrete_clone_result == ULIB_SUCCESS)
    {
        interface_result = ustream_interface;
    }
    else
    {
        interface_result = NULL;
        _concrete_clone_result = ULIB_SUCCESS;
    }
    return interface_result;
}

static ULIB_RESULT concrete_dispose(
        USTREAM* ustream_interface)
{
    (void)ustream_interface;

    ULIB_RESULT result = _concrete_dispose_result;
    _concrete_dispose_result = ULIB_SUCCESS;
    return result;
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

USTREAM* ustream_mock_create(void)
{
    _concrete_set_position_result = ULIB_SUCCESS;
    _concrete_reset_result = ULIB_SUCCESS;
    _concrete_read_result = ULIB_SUCCESS;
    _concrete_get_remaining_size_result = ULIB_SUCCESS;
    _concrete_get_position_result = ULIB_SUCCESS;
    _concrete_release_result = ULIB_SUCCESS;
    _concrete_clone_result = ULIB_SUCCESS;
    _concrete_dispose_result = ULIB_SUCCESS;

    return (USTREAM*)&USTREAM_COMPLIANCE_MOCK_BUFFER;
}

void set_set_position_result(ULIB_RESULT result)
{
    _concrete_set_position_result = result;
}

void set_reset_result(ULIB_RESULT result)
{
    _concrete_reset_result = result;
}

void set_read_result(ULIB_RESULT result)
{
    _concrete_read_result = result;
}

void set_get_remaining_size_result(ULIB_RESULT result)
{
    _concrete_get_remaining_size_result = result;
}

void set_get_position_result(ULIB_RESULT result)
{
    _concrete_get_position_result = result;
}

void set_release_result(ULIB_RESULT result)
{
    _concrete_release_result = result;
}

void set_clone_result(ULIB_RESULT result)
{
    _concrete_clone_result = result;
}

void set_dispose_result(ULIB_RESULT result)
{
    _concrete_dispose_result = result;
}
