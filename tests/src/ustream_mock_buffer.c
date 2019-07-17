// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ustream_mock_buffer.h"
#include "ustream_base.h"

static AZIOT_ULIB_RESULT _concrete_set_position_result = AZIOT_ULIB_SUCCESS;
static AZIOT_ULIB_RESULT _concrete_reset_result = AZIOT_ULIB_SUCCESS;
static AZIOT_ULIB_RESULT _concrete_read_result = AZIOT_ULIB_SUCCESS;
static AZIOT_ULIB_RESULT _concrete_get_remaining_size_result = AZIOT_ULIB_SUCCESS;
static AZIOT_ULIB_RESULT _concrete_get_position_result = AZIOT_ULIB_SUCCESS;
static AZIOT_ULIB_RESULT _concrete_release_result = AZIOT_ULIB_SUCCESS;
static AZIOT_ULIB_RESULT _concrete_clone_result = AZIOT_ULIB_SUCCESS;
static AZIOT_ULIB_RESULT _concrete_dispose_result = AZIOT_ULIB_SUCCESS;

static offset_t current_position = 0;

static AZIOT_ULIB_RESULT concrete_set_position(
        AZIOT_USTREAM* ustream_interface, 
        offset_t position)
{
    (void)ustream_interface;

    current_position = position;

    AZIOT_ULIB_RESULT result = _concrete_set_position_result;
    _concrete_set_position_result = AZIOT_ULIB_SUCCESS;
    return result;
}

static AZIOT_ULIB_RESULT concrete_reset(
        AZIOT_USTREAM* ustream_interface)
{
    (void)ustream_interface;

    AZIOT_ULIB_RESULT result = _concrete_reset_result;
    _concrete_reset_result = AZIOT_ULIB_SUCCESS;
    return result;
}

static AZIOT_ULIB_RESULT concrete_read(
        AZIOT_USTREAM* ustream_interface, 
        uint8_t* const buffer, 
        size_t buffer_length, 
        size_t* const size)
{
    (void)ustream_interface;
    (void)buffer;
    (void)size;

    current_position += buffer_length;

    AZIOT_ULIB_RESULT result = _concrete_read_result;
    _concrete_read_result = AZIOT_ULIB_SUCCESS;
    return result;
}

static AZIOT_ULIB_RESULT concrete_get_remaining_size(
        AZIOT_USTREAM* ustream_interface, 
        size_t* const size)
{
    (void)ustream_interface;

    *size = 10;

    AZIOT_ULIB_RESULT result = _concrete_get_remaining_size_result;
    _concrete_get_remaining_size_result = AZIOT_ULIB_SUCCESS;
    return result;
}

static AZIOT_ULIB_RESULT concrete_get_position(
        AZIOT_USTREAM* ustream_interface, 
        offset_t* const position)
{
    (void)ustream_interface;
    
    *position = current_position;

    AZIOT_ULIB_RESULT result = _concrete_get_position_result;
    _concrete_get_position_result = AZIOT_ULIB_SUCCESS;
    return result;
}

static AZIOT_ULIB_RESULT concrete_release(
        AZIOT_USTREAM* ustream_interface, 
        offset_t position)
{
    (void)ustream_interface;
    (void)position;

    AZIOT_ULIB_RESULT result = _concrete_release_result;
    _concrete_release_result = AZIOT_ULIB_SUCCESS;
    return result;
}

static AZIOT_USTREAM* concrete_clone(
        AZIOT_USTREAM* ustream_interface, 
        offset_t offset)
{
    current_position = offset;

    AZIOT_USTREAM* interface_result;
    if (_concrete_clone_result == AZIOT_ULIB_SUCCESS)
    {
        interface_result = ustream_interface;
    }
    else
    {
        interface_result = NULL;
        _concrete_clone_result = AZIOT_ULIB_SUCCESS;
    }
    return interface_result;
}

static AZIOT_ULIB_RESULT concrete_dispose(
        AZIOT_USTREAM* ustream_interface)
{
    (void)ustream_interface;

    AZIOT_ULIB_RESULT result = _concrete_dispose_result;
    _concrete_dispose_result = AZIOT_ULIB_SUCCESS;
    return result;
}

static const AZIOT_USTREAM_INTERFACE api =
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

static const AZIOT_USTREAM USTREAM_COMPLIANCE_MOCK_BUFFER =
{
    (const AZIOT_USTREAM_INTERFACE*)&api,
    (void*)&TEST_DATA
};

AZIOT_USTREAM* ustream_mock_create(void)
{
    _concrete_set_position_result = AZIOT_ULIB_SUCCESS;
    _concrete_reset_result = AZIOT_ULIB_SUCCESS;
    _concrete_read_result = AZIOT_ULIB_SUCCESS;
    _concrete_get_remaining_size_result = AZIOT_ULIB_SUCCESS;
    _concrete_get_position_result = AZIOT_ULIB_SUCCESS;
    _concrete_release_result = AZIOT_ULIB_SUCCESS;
    _concrete_clone_result = AZIOT_ULIB_SUCCESS;
    _concrete_dispose_result = AZIOT_ULIB_SUCCESS;

    return (AZIOT_USTREAM*)&USTREAM_COMPLIANCE_MOCK_BUFFER;
}

void set_set_position_result(AZIOT_ULIB_RESULT result)
{
    _concrete_set_position_result = result;
}

void set_reset_result(AZIOT_ULIB_RESULT result)
{
    _concrete_reset_result = result;
}

void set_read_result(AZIOT_ULIB_RESULT result)
{
    _concrete_read_result = result;
}

void set_get_remaining_size_result(AZIOT_ULIB_RESULT result)
{
    _concrete_get_remaining_size_result = result;
}

void set_get_position_result(AZIOT_ULIB_RESULT result)
{
    _concrete_get_position_result = result;
}

void set_release_result(AZIOT_ULIB_RESULT result)
{
    _concrete_release_result = result;
}

void set_clone_result(AZIOT_ULIB_RESULT result)
{
    _concrete_clone_result = result;
}

void set_dispose_result(AZIOT_ULIB_RESULT result)
{
    _concrete_dispose_result = result;
}
