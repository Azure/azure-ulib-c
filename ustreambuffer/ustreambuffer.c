// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "uStreamBuffer.h"
#include "ulib_port.h"
#include "ulib_heap.h"
#include "ulog.h"

typedef uint8_t                                 USTREAMBUFFER_FLAGS;
#define USTREAMBUFFER_FLAG_NONE                 (USTREAMBUFFER_FLAGS)0x00
#define USTREAMBUFFER_FLAG_RELEASE_ON_DESTROY   (USTREAMBUFFER_FLAGS)0x01

typedef struct USTREAMBUFFER_INNER_BUFFER_TAG
{
    uint8_t* ptr;
    size_t length;
    USTREAMBUFFER_FLAGS flags;
    volatile uint32_t refcount;
} USTREAMBUFFER_INNER_BUFFER;

typedef struct USTREAMBUFFER_INSTANCE_TAG
{
    /* Inner buffer. */
    USTREAMBUFFER_INNER_BUFFER* innerBuffer;

    /* Instance controls. */
    offset_t offsetDiff;
    offset_t innerCurrentPosition;
    offset_t innerFirstValidPosition;
} USTREAMBUFFER_INSTANCE;

static USTREAMBUFFER_RESULT concreteSeek(USTREAMBUFFER_INTERFACE uStreamBufferInterface, offset_t position);
static USTREAMBUFFER_RESULT concreteReset(USTREAMBUFFER_INTERFACE uStreamBufferInterface);
static USTREAMBUFFER_RESULT concreteGetNext(USTREAMBUFFER_INTERFACE uStreamBufferInterface, uint8_t* const buffer, size_t bufferLength, size_t* const size);
static USTREAMBUFFER_RESULT concreteGetRemainingSize(USTREAMBUFFER_INTERFACE uStreamBufferInterface, size_t* const size);
static USTREAMBUFFER_RESULT concreteGetCurrentPosition(USTREAMBUFFER_INTERFACE uStreamBufferInterface, offset_t* const position);
static USTREAMBUFFER_RESULT concreteRelease(USTREAMBUFFER_INTERFACE uStreamBufferInterface, offset_t position);
static USTREAMBUFFER_INTERFACE concreteClone(USTREAMBUFFER_INTERFACE uStreamBufferInterface, offset_t offset);
static USTREAMBUFFER_RESULT concreteDispose(USTREAMBUFFER_INTERFACE uStreamBufferInterface);
static const USTREAMBUFFER_API _api =
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

static USTREAMBUFFER_INTERFACE createInstance(
    USTREAMBUFFER_INNER_BUFFER* innerBuffer,
    offset_t innerCurrentPosition,
    offset_t offset)
{
    USTREAMBUFFER_INTERFACE uStreamBufferInterface = (USTREAMBUFFER_INTERFACE)ULIB_CONFIG_MALLOC(sizeof(USTREAMBUFFER));
    /*[uStreamBufferCreate_noMemoryToCreateInterfaceFailed]*/
    /*[uStreamBufferConstCreate_noMemoryToCreateInterfaceFailed]*/
    /*[uStreamBufferClone_noMemoryToCreateInterfaceFailed]*/
    /*[uStreamBufferClone_noMemoryToCreateInstanceFailed]*/
    if(uStreamBufferInterface == NULL)
    {
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "uStreamBufferInterface");
    }
    else
    {
        USTREAMBUFFER_INSTANCE* instance = (USTREAMBUFFER_INSTANCE*)ULIB_CONFIG_MALLOC(sizeof(USTREAMBUFFER_INSTANCE));
        /*[az_stdbufferClone_noMemoryToCreateInstanceFailed]*/
        if(instance != NULL)
        {
            uStreamBufferInterface->api = &_api;
            uStreamBufferInterface->handle = (void*)instance;

            instance->innerCurrentPosition = innerCurrentPosition;
            instance->innerFirstValidPosition = innerCurrentPosition;
            instance->offsetDiff = offset - innerCurrentPosition;
            instance->innerBuffer = innerBuffer;
            ULIB_PORT_ATOMIC_INC_W(&(instance->innerBuffer->refcount));
        }
        else
        {
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "uStreamBufferInstance");
            ULIB_CONFIG_FREE(uStreamBufferInterface);
            uStreamBufferInterface = NULL;
        }
    }

    return uStreamBufferInterface;
}

