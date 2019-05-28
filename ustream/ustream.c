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

typedef uint8_t                                 USTREAM_FLAGS;
#define USTREAM_FLAG_NONE                 (USTREAM_FLAGS)0x00
#define USTREAM_FLAG_RELEASE_ON_DESTROY   (USTREAM_FLAGS)0x01

typedef struct USTREAM_INNER_BUFFER_TAG
{
    uint8_t* ptr;
    size_t length;
    USTREAM_FLAGS flags;
    volatile uint32_t ref_count;
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

static ULIB_RESULT concrete_set_position(USTREAM* ustream_interface, offset_t position);
static ULIB_RESULT concrete_reset(USTREAM* ustream_interface);
static ULIB_RESULT concrete_read(USTREAM* ustream_interface, uint8_t* const buffer, size_t buffer_length, size_t* const size);
static ULIB_RESULT concrete_get_remaining_size(USTREAM* ustream_interface, size_t* const size);
static ULIB_RESULT concrete_get_position(USTREAM* ustream_interface, offset_t* const position);
static ULIB_RESULT concrete_release(USTREAM* ustream_interface, offset_t position);
static USTREAM* concrete_clone(USTREAM* ustream_interface, offset_t offset);
static ULIB_RESULT concrete_dispose(USTREAM* ustream_interface);
static const USTREAM_INTERFACE _api =
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

static USTREAM* create_instance(
    USTREAM_INNER_BUFFER* inner_buffer,
    offset_t inner_current_position,
    offset_t offset)
{
    USTREAM* ustream_interface = (USTREAM*)ULIB_CONFIG_MALLOC(sizeof(USTREAM));
    /*[uStreamCreate_noMemoryToCreateInterfaceFailed]*/
    /*[uStreamConstCreate_noMemoryToCreateInterfaceFailed]*/
    /*[uStreamClone_noMemoryToCreateInterfaceFailed]*/
    /*[uStreamClone_noMemoryTocreate_instanceFailed]*/
    if(ustream_interface == NULL)
    {
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "ustream_interface");
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ULIB_CONFIG_MALLOC(sizeof(USTREAM_INSTANCE));
        /*[az_stdbufferClone_noMemoryTocreate_instanceFailed]*/
        if(instance != NULL)
        {
            ustream_interface->api = &_api;
            ustream_interface->handle = (void*)instance;

            instance->inner_current_position = inner_current_position;
            instance->inner_first_valid_position = inner_current_position;
            instance->offset_diff = offset - inner_current_position;
            instance->inner_buffer = inner_buffer;
            ULIB_PORT_ATOMIC_INC_W(&(instance->inner_buffer->ref_count));
        }
        else
        {
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "uStreamInstance");
            ULIB_CONFIG_FREE(ustream_interface);
            ustream_interface = NULL;
        }
    }

    return ustream_interface;
}

static USTREAM_INNER_BUFFER* createInnerBuffer(
    const uint8_t* const buffer, 
    size_t buffer_length,
    bool take_ownership,
    bool release_on_destroy)
{
    uint8_t* ptr;
    USTREAM_INNER_BUFFER* inner_buffer;

    if(take_ownership)
    {
        ptr = (uint8_t*)buffer;
    }
    else
    {
        if((ptr = (uint8_t*)ULIB_CONFIG_MALLOC(buffer_length * sizeof(uint8_t))) != NULL)
        {
            (void)memcpy(ptr, buffer, buffer_length);
        }
        else
        {
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "inner buffer");
        }
    }

    if(ptr == NULL)
    {
        inner_buffer = NULL;
    }
    else if((inner_buffer = (USTREAM_INNER_BUFFER*)ULIB_CONFIG_MALLOC(sizeof(USTREAM_INNER_BUFFER))) == NULL)
    {
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "inner buffer control");
        if(!take_ownership)
        {
            ULIB_CONFIG_FREE(ptr);
        }
    }
    else
    {
        inner_buffer->ptr = ptr;
        inner_buffer->length = buffer_length;
        inner_buffer->flags = (release_on_destroy ? USTREAM_FLAG_RELEASE_ON_DESTROY : USTREAM_FLAG_NONE);
        inner_buffer->ref_count = 0;
    }

    return inner_buffer;
}

static void destroyInnerBuffer(USTREAM_INNER_BUFFER* inner_buffer)
{
    if((inner_buffer->flags & USTREAM_FLAG_RELEASE_ON_DESTROY) == USTREAM_FLAG_RELEASE_ON_DESTROY)
    {
        ULIB_CONFIG_FREE(inner_buffer->ptr);
    }
    ULIB_CONFIG_FREE(inner_buffer);
}

