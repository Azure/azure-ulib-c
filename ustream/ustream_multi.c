// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ustream.h"
#include "ulib_heap.h"
#include "ulog.h"

typedef struct BUFFER_LIST_NODE_TAG
{
    struct BUFFER_LIST_NODE_TAG* next;
    AZULIB_USTREAM* buffer;
} BUFFER_LIST_NODE;

typedef struct USTREAM_MULTI_INSTANCE_TAG
{
    /* Inner buffer. */
    BUFFER_LIST_NODE* buffer_list;
    BUFFER_LIST_NODE* current_node;
    size_t length;

    /* Instance controls. */
    offset_t offset_diff;
    offset_t inner_current_position;
    offset_t inner_first_valid_position;
} USTREAM_MULTI_INSTANCE;

static AZULIB_RESULT concrete_set_position(AZULIB_USTREAM* ustream_interface, offset_t position);
static AZULIB_RESULT concrete_reset(AZULIB_USTREAM* ustream_interface);
static AZULIB_RESULT concrete_read(AZULIB_USTREAM* ustream_interface, uint8_t* const buffer, size_t buffer_length, size_t* const size);
static AZULIB_RESULT concrete_get_remaining_size(AZULIB_USTREAM* ustream_interface, size_t* const size);
static AZULIB_RESULT concrete_get_position(AZULIB_USTREAM* ustream_interface, offset_t* const position);
static AZULIB_RESULT concrete_release(AZULIB_USTREAM* ustream_interface, offset_t position);
static AZULIB_USTREAM* concrete_clone(AZULIB_USTREAM* ustream_interface, offset_t offset);
static AZULIB_RESULT concrete_dispose(AZULIB_USTREAM* ustream_interface);
static const AZULIB_USTREAM_INTERFACE api =
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

static BUFFER_LIST_NODE* create_buffer_node(
    AZULIB_USTREAM* buffer, 
    offset_t offset)
{
    BUFFER_LIST_NODE* new_node = (BUFFER_LIST_NODE*)AZULIB_ULIB_CONFIG_MALLOC(sizeof(BUFFER_LIST_NODE));
    if(new_node == NULL)
    {
        /*[azulib_ustream_clone_no_memory_to_create_first_node_failed]*/
        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_OUT_OF_MEMORY_STRING, "buffer list");
    }
    else
    {
        new_node->next = NULL;
        new_node->buffer = azulib_ustream_clone(buffer, offset);
        if(new_node->buffer == NULL)
        {
            /*[azulib_ustream_clone_no_memory_to_clone_first_node_failed]*/
            AZULIB_ULIB_CONFIG_FREE(new_node);
            new_node = NULL;
        }
    }
    return new_node;
}

static void destroy_buffer_node(BUFFER_LIST_NODE* node)
{
    azulib_ustream_dispose(node->buffer);
    AZULIB_ULIB_CONFIG_FREE(node);
}

static void destroy_full_buffer_list(BUFFER_LIST_NODE* node)
{
    /*[ustream_multi_dispose_multibuffer_without_buffers_free_all_resources_succeed]*/
    /*[ustream_multi_dispose_multibuffer_with_buffers_free_all_resources_succeed]*/
    while(node != NULL)
    {
        BUFFER_LIST_NODE* temp = node;
        node = node->next;
        destroy_buffer_node(temp);
    }
}

static AZULIB_USTREAM* create_instance(void)
{
    AZULIB_USTREAM* ustream_interface = (AZULIB_USTREAM*)AZULIB_ULIB_CONFIG_MALLOC(sizeof(AZULIB_USTREAM));
    /*[azulib_ustream_multi_create_no_memory_to_create_instance_failed]*/
    /*[azulib_ustream_clone_no_memory_to_create_interface_failed]*/
    if(ustream_interface == NULL)
    {
        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_OUT_OF_MEMORY_STRING, "ustream_interface");
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)AZULIB_ULIB_CONFIG_MALLOC(sizeof(USTREAM_MULTI_INSTANCE));
        if(instance == NULL)
        {
            /*[azulib_ustream_clone_no_memory_to_create_instance_failed]*/
            AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_OUT_OF_MEMORY_STRING, "ustream_instance");
            AZULIB_ULIB_CONFIG_FREE(ustream_interface);
            ustream_interface = NULL;
        }
        else
        {
            ustream_interface->api = &api;
            ustream_interface->handle = (void*)instance;

            instance->inner_current_position = 0;
            instance->inner_first_valid_position = 0;
            instance->offset_diff = 0;
            instance->buffer_list = NULL;
            instance->current_node = NULL;
            instance->length = 0;
        }
    }
    return ustream_interface;
}

