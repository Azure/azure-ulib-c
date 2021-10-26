// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "az_ulib_pal_api.h"
#include "az_ulib_result.h"
#include "az_ulib_ustream.h"

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

static az_result concrete_set_position(az_ulib_ustream* ustream_instance, offset_t position);
static az_result concrete_reset(az_ulib_ustream* ustream_instance);
static az_result concrete_read(
    az_ulib_ustream* ustream_instance,
    uint8_t* const buffer,
    size_t buffer_length,
    size_t* const size);
static az_result concrete_get_remaining_size(az_ulib_ustream* ustream_instance, size_t* const size);
static az_result concrete_get_position(az_ulib_ustream* ustream_instance, offset_t* const position);
static az_result concrete_release(az_ulib_ustream* ustream_instance, offset_t position);
static az_result concrete_clone(
    az_ulib_ustream* ustream_instance_clone,
    az_ulib_ustream* ustream_instance,
    offset_t offset);
static az_result concrete_dispose(az_ulib_ustream* ustream_instance);
static const az_ulib_ustream_interface api
    = { concrete_set_position, concrete_reset,   concrete_read,  concrete_get_remaining_size,
        concrete_get_position, concrete_release, concrete_clone, concrete_dispose };

static void init_instance(
    az_ulib_ustream* ustream_instance,
    az_ulib_ustream_data_cb* control_block,
    offset_t inner_current_position,
    offset_t offset,
    size_t data_buffer_length)
{
  ustream_instance->inner_current_position = inner_current_position;
  ustream_instance->inner_first_valid_position = inner_current_position;
  ustream_instance->offset_diff = offset - inner_current_position;
  ustream_instance->control_block = control_block;
  ustream_instance->length = data_buffer_length;
  AZ_ULIB_PORT_ATOMIC_INC_W(&(ustream_instance->control_block->ref_count));
}

static void destroy_control_block(az_ulib_ustream_data_cb* control_block)
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

static az_result concrete_set_position(az_ulib_ustream* ustream_instance, offset_t position)
{
  _az_PRECONDITION(AZ_ULIB_USTREAM_IS_TYPE_OF(ustream_instance, api));

  az_result result;

  offset_t inner_position = position - ustream_instance->offset_diff;

  if ((inner_position > (offset_t)(ustream_instance->length))
      || (inner_position < ustream_instance->inner_first_valid_position))
  {
    result = AZ_ERROR_ITEM_NOT_FOUND;
  }
  else
  {
    ustream_instance->inner_current_position = inner_position;
    result = AZ_OK;
  }

  return result;
}

static az_result concrete_reset(az_ulib_ustream* ustream_instance)
{
  _az_PRECONDITION(AZ_ULIB_USTREAM_IS_TYPE_OF(ustream_instance, api));

  ustream_instance->inner_current_position = ustream_instance->inner_first_valid_position;

  return AZ_OK;
}

static az_result concrete_read(
    az_ulib_ustream* ustream_instance,
    uint8_t* const buffer,
    size_t buffer_length,
    size_t* const size)
{
  _az_PRECONDITION(AZ_ULIB_USTREAM_IS_TYPE_OF(ustream_instance, api));
  _az_PRECONDITION_NOT_NULL(buffer);
  _az_PRECONDITION(buffer_length > 0);
  _az_PRECONDITION_NOT_NULL(size);

  az_result result;

  az_ulib_ustream_data_cb* control_block = ustream_instance->control_block;

  if (ustream_instance->inner_current_position >= ustream_instance->length)
  {
    *size = 0;
    result = AZ_ULIB_EOF;
  }
  else
  {
    size_t remain_size
        = ustream_instance->length - (size_t)ustream_instance->inner_current_position;
    *size = (buffer_length < remain_size) ? buffer_length : remain_size;
    IGNORE_MEMCPY_TO_NULL
    memcpy(
        buffer,
        (const uint8_t*)control_block->ptr + ustream_instance->inner_current_position,
        *size);
    RESUME_WARNINGS
    ustream_instance->inner_current_position += *size;
    result = AZ_OK;
  }

  return result;
}

