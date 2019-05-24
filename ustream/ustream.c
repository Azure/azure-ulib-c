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

static USTREAM_RESULT concreteSeek(USTREAM* uStreamInterface, offset_t position);
static USTREAM_RESULT concreteReset(USTREAM* uStreamInterface);
static USTREAM_RESULT concreteGetNext(USTREAM* uStreamInterface, uint8_t* const buffer, size_t bufferLength, size_t* const size);
static USTREAM_RESULT concreteGetRemainingSize(USTREAM* uStreamInterface, size_t* const size);
static USTREAM_RESULT concreteGetCurrentPosition(USTREAM* uStreamInterface, offset_t* const position);
static USTREAM_RESULT concreteRelease(USTREAM* uStreamInterface, offset_t position);
static USTREAM* concreteClone(USTREAM* uStreamInterface, offset_t offset);
static USTREAM_RESULT concreteDispose(USTREAM* uStreamInterface);
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
    USTREAM* uStreamInterface = (USTREAM*)ULIB_CONFIG_MALLOC(sizeof(USTREAM));
    /*[uStreamCreate_noMemoryToCreateInterfaceFailed]*/
    /*[uStreamConstCreate_noMemoryToCreateInterfaceFailed]*/
    /*[uStreamClone_noMemoryToCreateInterfaceFailed]*/
    /*[uStreamClone_noMemoryToCreateInstanceFailed]*/
    if(uStreamInterface == NULL)
    {
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "uStreamInterface");
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)ULIB_CONFIG_MALLOC(sizeof(USTREAM_INSTANCE));
        /*[az_stdbufferClone_noMemoryToCreateInstanceFailed]*/
        if(instance != NULL)
        {
            uStreamInterface->api = &_api;
            uStreamInterface->handle = (void*)instance;

            instance->innerCurrentPosition = innerCurrentPosition;
            instance->innerFirstValidPosition = innerCurrentPosition;
            instance->offsetDiff = offset - innerCurrentPosition;
            instance->innerBuffer = innerBuffer;
            ULIB_PORT_ATOMIC_INC_W(&(instance->innerBuffer->refcount));
        }
        else
        {
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "uStreamInstance");
            ULIB_CONFIG_FREE(uStreamInterface);
            uStreamInterface = NULL;
        }
    }

    return uStreamInterface;
}

static USTREAM_INNER_BUFFER* createInnerBuffer(
    const uint8_t* const buffer, 
    size_t bufferLength,
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
        if((ptr = (uint8_t*)ULIB_CONFIG_MALLOC(bufferLength * sizeof(uint8_t))) != NULL)
        {
            (void)memcpy(ptr, buffer, bufferLength);
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
        innerBuffer->length = bufferLength;
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

static USTREAM_RESULT concreteSeek(USTREAM* uStreamInterface, offset_t position)
{
    USTREAM_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(uStreamInterface, _api))
    {
        /*[uStreamSeek_complianceNullBufferFailed]*/
        /*[uStreamSeek_complianceNonTypeOfBufferAPIFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)uStreamInterface->handle;
        USTREAM_INNER_BUFFER* innerBuffer = instance->innerBuffer;
        offset_t innerPosition = position - instance->offsetDiff;

        if((innerPosition > (offset_t)(innerBuffer->length)) || 
                (innerPosition < instance->innerFirstValidPosition))
        {
            /*[uStreamSeek_complianceForwardOutOfTheBufferFailed]*/
            /*[uStreamSeek_complianceBackBeforeFirstValidPositionFailed]*/
            result = USTREAM_NO_SUCH_ELEMENT_EXCEPTION;
        }
        else
        {
            /*[uStreamSeek_complianceBackToBeginningSucceed]*/
            /*[uStreamSeek_complianceBackPositionSucceed]*/
            /*[uStreamSeek_complianceForwardPositionSucceed]*/
            /*[uStreamSeek_complianceForwardToTheEndPositionSucceed]*/
            /*[uStreamSeek_complianceRunFullBufferByteByByteSucceed]*/
            /*[uStreamSeek_complianceRunFullBufferByteByByteReverseOrderSucceed]*/
            /*[uStreamSeek_complianceClonedBufferBackToBeginningSucceed]*/
            /*[uStreamSeek_complianceClonedBufferBackPositionSucceed]*/
            /*[uStreamSeek_complianceClonedBufferForwardPositionSucceed]*/
            /*[uStreamSeek_complianceClonedBufferForwardToTheEndPositionSucceed]*/
            /*[uStreamSeek_complianceClonedBufferRunFullBufferByteByByteSucceed]*/
            /*[uStreamSeek_complianceClonedBufferRunFullBufferByteByByteReverseOrderSucceed]*/
            instance->innerCurrentPosition = innerPosition;
            result = USTREAM_SUCCESS;
        }
    }

    return result;
}