static void destroy_instance(AZULIB_USTREAM* ustream_interface)
{
    USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

    destroy_full_buffer_list(instance->buffer_list);
    AZULIB_ULIB_CONFIG_FREE(instance);
    AZULIB_ULIB_CONFIG_FREE(ustream_interface);
}

static AZULIB_RESULT concrete_set_position(
        AZULIB_USTREAM* ustream_interface, 
        offset_t position)
{
    AZULIB_RESULT result;

    if(AZULIB_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[azulib_ustream_set_position_compliance_null_buffer_failed]*/
        /*[azulib_ustream_set_position_compliance_non_type_of_buffer_api_failed]*/
        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = AZULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;
        offset_t inner_position = position - instance->offset_diff;

        if(inner_position == instance->inner_current_position)
        {
            /*[azulib_ustream_set_position_compliance_forward_to_the_end_position_succeed]*/
            result = AZULIB_SUCCESS;
        }
        else if((inner_position > (offset_t)(instance->length)) || 
                (inner_position < instance->inner_first_valid_position))
        {
            /*[azulib_ustream_set_position_compliance_forward_out_of_the_buffer_failed]*/
            /*[azulib_ustream_set_position_compliance_back_before_first_valid_position_failed]*/
            result = AZULIB_NO_SUCH_ELEMENT_ERROR;
        }
        else
        {
            /*[azulib_ustream_set_position_compliance_back_to_beginning_succeed]*/
            /*[azulib_ustream_set_position_compliance_back_position_succeed]*/
            /*[azulib_ustream_set_position_compliance_forward_position_succeed]*/
            /*[azulib_ustream_set_position_compliance_run_full_buffer_byte_by_byte_succeed]*/
            /*[azulib_ustream_set_position_compliance_run_full_buffer_byte_by_byte_reverse_order_succeed]*/
            /*[azulib_ustream_set_position_compliance_cloned_buffer_back_to_beginning_succeed]*/
            /*[azulib_ustream_set_position_compliance_cloned_buffer_back_position_succeed]*/
            /*[azulib_ustream_set_position_compliance_cloned_buffer_forward_position_succeed]*/
            /*[azulib_ustream_set_position_compliance_cloned_buffer_forward_to_the_end_position_succeed]*/
            /*[azulib_ustream_set_position_compliance_cloned_buffer_run_full_buffer_byte_by_byte_succeed]*/
            /*[azulib_ustream_set_position_compliance_cloned_buffer_run_full_buffer_byte_by_byte_reverse_order_succeed]*/
            result = AZULIB_SUCCESS;
            BUFFER_LIST_NODE* node = instance->buffer_list;
            BUFFER_LIST_NODE* new_current_node = NULL; 
            bool bypass_old_current_node = false;
            while((node != NULL) && (result == AZULIB_SUCCESS))
            {
                if(node == instance->current_node)
                {
                    bypass_old_current_node = true;
                }
                
                if(new_current_node == NULL)
                {
                    offset_t current_position;
                    /*[ustream_multi_seek_inner_buffer_failed_in_get_current_position_failed]*/
                    if((result = azulib_ustream_get_position(node->buffer, &current_position)) == AZULIB_SUCCESS)
                    {
                        size_t size;
                        if((result = azulib_ustream_get_remaining_size(node->buffer, &size)) == AZULIB_SUCCESS)
                        {
                            if((current_position + size) > inner_position)
                            {
                                new_current_node = node;
                                result = azulib_ustream_set_position(node->buffer, inner_position);
                            }
                            else
                            {
                                result = azulib_ustream_set_position(node->buffer, (current_position + (offset_t)size - (offset_t)1));
                            }
                        }
                    }
                }
                else
                {
                    (void)azulib_ustream_reset(node->buffer);
                    if(bypass_old_current_node == true)
                    {
                        break;
                    }
                }
                node = node->next;
            } 

            if(result == AZULIB_SUCCESS)
            {
                instance->inner_current_position = inner_position;
                instance->current_node = new_current_node;
            }
            else
            {
                if(instance->current_node != NULL)
                {
                    AZULIB_RESULT rollback_result = 
                        azulib_ustream_set_position(instance->current_node->buffer, instance->inner_current_position);
                    if(rollback_result != AZULIB_SUCCESS)
                    {
                        AZULIB_ULIB_CONFIG_LOG(
                            AZULIB_ULOG_TYPE_ERROR,
                            AZULIB_ULOG_REPORT_EXCEPTION_STRING,
                            "ustream_multi_seek rollback",
                            rollback_result);
                    }
                }
                AZULIB_ULIB_CONFIG_LOG(
                    AZULIB_ULOG_TYPE_ERROR, 
                    AZULIB_ULOG_REPORT_EXCEPTION_STRING,
                    "ustream_multi_seek",
                    result);
            }
        }
    }

    return result;
}