static ULIB_RESULT concrete_set_position(USTREAM* ustream_interface, offset_t position)
{
    ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, _api))
    {
        /*[ustream_set_position_complianceNullBufferFailed]*/
        /*[ustream_set_position_complianceNonTypeOfBufferAPIFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ustream_interface->handle;
        USTREAM_INNER_BUFFER* inner_buffer = instance->inner_buffer;
        offset_t innerPosition = position - instance->offset_diff;

        if((innerPosition > (offset_t)(inner_buffer->length)) || 
                (innerPosition < instance->inner_first_valid_position))
        {
            /*[ustream_set_position_complianceForwardOutOfTheBufferFailed]*/
            /*[ustream_set_position_complianceBackBeforeFirstValidPositionFailed]*/
            result = ULIB_NO_SUCH_ELEMENT_ERROR;
        }
        else
        {
            /*[ustream_set_position_complianceBackToBeginningSucceed]*/
            /*[ustream_set_position_complianceBackPositionSucceed]*/
            /*[ustream_set_position_complianceForwardPositionSucceed]*/
            /*[ustream_set_position_complianceForwardToTheEndPositionSucceed]*/
            /*[ustream_set_position_complianceRunFullBufferByteByByteSucceed]*/
            /*[ustream_set_position_complianceRunFullBufferByteByByteReverseOrderSucceed]*/
            /*[ustream_set_position_complianceClonedBufferBackToBeginningSucceed]*/
            /*[ustream_set_position_complianceClonedBufferBackPositionSucceed]*/
            /*[ustream_set_position_complianceClonedBufferForwardPositionSucceed]*/
            /*[ustream_set_position_complianceClonedBufferForwardToTheEndPositionSucceed]*/
            /*[ustream_set_position_complianceClonedBufferRunFullBufferByteByByteSucceed]*/
            /*[ustream_set_position_complianceClonedBufferRunFullBufferByteByByteReverseOrderSucceed]*/
            instance->inner_current_position = innerPosition;
            result = ULIB_SUCCESS;
        }
    }

    return result;
}

static ULIB_RESULT concrete_reset(USTREAM* ustream_interface)
{
    ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, _api))
    {
        /*[uStreamReset_complianceNullBufferFailed]*/
        /*[uStreamReset_complianceNonTypeOfBufferAPIFailed]*/

        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ustream_interface->handle;

        /*[uStreamReset_complianceBackToBeginningSucceed]*/
        /*[uStreamReset_complianceBackPositionSucceed]*/
        instance->inner_current_position = instance->inner_first_valid_position;
        result = ULIB_SUCCESS;
    }

    return result;
}

static ULIB_RESULT concrete_read(
        USTREAM* ustream_interface,
        uint8_t* const buffer,
        size_t buffer_length,
        size_t* const size)
{
    ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, _api))
    {
        /*[ustream_read_complianceNullBufferFailed]*/
        /*[ustream_read_complianceNonTypeOfBufferAPIFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if((buffer == NULL) || (size == NULL))
    {
        /*[ustream_read_complianceNullReturnBufferFailed]*/
        /*[ustream_read_complianceNullReturnSizeFailed]*/
        ULIB_CONFIG_LOG(
            ULOG_TYPE_ERROR,
            ULOG_REQUIRE_NOT_NULL_STRING, 
            (buffer == NULL ? "buffer" : "size"));
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if(buffer_length == 0)
    {
        /*[ustream_read_complianceBufferWithZeroSizeFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_EQUALS_STRING, "buffer_length", "0");
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ustream_interface->handle;
        USTREAM_INNER_BUFFER* inner_buffer = instance->inner_buffer;

        if(instance->inner_current_position >= inner_buffer->length)
        {
            /*[ustream_read_complianceSucceed_3]*/
            *size = 0;
            result = ULIB_EOF;
        }
        else
        {
            /*[ustream_read_complianceSucceed_2]*/
            size_t remainSize = inner_buffer->length - (size_t)instance->inner_current_position;
            *size = (buffer_length < remainSize) ? buffer_length : remainSize;
            /*[ustream_read_complianceSucceed_1]*/
            /*[ustream_read_complianceSingleBufferSucceed]*/
            /*[ustream_read_complianceRightBoundaryConditionSucceed]*/
            /*[ustream_read_complianceBoundaryConditionSucceed]*/
            /*[ustream_read_complianceLeftBoundaryConditionSucceed]*/
            /*[ustream_read_complianceSingleByteSucceed]*/
            /*[ustream_read_complianceGetFromClonedBufferSucceed]*/
            /*[ustream_read_complianceClonedBufferRightBoundaryConditionSucceed]*/
            (void)memcpy(buffer, inner_buffer->ptr + instance->inner_current_position, *size);
            instance->inner_current_position += *size;
            result = ULIB_SUCCESS;
        }
    }

    return result;
}

