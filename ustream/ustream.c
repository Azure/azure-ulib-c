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
#include "ulib_heap.h"
#include "ulog.h"

static const char *const USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING = "Passed ustream is not the correct type\r\n";

static AZ_ULIB_RESULT concrete_set_position(AZ_USTREAM* ustream_interface, offset_t position);
static AZ_ULIB_RESULT concrete_reset(AZ_USTREAM* ustream_interface);
static AZ_ULIB_RESULT concrete_read(AZ_USTREAM* ustream_interface, uint8_t* const buffer, size_t buffer_length, size_t* const size);
static AZ_ULIB_RESULT concrete_get_remaining_size(AZ_USTREAM* ustream_interface, size_t* const size);
static AZ_ULIB_RESULT concrete_get_position(AZ_USTREAM* ustream_interface, offset_t* const position);
static AZ_ULIB_RESULT concrete_release(AZ_USTREAM* ustream_interface, offset_t position);
static AZ_ULIB_RESULT concrete_clone(AZ_USTREAM* ustream_interface, AZ_USTREAM* ustream_interface_to_clone, offset_t offset);
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

static void init_instance(
    AZ_USTREAM* ustream_instance,
    AZ_USTREAM_DATA_CB* inner_buffer,
    offset_t inner_current_position,
    offset_t offset,
    size_t data_buffer_length)
{
    ustream_instance->inner_current_position = inner_current_position;
    ustream_instance->inner_first_valid_position = inner_current_position;
    ustream_instance->offset_diff = offset - inner_current_position;
    ustream_instance->inner_buffer = inner_buffer;
    ustream_instance->length = data_buffer_length;
    AZ_ULIB_PORT_ATOMIC_INC_W(&(ustream_instance->inner_buffer->ref_count));
}

static void destroy_inner_buffer(AZ_USTREAM_DATA_CB* inner_buffer)
{
    if(inner_buffer->data_release)
    {
        inner_buffer->data_release(inner_buffer->ptr);
    }
    if(inner_buffer->inner_buffer_release)
    {
        inner_buffer->inner_buffer_release(inner_buffer);
    }
}

static AZ_ULIB_RESULT concrete_set_position(AZ_USTREAM* ustream_interface, offset_t position)
{
    /*[az_ustream_set_position_compliance_null_buffer_failed]*/
    /*[az_ustream_set_position_compliance_non_type_of_buffer_api_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING));
    AZ_ULIB_RESULT result;

    offset_t inner_position = position - ustream_interface->offset_diff;

    if((inner_position > (offset_t)(ustream_interface->length)) || 
            (inner_position < ustream_interface->inner_first_valid_position))
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
                                AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING));
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
                                    AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(buffer, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE_NOT_EQUALS(buffer_length, 0, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(size, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));

    AZ_ULIB_RESULT result;

    AZ_USTREAM_DATA_CB* inner_buffer = ustream_interface->inner_buffer;

    if(ustream_interface->inner_current_position >= ustream_interface->length)
    {
        /*[az_ustream_read_compliance_succeed_3]*/
        *size = 0;
        result = AZ_ULIB_EOF;
    }
    else
    {
        /*[az_ustream_read_compliance_succeed_2]*/
        size_t remain_size = ustream_interface->length - (size_t)ustream_interface->inner_current_position;
        *size = (buffer_length < remain_size) ? buffer_length : remain_size;
        /*[az_ustream_read_compliance_succeed_1]*/
        /*[az_ustream_read_compliance_single_buffer_succeed]*/
        /*[az_ustream_read_compliance_right_boundary_condition_succeed]*/
        /*[az_ustream_read_compliance_boundary_condition_succeed]*/
        /*[az_ustream_read_compliance_left_boundary_condition_succeed]*/
        /*[az_ustream_read_compliance_single_byte_succeed]*/
        /*[az_ustream_read_compliance_get_from_cloned_buffer_succeed]*/
        /*[az_ustream_read_compliance_cloned_buffer_right_boundary_condition_succeed]*/
        (void)memcpy(buffer, (uint8_t*)inner_buffer->ptr + ustream_interface->inner_current_position, *size);
        ustream_interface->inner_current_position += *size;
        result = AZ_ULIB_SUCCESS;
    }

    return result;
}

