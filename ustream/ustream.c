// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ustream.h"
#include "ulib_port.h"
#include "ulib_heap.h"
#include "ulog.h"


typedef struct USTREAM_INNER_BUFFER_TAG
{
    uint8_t* ptr;
    size_t length;
    volatile uint32_t ref_count;
    AZIOT_USTREAM_BUFFER_RELEASE_CALLBACK inner_free;
} USTREAM_INNER_BUFFER;

typedef struct USTREAM_INSTANCE_TAG
{
    /* Inner buffer. */
    USTREAM_INNER_BUFFER* inner_buffer;

    /* Instance controls. */
    offset_t offset_diff;
    offset_t inner_current_position;
    offset_t inner_first_valid_position;
} USTREAM_INSTANCE;

static AZIOT_ULIB_RESULT concrete_set_position(AZIOT_USTREAM* ustream_interface, offset_t position);
static AZIOT_ULIB_RESULT concrete_reset(AZIOT_USTREAM* ustream_interface);
static AZIOT_ULIB_RESULT concrete_read(AZIOT_USTREAM* ustream_interface, uint8_t* const buffer, size_t buffer_length, size_t* const size);
static AZIOT_ULIB_RESULT concrete_get_remaining_size(AZIOT_USTREAM* ustream_interface, size_t* const size);
static AZIOT_ULIB_RESULT concrete_get_position(AZIOT_USTREAM* ustream_interface, offset_t* const position);
static AZIOT_ULIB_RESULT concrete_release(AZIOT_USTREAM* ustream_interface, offset_t position);
static AZIOT_USTREAM* concrete_clone(AZIOT_USTREAM* ustream_interface, offset_t offset);
static AZIOT_ULIB_RESULT concrete_dispose(AZIOT_USTREAM* ustream_interface);
static const AZIOT_USTREAM_INTERFACE api =
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

static AZIOT_USTREAM* create_instance(
    USTREAM_INNER_BUFFER* inner_buffer,
    offset_t inner_current_position,
    offset_t offset)
{
    AZIOT_USTREAM* ustream_interface = (AZIOT_USTREAM*)ULIB_CONFIG_MALLOC(sizeof(AZIOT_USTREAM));
    /*[ustream_create_no_memory_to_create_interface_failed]*/
    /*[ustream_clone_no_memory_to_create_interface_failed]*/
    /*[ustream_clone_no_memory_to_create_instance_failed]*/
    if(ustream_interface == NULL)
    {
        ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_OUT_OF_MEMORY_STRING, "ustream_interface");
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ULIB_CONFIG_MALLOC(sizeof(USTREAM_INSTANCE));
        if(instance != NULL)
        {
            ustream_interface->api = &api;
            ustream_interface->handle = (void*)instance;

            instance->inner_current_position = inner_current_position;
            instance->inner_first_valid_position = inner_current_position;
            instance->offset_diff = offset - inner_current_position;
            instance->inner_buffer = inner_buffer;
            ULIB_PORT_ATOMIC_INC_W(&(instance->inner_buffer->ref_count));
        }
        else
        {
            ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_OUT_OF_MEMORY_STRING, "ustream_instance");
            ULIB_CONFIG_FREE(ustream_interface);
            ustream_interface = NULL;
        }
    }

    return ustream_interface;
}

static USTREAM_INNER_BUFFER* create_inner_buffer(
    const uint8_t* const buffer, 
    size_t buffer_length,
    AZIOT_USTREAM_BUFFER_RELEASE_CALLBACK inner_free)
{
    USTREAM_INNER_BUFFER* inner_buffer;

    if((inner_buffer = (USTREAM_INNER_BUFFER*)ULIB_CONFIG_MALLOC(sizeof(USTREAM_INNER_BUFFER))) == NULL)
    {
        ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_OUT_OF_MEMORY_STRING, "inner buffer control");
    }
    else
    {
        inner_buffer->ptr = (uint8_t*)buffer;
        inner_buffer->length = buffer_length;
        inner_buffer->ref_count = 0;
        inner_buffer->inner_free = inner_free;
    }

    return inner_buffer;
}

