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
    volatile uint32_t refcount;
} USTREAM_INNER_BUFFER;

typedef struct USTREAM_INSTANCE_TAG
{
    /* Inner buffer. */
    USTREAM_INNER_BUFFER* innerBuffer;

    /* Instance controls. */
    offset_t offsetDiff;
    offset_t innerCurrentPosition;
    offset_t innerFirstValidPosition;
} USTREAM_INSTANCE;

static ULIB_RESULT concreteSeek(USTREAM* ustream_interface, offset_t position);
static ULIB_RESULT concreteReset(USTREAM* ustream_interface);
static ULIB_RESULT concreteGetNext(USTREAM* ustream_interface, uint8_t* const buffer, size_t buffer_length, size_t* const size);
static ULIB_RESULT concreteGetRemainingSize(USTREAM* ustream_interface, size_t* const size);
static ULIB_RESULT concreteGetCurrentPosition(USTREAM* ustream_interface, offset_t* const position);
static ULIB_RESULT concreteRelease(USTREAM* ustream_interface, offset_t position);
static USTREAM* concreteClone(USTREAM* ustream_interface, offset_t offset);
static ULIB_RESULT concreteDispose(USTREAM* ustream_interface);
static const USTREAM_INTERFACE _api =
{
        concreteSeek,
        concreteReset,
        concreteGetNext,
        concreteGetRemainingSize,
        concreteGetCurrentPosition,
        concreteRelease,
        concreteClone,
        concreteDispose
};

static USTREAM* createInstance(
    USTREAM_INNER_BUFFER* innerBuffer,
    offset_t innerCurrentPosition,
    offset_t offset)
{
    USTREAM* ustream_interface = (USTREAM*)ULIB_CONFIG_MALLOC(sizeof(USTREAM));
    /*[uStreamCreate_noMemoryToCreateInterfaceFailed]*/
    /*[uStreamConstCreate_noMemoryToCreateInterfaceFailed]*/
    /*[uStreamClone_noMemoryToCreateInterfaceFailed]*/
    /*[uStreamClone_noMemoryToCreateInstanceFailed]*/
    if(ustream_interface == NULL)
    {
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "ustream_interface");
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ULIB_CONFIG_MALLOC(sizeof(USTREAM_INSTANCE));
        /*[az_stdbufferClone_noMemoryToCreateInstanceFailed]*/
        if(instance != NULL)
        {
            ustream_interface->api = &_api;
            ustream_interface->handle = (void*)instance;

            instance->innerCurrentPosition = innerCurrentPosition;
            instance->innerFirstValidPosition = innerCurrentPosition;
            instance->offsetDiff = offset - innerCurrentPosition;
            instance->innerBuffer = innerBuffer;
            ULIB_PORT_ATOMIC_INC_W(&(instance->innerBuffer->refcount));
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
    bool takeOwnership,
    bool releaseOnDestroy)
{
    uint8_t* ptr;
    USTREAM_INNER_BUFFER* innerBuffer;

    if(takeOwnership)
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
        innerBuffer = NULL;
    }
    else if((innerBuffer = (USTREAM_INNER_BUFFER*)ULIB_CONFIG_MALLOC(sizeof(USTREAM_INNER_BUFFER))) == NULL)
    {
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "inner buffer control");
        if(!takeOwnership)
        {
            ULIB_CONFIG_FREE(ptr);
        }
    }
    else
    {
        innerBuffer->ptr = ptr;
        innerBuffer->length = buffer_length;
        innerBuffer->flags = (releaseOnDestroy ? USTREAM_FLAG_RELEASE_ON_DESTROY : USTREAM_FLAG_NONE);
        innerBuffer->refcount = 0;
    }

    return innerBuffer;
}

static void destroyInnerBuffer(USTREAM_INNER_BUFFER* innerBuffer)
{
    if((innerBuffer->flags & USTREAM_FLAG_RELEASE_ON_DESTROY) == USTREAM_FLAG_RELEASE_ON_DESTROY)
    {
        ULIB_CONFIG_FREE(innerBuffer->ptr);
    }
    ULIB_CONFIG_FREE(innerBuffer);
}

static ULIB_RESULT concreteSeek(USTREAM* ustream_interface, offset_t position)
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
        USTREAM_INNER_BUFFER* innerBuffer = instance->innerBuffer;
        offset_t innerPosition = position - instance->offsetDiff;

        if((innerPosition > (offset_t)(innerBuffer->length)) || 
                (innerPosition < instance->innerFirstValidPosition))
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
            instance->innerCurrentPosition = innerPosition;
            result = ULIB_SUCCESS;
        }
    }

    return result;
}

static ULIB_RESULT concreteReset(USTREAM* ustream_interface)
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
        instance->innerCurrentPosition = instance->innerFirstValidPosition;
        result = ULIB_SUCCESS;
    }

    return result;
}

