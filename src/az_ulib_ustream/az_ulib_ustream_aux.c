// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "az_ulib_pal_os.h"
#include "az_ulib_pal_os_api.h"
#include "az_ulib_result.h"
#include "az_ulib_ucontract.h"
#include "az_ulib_ulog.h"
#include "az_ulib_ustream.h"

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
  az_ulib_ustream_multi_data_cb* multidata
      = (az_ulib_ustream_multi_data_cb*)ustream_instance->control_block->ptr;
  az_pal_os_lock_deinit(&multidata->lock);

  if (ustream_instance->control_block->data_release != NULL)
  {
    ustream_instance->control_block->data_release(ustream_instance->control_block->ptr);
  }
}

static az_result concrete_set_position(az_ulib_ustream* ustream_instance, offset_t position)
{
  /*[az_ulib_ustream_set_position_compliance_null_buffer_failed]*/
  /*[az_ulib_ustream_set_position_compliance_non_type_of_buffer_api_failed]*/
  AZ_ULIB_UCONTRACT(AZ_ULIB_UCONTRACT_REQUIRE(
      !AZ_ULIB_USTREAM_IS_NOT_TYPE_OF(ustream_instance, api),
      AZ_ERROR_ARG,
      AZ_ULIB_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING));
  az_result result;

  offset_t inner_position = position - ustream_instance->offset_diff;

  /*[az_ulib_ustream_set_position_compliance_forward_out_of_the_buffer_failed]*/
  /*[az_ulib_ustream_set_position_compliance_back_before_first_valid_position_failed]*/
  /*[az_ulib_ustream_set_position_compliance_back_before_first_valid_position_with_offset_failed]*/
  if ((inner_position > (offset_t)(ustream_instance->length))
      || (inner_position < ustream_instance->inner_first_valid_position))
  {
    result = AZ_ERROR_ITEM_NOT_FOUND;
  }
  else
  {
    /*[az_ulib_ustream_set_position_compliance_back_to_beginning_succeed]*/
    /*[az_ulib_ustream_set_position_compliance_back_position_succeed]*/
    /*[az_ulib_ustream_set_position_compliance_forward_position_succeed]*/
    /*[az_ulib_ustream_set_position_compliance_forward_to_the_end_position_succeed]*/
    /*[az_ulib_ustream_set_position_compliance_run_full_buffer_byte_by_byte_succeed]*/
    /*[az_ulib_ustream_set_position_compliance_run_full_buffer_byte_by_byte_reverse_order_succeed]*/
    /*[az_ulib_ustream_set_position_compliance_cloned_buffer_back_to_beginning_succeed]*/
    /*[az_ulib_ustream_set_position_compliance_cloned_buffer_back_position_succeed]*/
    /*[az_ulib_ustream_set_position_compliance_cloned_buffer_forward_position_succeed]*/
    /*[az_ulib_ustream_set_position_compliance_cloned_buffer_forward_to_the_end_position_succeed]*/
    /*[az_ulib_ustream_set_position_compliance_cloned_buffer_run_full_buffer_byte_by_byte_succeed]*/
    /*[az_ulib_ustream_set_position_compliance_cloned_buffer_run_full_buffer_byte_by_byte_reverse_order_succeed]*/
    ustream_instance->inner_current_position = inner_position;
    result = AZ_OK;
  }
  return result;
}

static az_result concrete_reset(az_ulib_ustream* ustream_instance)
{
  /*[az_ulib_ustream_reset_compliance_null_buffer_failed]*/
  /*[az_ulib_ustream_reset_compliance_non_type_of_buffer_api_failed]*/
  AZ_ULIB_UCONTRACT(AZ_ULIB_UCONTRACT_REQUIRE(
      !AZ_ULIB_USTREAM_IS_NOT_TYPE_OF(ustream_instance, api),
      AZ_ERROR_ARG,
      AZ_ULIB_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING));

  /*[az_ulib_ustream_reset_compliance_back_to_beginning_succeed]*/
  /*[az_ulib_ustream_reset_compliance_back_position_succeed]*/
  /*[az_ulib_ustream_reset_compliance_cloned_buffer_succeed]*/
  ustream_instance->inner_current_position = ustream_instance->inner_first_valid_position;
  return AZ_OK;
}