static ULIB_RESULT concrete_get_remaining_size(USTREAM* ustream_interface, size_t* const size)
{
    ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, _api))
    {
        /*[uStreamGetRemainingSize_complianceNullBufferFailed]*/
        /*[uStreamGetRemainingSize_complianceBufferIsNotTypeOfBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if(size == NULL)
    {
        /*[uStreamGetRemainingSize_complianceNullSizeFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "size");
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ustream_interface->handle;

        /*[uStreamGetRemainingSize_complianceNewBufferSucceed]*/
        /*[uStreamGetRemainingSize_complianceNewBufferWithNonZeroCurrentPositionSucceed]*/
        /*[uStreamGetRemainingSize_complianceClonedBufferWithNonZeroCurrentPositionSucceed]*/
        *size = instance->inner_buffer->length - instance->inner_current_position;
        result = ULIB_SUCCESS;
    }

    return result;
}

static ULIB_RESULT concrete_get_position(USTREAM* ustream_interface, offset_t* const position)
{
    ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, _api))
    {
        /*[uStreamGetCurrentPosition_complianceNullBufferFailed]*/
        /*[uStreamGetCurrentPosition_complianceBufferIsNotTypeOfBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if(position == NULL)
    {
        /*[uStreamGetCurrentPosition_complianceNullPositionFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "position");
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ustream_interface->handle;

        /*[uStreamGetCurrentPosition_complianceNewBufferSucceed]*/
        /*[uStreamGetCurrentPosition_complianceNewBufferWithNonZeroCurrentPositionSucceed]*/
        /*[uStreamGetCurrentPosition_complianceClonedBufferWithNonZeroCurrentPositionSucceed]*/
        *position = instance->inner_current_position + instance->offset_diff;
        result = ULIB_SUCCESS;
    }

    return result;
}

static ULIB_RESULT concrete_release(USTREAM* ustream_interface, offset_t position)
{
    ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, _api))
    {
        /*[uStreamRelease_complianceNullBufferFailed]*/
        /*[uStreamRelease_complianceNonTypeOfBufferAPIFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ustream_interface->handle;
        offset_t innerPosition = position - instance->offset_diff;

        if((innerPosition >= instance->inner_current_position) ||
                (innerPosition < instance->inner_first_valid_position))
        {
            /*[uStreamRelease_complianceReleaseAfterCurrentFailed]*/
            /*[uStreamRelease_complianceReleasePositionAlreayReleasedFailed]*/
            result = ULIB_ILLEGAL_ARGUMENT_ERROR;
        }
        else
        {
            /*[uStreamRelease_complianceSucceed]*/
            /*[uStreamRelease_complianceReleaseAllSucceed]*/
            /*[uStreamRelease_complianceRunFullBufferByteByByteSucceed]*/
            /*[uStreamRelease_complianceClonedBufferSucceed]*/
            /*[uStreamRelease_complianceClonedBufferReleaseAllSucceed]*/
            /*[uStreamRelease_complianceClonedBufferRunFullBufferByteByByteSucceed]*/
            instance->inner_first_valid_position = innerPosition + (offset_t)1;
            result = ULIB_SUCCESS;
        }
    }

    return result;
}

