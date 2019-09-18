// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ucontract.h"
#include "ustream.h"
#include "ustream_message.h"
#include "ulib_result.h"
#include "ulib_port.h"
#include "ulib_heap.h"
#include "ulog.h"

static const char* const MESSAGE_FORMAT =
"\
%s %s %s\r\n\
%s: %s\r\n\
";

static const char *const MESSAGE_OPTION_STRING_FORMAT = "%s: %s\r\n";
static const char *const MESSAGE_OPTION_INT_FORMAT = "%S: %i\r\n";

static AZIOT_ULIB_RESULT concrete_set_position(AZIOT_USTREAM* ustream_interface, offset_t position);
static AZIOT_ULIB_RESULT concrete_reset(AZIOT_USTREAM* ustream_interface);
static AZIOT_ULIB_RESULT concrete_read(AZIOT_USTREAM* ustream_interface, uint8_t* const buffer, size_t buffer_length, size_t* const size);
static AZIOT_ULIB_RESULT concrete_get_remaining_size(AZIOT_USTREAM* ustream_interface, size_t* const size);
static AZIOT_ULIB_RESULT concrete_get_position(AZIOT_USTREAM* ustream_interface, offset_t* const position);
static AZIOT_ULIB_RESULT concrete_release(AZIOT_USTREAM* ustream_interface, offset_t position);
static AZIOT_ULIB_RESULT concrete_clone(AZIOT_USTREAM* ustream_interface, AZIOT_USTREAM* ustream_interface_to_clone, offset_t offset);
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

static void init_instance(
    AZIOT_USTREAM* ustream_instance,
    AZIOT_USTREAM_INNER_BUFFER* inner_buffer,
    offset_t inner_current_position,
    offset_t offset,
    size_t message_size)
{
    ustream_instance->inner_current_position = inner_current_position;
    ustream_instance->inner_first_valid_position = inner_current_position;
    ustream_instance->offset_diff = offset - inner_current_position;
    ustream_instance->inner_buffer = inner_buffer;
    ustream_instance->length = message_size;
    AZIOT_ULIB_PORT_ATOMIC_INC_W(&(ustream_instance->inner_buffer->ref_count));
}

static void destroy_inner_buffer(AZIOT_USTREAM_INNER_BUFFER* inner_buffer)
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

static AZIOT_ULIB_RESULT concrete_set_position(AZIOT_USTREAM* ustream_interface, offset_t position)
{
    AZIOT_UCONTRACT(AZIOT_UCONTRACT_REQUIRE(!AZIOT_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, "Passed ustream is not the correct type\r\n"));
    AZIOT_ULIB_RESULT result;

    offset_t inner_position = position - ustream_interface->offset_diff;

    if((inner_position > (offset_t)(ustream_interface->length)) || 
            (inner_position < ustream_interface->inner_first_valid_position))
    {
        result = AZIOT_ULIB_NO_SUCH_ELEMENT_ERROR;
    }
    else
    {
        ustream_interface->inner_current_position = inner_position;
        result = AZIOT_ULIB_SUCCESS;
    }

    return result;
}

static AZIOT_ULIB_RESULT concrete_reset(AZIOT_USTREAM* ustream_interface)
{
    AZIOT_UCONTRACT(AZIOT_UCONTRACT_REQUIRE(!AZIOT_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api), 
                                AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, "Passed ustream is not the correct type\r\n"));
    ustream_interface->inner_current_position = ustream_interface->inner_first_valid_position;

    return AZIOT_ULIB_SUCCESS;
}