static az_result concrete_read(
    az_ulib_ustream* ustream_instance,
    uint8_t* const buffer,
    size_t buffer_length,
    size_t* const size)
{
  /*[az_ulib_ustream_read_compliance_null_buffer_failed]*/
  /*[az_ulib_ustream_read_compliance_non_type_of_buffer_api_failed]*/
  /*[az_ulib_ustream_read_compliance_null_return_buffer_failed]*/
  /*[az_ulib_ustream_read_compliance_null_return_size_failed]*/
  /*[az_ulib_ustream_read_compliance_buffer_with_zero_size_failed]*/
  AZ_ULIB_UCONTRACT(
      AZ_ULIB_UCONTRACT_REQUIRE(
          !AZ_ULIB_USTREAM_IS_NOT_TYPE_OF(ustream_instance, api),
          AZ_ERROR_ARG,
          AZ_ULIB_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING),
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(buffer, AZ_ERROR_ARG),
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_EQUALS(buffer_length, 0, AZ_ERROR_ARG),
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(size, AZ_ERROR_ARG));

  az_result result;

  az_ulib_ustream_multi_data_cb* multi_data
      = (az_ulib_ustream_multi_data_cb*)ustream_instance->control_block->ptr;
  az_ulib_ustream* current_ustream
      = (ustream_instance->inner_current_position < multi_data->ustream_one.length)
      ? &multi_data->ustream_one
      : &multi_data->ustream_two;

  /*[az_ulib_ustream_read_compliance_single_buffer_succeed]*/
  /*[az_ulib_ustream_read_compliance_right_boundary_condition_succeed]*/
  /*[az_ulib_ustream_read_compliance_boundary_condition_succeed]*/
  /*[az_ulib_ustream_read_compliance_left_boundary_condition_succeed]*/
  /*[az_ulib_ustream_read_compliance_single_byte_succeed]*/
  /*[az_ulib_ustream_read_compliance_get_from_cloned_buffer_succeed]*/
  /*[az_ulib_ustream_read_compliance_cloned_buffer_right_boundary_condition_succeed]*/
  *size = 0;
  az_result intermediate_result = AZ_OK;
  while ((intermediate_result == AZ_OK) && (*size < buffer_length) && (current_ustream != NULL))
  {
    size_t copied_size;
    size_t remain_size = buffer_length - *size;

    // Critical section to make sure another instance doesn't set_position before this one reads
    az_pal_os_lock_acquire(&multi_data->lock);
    /*[az_ulib_ustream_multi_read_clone_and_original_in_parallel_succeed]*/
    az_ulib_ustream_set_position(current_ustream, ustream_instance->inner_current_position + *size);
    intermediate_result
        = az_ulib_ustream_read(current_ustream, &buffer[*size], remain_size, &copied_size);
    az_pal_os_lock_release(&multi_data->lock);

    switch (intermediate_result)
    {
      case AZ_OK:
        *size += copied_size;
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
        /*[az_ulib_ustream_multi_read_control_block_failed_in_read_with_some_valid_content_succeed]*/
        break;
    }
  }

  if (*size != 0)
  {
    /*[az_ulib_ustream_concat_read_from_multiple_buffers_succeed]*/
    ustream_instance->inner_current_position += *size;
    result = AZ_OK;
  }
  else
  {
    /*[az_ulib_ustream_multi_read_control_block_failed_in_read_failed]*/
    result = intermediate_result;
  }

  return result;
}

static az_result concrete_get_remaining_size(az_ulib_ustream* ustream_instance, size_t* const size)
{
  /*[az_ulib_ustream_get_remaining_size_compliance_null_buffer_failed]*/
  /*[az_ulib_ustream_get_remaining_size_compliance_buffer_is_not_type_of_buffer_failed]*/
  /*[az_ulib_ustream_get_remaining_size_compliance_null_size_failed]*/
  AZ_ULIB_UCONTRACT(
      AZ_ULIB_UCONTRACT_REQUIRE(
          !AZ_ULIB_USTREAM_IS_NOT_TYPE_OF(ustream_instance, api),
          AZ_ERROR_ARG,
          AZ_ULIB_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING),
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(size, AZ_ERROR_ARG));

  /*[az_ulib_ustream_get_remaining_size_compliance_new_buffer_succeed]*/
  /*[az_ulib_ustream_get_remaining_size_compliance_new_buffer_with_non_zero_current_position_succeed]*/
  /*[az_ulib_ustream_get_remaining_size_compliance_cloned_buffer_with_non_zero_current_position_succeed]*/
  *size = ustream_instance->length - ustream_instance->inner_current_position;

  return AZ_OK;
}