static USTREAMBUFFER_INNER_BUFFER* createInnerBuffer(
    const uint8_t* const buffer, 
    size_t bufferLength,
    bool takeOwnership,
    bool releaseOnDestroy)
{
    uint8_t* ptr;
    USTREAMBUFFER_INNER_BUFFER* innerBuffer;

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
    else if((innerBuffer = (USTREAMBUFFER_INNER_BUFFER*)ULIB_CONFIG_MALLOC(sizeof(USTREAMBUFFER_INNER_BUFFER))) == NULL)
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
        innerBuffer->flags = (releaseOnDestroy ? USTREAMBUFFER_FLAG_RELEASE_ON_DESTROY : USTREAMBUFFER_FLAG_NONE);
        innerBuffer->refcount = 0;
    }

    return innerBuffer;
}

static void destroyInnerBuffer(USTREAMBUFFER_INNER_BUFFER* innerBuffer)
{
    if((innerBuffer->flags & USTREAMBUFFER_FLAG_RELEASE_ON_DESTROY) == USTREAMBUFFER_FLAG_RELEASE_ON_DESTROY)
    {
        ULIB_CONFIG_FREE(innerBuffer->ptr);
    }
    ULIB_CONFIG_FREE(innerBuffer);
}

static USTREAMBUFFER_RESULT concreteSeek(USTREAMBUFFER_INTERFACE uStreamBufferInterface, offset_t position)
{
    USTREAMBUFFER_RESULT result;

    if(USTREAMBUFFER_IS_NOT_TYPE_OF(uStreamBufferInterface, _api))
    {
        /*[uStreamBufferSeek_complianceNullBufferFailed]*/
        /*[uStreamBufferSeek_complianceNonTypeOfBufferAPIFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAMBUFFER_STRING);
        result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        USTREAMBUFFER_INSTANCE* instance = (USTREAMBUFFER_INSTANCE*)uStreamBufferInterface->handle;
        USTREAMBUFFER_INNER_BUFFER* innerBuffer = instance->innerBuffer;
        offset_t innerPosition = position - instance->offsetDiff;

        if((innerPosition > (offset_t)(innerBuffer->length)) || 
                (innerPosition < instance->innerFirstValidPosition))
        {
            /*[uStreamBufferSeek_complianceForwardOutOfTheBufferFailed]*/
            /*[uStreamBufferSeek_complianceBackBeforeFirstValidPositionFailed]*/
            result = USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION;
        }
        else
        {
            /*[uStreamBufferSeek_complianceBackToBeginningSucceed]*/
            /*[uStreamBufferSeek_complianceBackPositionSucceed]*/
            /*[uStreamBufferSeek_complianceForwardPositionSucceed]*/
            /*[uStreamBufferSeek_complianceForwardToTheEndPositionSucceed]*/
            /*[uStreamBufferSeek_complianceRunFullBufferByteByByteSucceed]*/
            /*[uStreamBufferSeek_complianceRunFullBufferByteByByteReverseOrderSucceed]*/
            /*[uStreamBufferSeek_complianceClonedBufferBackToBeginningSucceed]*/
            /*[uStreamBufferSeek_complianceClonedBufferBackPositionSucceed]*/
            /*[uStreamBufferSeek_complianceClonedBufferForwardPositionSucceed]*/
            /*[uStreamBufferSeek_complianceClonedBufferForwardToTheEndPositionSucceed]*/
            /*[uStreamBufferSeek_complianceClonedBufferRunFullBufferByteByByteSucceed]*/
            /*[uStreamBufferSeek_complianceClonedBufferRunFullBufferByteByByteReverseOrderSucceed]*/
            instance->innerCurrentPosition = innerPosition;
            result = USTREAMBUFFER_SUCCESS;
        }
    }

    return result;
}

static USTREAMBUFFER_RESULT concreteReset(USTREAMBUFFER_INTERFACE uStreamBufferInterface)
{
    USTREAMBUFFER_RESULT result;

    if(USTREAMBUFFER_IS_NOT_TYPE_OF(uStreamBufferInterface, _api))
    {
        /*[uStreamBufferReset_complianceNullBufferFailed]*/
        /*[uStreamBufferReset_complianceNonTypeOfBufferAPIFailed]*/

        result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        USTREAMBUFFER_INSTANCE* instance = (USTREAMBUFFER_INSTANCE*)uStreamBufferInterface->handle;

        /*[uStreamBufferReset_complianceBackToBeginningSucceed]*/
        /*[uStreamBufferReset_complianceBackPositionSucceed]*/
        instance->innerCurrentPosition = instance->innerFirstValidPosition;
        result = USTREAMBUFFER_SUCCESS;
    }

    return result;
}