static USTREAM* concrete_clone(USTREAM* ustream_interface, offset_t offset)
{
    USTREAM* interfaceResult;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, _api))
    {
        /*[uStreamClone_complianceNullBufferFailed]*/
        /*[uStreamClone_complianceBufferIsNotTypeOfBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        interfaceResult = NULL;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ustream_interface->handle;

        if(offset > (UINT32_MAX - instance->inner_buffer->length))
        {
            /*[uStreamClone_complianceOffsetExceedSizeFailed]*/
            interfaceResult = NULL;
        }
        else
        {
            /*[uStreamClone_complianceNewBufferClonedWithZeroOffsetSucceed]*/
            /*[uStreamClone_complianceNewBufferClonedWithOffsetSucceed]*/
            /*[uStreamClone_complianceNewBufferWithNonZeroCurrentAndReleasedPositionsClonedWithOffsetSucceed]*/
            /*[uStreamClone_complianceNewBufferWithNonZeroCurrentAndReleasedPositionsClonedWithNegativeOffsetSucceed]*/
            /*[uStreamClone_complianceClonedBufferWithNonZeroCurrentAndReleasedPositionsClonedWithOffsetSucceed]*/
            /*[uStreamClone_complianceNoMemoryTocreate_instanceFailed]*/
            /*[uStreamClone_complianceEmptyBufferSucceed]*/
            interfaceResult = create_instance(instance->inner_buffer, instance->inner_current_position, offset);
        }
    }

    return interfaceResult;
}

static ULIB_RESULT concrete_dispose(USTREAM* ustream_interface)
{
    ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, _api))
    {
        /*[uStreamDispose_complianceNullBufferFailed]*/
        /*[uStreamDispose_complianceBufferIsNotTypeOfBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ustream_interface->handle;
        USTREAM_INNER_BUFFER* inner_buffer = instance->inner_buffer;

        /*[uStreamDispose_complianceClonedInstanceDisposedFirstSucceed]*/
        /*[uStreamDispose_complianceClonedInstanceDisposedSecondSucceed]*/
        /*[uStreamDispose_complianceSingleInstanceSucceed]*/
        ULIB_PORT_ATOMIC_DEC_W(&(inner_buffer->ref_count));
        if(inner_buffer->ref_count == 0)
        {
            destroyInnerBuffer(inner_buffer);
        }
        ULIB_CONFIG_FREE(instance);
        ULIB_CONFIG_FREE(ustream_interface);
        result = ULIB_SUCCESS;
    }

    return result;
}

USTREAM* ustream_create(
        const uint8_t* const buffer, 
        size_t buffer_length,
        bool take_ownership)
{
    USTREAM* interfaceResult;

    if(buffer == NULL)
    {
        /*[uStreamCreate_NULLBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "buffer");
        interfaceResult = NULL;
    }
    else if(buffer_length == 0)
    {
        /*[uStreamCreate_zeroLengthFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_EQUALS_STRING, "buffer_length", "0");
        interfaceResult = NULL;
    }
    else
    {
        /*[uStreamCreate_succeed]*/
        USTREAM_INNER_BUFFER* inner_buffer = createInnerBuffer(buffer, buffer_length, take_ownership, true);
        /*[uStreamCreate_noMemoryToCreateProtectedBufferFailed]*/
        /*[uStreamCreate_noMemoryToCreateInnerBufferFailed]*/
        if(inner_buffer == NULL)
        {
            interfaceResult = NULL;
        }
        else
        {
            interfaceResult = create_instance(inner_buffer, 0, 0);
            /*[uStreamCreate_noMemoryTocreate_instanceFailed]*/
            if(interfaceResult == NULL)
            {
                if(take_ownership)
                {
                    ULIB_CONFIG_FREE(inner_buffer);
                }
                else
                {
                    destroyInnerBuffer(inner_buffer);
                }
            }
        }
    }

    return interfaceResult;
}

USTREAM* ustream_const_create(
    const uint8_t* const buffer,
    size_t buffer_length)
{
    USTREAM* interfaceResult;

    if(buffer == NULL)
    {
        /*[uStreamConstCreate_NULLBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "buffer");
        interfaceResult = NULL;
    }
    else if(buffer_length == 0)
    {
        /*[uStreamConstCreate_zeroLengthFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_EQUALS_STRING, "buffer_length", "0");
        interfaceResult = NULL;
    }
    else
    {
        /*[uStreamConstCreate_succeed]*/
        USTREAM_INNER_BUFFER* inner_buffer = createInnerBuffer(buffer, buffer_length, true, false);
        /*[uStreamConstCreate_noMemoryToCreateProtectedBufferFailed]*/
        /*[uStreamConstCreate_noMemoryToCreateInnerBufferFailed]*/
        if(inner_buffer == NULL)
        {
            interfaceResult = NULL;
        }
        else
        {
            interfaceResult = create_instance(inner_buffer, 0, 0);
            /*[uStreamConstCreate_noMemoryTocreate_instanceFailed]*/
            if(interfaceResult == NULL)
            {
                destroyInnerBuffer(inner_buffer);
            }
        }
    }

    return interfaceResult;
}