static AZULIB_RESULT concrete_reset(AZULIB_USTREAM* ustream_interface)
{
    AZULIB_RESULT result;

    if(AZULIB_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[azulib_ustream_reset_compliance_null_buffer_failed]*/
        /*[azulib_ustream_reset_compliance_non_type_of_buffer_api_failed]*/
        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = AZULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

        /*[azulib_ustream_reset_compliance_back_to_beginning_succeed]*/
        /*[azulib_ustream_reset_compliance_back_position_succeed]*/
        result = concrete_set_position(ustream_interface, 
                (instance->inner_first_valid_position + instance->offset_diff));
    }

    return result;
}

static AZULIB_RESULT concrete_read(
        AZULIB_USTREAM* ustream_interface,
        uint8_t* const buffer,
        size_t buffer_length,
        size_t* const size)
{
    AZULIB_RESULT result;

    if(AZULIB_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[azulib_ustream_read_compliance_null_buffer_failed]*/
        /*[azulib_ustream_read_compliance_non_type_of_buffer_api_failed]*/
        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = AZULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if((buffer == NULL) || (size == NULL))
    {
        /*[azulib_ustream_read_compliance_null_return_buffer_failed]*/
        /*[azulib_ustream_read_compliance_null_return_size_failed]*/
        AZULIB_ULIB_CONFIG_LOG(
            AZULIB_ULOG_TYPE_ERROR,
            AZULIB_ULOG_REQUIRE_NOT_NULL_STRING,
            (buffer == NULL ? "buffer" : "size"));
        result = AZULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if(buffer_length == 0)
    {
        /*[azulib_ustream_read_compliance_buffer_with_zero_size_failed]*/
        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_NOT_EQUALS_STRING, "buffer_length", "0");
        result = AZULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

        BUFFER_LIST_NODE* node = instance->current_node;
        if(node == NULL)
        {
            *size = 0;
            result = AZULIB_EOF;
        }
        else
        {
            /*[azulib_ustream_read_compliance_single_buffer_succeed]*/
            /*[azulib_ustream_read_compliance_right_boundary_condition_succeed]*/
            /*[azulib_ustream_read_compliance_boundary_condition_succeed]*/
            /*[azulib_ustream_read_compliance_left_boundary_condition_succeed]*/
            /*[azulib_ustream_read_compliance_single_byte_succeed]*/
            /*[azulib_ustream_read_compliance_get_from_cloned_buffer_succeed]*/
            /*[azulib_ustream_read_compliance_cloned_buffer_right_boundary_condition_succeed]*/
            *size = 0;
            AZULIB_RESULT intermediate_result = AZULIB_SUCCESS;
            while((node != NULL) &&
                    (*size < buffer_length) &&
                    (intermediate_result == AZULIB_SUCCESS))
            {
                size_t total_copy_size;
                /*[azulib_ustream_read_compliance_succeed_2]*/
                size_t remain_size = buffer_length - *size;
                /*[azulib_ustream_read_compliance_succeed_1]*/
                intermediate_result = azulib_ustream_read(node->buffer, &buffer[*size], remain_size, &total_copy_size);
                switch(intermediate_result)
                {
                case AZULIB_SUCCESS:
                    *size += total_copy_size;
                    /* do not **break** here.*/
                case AZULIB_EOF:
                    if(*size < buffer_length)
                    {
                        node = node->next;
                        instance->current_node = node;
                        if(node != NULL)
                        {
                            intermediate_result = AZULIB_SUCCESS;
                            (void)azulib_ustream_reset(node->buffer);
                        }
                    }
                    break;
                default:
                    break;
                }
            }

            if(*size != 0)
            {
                /* if the size is bigger than 0 is because at least one inner buffer was copied, so use it and return success. */
                instance->inner_current_position += *size;
                result = AZULIB_SUCCESS;
            }
            else
            {
                /*[azulib_ustream_read_compliance_succeed_3]*/
                result = intermediate_result;
            }
        }
    }

    return result;
}

static AZULIB_RESULT concrete_get_remaining_size(AZULIB_USTREAM* ustream_interface, size_t* const size)
{
    AZULIB_RESULT result;

    if(AZULIB_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[azulib_ustream_get_remaining_size_compliance_null_buffer_failed]*/
        /*[azulib_ustream_get_remaining_size_compliance_buffer_is_not_type_of_buffer_failed]*/
        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = AZULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if(size == NULL)
    {
        /*[azulib_ustream_get_remaining_size_compliance_null_size_failed]*/
        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_NOT_NULL_STRING, "size");
        result = AZULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

        /*[azulib_ustream_get_remaining_size_compliance_new_buffer_succeed]*/
        /*[azulib_ustream_get_remaining_size_compliance_new_buffer_with_non_zero_current_position_succeed]*/
        /*[azulib_ustream_get_remaining_size_compliance_cloned_buffer_with_non_zero_current_position_succeed]*/
        *size = instance->length - instance->inner_current_position;
        result = AZULIB_SUCCESS;
    }

    return result;
}

static AZULIB_RESULT concrete_get_position(AZULIB_USTREAM* ustream_interface, offset_t* const position)
{
    AZULIB_RESULT result;

    if(AZULIB_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[ustream_get_current_position_compliance_null_buffer_failed]*/
        /*[ustream_get_current_position_compliance_buffer_is_not_type_of_buffer_failed]*/
        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = AZULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if(position == NULL)
    {
        /*[ustream_get_current_position_compliance_null_position_failed]*/
        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_NOT_NULL_STRING, "position");
        result = AZULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

        /*[ustream_get_current_position_compliance_new_buffer_succeed]*/
        /*[ustream_get_current_position_compliance_new_buffer_with_non_zero_current_position_succeed]*/
        /*[ustream_get_current_position_compliance_cloned_buffer_with_non_zero_current_position_succeed]*/
        *position = instance->inner_current_position + instance->offset_diff;
        result = AZULIB_SUCCESS;
    }

    return result;
}

static AZULIB_RESULT concrete_release(AZULIB_USTREAM* ustream_interface, offset_t position)
{
    AZULIB_RESULT result;

    if(AZULIB_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[azulib_ustream_release_compliance_null_buffer_failed]*/
        /*[azulib_ustream_release_compliance_non_type_of_buffer_api_failed]*/
        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = AZULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;
        offset_t inner_position = position - instance->offset_diff;
        BUFFER_LIST_NODE* new_buffer_list_start = instance->buffer_list;

        if((inner_position >= instance->inner_current_position) ||
                (inner_position < instance->inner_first_valid_position))
        {
            /*[azulib_ustream_release_compliance_release_after_current_failed]*/
            /*[azulib_ustream_release_compliance_release_position_already_released_failed]*/
            result = AZULIB_ILLEGAL_ARGUMENT_ERROR;
        }
        else
        {
            /*[azulib_ustream_release_compliance_release_all_succeed]*/
            /*[azulib_ustream_release_compliance_run_full_buffer_byte_by_byte_succeed]*/
            /*[azulib_ustream_release_compliance_cloned_buffer_release_all_succeed]*/
            /*[azulib_ustream_release_compliance_cloned_buffer_run_full_buffer_byte_by_byte_succeed]*/
            result = AZULIB_SUCCESS;
            offset_t last_position = 0;
            while((new_buffer_list_start != NULL) &&
                    ((result = azulib_ustream_get_position(new_buffer_list_start->buffer, &last_position)) == AZULIB_SUCCESS) && 
                    (inner_position > last_position))
            {
                new_buffer_list_start = new_buffer_list_start->next;
            }

            if(result == AZULIB_SUCCESS)
            {
                size_t size;
                if((result = azulib_ustream_get_remaining_size(new_buffer_list_start->buffer, &size)) == AZULIB_SUCCESS)
                {
                    if((size == 0) && (inner_position == (last_position - 1)))
                    {
                        new_buffer_list_start = new_buffer_list_start->next;
                    }

                    /*[azulib_ustream_release_compliance_succeed]*/
                    /*[azulib_ustream_release_compliance_cloned_buffer_succeed]*/
                    // move the first valid position.
                    instance->inner_first_valid_position = inner_position + (offset_t)1;

                    // release all unnecessary buffers.
                    while(instance->buffer_list != new_buffer_list_start)
                    {
                        BUFFER_LIST_NODE* node = instance->buffer_list;
                        instance->buffer_list = instance->buffer_list->next;
                        destroy_buffer_node(node); 
                    }
                }
            }
        }
    }   

    return result;
}

static AZULIB_USTREAM* concrete_clone(AZULIB_USTREAM* ustream_interface, offset_t offset)
{
    AZULIB_USTREAM* interface_result;

    if(AZULIB_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[azulib_ustream_clone_compliance_null_buffer_failed]*/
        /*[azulib_ustream_clone_compliance_buffer_is_not_type_of_buffer_failed]*/
        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        interface_result = NULL;
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

        if(offset > (UINT32_MAX - instance->length))
        {
            /*[azulib_ustream_clone_compliance_offset_exceed_size_failed]*/
            interface_result = NULL;
        }
        else
        {
            /*[azulib_ustream_clone_compliance_empty_buffer_succeed]*/
            interface_result = create_instance();

            /*[azulib_ustream_clone_compliance_no_memory_to_create_instance_failed]*/
            if(interface_result != NULL)
            {
                USTREAM_MULTI_INSTANCE* new_instance = (USTREAM_MULTI_INSTANCE*)interface_result->handle;
                /*[azulib_ustream_clone_compliance_new_buffer_cloned_with_zero_offset_succeed]*/
                /*[azulib_ustream_clone_compliance_new_buffer_cloned_with_offset_succeed]*/
                /*[azulib_ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed]*/
                /*[azulib_ustream_clone_compliance_new_buffer_with_non_zero_current_and_released_positions_cloned_with_negative_offset_succeed]*/
                /*[azulib_ustream_clone_compliance_cloned_buffer_with_non_zero_current_and_released_positions_cloned_with_offset_succeed]*/
                BUFFER_LIST_NODE* node_list_to_clone = instance->current_node;
                BUFFER_LIST_NODE** insert_position = &(new_instance->buffer_list);
                bool fail = false;
                while((node_list_to_clone != NULL) && (fail == false))
                {
                    size_t new_buffer_size;
                    BUFFER_LIST_NODE* new_node;

                    if((new_node = create_buffer_node(node_list_to_clone->buffer, (offset_t)(new_instance->length))) == NULL)
                    {
                        fail = true;
                    }
                    else if(azulib_ustream_get_remaining_size(new_node->buffer, &(new_buffer_size)) == AZULIB_SUCCESS)
                    {
                        // find insertion position in the list
                        *insert_position = new_node;
                        new_instance->length += new_buffer_size;
                        insert_position = &(new_node->next);
                    }
                    else
                    {
                        destroy_buffer_node(new_node);
                        fail = true;
                    }
                    node_list_to_clone = node_list_to_clone->next;
                }

                if(fail == true)
                {
                    destroy_instance(interface_result);
                    interface_result = NULL;
                }
                else
                {
                    new_instance->current_node = new_instance->buffer_list;
                    new_instance->offset_diff = offset - new_instance->inner_current_position;
                }
            }
        }
    }

    return interface_result;
}

static AZULIB_RESULT concrete_dispose(AZULIB_USTREAM* ustream_interface)
{
    AZULIB_RESULT result;

    if(AZULIB_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[azulib_ustream_dispose_compliance_null_buffer_failed]*/
        /*[azulib_ustream_dispose_compliance_buffer_is_not_type_of_buffer_failed]*/
        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = AZULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        /*[azulib_ustream_dispose_compliance_cloned_instance_disposed_first_succeed]*/
        /*[azulib_ustream_dispose_compliance_cloned_instance_disposed_second_succeed]*/
        /*[azulib_ustream_dispose_compliance_single_instance_succeed]*/
        destroy_instance(ustream_interface);
        result = AZULIB_SUCCESS;
    }

    return result;
}

AZULIB_USTREAM* azulib_ustream_multi_create(void)
{
    /*[azulib_ustream_multi_create_succeed]*/
    /*[azulib_ustream_multi_create_no_memory_to_create_interface_failed]*/
    return create_instance();
}

AZULIB_RESULT azulib_ustream_multi_append(
        AZULIB_USTREAM* ustream_interface,
        AZULIB_USTREAM* ustream_to_append)
{
    AZULIB_RESULT result;

    if(AZULIB_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[azulib_ustream_multi_append_null_multibuffer_failed]*/
        /*[azulib_ustream_multi_append_not_multibuffer_type_failed]*/
        result = AZULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if(ustream_to_append == NULL)
    {
        /*[azulib_ustream_multi_append_null_buffer_to_add_failed]*/
        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REQUIRE_NOT_NULL_STRING, "ustream_to_append");
        result = AZULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        size_t new_buffer_size;
        BUFFER_LIST_NODE* new_node;
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

        /*[azulib_ustream_multi_append_not_enough_memory_failed]*/
        /*[azulib_ustream_multi_append_not_enough_memory_to_clone_the_buffer_failed]*/
        if((new_node = create_buffer_node(ustream_to_append, (offset_t)(instance->length))) == NULL)
        {
            result = AZULIB_OUT_OF_MEMORY_ERROR;
        }
        else if((result = azulib_ustream_get_remaining_size(new_node->buffer, &(new_buffer_size))) != AZULIB_SUCCESS)
        {
            /*[azulib_ustream_multi_append_new_inner_buffer_failed_on_get_remaining_size_failed]*/
            AZULIB_ULIB_CONFIG_LOG(
                AZULIB_ULOG_TYPE_ERROR,
                AZULIB_ULOG_REPORT_EXCEPTION_STRING,
                "azulib_ustream_multi_append",
                result);
            destroy_buffer_node(new_node);
        }
        else
        {
            /*[azulib_ustream_multi_append_succeed]*/
            /* find insertion position in the list */
            BUFFER_LIST_NODE** insert_position = &(instance->buffer_list);

            while(*insert_position != NULL)
            {
                insert_position = &((*insert_position)->next);
            }

            *insert_position = new_node;
            instance->length += new_buffer_size;
            if(instance->current_node == NULL)
            {
                instance->current_node = new_node;
            }
        }
    }

    return result;
}