static USTREAMBUFFER_RESULT concreteGetNext(
        USTREAMBUFFER_INTERFACE uStreamBufferInterface,
        uint8_t* const buffer,
        size_t bufferLength,
        size_t* const size)
{
    USTREAMBUFFER_RESULT result;

    if(USTREAMBUFFER_IS_NOT_TYPE_OF(uStreamBufferInterface, _api))
    {
        /*[uStreamBufferGetNext_complianceNullBufferFailed]*/
        /*[uStreamBufferGetNext_complianceNonTypeOfBufferAPIFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAMBUFFER_STRING);
        result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else if((buffer == NULL) || (size == NULL))
    {
        /*[uStreamBufferGetNext_complianceNullReturnBufferFailed]*/
        /*[uStreamBufferGetNext_complianceNullReturnSizeFailed]*/
        ULIB_CONFIG_LOG(
            ULOG_TYPE_ERROR,
            ULOG_REQUIRE_NOT_NULL_STRING, 
            (buffer == NULL ? "buffer" : "size"));
        result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else if(bufferLength == 0)
    {
        /*[uStreamBufferGetNext_complianceBufferWithZeroSizeFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_EQUALS_STRING, "bufferLength", "0");
        result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        USTREAMBUFFER_INSTANCE* instance = (USTREAMBUFFER_INSTANCE*)uStreamBufferInterface->handle;
        USTREAMBUFFER_INNER_BUFFER* innerBuffer = instance->innerBuffer;

        if(instance->innerCurrentPosition >= innerBuffer->length)
        {
            /*[uStreamBufferGetNext_complianceSucceed_3]*/
            *size = 0;
            result = USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION;
        }
        else
        {
            /*[uStreamBufferGetNext_complianceSucceed_2]*/
            size_t remainSize = innerBuffer->length - (size_t)instance->innerCurrentPosition;
            *size = (bufferLength < remainSize) ? bufferLength : remainSize;
            /*[uStreamBufferGetNext_complianceSucceed_1]*/
            /*[uStreamBufferGetNext_complianceSingleBufferSucceed]*/
            /*[uStreamBufferGetNext_complianceRightBoundaryConditionSucceed]*/
            /*[uStreamBufferGetNext_complianceBoundaryConditionSucceed]*/
            /*[uStreamBufferGetNext_complianceLeftBoundaryConditionSucceed]*/
            /*[uStreamBufferGetNext_complianceSingleByteSucceed]*/
            /*[uStreamBufferGetNext_complianceGetFromClonedBufferSucceed]*/
            /*[uStreamBufferGetNext_complianceClonedBufferRightBoundaryConditionSucceed]*/
            (void)memcpy(buffer, innerBuffer->ptr + instance->innerCurrentPosition, *size);
            instance->innerCurrentPosition += *size;
            result = USTREAMBUFFER_SUCCESS;
        }
    }

    return result;
}

static USTREAMBUFFER_RESULT concreteGetRemainingSize(USTREAMBUFFER_INTERFACE uStreamBufferInterface, size_t* const size)
{
    USTREAMBUFFER_RESULT result;

    if(USTREAMBUFFER_IS_NOT_TYPE_OF(uStreamBufferInterface, _api))
    {
        /*[uStreamBufferGetRemainingSize_complianceNullBufferFailed]*/
        /*[uStreamBufferGetRemainingSize_complianceBufferIsNotTypeOfBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAMBUFFER_STRING);
        result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else if(size == NULL)
    {
        /*[uStreamBufferGetRemainingSize_complianceNullSizeFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "size");
        result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        USTREAMBUFFER_INSTANCE* instance = (USTREAMBUFFER_INSTANCE*)uStreamBufferInterface->handle;

        /*[uStreamBufferGetRemainingSize_complianceNewBufferSucceed]*/
        /*[uStreamBufferGetRemainingSize_complianceNewBufferWithNonZeroCurrentPositionSucceed]*/
        /*[uStreamBufferGetRemainingSize_complianceClonedBufferWithNonZeroCurrentPositionSucceed]*/
        *size = instance->innerBuffer->length - instance->innerCurrentPosition;
        result = USTREAMBUFFER_SUCCESS;
    }

    return result;
}

