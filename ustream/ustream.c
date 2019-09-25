// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ucontract.h"
#include "ustream.h"
#include "ulib_result.h"
#include "ulib_port.h"
#include "ulog.h"

static AZ_ULIB_RESULT concrete_set_position(AZ_USTREAM* ustream_instance, offset_t position);
static AZ_ULIB_RESULT concrete_reset(AZ_USTREAM* ustream_instance);
static AZ_ULIB_RESULT concrete_read(AZ_USTREAM* ustream_instance, uint8_t* const buffer, size_t buffer_length, size_t* const size);
static AZ_ULIB_RESULT concrete_get_remaining_size(AZ_USTREAM* ustream_instance, size_t* const size);
static AZ_ULIB_RESULT concrete_get_position(AZ_USTREAM* ustream_instance, offset_t* const position);
static AZ_ULIB_RESULT concrete_release(AZ_USTREAM* ustream_instance, offset_t position);
static AZ_ULIB_RESULT concrete_clone(AZ_USTREAM* ustream_instance_clone, AZ_USTREAM* ustream_instance, offset_t offset);
static AZ_ULIB_RESULT concrete_dispose(AZ_USTREAM* ustream_instance);
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

static void init_instance(
    AZ_USTREAM* ustream_instance,
    AZ_USTREAM_DATA_CB* control_block,
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

static void destroy_control_block(AZ_USTREAM_DATA_CB* control_block)
{
    if(control_block->data_release)
    {
        control_block->data_release(control_block->ptr);
    }
    if(control_block->control_block_release)
    {
        control_block->control_block_release(control_block);
    }
}

static AZ_ULIB_RESULT concrete_set_position(AZ_USTREAM* ustream_instance, offset_t position)
{
    /*[az_ustream_set_position_compliance_null_buffer_failed]*/
    /*[az_ustream_set_position_compliance_non_type_of_buffer_api_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_instance, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, AZ_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING));
    AZ_ULIB_RESULT result;

    offset_t inner_position = position - ustream_instance->offset_diff;

    if((inner_position > (offset_t)(ustream_instance->length)) || 
            (inner_position < ustream_instance->inner_first_valid_position))
    {
        /*[az_ustream_set_position_compliance_forward_out_of_the_buffer_failed]*/
        /*[az_ustream_set_position_compliance_back_before_first_valid_position_failed]*/
        /*[az_ustream_set_position_compliance_back_before_first_valid_position_with_offset_failed]*/
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
        ustream_instance->inner_current_position = inner_position;
        result = AZ_ULIB_SUCCESS;
    }

    return result;
}

static AZ_ULIB_RESULT concrete_reset(AZ_USTREAM* ustream_instance)
{
    /*[az_ustream_reset_compliance_null_buffer_failed]*/
    /*[az_ustream_reset_compliance_non_type_of_buffer_api_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_instance, api), 
                                AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, AZ_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING));
    /*[az_ustream_reset_compliance_back_to_beginning_succeed]*/
    /*[az_ustream_reset_compliance_back_position_succeed]*/
    /*[az_ustream_reset_compliance_cloned_buffer_succeed]*/
    ustream_instance->inner_current_position = ustream_instance->inner_first_valid_position;

    return AZ_ULIB_SUCCESS;
}

static AZ_ULIB_RESULT concrete_read(
        AZ_USTREAM* ustream_instance,
        uint8_t* const buffer,
        size_t buffer_length,
        size_t* const size)
{
    /*[az_ustream_read_compliance_null_buffer_failed]*/
    /*[az_ustream_read_compliance_non_type_of_buffer_api_failed]*/
    /*[az_ustream_read_compliance_null_return_buffer_failed]*/
    /*[az_ustream_read_compliance_null_return_size_failed]*/
    /*[az_ustream_read_compliance_buffer_with_zero_size_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_instance, api), 
                                    AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, AZ_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(buffer, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE_NOT_EQUALS(buffer_length, 0, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(size, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));

    AZ_ULIB_RESULT result;

    AZ_USTREAM_DATA_CB* control_block = ustream_instance->control_block;

    if(ustream_instance->inner_current_position >= ustream_instance->length)
    {
        /*[az_ustream_read_compliance_succeed_3]*/
        *size = 0;
        result = AZ_ULIB_EOF;
    }
    else
    {
        /*[az_ustream_read_compliance_succeed_2]*/
        size_t remain_size = ustream_instance->length - (size_t)ustream_instance->inner_current_position;
        *size = (buffer_length < remain_size) ? buffer_length : remain_size;
        /*[az_ustream_read_compliance_succeed_1]*/
        /*[az_ustream_read_compliance_single_buffer_succeed]*/
        /*[az_ustream_read_compliance_right_boundary_condition_succeed]*/
        /*[az_ustream_read_compliance_boundary_condition_succeed]*/
        /*[az_ustream_read_compliance_left_boundary_condition_succeed]*/
        /*[az_ustream_read_compliance_single_byte_succeed]*/
        /*[az_ustream_read_compliance_get_from_cloned_buffer_succeed]*/
        /*[az_ustream_read_compliance_cloned_buffer_right_boundary_condition_succeed]*/
        (void)memcpy(buffer, (uint8_t*)control_block->ptr + ustream_instance->inner_current_position, *size);
        ustream_instance->inner_current_position += *size;
        result = AZ_ULIB_SUCCESS;
    }

    return result;
}