static ULIB_RESULT concreteGetNext(
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
        USTREAM_INNER_BUFFER* innerBuffer = instance->innerBuffer;

        if(instance->innerCurrentPosition >= innerBuffer->length)
        {
            /*[ustream_read_complianceSucceed_3]*/
            *size = 0;
            result = ULIB_EOF;
        }
        else
        {
            /*[ustream_read_complianceSucceed_2]*/
            size_t remainSize = innerBuffer->length - (size_t)instance->innerCurrentPosition;
            *size = (buffer_length < remainSize) ? buffer_length : remainSize;
            /*[ustream_read_complianceSucceed_1]*/
            /*[ustream_read_complianceSingleBufferSucceed]*/
            /*[ustream_read_complianceRightBoundaryConditionSucceed]*/
            /*[ustream_read_complianceBoundaryConditionSucceed]*/
            /*[ustream_read_complianceLeftBoundaryConditionSucceed]*/
            /*[ustream_read_complianceSingleByteSucceed]*/
            /*[ustream_read_complianceGetFromClonedBufferSucceed]*/
            /*[ustream_read_complianceClonedBufferRightBoundaryConditionSucceed]*/
            (void)memcpy(buffer, innerBuffer->ptr + instance->innerCurrentPosition, *size);
            instance->innerCurrentPosition += *size;
            result = ULIB_SUCCESS;
        }
    }

    return result;
}

static ULIB_RESULT concreteGetRemainingSize(USTREAM* ustream_interface, size_t* const size)
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
        *size = instance->innerBuffer->length - instance->innerCurrentPosition;
        result = ULIB_SUCCESS;
    }

    return result;
}

static ULIB_RESULT concreteGetCurrentPosition(USTREAM* ustream_interface, offset_t* const position)
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
        *position = instance->innerCurrentPosition + instance->offsetDiff;
        result = ULIB_SUCCESS;
    }

    return result;
}

static ULIB_RESULT concreteRelease(USTREAM* ustream_interface, offset_t position)
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
        offset_t innerPosition = position - instance->offsetDiff;

        if((innerPosition >= instance->innerCurrentPosition) ||
                (innerPosition < instance->innerFirstValidPosition))
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
            instance->innerFirstValidPosition = innerPosition + (offset_t)1;
            result = ULIB_SUCCESS;
        }
    }

    return result;
}

static USTREAM* concreteClone(USTREAM* ustream_interface, offset_t offset)
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

        if(offset > (UINT32_MAX - instance->innerBuffer->length))
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
            /*[uStreamClone_complianceNoMemoryToCreateInstanceFailed]*/
            /*[uStreamClone_complianceEmptyBufferSucceed]*/
            interfaceResult = createInstance(instance->innerBuffer, instance->innerCurrentPosition, offset);
        }
    }

    return interfaceResult;
}

static ULIB_RESULT concreteDispose(USTREAM* ustream_interface)
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
        USTREAM_INNER_BUFFER* innerBuffer = instance->innerBuffer;

        /*[uStreamDispose_complianceClonedInstanceDisposedFirstSucceed]*/
        /*[uStreamDispose_complianceClonedInstanceDisposedSecondSucceed]*/
        /*[uStreamDispose_complianceSingleInstanceSucceed]*/
        ULIB_PORT_ATOMIC_DEC_W(&(innerBuffer->refcount));
        if(innerBuffer->refcount == 0)
        {
            destroyInnerBuffer(innerBuffer);
        }
        ULIB_CONFIG_FREE(instance);
        ULIB_CONFIG_FREE(ustream_interface);
        result = ULIB_SUCCESS;
    }

    return result;
}

USTREAM* uStreamCreate(
        const uint8_t* const buffer, 
        size_t buffer_length,
        bool takeOwnership)
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
        USTREAM_INNER_BUFFER* innerBuffer = createInnerBuffer(buffer, buffer_length, takeOwnership, true);
        /*[uStreamCreate_noMemoryToCreateProtectedBufferFailed]*/
        /*[uStreamCreate_noMemoryToCreateInnerBufferFailed]*/
        if(innerBuffer == NULL)
        {
            interfaceResult = NULL;
        }
        else
        {
            interfaceResult = createInstance(innerBuffer, 0, 0);
            /*[uStreamCreate_noMemoryToCreateInstanceFailed]*/
            if(interfaceResult == NULL)
            {
                if(takeOwnership)
                {
                    ULIB_CONFIG_FREE(innerBuffer);
                }
                else
                {
                    destroyInnerBuffer(innerBuffer);
                }
            }
        }
    }

    return interfaceResult;
}

USTREAM* uStreamConstCreate(
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
        USTREAM_INNER_BUFFER* innerBuffer = createInnerBuffer(buffer, buffer_length, true, false);
        /*[uStreamConstCreate_noMemoryToCreateProtectedBufferFailed]*/
        /*[uStreamConstCreate_noMemoryToCreateInnerBufferFailed]*/
        if(innerBuffer == NULL)
        {
            interfaceResult = NULL;
        }
        else
        {
            interfaceResult = createInstance(innerBuffer, 0, 0);
            /*[uStreamConstCreate_noMemoryToCreateInstanceFailed]*/
            if(interfaceResult == NULL)
            {
                destroyInnerBuffer(innerBuffer);
            }
        }
    }

    return interfaceResult;
}