static USTREAM_RESULT concreteReset(USTREAM* uStreamInterface)
{
    USTREAM_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(uStreamInterface, _api))
    {
        /*[uStreamReset_complianceNullBufferFailed]*/
        /*[uStreamReset_complianceNonTypeOfBufferAPIFailed]*/

        result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)uStreamInterface->handle;

        /*[uStreamReset_complianceBackToBeginningSucceed]*/
        /*[uStreamReset_complianceBackPositionSucceed]*/
        instance->innerCurrentPosition = instance->innerFirstValidPosition;
        result = USTREAM_SUCCESS;
    }

    return result;
}

static USTREAM_RESULT concreteGetNext(
        USTREAM* uStreamInterface,
        uint8_t* const buffer,
        size_t bufferLength,
        size_t* const size)
{
    USTREAM_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(uStreamInterface, _api))
    {
        /*[uStreamGetNext_complianceNullBufferFailed]*/
        /*[uStreamGetNext_complianceNonTypeOfBufferAPIFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else if((buffer == NULL) || (size == NULL))
    {
        /*[uStreamGetNext_complianceNullReturnBufferFailed]*/
        /*[uStreamGetNext_complianceNullReturnSizeFailed]*/
        ULIB_CONFIG_LOG(
            ULOG_TYPE_ERROR,
            ULOG_REQUIRE_NOT_NULL_STRING, 
            (buffer == NULL ? "buffer" : "size"));
        result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else if(bufferLength == 0)
    {
        /*[uStreamGetNext_complianceBufferWithZeroSizeFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_EQUALS_STRING, "bufferLength", "0");
        result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)uStreamInterface->handle;
        USTREAM_INNER_BUFFER* innerBuffer = instance->innerBuffer;

        if(instance->innerCurrentPosition >= innerBuffer->length)
        {
            /*[uStreamGetNext_complianceSucceed_3]*/
            *size = 0;
            result = USTREAM_NO_SUCH_ELEMENT_EXCEPTION;
        }
        else
        {
            /*[uStreamGetNext_complianceSucceed_2]*/
            size_t remainSize = innerBuffer->length - (size_t)instance->innerCurrentPosition;
            *size = (bufferLength < remainSize) ? bufferLength : remainSize;
            /*[uStreamGetNext_complianceSucceed_1]*/
            /*[uStreamGetNext_complianceSingleBufferSucceed]*/
            /*[uStreamGetNext_complianceRightBoundaryConditionSucceed]*/
            /*[uStreamGetNext_complianceBoundaryConditionSucceed]*/
            /*[uStreamGetNext_complianceLeftBoundaryConditionSucceed]*/
            /*[uStreamGetNext_complianceSingleByteSucceed]*/
            /*[uStreamGetNext_complianceGetFromClonedBufferSucceed]*/
            /*[uStreamGetNext_complianceClonedBufferRightBoundaryConditionSucceed]*/
            (void)memcpy(buffer, innerBuffer->ptr + instance->innerCurrentPosition, *size);
            instance->innerCurrentPosition += *size;
            result = USTREAM_SUCCESS;
        }
    }

    return result;
}

static USTREAM_RESULT concreteGetRemainingSize(USTREAM* uStreamInterface, size_t* const size)
{
    USTREAM_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(uStreamInterface, _api))
    {
        /*[uStreamGetRemainingSize_complianceNullBufferFailed]*/
        /*[uStreamGetRemainingSize_complianceBufferIsNotTypeOfBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else if(size == NULL)
    {
        /*[uStreamGetRemainingSize_complianceNullSizeFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "size");
        result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)uStreamInterface->handle;

        /*[uStreamGetRemainingSize_complianceNewBufferSucceed]*/
        /*[uStreamGetRemainingSize_complianceNewBufferWithNonZeroCurrentPositionSucceed]*/
        /*[uStreamGetRemainingSize_complianceClonedBufferWithNonZeroCurrentPositionSucceed]*/
        *size = instance->innerBuffer->length - instance->innerCurrentPosition;
        result = USTREAM_SUCCESS;
    }

    return result;
}

