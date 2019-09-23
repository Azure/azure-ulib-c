// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ucontract.h"
#include "ustream.h"
#include "ulib_result.h"
#include "ulib_heap.h"
#include "az_pal_os.h"
#include "az_pal_os_api.h"
#include "ulog.h"

static AZ_ULIB_RESULT concrete_set_position(AZ_USTREAM* ustream_interface, offset_t position);
static AZ_ULIB_RESULT concrete_reset(AZ_USTREAM* ustream_interface);
static AZ_ULIB_RESULT concrete_read(AZ_USTREAM* ustream_interface, uint8_t* const buffer, size_t buffer_length, size_t* const size);
static AZ_ULIB_RESULT concrete_get_remaining_size(AZ_USTREAM* ustream_interface, size_t* const size);
static AZ_ULIB_RESULT concrete_get_position(AZ_USTREAM* ustream_interface, offset_t* const position);
static AZ_ULIB_RESULT concrete_release(AZ_USTREAM* ustream_interface, offset_t position);
static AZ_ULIB_RESULT concrete_clone(AZ_USTREAM* ustream_interface_clone, AZ_USTREAM* ustream_interface, offset_t offset);
static AZ_ULIB_RESULT concrete_dispose(AZ_USTREAM* ustream_interface);
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

static void destroy_instance(AZ_USTREAM* ustream_interface)
{
    AZ_USTREAM_MULTI_DATA_CB* multidata = (AZ_USTREAM_MULTI_DATA_CB*)ustream_interface->control_block->ptr;
    az_pal_os_lock_deinit(&multidata->lock);

    if(ustream_interface->control_block->data_release != NULL)
    {
        ustream_interface->control_block->data_release(ustream_interface->control_block->ptr);
    }
}

static AZ_ULIB_RESULT concrete_set_position(
        AZ_USTREAM* ustream_interface, 
        offset_t position)
{
    /*[az_ustream_set_position_compliance_null_buffer_failed]*/
    /*[az_ustream_set_position_compliance_non_type_of_buffer_api_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, "Passed ustream is not the correct type\r\n"));
    AZ_ULIB_RESULT result;

    offset_t inner_position = position - ustream_interface->offset_diff;

    /*[az_ustream_set_position_compliance_forward_out_of_the_buffer_failed]*/
    /*[az_ustream_set_position_compliance_back_before_first_valid_position_failed]*/
    /*[az_ustream_set_position_compliance_back_before_first_valid_position_with_offset_failed]*/
    if((inner_position > (offset_t)(ustream_interface->length)) ||
       (inner_position < ustream_interface->inner_first_valid_position))
    {
        result = AZ_ULIB_NO_SUCH_ELEMENT_ERROR;
    }
    else
    {
        /*[az_ustream_set_position_compliance_back_to_beginning_succeed]*/
        /*[az_ustream_set_position_compliance_back_position_succeed]*/
        /*[az_ustream_set_position_compliance_forward_position_succeed]*/
        /*[az_ustream_set_position_compliance_forward_to_the_end_position_succeed]*/
        /*[az_ustream_set_position_compliance_run_full_buffer_byte_by_byte_succeed]*/
        /*[az_ustream_set_position_compliance_run_full_buffer_byte_by_byte_reverse_order_succeed]*/
        /*[az_ustream_set_position_compliance_cloned_buffer_back_to_beginning_succeed]*/
        /*[az_ustream_set_position_compliance_cloned_buffer_back_position_succeed]*/
        /*[az_ustream_set_position_compliance_cloned_buffer_forward_position_succeed]*/
        /*[az_ustream_set_position_compliance_cloned_buffer_forward_to_the_end_position_succeed]*/
        /*[az_ustream_set_position_compliance_cloned_buffer_run_full_buffer_byte_by_byte_succeed]*/
        /*[az_ustream_set_position_compliance_cloned_buffer_run_full_buffer_byte_by_byte_reverse_order_succeed]*/
        ustream_interface->inner_current_position = inner_position;
        result = AZ_ULIB_SUCCESS;
    }
    return result;
}