static az_result concrete_get_remaining_size(az_ulib_ustream* ustream_instance, size_t* const size)
{
  _az_PRECONDITION(AZ_ULIB_USTREAM_IS_TYPE_OF(ustream_instance, api));
  _az_PRECONDITION_NOT_NULL(size);

  *size = ustream_instance->length - ustream_instance->inner_current_position;

  return AZ_OK;
}

static az_result concrete_get_position(az_ulib_ustream* ustream_instance, offset_t* const position)
{
  _az_PRECONDITION(AZ_ULIB_USTREAM_IS_TYPE_OF(ustream_instance, api));
  _az_PRECONDITION_NOT_NULL(position);

  *position = ustream_instance->inner_current_position + ustream_instance->offset_diff;

  return AZ_OK;
}

static az_result concrete_release(az_ulib_ustream* ustream_instance, offset_t position)
{
  _az_PRECONDITION(AZ_ULIB_USTREAM_IS_TYPE_OF(ustream_instance, api));

  az_result result;

  offset_t inner_position = position - ustream_instance->offset_diff;

  if ((inner_position >= ustream_instance->inner_current_position)
      || (inner_position < ustream_instance->inner_first_valid_position))
  {
    result = AZ_ERROR_ARG;
  }
  else
  {
    ustream_instance->inner_first_valid_position = inner_position + (offset_t)1;
    result = AZ_OK;
  }

  return result;
}

static az_result concrete_clone(
    az_ulib_ustream* ustream_instance_clone,
    az_ulib_ustream* ustream_instance,
    offset_t offset)
{
  _az_PRECONDITION(AZ_ULIB_USTREAM_IS_TYPE_OF(ustream_instance, api));
  _az_PRECONDITION_NOT_NULL(ustream_instance_clone);

  az_result result;

  if (offset > (UINT32_MAX - ustream_instance->length))
  {
    result = AZ_ERROR_ARG;
  }
  else
  {
    init_instance(
        ustream_instance_clone,
        ustream_instance->control_block,
        ustream_instance->inner_current_position,
        offset,
        ustream_instance->length);
    result = AZ_OK;
  }

  return result;
}

static az_result concrete_dispose(az_ulib_ustream* ustream_instance)
{
  _az_PRECONDITION(AZ_ULIB_USTREAM_IS_TYPE_OF(ustream_instance, api));

  az_ulib_ustream_data_cb* control_block = ustream_instance->control_block;

  AZ_ULIB_PORT_ATOMIC_DEC_W(&(control_block->ref_count));
  if (control_block->ref_count == 0)
  {
    destroy_control_block(control_block);
  }

  return AZ_OK;
}

AZ_NODISCARD az_result az_ulib_ustream_init(
    az_ulib_ustream* ustream_instance,
    az_ulib_ustream_data_cb* ustream_control_block,
    az_ulib_release_callback control_block_release,
    const uint8_t* const data_buffer,
    size_t data_buffer_length,
    az_ulib_release_callback data_buffer_release)
{
  _az_PRECONDITION_NOT_NULL(ustream_instance);
  _az_PRECONDITION_NOT_NULL(ustream_control_block);
  _az_PRECONDITION_NOT_NULL(data_buffer);
  _az_PRECONDITION(data_buffer_length > 0);

  ustream_control_block->api = &api;
  ustream_control_block->ptr = (const az_ulib_ustream_data*)data_buffer;
  ustream_control_block->ref_count = 0;
  ustream_control_block->data_release = data_buffer_release;
  ustream_control_block->control_block_release = control_block_release;

  init_instance(ustream_instance, ustream_control_block, 0, 0, data_buffer_length);

  return AZ_OK;
}