static void destroy_inner_buffer(USTREAM_INNER_BUFFER* inner_buffer)
{
    if(inner_buffer->inner_free != NULL)
    {
        inner_buffer->inner_free(inner_buffer->ptr);
    }
    ULIB_CONFIG_FREE(inner_buffer);
}

static AZIOT_ULIB_RESULT concrete_set_position(AZIOT_USTREAM* ustream_interface, offset_t position)
{
    AZIOT_ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[aziot_ustream_set_position_compliance_null_buffer_failed]*/
        /*[aziot_ustream_set_position_compliance_non_type_of_buffer_api_failed]*/
        ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ustream_interface->handle;
        USTREAM_INNER_BUFFER* inner_buffer = instance->inner_buffer;
        offset_t inner_position = position - instance->offset_diff;

        if((inner_position > (offset_t)(inner_buffer->length)) || 
                (inner_position < instance->inner_first_valid_position))
        {
            /*[aziot_ustream_set_position_compliance_forward_out_of_the_buffer_failed]*/
            /*[aziot_ustream_set_position_compliance_back_before_first_valid_position_failed]*/
            result = AZIOT_ULIB_NO_SUCH_ELEMENT_ERROR;
        }
        else
        {
            /*[aziot_ustream_set_position_compliance_back_to_beginning_succeed]*/
            /*[aziot_ustream_set_position_compliance_back_position_succeed]*/
            /*[aziot_ustream_set_position_compliance_forward_position_succeed]*/
            /*[aziot_ustream_set_position_compliance_forward_to_the_end_position_succeed]*/
            /*[aziot_ustream_set_position_compliance_run_full_buffer_byte_by_byte_succeed]*/
            /*[aziot_ustream_set_position_compliance_run_full_buffer_byte_by_byte_reverse_order_succeed]*/
            /*[aziot_ustream_set_position_compliance_cloned_buffer_back_to_beginning_succeed]*/
            /*[aziot_ustream_set_position_compliance_cloned_buffer_back_position_succeed]*/
            /*[aziot_ustream_set_position_compliance_cloned_buffer_forward_position_succeed]*/
            /*[aziot_ustream_set_position_compliance_cloned_buffer_forward_to_the_end_position_succeed]*/
            /*[aziot_ustream_set_position_compliance_cloned_buffer_run_full_buffer_byte_by_byte_succeed]*/
            /*[aziot_ustream_set_position_compliance_cloned_buffer_run_full_buffer_byte_by_byte_reverse_order_succeed]*/
            instance->inner_current_position = inner_position;
            result = AZIOT_ULIB_SUCCESS;
        }
    }

    return result;
}

static AZIOT_ULIB_RESULT concrete_reset(AZIOT_USTREAM* ustream_interface)
{
    AZIOT_ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[aziot_ustream_reset_compliance_null_buffer_failed]*/
        /*[aziot_ustream_reset_compliance_non_type_of_buffer_api_failed]*/

        result = AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ustream_interface->handle;

        /*[aziot_ustream_reset_compliance_back_to_beginning_succeed]*/
        /*[aziot_ustream_reset_compliance_back_position_succeed]*/
        instance->inner_current_position = instance->inner_first_valid_position;
        result = AZIOT_ULIB_SUCCESS;
    }

    return result;
}

