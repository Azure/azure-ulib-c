// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "az_ulib_pal_api.h"
#include "az_ulib_result.h"
#include "az_ulib_ustream.h"

#include <azure/core/internal/az_precondition_internal.h>

#ifdef __clang__
#define IGNORE_CAST_QUALIFICATION \
  _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wcast-qual\"")
#define RESUME_WARNINGS _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#define IGNORE_CAST_QUALIFICATION \
  _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#define RESUME_WARNINGS _Pragma("GCC diagnostic pop")
#else
#define IGNORE_CAST_QUALIFICATION
#define RESUME_WARNINGS
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

static void destroy_instance(az_ulib_ustream* ustream_instance)
{
  /* In multidata, `ptr` points to a internal multidata control block, and the multidata code needs
   * write permission to execute its function. So, we have an Warning exception here to remove the
   * `const` qualification of the `ptr`. */
  IGNORE_CAST_QUALIFICATION
  az_ulib_ustream_multi_data_cb* multidata
      = (az_ulib_ustream_multi_data_cb*)ustream_instance->control_block->ptr;
  RESUME_WARNINGS
  az_pal_os_lock_deinit(&multidata->lock);

  if (ustream_instance->control_block->data_release != NULL)
  {
    ustream_instance->control_block->data_release(multidata);
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

  /* In multidata, `ptr` points to a internal multidata control block, and the multidata code needs
   * write permission to execute its function. So, we have an Warning exception here to remove the
   * `const` qualification of the `ptr`. */
  IGNORE_CAST_QUALIFICATION
  az_ulib_ustream_multi_data_cb* multi_data
      = (az_ulib_ustream_multi_data_cb*)ustream_instance->control_block->ptr;
  RESUME_WARNINGS

  az_ulib_ustream* current_ustream
      = (ustream_instance->inner_current_position < multi_data->ustream_one.length)
      ? &multi_data->ustream_one
      : &multi_data->ustream_two;

  *size = 0;
  az_result intermediate_result = AZ_OK;
  while ((intermediate_result == AZ_OK) && (*size < buffer_length) && (current_ustream != NULL))
  {
    size_t copied_size;
    size_t remain_size = buffer_length - *size;

    // Critical section to make sure another instance doesn't set_position before this one reads
    az_pal_os_lock_acquire(&multi_data->lock);
    az_ulib_ustream_set_position(current_ustream, ustream_instance->inner_current_position + *size);
    intermediate_result
        = az_ulib_ustream_read(current_ustream, &buffer[*size], remain_size, &copied_size);
    az_pal_os_lock_release(&multi_data->lock);

    switch (intermediate_result)
    {
      case AZ_OK:
        *size += copied_size;
        _az_FALLTHROUGH;
      case AZ_ULIB_EOF:
        if (*size < buffer_length)
        {
          if (current_ustream == &multi_data->ustream_one)
          {
            current_ustream = &multi_data->ustream_two;
            intermediate_result = AZ_OK;
          }
          else
          {
            current_ustream = NULL;
          }
        }
        break;
      default:
        break;
    }
  }

  if (*size != 0)
  {
    ustream_instance->inner_current_position += *size;
    result = AZ_OK;
  }
  else
  {
    result = intermediate_result;
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
    ustream_instance_clone->inner_current_position = ustream_instance->inner_current_position;
    ustream_instance_clone->inner_first_valid_position = ustream_instance->inner_current_position;
    ustream_instance_clone->offset_diff = offset - ustream_instance->inner_current_position;
    ustream_instance_clone->control_block = ustream_instance->control_block;
    ustream_instance_clone->length = ustream_instance->length;

    AZ_ULIB_PORT_ATOMIC_INC_W(&(ustream_instance->control_block->ref_count));

    /* In multidata, `ptr` points to a internal multidata control block, and the multidata code
     * needs write permission to execute its function. So, we have an Warning exception here to
     * remove the `const` qualification of the `ptr`. */
    IGNORE_CAST_QUALIFICATION
    az_ulib_ustream_multi_data_cb* multi_data
        = (az_ulib_ustream_multi_data_cb*)ustream_instance->control_block->ptr;
    RESUME_WARNINGS
    AZ_ULIB_PORT_ATOMIC_INC_W(&(multi_data->ustream_one_ref_count));
    AZ_ULIB_PORT_ATOMIC_INC_W(&(multi_data->ustream_two_ref_count));
    result = AZ_OK;
  }

  return result;
}

static az_result concrete_dispose(az_ulib_ustream* ustream_instance)
{
  _az_PRECONDITION(AZ_ULIB_USTREAM_IS_TYPE_OF(ustream_instance, api));

  /* In multidata, `ptr` points to a internal multidata control block, and the multidata code needs
   * write permission to execute its function. So, we have an Warning exception here to remove the
   * `const` qualification of the `ptr`. */
  IGNORE_CAST_QUALIFICATION
  az_ulib_ustream_multi_data_cb* multi_data
      = (az_ulib_ustream_multi_data_cb*)ustream_instance->control_block->ptr;
  RESUME_WARNINGS
  AZ_ULIB_PORT_ATOMIC_DEC_W(&(multi_data->ustream_one_ref_count));
  AZ_ULIB_PORT_ATOMIC_DEC_W(&(multi_data->ustream_two_ref_count));
  if (multi_data->ustream_one_ref_count == 0 && multi_data->ustream_one.control_block != NULL)
  {
    az_ulib_ustream_dispose(&(multi_data->ustream_one));
  }
  if (multi_data->ustream_two_ref_count == 0 && multi_data->ustream_two.control_block != NULL)
  {
    az_ulib_ustream_dispose(&(multi_data->ustream_two));
  }

  az_ulib_ustream_data_cb* control_block = ustream_instance->control_block;

  AZ_ULIB_PORT_ATOMIC_DEC_W(&(control_block->ref_count));
  if (control_block->ref_count == 0)
  {
    destroy_instance(ustream_instance);
  }

  return AZ_OK;
}

static void ustream_multi_init(
    az_ulib_ustream* ustream_instance,
    az_ulib_ustream_data_cb* control_block,
    az_ulib_ustream_multi_data_cb* multi_data,
    az_ulib_release_callback multi_data_release)
{
  multi_data->ustream_one.control_block = ustream_instance->control_block;
  multi_data->ustream_one.inner_current_position = ustream_instance->inner_current_position;
  multi_data->ustream_one.inner_first_valid_position = ustream_instance->inner_first_valid_position;
  multi_data->ustream_one.length = ustream_instance->length;
  multi_data->ustream_one.offset_diff = ustream_instance->offset_diff;
  multi_data->ustream_one_ref_count = 1;

  multi_data->ustream_two.control_block = NULL;
  multi_data->ustream_two.inner_current_position = 0;
  multi_data->ustream_two.inner_first_valid_position = 0;
  multi_data->ustream_two.length = 0;
  multi_data->ustream_two.offset_diff = 0;
  multi_data->ustream_two_ref_count = 0;

  az_pal_os_lock_init(&multi_data->lock);

  control_block->api = &api;
  control_block->ptr = (void*)multi_data;
  control_block->ref_count = 1;
  control_block->control_block_release = NULL;
  control_block->data_release = multi_data_release;

  ustream_instance->control_block = control_block;
}

AZ_NODISCARD az_result az_ulib_ustream_concat(
    az_ulib_ustream* ustream_instance,
    az_ulib_ustream* ustream_to_concat,
    az_ulib_ustream_multi_data_cb* multi_data,
    az_ulib_release_callback multi_data_release)
{
  _az_PRECONDITION_NOT_NULL(ustream_instance);
  _az_PRECONDITION_NOT_NULL(ustream_to_concat);
  _az_PRECONDITION_NOT_NULL(multi_data);

  az_result result;

  ustream_multi_init(ustream_instance, &multi_data->control_block, multi_data, multi_data_release);
  if ((result = az_ulib_ustream_clone(
           &multi_data->ustream_two, ustream_to_concat, ustream_instance->length))
      == AZ_OK)
  {
    size_t remaining_size;
    if ((result = az_ulib_ustream_get_remaining_size(&(multi_data->ustream_two), &remaining_size))
        == AZ_OK)
    {
      ustream_instance->length += remaining_size;
      AZ_ULIB_PORT_ATOMIC_INC_W(&(multi_data->ustream_two_ref_count));
    }
    else
    {
      az_ulib_ustream_dispose(&(multi_data->ustream_two));
    }
  }

  return result;
}

AZ_NODISCARD az_result az_ulib_ustream_split(
    az_ulib_ustream* ustream_instance,
    az_ulib_ustream* ustream_instance_split,
    offset_t split_pos)
{
  _az_PRECONDITION_NOT_NULL(ustream_instance);
  _az_PRECONDITION_NOT_NULL(ustream_instance_split);

  az_result result;

  offset_t old_position;
  if ((result = az_ulib_ustream_get_position(ustream_instance, &old_position)) == AZ_OK)
  {
    if (split_pos == old_position)
    {
      result = AZ_ERROR_ARG;
    }
    else
    {
      size_t ustream_remaining_size;
      if ((result = az_ulib_ustream_get_remaining_size(ustream_instance, &ustream_remaining_size))
          == AZ_OK)
      {
        if (old_position + ustream_remaining_size == split_pos)
        {
          result = AZ_ERROR_ARG;
        }
        else
        {
          if ((result = az_ulib_ustream_set_position(ustream_instance, split_pos)) == AZ_OK)
          {
            if ((result
                 = az_ulib_ustream_clone(ustream_instance_split, ustream_instance, split_pos))
                == AZ_OK)
            {
              if ((result = az_ulib_ustream_set_position(ustream_instance, old_position)) == AZ_OK)
              {
                ustream_instance->length = split_pos - ustream_instance->inner_first_valid_position;
              }
              else
              {
                az_ulib_ustream_dispose(ustream_instance_split);
              }
            }
            else
            {
              az_ulib_ustream_set_position(ustream_instance, old_position);
            }
          }
        }
      }
    }
  }

  return result;
}