static AZIOT_ULIB_RESULT concrete_read(
        AZIOT_USTREAM* ustream_interface,
        uint8_t* const buffer,
        size_t buffer_length,
        size_t* const size)
{
    AZIOT_UCONTRACT(AZIOT_UCONTRACT_REQUIRE(!AZIOT_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api), 
                                    AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, "Passed ustream is not the correct type\r\n"),
                    AZIOT_UCONTRACT_REQUIRE_NOT_NULL(buffer, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZIOT_UCONTRACT_REQUIRE_NOT_EQUALS(buffer_length, 0, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZIOT_UCONTRACT_REQUIRE_NOT_NULL(size, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR));

    AZIOT_ULIB_RESULT result;

    AZIOT_USTREAM_INNER_BUFFER* inner_buffer = ustream_interface->inner_buffer;

    if(ustream_interface->inner_current_position >= ustream_interface->length)
    {
        *size = 0;
        result = AZIOT_ULIB_EOF;
    }
    else
    {
        size_t remain_size = ustream_interface->length - (size_t)ustream_interface->inner_current_position;
        *size = (buffer_length < remain_size) ? buffer_length : remain_size;
        AZIOT_USTREAM_MESSAGE *inner_message = (AZIOT_USTREAM_MESSAGE*)inner_buffer->ptr;
        (void)snprintf(buffer, buffer_length, MESSAGE_FORMAT, 
                    inner_message->message_verb, inner_message->host_name, AZIOT_ULIB_OPTION_HTTP_VERSION,
                    AZIOT_ULIB_OPTION_HOST, inner_message->host_name);
        ustream_interface->inner_current_position += *size;
        result = AZIOT_ULIB_SUCCESS;
    }

    return result;
}

static AZIOT_ULIB_RESULT concrete_get_remaining_size(AZIOT_USTREAM* ustream_interface, size_t* const size)
{
    AZIOT_UCONTRACT(AZIOT_UCONTRACT_REQUIRE(!AZIOT_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, "Passed ustream is not the correct type\r\n"),
                    AZIOT_UCONTRACT_REQUIRE_NOT_NULL(size, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR));
    *size = ustream_interface->length - ustream_interface->inner_current_position;

    return AZIOT_ULIB_SUCCESS;
}

static AZIOT_ULIB_RESULT concrete_get_position(AZIOT_USTREAM* ustream_interface, offset_t* const position)
{
    AZIOT_UCONTRACT(AZIOT_UCONTRACT_REQUIRE(!AZIOT_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, "Passed ustream is not the correct type\r\n"),
                    AZIOT_UCONTRACT_REQUIRE_NOT_NULL(position, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR));
    *position = ustream_interface->inner_current_position + ustream_interface->offset_diff;

    return AZIOT_ULIB_SUCCESS;
}

static AZIOT_ULIB_RESULT concrete_release(AZIOT_USTREAM* ustream_interface, offset_t position)
{
    AZIOT_UCONTRACT(AZIOT_UCONTRACT_REQUIRE(!AZIOT_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, "Passed ustream is not the correct type\r\n"));

    AZIOT_ULIB_RESULT result;

    offset_t inner_position = position - ustream_interface->offset_diff;

    if((inner_position >= ustream_interface->inner_current_position) ||
            (inner_position < ustream_interface->inner_first_valid_position))
    {
        result = AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        ustream_interface->inner_first_valid_position = inner_position + (offset_t)1;
        result = AZIOT_ULIB_SUCCESS;
    }

    return result;
}

static AZIOT_ULIB_RESULT concrete_clone(AZIOT_USTREAM* ustream_interface_clone, AZIOT_USTREAM* ustream_interface, offset_t offset)
{
    AZIOT_UCONTRACT(AZIOT_UCONTRACT_REQUIRE(!AZIOT_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, "Passed ustream is not the correct type\r\n"),
                    AZIOT_UCONTRACT_REQUIRE_NOT_NULL(ustream_interface_clone, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZIOT_UCONTRACT_REQUIRE((offset <= (UINT32_MAX - ustream_interface->length)), AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, "offset exceeds max size"));
    init_instance(ustream_interface_clone, ustream_interface->inner_buffer, ustream_interface->inner_current_position, offset, ustream_interface->length);

    return AZIOT_ULIB_SUCCESS;
}

static AZIOT_ULIB_RESULT concrete_dispose(AZIOT_USTREAM* ustream_interface)
{
    AZIOT_UCONTRACT(AZIOT_UCONTRACT_REQUIRE(!AZIOT_USTREAM_IS_NOT_TYPE_OF(ustream_interface, api),
                                            AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, "Passed ustream is not the correct type\r\n"));

    AZIOT_USTREAM_INNER_BUFFER* inner_buffer = ustream_interface->inner_buffer;

    AZIOT_ULIB_PORT_ATOMIC_DEC_W(&(inner_buffer->ref_count));
    if(inner_buffer->ref_count == 0)
    {
        destroy_inner_buffer(inner_buffer);
    }

    return AZIOT_ULIB_SUCCESS;
}

AZIOT_ULIB_RESULT aziot_ustream_message_add_option(AZIOT_USTREAM_MESSAGE* message, AZIOT_ULIB_MESSAGE_OPTION option,
                                                    const char* option_string)
{
    AZIOT_UCONTRACT(AZIOT_UCONTRACT_REQUIRE_NOT_NULL(message, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZIOT_UCONTRACT_REQUIRE_NOT_NULL(option_string, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR));

    AZIOT_ULIB_RESULT result;

    switch(option)
    {
        case AZIOT_ULIB_MESSAGE_OPTION_CONTENT_TYPE:
            message->content_type = option_string;
            result = AZIOT_ULIB_SUCCESS;
            break;
        default:
            result = AZIOT_ULIB_NO_SUCH_ELEMENT_ERROR;
    }

    return result;
}

AZIOT_ULIB_RESULT aziot_ustream_message_init(
    AZIOT_USTREAM_MESSAGE* message,
    const char* host,
    AZIOT_ULIB_MESSAGE_VERB verb)
{
    AZIOT_UCONTRACT(AZIOT_UCONTRACT_REQUIRE_NOT_NULL(message, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZIOT_UCONTRACT_REQUIRE_NOT_NULL(host, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR));

    message->host_name = host;
    message->content_type = NULL;
    message->message_verb = AZIOT_ULIB_MESSAGE_VERB_STRINGS[verb];

    return AZIOT_ULIB_SUCCESS;
}

AZIOT_ULIB_RESULT aziot_ustream_from_message(
    AZIOT_USTREAM* ustream_instance,
    AZIOT_USTREAM_INNER_BUFFER* ustream_inner_buffer,
    AZIOT_RELEASE_CALLBACK inner_buffer_release,
    AZIOT_USTREAM_MESSAGE* message,
    AZIOT_RELEASE_CALLBACK message_release)
{
    AZIOT_UCONTRACT(AZIOT_UCONTRACT_REQUIRE_NOT_NULL(ustream_instance, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZIOT_UCONTRACT_REQUIRE_NOT_NULL(ustream_inner_buffer, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR),
                    AZIOT_UCONTRACT_REQUIRE_NOT_NULL(message, AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR));

    ustream_inner_buffer->api = &api;
    ustream_inner_buffer->ptr = (void*)message;
    ustream_inner_buffer->ref_count = 0;
    ustream_inner_buffer->data_release = message_release;
    ustream_inner_buffer->inner_buffer_release = inner_buffer_release;

    size_t size_of_message = snprintf(NULL, 0, MESSAGE_FORMAT,
                   message->message_verb, message->host_name, AZIOT_ULIB_OPTION_HTTP_VERSION,
                   AZIOT_ULIB_OPTION_HOST, message->host_name);

    init_instance(ustream_instance, ustream_inner_buffer, 0, 0, size_of_message);

    return AZIOT_ULIB_SUCCESS;
}
