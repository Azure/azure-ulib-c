// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include "ustream_mock_buffer.h"
#include "ustream_base.h"
#include "test_thread.h"

static AZ_ULIB_RESULT _concrete_set_position_result = AZ_ULIB_SUCCESS;
static AZ_ULIB_RESULT _concrete_reset_result = AZ_ULIB_SUCCESS;
static AZ_ULIB_RESULT _concrete_read_result = AZ_ULIB_SUCCESS;
static AZ_ULIB_RESULT _concrete_get_remaining_size_result = AZ_ULIB_SUCCESS;
static AZ_ULIB_RESULT _concrete_get_position_result = AZ_ULIB_SUCCESS;
static AZ_ULIB_RESULT _concrete_release_result = AZ_ULIB_SUCCESS;
static AZ_ULIB_RESULT _concrete_clone_result = AZ_ULIB_SUCCESS;
static AZ_ULIB_RESULT _concrete_dispose_result = AZ_ULIB_SUCCESS;

#define READ_BUFFER_SIZE 10
static offset_t current_position = 0;
static uint8_t read_buffer[READ_BUFFER_SIZE];

static bool concurrency_ustream;

void set_concurrency_ustream(void)
{
    concurrency_ustream = true;
}

static AZ_ULIB_RESULT concrete_set_position(
        AZ_USTREAM* ustream_interface, 
        offset_t position)
{
    (void)ustream_interface;

    current_position = position;

    AZ_ULIB_RESULT result;

    if(concurrency_ustream)
    {
        concurrency_ustream = false;
        test_thread_sleep(1000);
    }

    result = _concrete_set_position_result;
    _concrete_set_position_result = AZ_ULIB_SUCCESS;
    return result;
}

static AZ_ULIB_RESULT concrete_reset(
        AZ_USTREAM* ustream_interface)
{
    (void)ustream_interface;

    AZ_ULIB_RESULT result = _concrete_reset_result;
    _concrete_reset_result = AZ_ULIB_SUCCESS;
    return result;
}

static AZ_ULIB_RESULT concrete_read(
        AZ_USTREAM* ustream_interface, 
        uint8_t* const buffer, 
        size_t buffer_length, 
        size_t* const size)
{
    (void)ustream_interface;
    (void)buffer;
    (void)size;

    current_position += buffer_length;
    
    *size = buffer_length;

    AZ_ULIB_RESULT result = _concrete_read_result;
    _concrete_read_result = AZ_ULIB_SUCCESS;
    return result;
}

static AZ_ULIB_RESULT concrete_get_remaining_size(
        AZ_USTREAM* ustream_interface, 
        size_t* const size)
{
    (void)ustream_interface;

    *size = 10;

    AZ_ULIB_RESULT result = _concrete_get_remaining_size_result;
    _concrete_get_remaining_size_result = AZ_ULIB_SUCCESS;
    return result;
}

static AZ_ULIB_RESULT concrete_get_position(
        AZ_USTREAM* ustream_interface, 
        offset_t* const position)
{
    (void)ustream_interface;
    
    *position = current_position;

    AZ_ULIB_RESULT result = _concrete_get_position_result;
    _concrete_get_position_result = AZ_ULIB_SUCCESS;
    return result;
}

static AZ_ULIB_RESULT concrete_release(
        AZ_USTREAM* ustream_interface, 
        offset_t position)
{
    (void)ustream_interface;
    (void)position;

    AZ_ULIB_RESULT result = _concrete_release_result;
    _concrete_release_result = AZ_ULIB_SUCCESS;
    return result;
}

static AZ_ULIB_RESULT concrete_clone(
        AZ_USTREAM* ustream_interface_clone,
        AZ_USTREAM* ustream_interface, 
        offset_t offset)
{
    current_position = offset;

    if (_concrete_clone_result == AZ_ULIB_SUCCESS)
    {
        ustream_interface_clone->inner_buffer = ustream_interface->inner_buffer;
        ustream_interface_clone->inner_current_position = ustream_interface->inner_current_position;
        ustream_interface_clone->inner_first_valid_position = ustream_interface->inner_first_valid_position;
        ustream_interface_clone->length = ustream_interface->length;
        ustream_interface_clone->offset_diff = ustream_interface->offset_diff;
    }
    else
    {
        ustream_interface_clone = NULL;
        _concrete_clone_result = AZ_ULIB_SUCCESS;
    }
    return AZ_ULIB_SUCCESS;
}

static AZ_ULIB_RESULT concrete_dispose(
        AZ_USTREAM* ustream_interface)
{
    (void)ustream_interface;

    AZ_ULIB_RESULT result = _concrete_dispose_result;
    _concrete_dispose_result = AZ_ULIB_SUCCESS;
    return result;
}

static const AZ_USTREAM_INTERFACE api =
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

static AZ_USTREAM_INNER_BUFFER USTREAM_COMPLIANCE_MOCK_INNER_BUFFER =
{
    .api = (const AZ_USTREAM_INTERFACE*)&api,
    .ptr = NULL,
    .ref_count = 0,
    .data_release = NULL,
    .inner_buffer_release = NULL
};

static AZ_USTREAM USTREAM_COMPLIANCE_MOCK_BUFFER =
{
    .inner_buffer = (AZ_USTREAM_INNER_BUFFER*)&USTREAM_COMPLIANCE_MOCK_INNER_BUFFER,
    .offset_diff = 0,
    .inner_current_position = 0,
    .inner_first_valid_position = 0,
    .length = 1
};

AZ_USTREAM* ustream_mock_create(void)
{
    _concrete_set_position_result = AZ_ULIB_SUCCESS;
    _concrete_reset_result = AZ_ULIB_SUCCESS;
    _concrete_read_result = AZ_ULIB_SUCCESS;
    _concrete_get_remaining_size_result = AZ_ULIB_SUCCESS;
    _concrete_get_position_result = AZ_ULIB_SUCCESS;
    _concrete_release_result = AZ_ULIB_SUCCESS;
    _concrete_clone_result = AZ_ULIB_SUCCESS;
    _concrete_dispose_result = AZ_ULIB_SUCCESS;

    return (AZ_USTREAM*)&USTREAM_COMPLIANCE_MOCK_BUFFER;
}

void set_set_position_result(AZ_ULIB_RESULT result)
{
    _concrete_set_position_result = result;
}

void set_reset_result(AZ_ULIB_RESULT result)
{
    _concrete_reset_result = result;
}

void set_read_result(AZ_ULIB_RESULT result)
{
    _concrete_read_result = result;
}

void set_get_remaining_size_result(AZ_ULIB_RESULT result)
{
    _concrete_get_remaining_size_result = result;
}

void set_get_position_result(AZ_ULIB_RESULT result)
{
    _concrete_get_position_result = result;
}

void set_release_result(AZ_ULIB_RESULT result)
{
    _concrete_release_result = result;
}

void set_clone_result(AZ_ULIB_RESULT result)
{
    _concrete_clone_result = result;
}

void set_dispose_result(AZ_ULIB_RESULT result)
{
    _concrete_dispose_result = result;
}