static AZ_ULIB_RESULT concrete_get_remaining_size(AZ_USTREAM* ustream_instance, size_t* const size)
{
    /*[az_ustream_get_remaining_size_compliance_null_buffer_failed]*/
    /*[az_ustream_get_remaining_size_compliance_buffer_is_not_type_of_buffer_failed]*/
    /*[az_ustream_get_remaining_size_compliance_null_size_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_instance, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, AZ_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(size, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
    /*[az_ustream_get_remaining_size_compliance_new_buffer_succeed]*/
    /*[az_ustream_get_remaining_size_compliance_new_buffer_with_non_zero_current_position_succeed]*/
    /*[az_ustream_get_remaining_size_compliance_cloned_buffer_with_non_zero_current_position_succeed]*/
    *size = ustream_instance->length - ustream_instance->inner_current_position;

    return AZ_ULIB_SUCCESS;
}

static AZ_ULIB_RESULT concrete_get_position(AZ_USTREAM* ustream_instance, offset_t* const position)
{
    /*[az_ustream_get_current_position_compliance_null_buffer_failed]*/
    /*[az_ustream_get_current_position_compliance_buffer_is_not_type_of_buffer_failed]*/
    /*[az_ustream_get_current_position_compliance_null_position_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_instance, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, AZ_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(position, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));
    /*[az_ustream_get_current_position_compliance_new_buffer_succeed]*/
    /*[az_ustream_get_current_position_compliance_new_buffer_with_non_zero_current_position_succeed]*/
    /*[az_ustream_get_current_position_compliance_cloned_buffer_with_non_zero_current_position_succeed]*/
    *position = ustream_instance->inner_current_position + ustream_instance->offset_diff;

    return AZ_ULIB_SUCCESS;
}

static AZ_ULIB_RESULT concrete_release(AZ_USTREAM* ustream_instance, offset_t position)
{
    /*[az_ustream_release_compliance_null_buffer_failed]*/
    /*[az_ustream_release_compliance_non_type_of_buffer_api_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_instance, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, AZ_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING));

    AZ_ULIB_RESULT result;

    offset_t inner_position = position - ustream_instance->offset_diff;

    if((inner_position >= ustream_instance->inner_current_position) ||
            (inner_position < ustream_instance->inner_first_valid_position))
    {
        /*[az_ustream_release_compliance_release_after_current_failed]*/
        /*[az_ustream_release_compliance_release_position_already_released_failed]*/
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
        ustream_instance->inner_first_valid_position = inner_position + (offset_t)1;
        result = AZ_ULIB_SUCCESS;
    }

    return result;
}

static AZ_ULIB_RESULT concrete_clone(AZ_USTREAM* ustream_instance_clone, AZ_USTREAM* ustream_instance, offset_t offset)
{
    /*[az_ustream_clone_compliance_null_buffer_failed]*/
    /*[az_ustream_clone_compliance_buffer_is_not_type_of_buffer_failed]*/
    /*[az_ustream_clone_compliance_null_buffer_clone_failed]*/
    /*[az_ustream_clone_compliance_offset_exceed_size_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_instance, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, AZ_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(ustream_instance_clone, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE((offset <= (UINT32_MAX - ustream_instance->length)), AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, "offset exceeds max size"));
    /*[az_ustream_clone_compliance_new_buffer_cloned_with_zero_offset_succeed]*/
    /*[az_ustream_clone_compliance_new_buffer_cloned_with_offset_succeed]*/
    /*[az_ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed]*/
    /*[az_ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_negative_offset_succeed]*/
    /*[az_ustream_clone_compliance_cloned_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed]*/
    /*[az_ustream_clone_compliance_no_memory_to_create_instance_failed]*/
    /*[az_ustream_clone_compliance_empty_buffer_succeed]*/
    init_instance(ustream_instance_clone, ustream_instance->control_block, ustream_instance->inner_current_position, offset, 
                                                            ustream_instance->length);

    return AZ_ULIB_SUCCESS;
}

static AZ_ULIB_RESULT concrete_dispose(AZ_USTREAM* ustream_instance)
{
    /*[az_ustream_dispose_compliance_null_buffer_failed]*/
    /*[az_ustream_dispose_compliance_buffer_is_not_type_of_buffer_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_instance, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, AZ_ULOG_USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING));

    AZ_USTREAM_DATA_CB* control_block = ustream_instance->control_block;

    /*[az_ustream_dispose_compliance_cloned_instance_disposed_first_succeed]*/
    /*[az_ustream_dispose_compliance_cloned_instance_disposed_second_succeed]*/
    /*[az_ustream_dispose_compliance_single_instance_succeed]*/
    AZ_ULIB_PORT_ATOMIC_DEC_W(&(control_block->ref_count));
    if(control_block->ref_count == 0)
    {
        destroy_control_block(control_block);
    }

    return AZ_ULIB_SUCCESS;
}

AZ_ULIB_RESULT az_ustream_init(
    AZ_USTREAM* ustream_instance,
    AZ_USTREAM_DATA_CB* ustream_control_block,
    AZ_RELEASE_CALLBACK control_block_release,
    const uint8_t* const data_buffer,
    size_t data_buffer_length,
    AZ_RELEASE_CALLBACK data_buffer_release)
{
    /*[az_ustream_init_NULL_ustream_instance_failed]*/
    /*[az_ustream_init_NULL_control_block_failed]*/
    /*[az_ustream_init_null_buffer_failed]*/
    /*[az_ustream_init_zero_length_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE_NOT_NULL(ustream_instance, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(ustream_control_block, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(data_buffer, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE_NOT_EQUALS(data_buffer_length, 0, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));

    /*[az_ustream_init_succeed]*/
    /*[az_ustream_init_const_succeed*/
    ustream_control_block->api = &api;
    ustream_control_block->ptr = (void*)data_buffer;
    ustream_control_block->ref_count = 0;
    ustream_control_block->data_release = data_buffer_release;
    ustream_control_block->control_block_release = control_block_release;

    init_instance(ustream_instance, ustream_control_block, 0, 0, data_buffer_length);

    return AZ_ULIB_SUCCESS;
}