static AZ_ULIB_RESULT concrete_get_remaining_size(AZ_USTREAM* ustream_interface, size_t* const size)
{
    /*[az_ustream_get_remaining_size_compliance_null_buffer_failed]*/
    /*[az_ustream_get_remaining_size_compliance_buffer_is_not_type_of_buffer_failed]*/
    /*[az_ustream_get_remaining_size_compliance_null_size_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING),
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
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING),
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
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING));

    AZ_ULIB_RESULT result;

    offset_t inner_position = position - ustream_interface->offset_diff;

    if((inner_position >= ustream_interface->inner_current_position) ||
            (inner_position < ustream_interface->inner_first_valid_position))
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
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(ustream_interface_clone, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE((offset <= (UINT32_MAX - ustream_interface->length)), AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, "offset exceeds max size"));
    /*[az_ustream_clone_compliance_new_buffer_cloned_with_zero_offset_succeed]*/
    /*[az_ustream_clone_compliance_new_buffer_cloned_with_offset_succeed]*/
    /*[az_ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed]*/
    /*[az_ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_negative_offset_succeed]*/
    /*[az_ustream_clone_compliance_cloned_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed]*/
    /*[az_ustream_clone_compliance_no_memory_to_create_instance_failed]*/
    /*[az_ustream_clone_compliance_empty_buffer_succeed]*/
    init_instance(ustream_interface_clone, ustream_interface->inner_buffer, ustream_interface->inner_current_position, offset, 
                                                            ustream_interface->length);

    return AZ_ULIB_SUCCESS;
}

static AZ_ULIB_RESULT concrete_dispose(AZ_USTREAM* ustream_interface)
{
    /*[az_ustream_dispose_compliance_null_buffer_failed]*/
    /*[az_ustream_dispose_compliance_buffer_is_not_type_of_buffer_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE(!AZ_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZ_ULIB_ILLEGAL_ARGUMENT_ERROR, USTREAM_ILLEGAL_ARGUMENT_ERROR_STRING));

    AZ_USTREAM_DATA_CB* inner_buffer = ustream_interface->inner_buffer;

    /*[az_ustream_dispose_compliance_cloned_instance_disposed_first_succeed]*/
    /*[az_ustream_dispose_compliance_cloned_instance_disposed_second_succeed]*/
    /*[az_ustream_dispose_compliance_single_instance_succeed]*/
    AZ_ULIB_PORT_ATOMIC_DEC_W(&(inner_buffer->ref_count));
    if(inner_buffer->ref_count == 0)
    {
        destroy_inner_buffer(inner_buffer);
    }

    return AZ_ULIB_SUCCESS;
}

AZ_ULIB_RESULT az_ustream_init(
    AZ_USTREAM* ustream_instance,
    AZ_USTREAM_DATA_CB* ustream_inner_buffer,
    AZ_RELEASE_CALLBACK inner_buffer_release,
    const uint8_t* const data_buffer,
    size_t data_buffer_length,
    AZ_RELEASE_CALLBACK data_buffer_release)
{
    /*[az_ustream_init_NULL_ustream_instance_failed]*/
    /*[az_ustream_init_NULL_inner_buffer_failed]*/
    /*[az_ustream_init_null_buffer_failed]*/
    /*[az_ustream_init_zero_length_failed]*/
    AZ_UCONTRACT(AZ_UCONTRACT_REQUIRE_NOT_NULL(ustream_instance, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(ustream_inner_buffer, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE_NOT_NULL(data_buffer, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZ_UCONTRACT_REQUIRE_NOT_EQUALS(data_buffer_length, 0, AZ_ULIB_ILLEGAL_ARGUMENT_ERROR));

    /*[az_ustream_init_succeed]*/
    /*[az_ustream_init_const_succeed*/
    ustream_inner_buffer->api = &api;
    ustream_inner_buffer->ptr = (void*)data_buffer;
    ustream_inner_buffer->ref_count = 0;
    ustream_inner_buffer->data_release = data_buffer_release;
    ustream_inner_buffer->inner_buffer_release = inner_buffer_release;

    init_instance(ustream_instance, ustream_inner_buffer, 0, 0, data_buffer_length);

    return AZ_ULIB_SUCCESS;
}