static AZIOT_ULIB_RESULT concrete_read(
        AZIOT_USTREAM* ustream_interface,
        uint8_t* const buffer,
        size_t buffer_length,
        size_t* const size)
{
    AZIOT_ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[ustream_read_compliance_null_buffer_failed]*/
        /*[ustream_read_compliance_non_type_of_buffer_api_failed]*/
        ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if((buffer == NULL) || (size == NULL))
    {
        /*[ustream_read_compliance_null_return_buffer_failed]*/
        /*[ustream_read_compliance_null_return_size_failed]*/
        ULIB_CONFIG_LOG(
            AZIOT_ULOG_TYPE_ERROR,
            AZIOT_ULOG_REQUIRE_NOT_NULL_STRING, 
            (buffer == NULL ? "buffer" : "size"));
        result = AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if(buffer_length == 0)
    {
        /*[ustream_read_compliance_buffer_with_zero_size_failed]*/
        ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_NOT_EQUALS_STRING, "buffer_length", "0");
        result = AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ustream_interface->handle;
        USTREAM_INNER_BUFFER* inner_buffer = instance->inner_buffer;

        if(instance->inner_current_position >= inner_buffer->length)
        {
            /*[ustream_read_compliance_succeed_3]*/
            *size = 0;
            result = AZIOT_ULIB_EOF;
        }
        else
        {
            /*[ustream_read_compliance_succeed_2]*/
            size_t remain_size = inner_buffer->length - (size_t)instance->inner_current_position;
            *size = (buffer_length < remain_size) ? buffer_length : remain_size;
            /*[ustream_read_compliance_succeed_1]*/
            /*[ustream_read_compliance_single_buffer_succeed]*/
            /*[ustream_read_compliance_right_boundary_condition_succeed]*/
            /*[ustream_read_compliance_boundary_condition_succeed]*/
            /*[ustream_read_compliance_left_boundary_condition_succeed]*/
            /*[ustream_read_compliance_single_byte_succeed]*/
            /*[ustream_read_compliance_get_from_cloned_buffer_succeed]*/
            /*[ustream_read_compliance_cloned_buffer_right_boundary_condition_succeed]*/
            (void)memcpy(buffer, inner_buffer->ptr + instance->inner_current_position, *size);
            instance->inner_current_position += *size;
            result = AZIOT_ULIB_SUCCESS;
        }
    }

    return result;
}

static AZIOT_ULIB_RESULT concrete_get_remaining_size(AZIOT_USTREAM* ustream_interface, size_t* const size)
{
    AZIOT_ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[ustream_get_remaining_size_compliance_null_buffer_failed]*/
        /*[ustream_get_remaining_size_compliance_buffer_is_not_type_of_buffer_failed]*/
        ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if(size == NULL)
    {
        /*[ustream_get_remaining_size_compliance_null_size_failed]*/
        ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_NOT_NULL_STRING, "size");
        result = AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ustream_interface->handle;

        /*[ustream_get_remaining_size_compliance_new_buffer_succeed]*/
        /*[ustream_get_remaining_size_compliance_new_buffer_with_non_zero_current_position_succeed]*/
        /*[ustream_get_remaining_size_compliance_cloned_buffer_with_non_zero_current_position_succeed]*/
        *size = instance->inner_buffer->length - instance->inner_current_position;
        result = AZIOT_ULIB_SUCCESS;
    }

    return result;
}

static AZIOT_ULIB_RESULT concrete_get_position(AZIOT_USTREAM* ustream_interface, offset_t* const position)
{
    AZIOT_ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[ustream_get_current_position_compliance_null_buffer_failed]*/
        /*[ustream_get_current_position_compliance_buffer_is_not_type_of_buffer_failed]*/
        ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if(position == NULL)
    {
        /*[ustream_get_current_position_compliance_null_position_failed]*/
        ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_NOT_NULL_STRING, "position");
        result = AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ustream_interface->handle;

        /*[ustream_get_current_position_compliance_new_buffer_succeed]*/
        /*[ustream_get_current_position_compliance_new_buffer_with_non_zero_current_position_succeed]*/
        /*[ustream_get_current_position_compliance_cloned_buffer_with_non_zero_current_position_succeed]*/
        *position = instance->inner_current_position + instance->offset_diff;
        result = AZIOT_ULIB_SUCCESS;
    }

    return result;
}

static AZIOT_ULIB_RESULT concrete_release(AZIOT_USTREAM* ustream_interface, offset_t position)
{
    AZIOT_ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[ustream_release_compliance_null_buffer_failed]*/
        /*[ustream_release_compliance_non_type_of_buffer_api_failed]*/
        ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ustream_interface->handle;
        offset_t inner_position = position - instance->offset_diff;

        if((inner_position >= instance->inner_current_position) ||
                (inner_position < instance->inner_first_valid_position))
        {
            /*[ustream_release_compliance_release_after_current_failed]*/
            /*[ustream_release_compliance_release_position_already_released_failed]*/
            result = AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR;
        }
        else
        {
            /*[ustream_release_compliance_succeed]*/
            /*[ustream_release_compliance_release_all_succeed]*/
            /*[ustream_release_compliance_run_full_buffer_byte_by_byte_succeed]*/
            /*[ustream_release_compliance_cloned_buffer_succeed]*/
            /*[ustream_release_compliance_cloned_buffer_release_all_succeed]*/
            /*[ustream_release_compliance_cloned_buffer_run_full_buffer_byte_by_byte_succeed]*/
            instance->inner_first_valid_position = inner_position + (offset_t)1;
            result = AZIOT_ULIB_SUCCESS;
        }
    }

    return result;
}