static USTREAM_RESULT concreteGetCurrentPosition(USTREAM* uStreamInterface, offset_t* const position)
{
    USTREAM_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(uStreamInterface, _api))
    {
        /*[uStreamGetCurrentPosition_complianceNullBufferFailed]*/
        /*[uStreamGetCurrentPosition_complianceBufferIsNotTypeOfBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else if(position == NULL)
    {
        /*[uStreamGetCurrentPosition_complianceNullPositionFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "position");
        result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)uStreamInterface->handle;

        /*[uStreamGetCurrentPosition_complianceNewBufferSucceed]*/
        /*[uStreamGetCurrentPosition_complianceNewBufferWithNonZeroCurrentPositionSucceed]*/
        /*[uStreamGetCurrentPosition_complianceClonedBufferWithNonZeroCurrentPositionSucceed]*/
        *position = instance->innerCurrentPosition + instance->offsetDiff;
        result = USTREAM_SUCCESS;
    }

    return result;
}

static USTREAM_RESULT concreteRelease(USTREAM* uStreamInterface, offset_t position)
{
    USTREAM_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(uStreamInterface, _api))
    {
        /*[uStreamRelease_complianceNullBufferFailed]*/
        /*[uStreamRelease_complianceNonTypeOfBufferAPIFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)uStreamInterface->handle;
        offset_t innerPosition = position - instance->offsetDiff;

        if((innerPosition >= instance->innerCurrentPosition) ||
                (innerPosition < instance->innerFirstValidPosition))
        {
            /*[uStreamRelease_complianceReleaseAfterCurrentFailed]*/
            /*[uStreamRelease_complianceReleasePositionAlreayReleasedFailed]*/
            result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
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
            result = USTREAM_SUCCESS;
        }
    }

    return result;
}

static USTREAM* concreteClone(USTREAM* uStreamInterface, offset_t offset)
{
    USTREAM* interfaceResult;

    if(USTREAM_IS_NOT_TYPE_OF(uStreamInterface, _api))
    {
        /*[uStreamClone_complianceNullBufferFailed]*/
        /*[uStreamClone_complianceBufferIsNotTypeOfBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        interfaceResult = NULL;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)uStreamInterface->handle;

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

static USTREAM_RESULT concreteDispose(USTREAM* uStreamInterface)
{
    USTREAM_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(uStreamInterface, _api))
    {
        /*[uStreamDispose_complianceNullBufferFailed]*/
        /*[uStreamDispose_complianceBufferIsNotTypeOfBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        USTREAM_INSTANCE* instance = (USTREAM_INSTANCE*)uStreamInterface->handle;
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
        ULIB_CONFIG_FREE(uStreamInterface);
        result = USTREAM_SUCCESS;
    }

    return result;
}

USTREAM* uStreamCreate(
        const uint8_t* const buffer, 
        size_t bufferLength,
        bool takeOwnership)
{
    USTREAM* interfaceResult;

    if(buffer == NULL)
    {
        /*[uStreamCreate_NULLBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "buffer");
        interfaceResult = NULL;
    }
    else if(bufferLength == 0)
    {
        /*[uStreamCreate_zeroLengthFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_EQUALS_STRING, "bufferLength", "0");
        interfaceResult = NULL;
    }
    else
    {
        /*[uStreamCreate_succeed]*/
        USTREAM_INNER_BUFFER* innerBuffer = createInnerBuffer(buffer, bufferLength, takeOwnership, true);
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
    size_t bufferLength)
{
    USTREAM* interfaceResult;

    if(buffer == NULL)
    {
        /*[uStreamConstCreate_NULLBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "buffer");
        interfaceResult = NULL;
    }
    else if(bufferLength == 0)
    {
        /*[uStreamConstCreate_zeroLengthFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_EQUALS_STRING, "bufferLength", "0");
        interfaceResult = NULL;
    }
    else
    {
        /*[uStreamConstCreate_succeed]*/
        USTREAM_INNER_BUFFER* innerBuffer = createInnerBuffer(buffer, bufferLength, true, false);
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
