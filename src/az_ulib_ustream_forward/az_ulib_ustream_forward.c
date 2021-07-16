// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "az_ulib_port.h"
#include "az_ulib_result.h"
#include "azure/core/az_span.h"
#include "az_ulib_ustream_forward.h"

#include <azure/core/internal/az_precondition_internal.h>

#ifdef __clang__
#define IGNORE_POINTER_TYPE_QUALIFICATION \
  _Pragma("clang diagnostic push")        \
      _Pragma("clang diagnostic ignored \"-Wincompatible-pointer-types-discards-qualifiers\"")
#define IGNORE_MEMCPY_TO_NULL _Pragma("GCC diagnostic push")
#define RESUME_WARNINGS _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#define IGNORE_POINTER_TYPE_QUALIFICATION \
  _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wdiscarded-qualifiers\"")
#define IGNORE_MEMCPY_TO_NULL _Pragma("GCC diagnostic push")
#define RESUME_WARNINGS _Pragma("GCC diagnostic pop")
#else
#define IGNORE_POINTER_TYPE_QUALIFICATION __pragma(warning(push));
#define IGNORE_MEMCPY_TO_NULL \
  __pragma(warning(push));  \
  __pragma(warning(suppress: 6387));
#define RESUME_WARNINGS __pragma(warning(pop));
#endif // __clang__

static az_result concrete_flush(
    az_ulib_ustream_forward* ustream_forward_instance,
    az_ulib_flush_callback push_callback, 
    az_context* push_callback_context);
static az_result concrete_read(
    az_ulib_ustream_forward* ustream_forward_instance,
    uint8_t* const buffer,
    size_t buffer_length,
    size_t* const size);
static az_result concrete_get_remaining_size(
    az_ulib_ustream_forward* ustream_forward_instance, 
    size_t* const size);
static az_result concrete_dispose(
    az_ulib_ustream_forward* ustream_forward_instance);
static const az_ulib_ustream_forward_interface api
    = { concrete_flush, concrete_read,  concrete_get_remaining_size, concrete_dispose };

static void init_instance(
    az_ulib_ustream_forward* ustream_forward_instance,
    az_ulib_ustream_forward_data_cb* control_block,
    offset_t inner_current_position,
    offset_t offset,
    size_t data_buffer_length)
{
  ustream_forward_instance->inner_current_position = inner_current_position;
  ustream_forward_instance->offset_diff = offset - inner_current_position;
  ustream_forward_instance->control_block = control_block;
  ustream_forward_instance->length = data_buffer_length;
  AZ_ULIB_PORT_ATOMIC_INC_W(&(ustream_forward_instance->control_block->ref_count));
}
static void destroy_control_block(az_ulib_ustream_forward_data_cb* control_block)
{
  if (control_block->data_release)
  {
    /* If `data_relese` was provided is because `ptr` is not `const`. So, we have an Warning
     * exception here to remove the `const` qualification of the `ptr`. */
    IGNORE_POINTER_TYPE_QUALIFICATION
    control_block->data_release(control_block->ptr);
    RESUME_WARNINGS
  }
  if (control_block->control_block_release)
  {
    control_block->control_block_release(control_block);
  }
}

static az_result concrete_flush(
    az_ulib_ustream_forward* ustream_forward_instance,
    az_ulib_flush_callback push_callback, 
    az_context* push_callback_context)
{
  // precondition checks
  _az_PRECONDITION_NOT_NULL(ustream_forward_instance);
  _az_PRECONDITION(AZ_ULIB_USTREAM_FORWARD_IS_TYPE_OF(ustream_forward_instance, api));
  _az_PRECONDITION_NOT_NULL(push_callback);
  _az_PRECONDITION_NOT_NULL(push_callback_context);

  AZ_ULIB_TRY
  {
    // point to data from instance
    az_ulib_ustream_forward_data_cb* control_block = ustream_forward_instance->control_block;
    size_t buffer_size;
    AZ_ULIB_THROW_IF_AZ_ERROR(concrete_get_remaining_size(ustream_forward_instance, &buffer_size));
    az_span buffer = az_span_create((uint8_t*)control_block->ptr, (int32_t)buffer_size);

    // invoke callback
    (*push_callback)(&buffer, push_callback_context);
  }
  AZ_ULIB_CATCH (...) {}

  return AZ_ULIB_TRY_RESULT;
}

