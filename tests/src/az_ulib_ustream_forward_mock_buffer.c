// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_result.h"
#include "az_ulib_test_thread.h"
#include "az_ulib_ustream_base.h"
#include "az_ulib_ustream_forward_mock_buffer.h"
#include <stdbool.h>

static az_result _concrete_flush_result = AZ_OK;
static az_result _concrete_read_result = AZ_OK;
static az_result _concrete_dispose_result = AZ_OK;

static offset_t current_position = 0;

static bool concurrency_ustream_forward = false;
static uint32_t delay_return_value = 0;

void reset_mock_buffer(void)
{
  current_position = 0;
  concurrency_ustream_forward = false;
  delay_return_value = 0;
}

void set_concurrency_ustream_forward(void) { concurrency_ustream_forward = true; }

void set_delay_return_value(uint32_t delay) { delay_return_value = delay; }

static az_result concrete_flush(
    az_ulib_ustream_forward* ustream_forward,
    az_ulib_flush_callback flush_callback,
    az_ulib_callback_context flush_callback_context)
{
  (void)ustream_forward;
  (void)flush_callback;
  (void)flush_callback_context;

  az_result result = _concrete_flush_result;
  _concrete_flush_result = AZ_OK;

  return result;
}

static az_result concrete_read(
    az_ulib_ustream_forward* ustream_forward,
    uint8_t* const buffer,
    size_t buffer_length,
    size_t* const size)
{
  (void)ustream_forward;
  (void)buffer;
  (void)size;

  current_position += buffer_length;

  *size = buffer_length;

  az_result result = _concrete_read_result;
  _concrete_read_result = AZ_OK;
  return result;
}

static size_t concrete_get_size(az_ulib_ustream_forward* ustream_forward)
{
  (void)ustream_forward;

  return 10;
}

static az_result concrete_dispose(az_ulib_ustream_forward* ustream_forward)
{
  (void)ustream_forward;

  az_result result = _concrete_dispose_result;
  _concrete_dispose_result = AZ_OK;
  return result;
}

static const az_ulib_ustream_forward_interface api
    = { concrete_flush, concrete_read, concrete_get_size, concrete_dispose };

static az_ulib_ustream_forward USTREAM_FORWARD_COMPLIANCE_MOCK_BUFFER
    = { ._internal.api = (const az_ulib_ustream_forward_interface*)&api,
        ._internal.ptr = NULL,
        ._internal.data_release = NULL,
        ._internal.ustream_forward_release = NULL,
        ._internal.inner_current_position = 0,
        ._internal.length = 10 };

az_ulib_ustream_forward* ustream_forward_mock_create(void)
{
  _concrete_flush_result = AZ_OK;
  _concrete_read_result = AZ_OK;
  _concrete_dispose_result = AZ_OK;

  return &USTREAM_FORWARD_COMPLIANCE_MOCK_BUFFER;
}

void set_flush_result(az_result result) { _concrete_flush_result = result; }

void set_read_result(az_result result) { _concrete_read_result = result; }

void set_dispose_result(az_result result) { _concrete_dispose_result = result; }
