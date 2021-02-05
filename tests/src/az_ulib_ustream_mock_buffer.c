// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_ustream_mock_buffer.h"
#include "az_ulib_test_thread.h"
#include "az_ulib_ustream_base.h"
#include <stdbool.h>

static az_result _concrete_set_position_result = AZ_OK;
static az_result _concrete_reset_result = AZ_OK;
static az_result _concrete_read_result = AZ_OK;
static az_result _concrete_get_remaining_size_result = AZ_OK;
static az_result _concrete_get_position_result = AZ_OK;
static az_result _concrete_release_result = AZ_OK;
static az_result _concrete_clone_result = AZ_OK;
static az_result _concrete_dispose_result = AZ_OK;

#define READ_BUFFER_SIZE 10
static offset_t current_position = 0;
static uint8_t read_buffer[READ_BUFFER_SIZE];

static bool concurrency_ustream = false;
static uint32_t delay_return_value = 0;

void reset_mock_buffer(void)
{
  current_position = 0;
  concurrency_ustream = false;
  delay_return_value = 0;
}

void set_concurrency_ustream(void) { concurrency_ustream = true; }

void set_delay_return_value(uint32_t delay) { delay_return_value = delay; }

static az_result concrete_set_position(az_ulib_ustream* ustream_instance, offset_t position)
{
  (void)ustream_instance;

  current_position = position;

  az_result result;

  if (concurrency_ustream)
  {
    concurrency_ustream = false;
    test_thread_sleep(1000);
  }

  if (delay_return_value == 0)
  {
    result = _concrete_set_position_result;
    _concrete_set_position_result = AZ_OK;
  }
  else
  {
    delay_return_value--;
    result = AZ_OK;
  }

  return result;
}

static az_result concrete_reset(az_ulib_ustream* ustream_instance)
{
  (void)ustream_instance;

  az_result result = _concrete_reset_result;
  _concrete_reset_result = AZ_OK;
  return result;
}

static az_result concrete_read(
    az_ulib_ustream* ustream_instance,
    uint8_t* const buffer,
    size_t buffer_length,
    size_t* const size)
{
  (void)ustream_instance;
  (void)buffer;
  (void)size;

  current_position += buffer_length;

  *size = buffer_length;

  az_result result = _concrete_read_result;
  _concrete_read_result = AZ_OK;
  return result;
}

static az_result concrete_get_remaining_size(az_ulib_ustream* ustream_instance, size_t* const size)
{
  (void)ustream_instance;

  *size = 10;

  az_result result = _concrete_get_remaining_size_result;
  _concrete_get_remaining_size_result = AZ_OK;
  return result;
}

static az_result concrete_get_position(az_ulib_ustream* ustream_instance, offset_t* const position)
{
  (void)ustream_instance;

  *position = current_position;

  az_result result = _concrete_get_position_result;
  _concrete_get_position_result = AZ_OK;
  return result;
}

static az_result concrete_release(az_ulib_ustream* ustream_instance, offset_t position)
{
  (void)ustream_instance;
  (void)position;

  az_result result = _concrete_release_result;
  _concrete_release_result = AZ_OK;
  return result;
}

static az_result concrete_clone(
    az_ulib_ustream* ustream_instance_clone,
    az_ulib_ustream* ustream_instance,
    offset_t offset)
{
  az_result result;

  current_position = offset;

  if (_concrete_clone_result == AZ_OK)
  {
    ustream_instance_clone->control_block = ustream_instance->control_block;
    ustream_instance_clone->inner_current_position = ustream_instance->inner_current_position;
    ustream_instance_clone->inner_first_valid_position
        = ustream_instance->inner_first_valid_position;
    ustream_instance_clone->length = ustream_instance->length;
    ustream_instance_clone->offset_diff = ustream_instance->offset_diff;
    result = AZ_OK;
  }
  else
  {
    ustream_instance_clone = NULL;
    result = _concrete_clone_result;
    _concrete_clone_result = AZ_OK;
  }
  return result;
}

static az_result concrete_dispose(az_ulib_ustream* ustream_instance)
{
  (void)ustream_instance;

  az_result result = _concrete_dispose_result;
  _concrete_dispose_result = AZ_OK;
  return result;
}

static const az_ulib_ustream_interface api
    = { concrete_set_position, concrete_reset,   concrete_read,  concrete_get_remaining_size,
        concrete_get_position, concrete_release, concrete_clone, concrete_dispose };

static const int TEST_DATA = 1;

static az_ulib_ustream_data_cb USTREAM_COMPLIANCE_MOCK_CONTROL_BLOCK
    = { .api = (const az_ulib_ustream_interface*)&api,
        .ptr = NULL,
        .ref_count = 0,
        .data_release = NULL,
        .control_block_release = NULL };

static az_ulib_ustream USTREAM_COMPLIANCE_MOCK_BUFFER
    = { .control_block = (az_ulib_ustream_data_cb*)&USTREAM_COMPLIANCE_MOCK_CONTROL_BLOCK,
        .offset_diff = 0,
        .inner_current_position = 0,
        .inner_first_valid_position = 0,
        .length = 10 };

az_ulib_ustream* ustream_mock_create(void)
{
  _concrete_set_position_result = AZ_OK;
  _concrete_reset_result = AZ_OK;
  _concrete_read_result = AZ_OK;
  _concrete_get_remaining_size_result = AZ_OK;
  _concrete_get_position_result = AZ_OK;
  _concrete_release_result = AZ_OK;
  _concrete_clone_result = AZ_OK;
  _concrete_dispose_result = AZ_OK;

  return &USTREAM_COMPLIANCE_MOCK_BUFFER;
}

void set_set_position_result(az_result result) { _concrete_set_position_result = result; }

void set_reset_result(az_result result) { _concrete_reset_result = result; }

void set_read_result(az_result result) { _concrete_read_result = result; }

void set_get_remaining_size_result(az_result result)
{
  _concrete_get_remaining_size_result = result;
}

void set_get_position_result(az_result result) { _concrete_get_position_result = result; }

void set_release_result(az_result result) { _concrete_release_result = result; }

void set_clone_result(az_result result) { _concrete_clone_result = result; }

void set_dispose_result(az_result result) { _concrete_dispose_result = result; }