static az_result concrete_read(
    az_ulib_ustream_forward* ustream_forward_instance,
    uint8_t* const buffer,
    size_t buffer_length,
    size_t* const size)
{
  _az_PRECONDITION(AZ_ULIB_USTREAM_FORWARD_IS_TYPE_OF(ustream_forward_instance, api));
  _az_PRECONDITION_NOT_NULL(buffer);
  _az_PRECONDITION(buffer_length > 0);
  _az_PRECONDITION_NOT_NULL(size);

  az_result result;

  az_ulib_ustream_forward_data_cb* control_block = ustream_forward_instance->control_block;

  if (ustream_forward_instance->inner_current_position >= ustream_forward_instance->length)
  {
    *size = 0;
    result = AZ_ULIB_EOF;
  }
  else
  {
    size_t remain_size
        = ustream_forward_instance->length - (size_t)ustream_forward_instance->inner_current_position;
    *size = (buffer_length < remain_size) ? buffer_length : remain_size;
    IGNORE_MEMCPY_TO_NULL
    memcpy(
        buffer,
        (const uint8_t*)control_block->ptr + ustream_forward_instance->inner_current_position,
        *size);
    RESUME_WARNINGS
    ustream_forward_instance->inner_current_position += *size;
    result = AZ_OK;
  }

  return result;
}

static az_result concrete_get_remaining_size(az_ulib_ustream_forward* ustream_forward_instance, size_t* const size)
{
  _az_PRECONDITION(AZ_ULIB_USTREAM_FORWARD_IS_TYPE_OF(ustream_forward_instance, api));
  _az_PRECONDITION_NOT_NULL(size);

  *size = ustream_forward_instance->length - ustream_forward_instance->inner_current_position;

  return AZ_OK;
}

static az_result concrete_dispose(az_ulib_ustream_forward* ustream_forward_instance)
{
  _az_PRECONDITION(AZ_ULIB_USTREAM_FORWARD_IS_TYPE_OF(ustream_forward_instance, api));

  az_ulib_ustream_forward_data_cb* control_block = ustream_forward_instance->control_block;

  AZ_ULIB_PORT_ATOMIC_DEC_W(&(control_block->ref_count));
  if (control_block->ref_count == 0)
  {
    destroy_control_block(control_block);
  }

  return AZ_OK;
}

AZ_NODISCARD az_result az_ulib_ustream_forward_init(
    az_ulib_ustream_forward* ustream_forward_instance,
    az_ulib_ustream_forward_data_cb* ustream_forward_control_block,
    az_ulib_release_callback control_block_release,
    const uint8_t* const data_buffer,
    size_t data_buffer_length,
    az_ulib_release_callback data_buffer_release)
{
  _az_PRECONDITION_NOT_NULL(ustream_forward_instance);
  _az_PRECONDITION_NOT_NULL(ustream_forward_control_block);
  _az_PRECONDITION_NOT_NULL(data_buffer);
  _az_PRECONDITION(data_buffer_length > 0);

  ustream_forward_control_block->api = &api;
  ustream_forward_control_block->ptr = (const az_ulib_ustream_forward_data*)data_buffer;
  ustream_forward_control_block->data_release = data_buffer_release;
  ustream_forward_control_block->control_block_release = control_block_release;

  init_instance(ustream_forward_instance, ustream_forward_control_block, 0, 0, data_buffer_length);

  return AZ_OK;
}