static AZ_ULIB_RESULT concrete_reset(AZ_USTREAM* ustream_interface)
{
    /*[az_ustream_reset_compliance_null_buffer_failed]*/
    /*[az_ustream_reset_compliance_non_type_of_buffer_api_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, "Passed ustream is not the correct type\r\n"));

    /*[az_ustream_reset_compliance_back_to_beginning_succeed]*/
    /*[az_ustream_reset_compliance_back_position_succeed]*/
    /*[az_ustream_reset_compliance_cloned_buffer_succeed]*/
    ustream_interface->inner_current_position = ustream_interface->inner_first_valid_position;
    return AZ_ULIB_SUCCESS;
}

static AZ_ULIB_RESULT concrete_read(
        AZ_USTREAM* ustream_interface,
        uint8_t* const buffer,
        size_t buffer_length,
        size_t* const size)
{
    /*[az_ustream_read_compliance_null_buffer_failed]*/
    /*[az_ustream_read_compliance_non_type_of_buffer_api_failed]*/
    /*[az_ustream_read_compliance_null_return_buffer_failed]*/
    /*[az_ustream_read_compliance_null_return_size_failed]*/
    /*[az_ustream_read_compliance_buffer_with_zero_size_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, "Passed ustream is not the correct type\r\n"),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(buffer, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE_NOT_EQUALS(buffer_length, 0, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(size, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));

    AZ_ULIB_RESULT result;

    AZ_USTREAM_MULTI_DATA_CB* multi_data = (AZ_USTREAM_MULTI_DATA_CB*)ustream_interface->control_block->ptr;
    AZ_USTREAM* current_ustream = (ustream_interface->inner_current_position < multi_data->ustream_one.length) ?
                                                                &multi_data->ustream_one : &multi_data->ustream_two;

    /*[az_ustream_read_compliance_single_buffer_succeed]*/
    /*[az_ustream_read_compliance_right_boundary_condition_succeed]*/
    /*[az_ustream_read_compliance_boundary_condition_succeed]*/
    /*[az_ustream_read_compliance_left_boundary_condition_succeed]*/
    /*[az_ustream_read_compliance_single_byte_succeed]*/
    /*[az_ustream_read_compliance_get_from_cloned_buffer_succeed]*/
    /*[az_ustream_read_compliance_cloned_buffer_right_boundary_condition_succeed]*/
    *size = 0;
    AZ_ULIB_RESULT intermediate_result = AZ_ULIB_SUCCESS;
    while((intermediate_result == AZ_ULIB_SUCCESS) &&
            (*size < buffer_length) &&
            (current_ustream != NULL))
    {
        size_t copied_size;
        size_t remain_size = buffer_length - *size;

        //Critical section to make sure another instance doesn't set_position before this one reads
        az_pal_os_lock_acquire(&multi_data->lock);
        /*[ustream_multi_read_clone_and_original_in_parallel_succeed]*/
        az_ustream_set_position(current_ustream, ustream_interface->inner_current_position + *size);
        intermediate_result = az_ustream_read(current_ustream, &buffer[*size], remain_size, &copied_size);
        az_pal_os_lock_release(&multi_data->lock);

        switch(intermediate_result)
        {
        case AZ_ULIB_SUCCESS:
            *size += copied_size;
        case AZ_ULIB_EOF:
            if(*size < buffer_length)
            {
                if(current_ustream == &multi_data->ustream_one)
                {
                    current_ustream = &multi_data->ustream_two;
                    intermediate_result = AZ_ULIB_SUCCESS;
                }
                else
                {
                    current_ustream = NULL;
                }
            }
            break;
        default:
            /*[ustream_multi_read_control_block_failed_in_read_with_some_valid_content_succeed]*/
            break;
        }
    }

    if(*size != 0)
    {
        /*[az_ustream_concat_read_from_multiple_buffers_succeed]*/
        ustream_interface->inner_current_position += *size;
        result = AZ_ULIB_SUCCESS;
    }
    else
    {
        /*[ustream_multi_read_control_block_failed_in_read_failed]*/
        result = intermediate_result;
    }

    return result;
}

static AZ_ULIB_RESULT concrete_get_remaining_size(AZ_USTREAM* ustream_interface, size_t* const size)
{
    /*[az_ustream_get_remaining_size_compliance_null_buffer_failed]*/
    /*[az_ustream_get_remaining_size_compliance_buffer_is_not_type_of_buffer_failed]*/
    /*[az_ustream_get_remaining_size_compliance_null_size_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, "Passed ustream is not the correct type\r\n"),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(size, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));

    /*[az_ustream_get_remaining_size_compliance_new_buffer_succeed]*/
    /*[az_ustream_get_remaining_size_compliance_new_buffer_with_non_zero_current_position_succeed]*/
    /*[az_ustream_get_remaining_size_compliance_cloned_buffer_with_non_zero_current_position_succeed]*/
    *size = ustream_interface->length - ustream_interface->inner_current_position;

    return AZ_ULIB_SUCCESS;
}