static AZIOT_USTREAM* concrete_clone(AZIOT_USTREAM* ustream_interface, offset_t offset)
{
    AZIOT_USTREAM* interface_result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[ustream_clone_compliance_null_buffer_failed]*/
        /*[ustream_clone_compliance_buffer_is_not_type_of_buffer_failed]*/
        ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        interface_result = NULL;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ustream_interface->handle;

        if(offset > (UINT32_MAX - instance->inner_buffer->length))
        {
            /*[ustream_clone_compliance_offset_exceed_size_failed]*/
            interface_result = NULL;
        }
        else
        {
            /*[ustream_clone_compliance_new_buffer_cloned_with_zero_offset_succeed]*/
            /*[ustream_clone_compliance_new_buffer_cloned_with_offset_succeed]*/
            /*[ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed]*/
            /*[ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_negative_offset_succeed]*/
            /*[ustream_clone_compliance_cloned_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed]*/
            /*[ustream_clone_compliance_no_memory_to_create_instance_failed]*/
            /*[ustream_clone_compliance_empty_buffer_succeed]*/
            interface_result = create_instance(instance->inner_buffer, instance->inner_current_position, offset);
        }
    }

    return interface_result;
}

static AZIOT_ULIB_RESULT concrete_dispose(AZIOT_USTREAM* ustream_interface)
{
    AZIOT_ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[ustream_dispose_compliance_null_buffer_failed]*/
        /*[ustream_dispose_compliance_buffer_is_not_type_of_buffer_failed]*/
        ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ustream_interface->handle;
        USTREAM_INNER_BUFFER* inner_buffer = instance->inner_buffer;

        /*[ustream_dispose_compliance_cloned_instance_disposed_first_succeed]*/
        /*[ustream_dispose_compliance_cloned_instance_disposed_second_succeed]*/
        /*[ustream_dispose_compliance_single_instance_succeed]*/
        ULIB_PORT_ATOMIC_DEC_W(&(inner_buffer->ref_count));
        if(inner_buffer->ref_count == 0)
        {
            destroy_inner_buffer(inner_buffer);
        }
        ULIB_CONFIG_FREE(instance);
        ULIB_CONFIG_FREE(ustream_interface);
        result = AZIOT_ULIB_SUCCESS;
    }

    return result;
}

AZIOT_USTREAM* ustream_create(
        const uint8_t* const buffer, 
        size_t buffer_length,
        AZIOT_USTREAM_BUFFER_RELEASE_CALLBACK inner_free)
{
    AZIOT_USTREAM* interface_result;

    if(buffer == NULL)
    {
        /*[ustream_create_null_buffer_failed]*/
        ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_NOT_NULL_STRING, "buffer");
        interface_result = NULL;
    }
    else if(buffer_length == 0)
    {
        /*[ustream_create_zero_length_failed]*/
        ULIB_CONFIG_LOG(AZIOT_ULOG_TYPE_ERROR, AZIOT_ULOG_REQUIRE_NOT_EQUALS_STRING, "buffer_length", "0");
        interface_result = NULL;
    }
    else
    {
        /*[ustream_create_succeed]*/
        USTREAM_INNER_BUFFER* inner_buffer = create_inner_buffer(buffer, buffer_length, inner_free);
        /*[ustream_create_no_memory_to_create_inner_buffer_failed]*/
        if(inner_buffer == NULL)
        {
            interface_result = NULL;
        }
        else
        {
            interface_result = create_instance(inner_buffer, 0, 0);
            /*[ustream_create_no_memory_to_create_instance_failed]*/
            if(interface_result == NULL)
            {
                ULIB_CONFIG_FREE(inner_buffer);
            }
        }
    }

    return interface_result;
}