static USTREAMBUFFER_RESULT concreteGetCurrentPosition(USTREAMBUFFER_INTERFACE uStreamBufferInterface, offset_t* const position)
{
    USTREAMBUFFER_RESULT result;

    if(USTREAMBUFFER_IS_NOT_TYPE_OF(uStreamBufferInterface, _api))
    {
        /*[uStreamBufferGetCurrentPosition_complianceNullBufferFailed]*/
        /*[uStreamBufferGetCurrentPosition_complianceBufferIsNotTypeOfBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAMBUFFER_STRING);
        result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else if(position == NULL)
    {
        /*[uStreamBufferGetCurrentPosition_complianceNullPositionFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "position");
        result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        USTREAMBUFFER_INSTANCE* instance = (USTREAMBUFFER_INSTANCE*)uStreamBufferInterface->handle;

        /*[uStreamBufferGetCurrentPosition_complianceNewBufferSucceed]*/
        /*[uStreamBufferGetCurrentPosition_complianceNewBufferWithNonZeroCurrentPositionSucceed]*/
        /*[uStreamBufferGetCurrentPosition_complianceClonedBufferWithNonZeroCurrentPositionSucceed]*/
        *position = instance->innerCurrentPosition + instance->offsetDiff;
        result = USTREAMBUFFER_SUCCESS;
    }

    return result;
}

static USTREAMBUFFER_RESULT concreteRelease(USTREAMBUFFER_INTERFACE uStreamBufferInterface, offset_t position)
{
    USTREAMBUFFER_RESULT result;

    if(USTREAMBUFFER_IS_NOT_TYPE_OF(uStreamBufferInterface, _api))
    {
        /*[uStreamBufferRelease_complianceNullBufferFailed]*/
        /*[uStreamBufferRelease_complianceNonTypeOfBufferAPIFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAMBUFFER_STRING);
        result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        USTREAMBUFFER_INSTANCE* instance = (USTREAMBUFFER_INSTANCE*)uStreamBufferInterface->handle;
        offset_t innerPosition = position - instance->offsetDiff;

        if((innerPosition >= instance->innerCurrentPosition) ||
                (innerPosition < instance->innerFirstValidPosition))
        {
            /*[uStreamBufferRelease_complianceReleaseAfterCurrentFailed]*/
            /*[uStreamBufferRelease_complianceReleasePositionAlreayReleasedFailed]*/
            result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
        }
        else
        {
            /*[uStreamBufferRelease_complianceSucceed]*/
            /*[uStreamBufferRelease_complianceReleaseAllSucceed]*/
            /*[uStreamBufferRelease_complianceRunFullBufferByteByByteSucceed]*/
            /*[uStreamBufferRelease_complianceClonedBufferSucceed]*/
            /*[uStreamBufferRelease_complianceClonedBufferReleaseAllSucceed]*/
            /*[uStreamBufferRelease_complianceClonedBufferRunFullBufferByteByByteSucceed]*/
            instance->innerFirstValidPosition = innerPosition + (offset_t)1;
            result = USTREAMBUFFER_SUCCESS;
        }
    }

    return result;
}

static USTREAMBUFFER_INTERFACE concreteClone(USTREAMBUFFER_INTERFACE uStreamBufferInterface, offset_t offset)
{
    USTREAMBUFFER_INTERFACE interfaceResult;

    if(USTREAMBUFFER_IS_NOT_TYPE_OF(uStreamBufferInterface, _api))
    {
        /*[uStreamBufferClone_complianceNullBufferFailed]*/
        /*[uStreamBufferClone_complianceBufferIsNotTypeOfBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAMBUFFER_STRING);
        interfaceResult = NULL;
    }
    else
    {
        USTREAMBUFFER_INSTANCE* instance = (USTREAMBUFFER_INSTANCE*)uStreamBufferInterface->handle;

        if(offset > (UINT32_MAX - instance->innerBuffer->length))
        {
            /*[uStreamBufferClone_complianceOffsetExceedSizeFailed]*/
            interfaceResult = NULL;
        }
        else
        {
            /*[uStreamBufferClone_complianceNewBufferClonedWithZeroOffsetSucceed]*/
            /*[uStreamBufferClone_complianceNewBufferClonedWithOffsetSucceed]*/
            /*[uStreamBufferClone_complianceNewBufferWithNonZeroCurrentAndReleasedPositionsClonedWithOffsetSucceed]*/
            /*[uStreamBufferClone_complianceNewBufferWithNonZeroCurrentAndReleasedPositionsClonedWithNegativeOffsetSucceed]*/
            /*[uStreamBufferClone_complianceClonedBufferWithNonZeroCurrentAndReleasedPositionsClonedWithOffsetSucceed]*/
            /*[uStreamBufferClone_complianceNoMemoryToCreateInstanceFailed]*/
            /*[uStreamBufferClone_complianceEmptyBufferSucceed]*/
            interfaceResult = createInstance(instance->innerBuffer, instance->innerCurrentPosition, offset);
        }
    }

    return interfaceResult;
}