static AZ_ULIB_RESULT concrete_get_position(AZ_USTREAM* ustream_interface, offset_t* const position)
{
    /*[ustream_get_current_position_compliance_null_buffer_failed]*/
    /*[ustream_get_current_position_compliance_buffer_is_not_type_of_buffer_failed]*/
    /*[ustream_get_current_position_compliance_null_position_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, "Passed ustream is not the correct type\r\n"),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(position, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));

    /*[ustream_get_current_position_compliance_new_buffer_succeed]*/
    /*[ustream_get_current_position_compliance_new_buffer_with_non_zero_current_position_succeed]*/
    /*[ustream_get_current_position_compliance_cloned_buffer_with_non_zero_current_position_succeed]*/
    *position = ustream_interface->inner_current_position + ustream_interface->offset_diff;

    return AZ_ULIB_SUCCESS;
}

static AZ_ULIB_RESULT concrete_release(AZ_USTREAM* ustream_interface, offset_t position)
{
    /*[az_ustream_release_compliance_null_buffer_failed]*/
    /*[az_ustream_release_compliance_non_type_of_buffer_api_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, "Passed ustream is not the correct type\r\n"));
    AZ_ULIB_RESULT result;

    offset_t inner_position = position - ustream_interface->offset_diff;

    /*[az_ustream_release_compliance_release_after_current_failed]*/
    /*[az_ustream_release_compliance_release_position_already_released_failed]*/
    if((inner_position >= ustream_interface->inner_current_position) ||
            (inner_position < ustream_interface->inner_first_valid_position))
    {
        result = AZ_ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        /*[az_ustream_release_compliance_succeed]*/
        /*[az_ustream_release_compliance_release_all_succeed]*/
        /*[az_ustream_release_compliance_run_full_buffer_byte_by_byte_succeed]*/
        /*[az_ustream_release_compliance_cloned_buffer_succeed]*/
        /*[az_ustream_release_compliance_cloned_buffer_release_all_succeed]*/
        /*[az_ustream_release_compliance_cloned_buffer_run_full_buffer_byte_by_byte_succeed]*/
        ustream_interface->inner_first_valid_position = inner_position + (offset_t)1;
        result = AZ_ULIB_SUCCESS;
    }

    return result;
}

static AZ_ULIB_RESULT concrete_clone(AZ_USTREAM* ustream_interface_clone, AZ_USTREAM* ustream_interface, offset_t offset)
{
    /*[az_ustream_clone_compliance_null_buffer_failed]*/
    /*[az_ustream_clone_compliance_buffer_is_not_type_of_buffer_failed]*/
    /*[az_ustream_clone_compliance_null_buffer_clone_failed]*/
    /*[az_ustream_clone_compliance_offset_exceed_size_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, "Passed ustream is not the correct type\r\n"),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(ustream_interface_clone, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE((offset <= (UINT32_MAX - ustream_interface->length)), AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, "offset exceeds max size"));

    /*[az_ustream_clone_compliance_new_buffer_cloned_with_zero_offset_succeed]*/
    /*[az_ustream_clone_compliance_new_buffer_cloned_with_offset_succeed]*/
    /*[az_ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed]*/
    /*[az_ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_negative_offset_succeed]*/
    /*[az_ustream_clone_compliance_cloned_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed]*/
    /*[az_ustream_clone_compliance_no_memory_to_create_instance_failed]*/
    /*[az_ustream_clone_compliance_empty_buffer_succeed]*/
    ustream_interface_clone->inner_current_position = ustream_interface->inner_current_position;
    ustream_interface_clone->inner_first_valid_position = ustream_interface->inner_current_position;
    ustream_interface_clone->offset_diff = offset - ustream_interface->inner_current_position;
    ustream_interface_clone->control_block = ustream_interface->control_block;
    ustream_interface_clone->length = ustream_interface->length;

    AZ_ULIB_PORT_ATOMIC_INC_W(&(ustream_interface->control_block->ref_count));

    AZ_USTREAM_MULTI_DATA_CB* multi_data = (AZ_USTREAM_MULTI_DATA_CB*)ustream_interface->control_block->ptr;
    AZ_ULIB_PORT_ATOMIC_INC_W(&(multi_data->ustream_one_ref_count));
    AZ_ULIB_PORT_ATOMIC_INC_W(&(multi_data->ustream_two_ref_count));

    return AZ_ULIB_SUCCESS;
}