static az_result concrete_get_position(az_ulib_ustream* ustream_instance, offset_t* const position)
{
  /*[az_ulib_ustream_get_current_position_compliance_null_buffer_failed]*/
  /*[az_ulib_ustream_get_current_position_compliance_buffer_is_not_type_of_buffer_failed]*/
  /*[az_ulib_ustream_get_current_position_compliance_null_position_failed]*/
  AZ_ULIB_UCONTRACT(
      AZ_ULIB_UCONTRACT_REQUIRE(
          !AZ_ULIB_USTREAM_IS_NOT_TYPE_OF(ustream_instance, api),
          AZ_ERROR_ARG,
          AZ_ULIB_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING),
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(position, AZ_ERROR_ARG));

  /*[az_ulib_ustream_get_current_position_compliance_new_buffer_succeed]*/
  /*[az_ulib_ustream_get_current_position_compliance_new_buffer_with_non_zero_current_position_succeed]*/
  /*[az_ulib_ustream_get_current_position_compliance_cloned_buffer_with_non_zero_current_position_succeed]*/
  *position = ustream_instance->inner_current_position + ustream_instance->offset_diff;

  return AZ_OK;
}

static az_result concrete_release(az_ulib_ustream* ustream_instance, offset_t position)
{
  /*[az_ulib_ustream_release_compliance_null_buffer_failed]*/
  /*[az_ulib_ustream_release_compliance_non_type_of_buffer_api_failed]*/
  AZ_ULIB_UCONTRACT(AZ_ULIB_UCONTRACT_REQUIRE(
      !AZ_ULIB_USTREAM_IS_NOT_TYPE_OF(ustream_instance, api),
      AZ_ERROR_ARG,
      AZ_ULIB_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING));
  az_result result;

  offset_t inner_position = position - ustream_instance->offset_diff;

  /*[az_ulib_ustream_release_compliance_release_after_current_failed]*/
  /*[az_ulib_ustream_release_compliance_release_position_already_released_failed]*/
  if ((inner_position >= ustream_instance->inner_current_position)
      || (inner_position < ustream_instance->inner_first_valid_position))
  {
    result = AZ_ERROR_ARG;
  }
  else
  {
    /*[az_ulib_ustream_release_compliance_succeed]*/
    /*[az_ulib_ustream_release_compliance_release_all_succeed]*/
    /*[az_ulib_ustream_release_compliance_run_full_buffer_byte_by_byte_succeed]*/
    /*[az_ulib_ustream_release_compliance_cloned_buffer_succeed]*/
    /*[az_ulib_ustream_release_compliance_cloned_buffer_release_all_succeed]*/
    /*[az_ulib_ustream_release_compliance_cloned_buffer_run_full_buffer_byte_by_byte_succeed]*/
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
  /*[az_ulib_ustream_clone_compliance_null_buffer_failed]*/
  /*[az_ulib_ustream_clone_compliance_buffer_is_not_type_of_buffer_failed]*/
  /*[az_ulib_ustream_clone_compliance_null_buffer_clone_failed]*/
  /*[az_ulib_ustream_clone_compliance_offset_exceed_size_failed]*/
  AZ_ULIB_UCONTRACT(
      AZ_ULIB_UCONTRACT_REQUIRE(
          !AZ_ULIB_USTREAM_IS_NOT_TYPE_OF(ustream_instance, api),
          AZ_ERROR_ARG,
          AZ_ULIB_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING),
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(ustream_instance_clone, AZ_ERROR_ARG),
      AZ_ULIB_UCONTRACT_REQUIRE(
          (offset <= (UINT32_MAX - ustream_instance->length)),
          AZ_ERROR_ARG,
          "offset exceeds max size"));

  /*[az_ulib_ustream_clone_compliance_new_buffer_cloned_with_zero_offset_succeed]*/
  /*[az_ulib_ustream_clone_compliance_new_buffer_cloned_with_offset_succeed]*/
  /*[az_ulib_ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed]*/
  /*[az_ulib_ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_negative_offset_succeed]*/
  /*[az_ulib_ustream_clone_compliance_cloned_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed]*/
  /*[az_ulib_ustream_clone_compliance_no_memory_to_create_instance_failed]*/
  /*[az_ulib_ustream_clone_compliance_empty_buffer_succeed]*/
  ustream_instance_clone->inner_current_position = ustream_instance->inner_current_position;
  ustream_instance_clone->inner_first_valid_position = ustream_instance->inner_current_position;
  ustream_instance_clone->offset_diff = offset - ustream_instance->inner_current_position;
  ustream_instance_clone->control_block = ustream_instance->control_block;
  ustream_instance_clone->length = ustream_instance->length;

  AZ_ULIB_PORT_ATOMIC_INC_W(&(ustream_instance->control_block->ref_count));

  az_ulib_ustream_multi_data_cb* multi_data
      = (az_ulib_ustream_multi_data_cb*)ustream_instance->control_block->ptr;
  AZ_ULIB_PORT_ATOMIC_INC_W(&(multi_data->ustream_one_ref_count));
  AZ_ULIB_PORT_ATOMIC_INC_W(&(multi_data->ustream_two_ref_count));

  return AZ_OK;
}

static az_result concrete_dispose(az_ulib_ustream* ustream_instance)
{
  /*[az_ulib_ustream_dispose_compliance_null_buffer_failed]*/
  /*[az_ulib_ustream_dispose_compliance_buffer_is_not_type_of_buffer_failed]*/
  AZ_ULIB_UCONTRACT(AZ_ULIB_UCONTRACT_REQUIRE(
      !AZ_ULIB_USTREAM_IS_NOT_TYPE_OF(ustream_instance, api),
      AZ_ERROR_ARG,
      AZ_ULIB_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING));

  /*[az_ulib_ustream_dispose_compliance_cloned_instance_disposed_first_succeed]*/
  /*[az_ulib_ustream_dispose_compliance_cloned_instance_disposed_second_succeed]*/
  /*[az_ulib_ustream_dispose_compliance_single_instance_succeed]*/
  az_ulib_ustream_multi_data_cb* multi_data
      = (az_ulib_ustream_multi_data_cb*)ustream_instance->control_block->ptr;
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
  /*[az_ulib_ustream_concat_null_buffer_to_add_failed]*/
  /*[az_ulib_ustream_concat_null_instance_failed]*/
  /*[az_ulib_ustream_concat_null_multi_data_failed]*/
  AZ_ULIB_UCONTRACT(
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(ustream_instance, AZ_ERROR_ARG),
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(ustream_to_concat, AZ_ERROR_ARG),
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(multi_data, AZ_ERROR_ARG));

  az_result result;

  /*[ustream_multi_init_succeed]*/
  ustream_multi_init(ustream_instance, &multi_data->control_block, multi_data, multi_data_release);
  /*[az_ulib_ustream_concat_multiple_buffers_succeed]*/
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
      /*[az_ulib_ustream_concat_new_control_block_failed_on_get_remaining_size_failed]*/
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
  /*[az_ulib_ustream_split_null_instance_failed]*/
  /*[az_ulib_ustream_split_null_split_instance_failed]*/
  AZ_ULIB_UCONTRACT(
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(ustream_instance, AZ_ERROR_ARG),
      AZ_ULIB_UCONTRACT_REQUIRE_NOT_NULL(ustream_instance_split, AZ_ERROR_ARG));

  az_result result;

  offset_t old_position;
  /*[az_ulib_ustream_split_get_position_failed]*/
  if ((result = az_ulib_ustream_get_position(ustream_instance, &old_position)) == AZ_OK)
  {
    /*[az_ulib_ustream_split_position_same_as_current_failed]*/
    if (split_pos == old_position)
    {
      result = AZ_ERROR_ARG;
    }
    else
    {
      size_t ustream_remaining_size;
      /*[az_ulib_ustream_split_get_remaining_size_failed]*/
      if ((result = az_ulib_ustream_get_remaining_size(ustream_instance, &ustream_remaining_size))
          == AZ_OK)
      {
        /*[az_ulib_ustream_split_position_end_of_ustream_failed]*/
        if (old_position + ustream_remaining_size == split_pos)
        {
          result = AZ_ERROR_ARG;
        }
        else
        {
          /*[az_ulib_ustream_split_invalid_split_position_with_offset_failed]*/
          /*[az_ulib_ustream_split_invalid_split_position_with_offset_after_failed]*/
          /*[az_ulib_ustream_split_set_position_failed]*/
          if ((result = az_ulib_ustream_set_position(ustream_instance, split_pos)) == AZ_OK)
          {
            /*[az_ulib_ustream_split_clone_failed]*/
            if ((result
                 = az_ulib_ustream_clone(ustream_instance_split, ustream_instance, split_pos))
                == AZ_OK)
            {
              /*[az_ulib_ustream_split_set_position_second_failed]*/
              if ((result = az_ulib_ustream_set_position(ustream_instance, old_position)) == AZ_OK)
              {
                /*[az_ulib_ustream_split_success]*/
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