static USTREAMBUFFER_RESULT concreteDispose(USTREAMBUFFER_INTERFACE uStreamBufferInterface)
{
    USTREAMBUFFER_RESULT result;

    if(USTREAMBUFFER_IS_NOT_TYPE_OF(uStreamBufferInterface, _api))
    {
        /*[uStreamBufferDispose_complianceNullBufferFailed]*/
        /*[uStreamBufferDispose_complianceBufferIsNotTypeOfBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAMBUFFER_STRING);
        result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        USTREAMBUFFER_INSTANCE* instance = (USTREAMBUFFER_INSTANCE*)uStreamBufferInterface->handle;
        USTREAMBUFFER_INNER_BUFFER* innerBuffer = instance->innerBuffer;

        /*[uStreamBufferDispose_complianceClonedInstanceDisposedFirstSucceed]*/
        /*[uStreamBufferDispose_complianceClonedInstanceDisposedSecondSucceed]*/
        /*[uStreamBufferDispose_complianceSingleInstanceSucceed]*/
        ULIB_PORT_ATOMIC_DEC_W(&(innerBuffer->refcount));
        if(innerBuffer->refcount == 0)
        {
            destroyInnerBuffer(innerBuffer);
        }
        ULIB_CONFIG_FREE(instance);
        ULIB_CONFIG_FREE(uStreamBufferInterface);
        result = USTREAMBUFFER_SUCCESS;
    }

    return result;
}

USTREAMBUFFER_INTERFACE uStreamBufferCreate(
        const uint8_t* const buffer, 
        size_t bufferLength,
        bool takeOwnership)
{
    USTREAMBUFFER_INTERFACE interfaceResult;

    if(buffer == NULL)
    {
        /*[uStreamBufferCreate_NULLBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "buffer");
        interfaceResult = NULL;
    }
    else if(bufferLength == 0)
    {
        /*[uStreamBufferCreate_zeroLengthFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_EQUALS_STRING, "bufferLength", "0");
        interfaceResult = NULL;
    }
    else
    {
        /*[uStreamBufferCreate_succeed]*/
        USTREAMBUFFER_INNER_BUFFER* innerBuffer = createInnerBuffer(buffer, bufferLength, takeOwnership, true);
        /*[uStreamBufferCreate_noMemoryToCreateProtectedBufferFailed]*/
        /*[uStreamBufferCreate_noMemoryToCreateInnerBufferFailed]*/
        if(innerBuffer == NULL)
        {
            interfaceResult = NULL;
        }
        else
        {
            interfaceResult = createInstance(innerBuffer, 0, 0);
            /*[uStreamBufferCreate_noMemoryToCreateInstanceFailed]*/
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

USTREAMBUFFER_INTERFACE uStreamBufferConstCreate(
    const uint8_t* const buffer,
    size_t bufferLength)
{
    USTREAMBUFFER_INTERFACE interfaceResult;

    if(buffer == NULL)
    {
        /*[uStreamBufferConstCreate_NULLBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "buffer");
        interfaceResult = NULL;
    }
    else if(bufferLength == 0)
    {
        /*[uStreamBufferConstCreate_zeroLengthFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_EQUALS_STRING, "bufferLength", "0");
        interfaceResult = NULL;
    }
    else
    {
        /*[uStreamBufferConstCreate_succeed]*/
        USTREAMBUFFER_INNER_BUFFER* innerBuffer = createInnerBuffer(buffer, bufferLength, true, false);
        /*[uStreamBufferConstCreate_noMemoryToCreateProtectedBufferFailed]*/
        /*[uStreamBufferConstCreate_noMemoryToCreateInnerBufferFailed]*/
        if(innerBuffer == NULL)
        {
            interfaceResult = NULL;
        }
        else
        {
            interfaceResult = createInstance(innerBuffer, 0, 0);
            /*[uStreamBufferConstCreate_noMemoryToCreateInstanceFailed]*/
            if(interfaceResult == NULL)
            {
                destroyInnerBuffer(innerBuffer);
            }
        }
    }

    return interfaceResult;
}