static AZ_ULIB_RESULT concrete_dispose(AZ_USTREAM* ustream_interface)
{
    /*[az_ustream_dispose_compliance_null_buffer_failed]*/
    /*[az_ustream_dispose_compliance_buffer_is_not_type_of_buffer_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, "Passed ustream is not the correct type\r\n"));

    /*[az_ustream_dispose_compliance_cloned_instance_disposed_first_succeed]*/
    /*[az_ustream_dispose_compliance_cloned_instance_disposed_second_succeed]*/
    /*[az_ustream_dispose_compliance_single_instance_succeed]*/
    AZ_USTREAM_MULTI_DATA_CB* multi_data = (AZ_USTREAM_MULTI_DATA_CB*)ustream_interface->control_block->ptr;
    AZ_ULIB_PORT_ATOMIC_DEC_W(&(multi_data->ustream_one_ref_count));
    AZ_ULIB_PORT_ATOMIC_DEC_W(&(multi_data->ustream_two_ref_count));
    /*[ustream_multi_dispose_multibuffer_with_buffers_free_all_resources_succeed]*/
    if(multi_data->ustream_one_ref_count == 0 && multi_data->ustream_one.control_block != NULL)
    {
        az_ustream_dispose(&(multi_data->ustream_one));
    }
    if(multi_data->ustream_two_ref_count == 0 && multi_data->ustream_two.control_block != NULL)
    {
        az_ustream_dispose(&(multi_data->ustream_two));
    }

    AZ_USTREAM_DATA_CB* control_block = ustream_interface->control_block;

    AZ_ULIB_PORT_ATOMIC_DEC_W(&(control_block->ref_count));
    if(control_block->ref_count == 0)
    {
        destroy_instance(ustream_interface);
    }

    return AZ_ULIB_SUCCESS;
}

static void az_ustream_multi_init(AZ_USTREAM* ustream_interface, AZ_USTREAM_DATA_CB* control_block,
                                    AZ_USTREAM_MULTI_DATA_CB* multi_data, AZ_RELEASE_CALLBACK multi_data_release)
{
    multi_data->ustream_one.control_block = ustream_interface->control_block;
    multi_data->ustream_one.inner_current_position = ustream_interface->inner_current_position;
    multi_data->ustream_one.inner_first_valid_position = ustream_interface->inner_first_valid_position;
    multi_data->ustream_one.length = ustream_interface->length;
    multi_data->ustream_one.offset_diff = ustream_interface->offset_diff;
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

    ustream_interface->control_block = control_block;
}

AZ_ULIB_RESULT az_ustream_concat(
    AZ_USTREAM* ustream_interface,
    AZ_USTREAM* ustream_to_concat,
    AZ_USTREAM_MULTI_DATA_CB* multi_data,
    AZ_RELEASE_CALLBACK multi_data_release)
{
    /*[az_ustream_concat_null_buffer_to_add_failed]*/
    /*[az_ustream_concat_null_interface_failed]*/
    /*[az_ustream_concat_null_multi_data_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE_NOT_NULL(ustream_interface, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(ustream_to_concat, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(multi_data, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));

    AZ_ULIB_RESULT result;

    /*[az_ustream_multi_init_succeed]*/
    az_ustream_multi_init(ustream_interface, &multi_data->control_block, multi_data, multi_data_release);
    /*[az_ustream_concat_multiple_buffers_succeed]*/
    if((result = az_ustream_clone(&multi_data->ustream_two, ustream_to_concat, ustream_interface->length)) == AZ_ULIB_SUCCESS)
    {
        size_t remaining_size;
        if((result = az_ustream_get_remaining_size(&(multi_data->ustream_two), &remaining_size)) == AZ_ULIB_SUCCESS)
        {
            ustream_interface->length += remaining_size;
            AZ_ULIB_PORT_ATOMIC_INC_W(&(multi_data->ustream_two_ref_count));
        }
        else
        {
            /*[az_ustream_concat_new_control_block_failed_on_get_remaining_size_failed]*/
            az_ustream_dispose(&(multi_data->ustream_two));
        }
    }

    return result;
}
