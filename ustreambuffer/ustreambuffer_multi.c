// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ustreambuffer.h"
#include "ulib_heap.h"
#include "ulog.h"

typedef struct BUFFER_LIST_NODE_TAG
{
    struct BUFFER_LIST_NODE_TAG* next;
    USTREAMBUFFER_INTERFACE buffer;
} BUFFER_LIST_NODE;

typedef struct USTREAMBUFFER_MULTI_INSTANCE_TAG
{
    /* Inner buffer. */
    BUFFER_LIST_NODE* bufferList;
    BUFFER_LIST_NODE* currentNode;
    size_t length;

    /* Instance controls. */
    offset_t offsetDiff;
    offset_t innerCurrentPosition;
    offset_t innerFirstValidPosition;
} USTREAMBUFFER_MULTI_INSTANCE;

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

static BUFFER_LIST_NODE* createBufferNode(
    USTREAMBUFFER_INTERFACE buffer, 
    offset_t offset)
{
    BUFFER_LIST_NODE* newNode = (BUFFER_LIST_NODE*)ULIB_CONFIG_MALLOC(sizeof(BUFFER_LIST_NODE));
    if(newNode == NULL)
    {
        /*[uStreamBufferClone_noMemoryToCreateFirstNodeFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "buffer list");
    }
    else
    {
        newNode->next = NULL;
        newNode->buffer = uStreamBufferClone(buffer, offset);
        if(newNode->buffer == NULL)
        {
            /*[uStreamBufferClone_noMemoryToCloneFirstNodeFailed]*/
            ULIB_CONFIG_FREE(newNode);
            newNode = NULL;
        }
    }
    return newNode;
}

static void destroyBufferNode(BUFFER_LIST_NODE* node)
{
    uStreamBufferDispose(node->buffer);
    ULIB_CONFIG_FREE(node);
}

static void destroyFullBufferList(BUFFER_LIST_NODE* node)
{
    /*[uStreamBufferMultiDispose_multibufferWithoutBuffersFreeAllResourcesSucceed]*/
    /*[uStreamBufferMultiDispose_multibufferWithBuffersFreeAllResourcesSucceed]*/
    while(node != NULL)
    {
        BUFFER_LIST_NODE* temp = node;
        node = node->next;
        destroyBufferNode(temp);
    }
}

static USTREAMBUFFER_INTERFACE createInstance(void)
{
    USTREAMBUFFER_INTERFACE uStreamBufferInterface = (USTREAMBUFFER_INTERFACE)ULIB_CONFIG_MALLOC(sizeof(USTREAMBUFFER));
    /*[uStreamBufferMultiCreate_noMemoryToCreateInstanceFailed]*/
    /*[uStreamBufferClone_noMemoryToCreateInterfaceFailed]*/
    if(uStreamBufferInterface == NULL)
    {
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "uStreamBufferInterface");
    }
    else
    {
        USTREAMBUFFER_MULTI_INSTANCE* instance = (USTREAMBUFFER_MULTI_INSTANCE*)ULIB_CONFIG_MALLOC(sizeof(USTREAMBUFFER_MULTI_INSTANCE));
        if(instance == NULL)
        {
            /*[uStreamBufferClone_noMemoryToCreateInstanceFailed]*/
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "uStreamBufferInstance");
            ULIB_CONFIG_FREE(uStreamBufferInterface);
            uStreamBufferInterface = NULL;
        }
        else
        {
            uStreamBufferInterface->api = &_api;
            uStreamBufferInterface->handle = (void*)instance;

            instance->innerCurrentPosition = 0;
            instance->innerFirstValidPosition = 0;
            instance->offsetDiff = 0;
            instance->bufferList = NULL;
            instance->currentNode = NULL;
            instance->length = 0;
        }
    }
    return uStreamBufferInterface;
}

static void destroyInstance(USTREAMBUFFER_INTERFACE uStreamBufferInterface)
{
    USTREAMBUFFER_MULTI_INSTANCE* instance = (USTREAMBUFFER_MULTI_INSTANCE*)uStreamBufferInterface->handle;

    destroyFullBufferList(instance->bufferList);
    ULIB_CONFIG_FREE(instance);
    ULIB_CONFIG_FREE(uStreamBufferInterface);
}

static USTREAMBUFFER_RESULT concreteSeek(
        USTREAMBUFFER_INTERFACE uStreamBufferInterface, 
        offset_t position)
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
        USTREAMBUFFER_MULTI_INSTANCE* instance = (USTREAMBUFFER_MULTI_INSTANCE*)uStreamBufferInterface->handle;
        offset_t innerPosition = position - instance->offsetDiff;

        if(innerPosition == instance->innerCurrentPosition)
        {
            /*[uStreamBufferSeek_complianceForwardToTheEndPositionSucceed]*/
            result = USTREAMBUFFER_SUCCESS;
        }
        else if((innerPosition > (offset_t)(instance->length)) || 
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
            /*[uStreamBufferSeek_complianceRunFullBufferByteByByteSucceed]*/
            /*[uStreamBufferSeek_complianceRunFullBufferByteByByteReverseOrderSucceed]*/
            /*[uStreamBufferSeek_complianceClonedBufferBackToBeginningSucceed]*/
            /*[uStreamBufferSeek_complianceClonedBufferBackPositionSucceed]*/
            /*[uStreamBufferSeek_complianceClonedBufferForwardPositionSucceed]*/
            /*[uStreamBufferSeek_complianceClonedBufferForwardToTheEndPositionSucceed]*/
            /*[uStreamBufferSeek_complianceClonedBufferRunFullBufferByteByByteSucceed]*/
            /*[uStreamBufferSeek_complianceClonedBufferRunFullBufferByteByByteReverseOrderSucceed]*/
            result = USTREAMBUFFER_SUCCESS;
            BUFFER_LIST_NODE* node = instance->bufferList;
            BUFFER_LIST_NODE* newCurrentNode = NULL; 
            bool bypassOldCurrentNode = false;
            while((node != NULL) && (result == USTREAMBUFFER_SUCCESS))
            {
                if(node == instance->currentNode)
                {
                    bypassOldCurrentNode = true;
                }
                
                if(newCurrentNode == NULL)
                {
                    offset_t currentPosition;
                    /*[uStreamBufferMultiSeek_innerBufferFailedInGetCurrentPositionFailed]*/
                    if((result = uStreamBufferGetCurrentPosition(node->buffer, &currentPosition)) == USTREAMBUFFER_SUCCESS)
                    {
                        size_t size;
                        if((result = uStreamBufferGetRemainingSize(node->buffer, &size)) == USTREAMBUFFER_SUCCESS)
                        {
                            if((currentPosition + size) > innerPosition)
                            {
                                newCurrentNode = node;
                                result = uStreamBufferSeek(node->buffer, innerPosition);
                            }
                            else
                            {
                                result = uStreamBufferSeek(node->buffer, (currentPosition + (offset_t)size - (offset_t)1));
                            }
                        }
                    }
                }
                else
                {
                    (void)uStreamBufferReset(node->buffer);
                    if(bypassOldCurrentNode == true)
                    {
                        break;
                    }
                }
                node = node->next;
            } 

            if(result == USTREAMBUFFER_SUCCESS)
            {
                instance->innerCurrentPosition = innerPosition;
                instance->currentNode = newCurrentNode;
            }
            else
            {
                if(instance->currentNode != NULL)
                {
                    USTREAMBUFFER_RESULT rollbackResult = 
                        uStreamBufferSeek(instance->currentNode->buffer, instance->innerCurrentPosition);
                    if(rollbackResult != USTREAMBUFFER_SUCCESS)
                    {
                        ULIB_CONFIG_LOG(
                            ULOG_TYPE_ERROR,
                            ULOG_REPORT_EXCEPTION_STRING,
                            "uStreamBufferMultiSeek rollback",
                            rollbackResult);
                    }
                }
                ULIB_CONFIG_LOG(
                    ULOG_TYPE_ERROR, 
                    ULOG_REPORT_EXCEPTION_STRING,
                    "uStreamBufferMultiSeek",
                    result);
            }
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
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAMBUFFER_STRING);
        result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        USTREAMBUFFER_MULTI_INSTANCE* instance = (USTREAMBUFFER_MULTI_INSTANCE*)uStreamBufferInterface->handle;

        /*[uStreamBufferReset_complianceBackToBeginningSucceed]*/
        /*[uStreamBufferReset_complianceBackPositionSucceed]*/
        result = concreteSeek(uStreamBufferInterface, 
                (instance->innerFirstValidPosition + instance->offsetDiff));
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
        USTREAMBUFFER_MULTI_INSTANCE* instance = (USTREAMBUFFER_MULTI_INSTANCE*)uStreamBufferInterface->handle;

        BUFFER_LIST_NODE* node = instance->currentNode;
        if(node == NULL)
        {
            *size = 0;
            result = USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION;
        }
        else
        {
            /*[uStreamBufferGetNext_complianceSingleBufferSucceed]*/
            /*[uStreamBufferGetNext_complianceRightBoundaryConditionSucceed]*/
            /*[uStreamBufferGetNext_complianceBoundaryConditionSucceed]*/
            /*[uStreamBufferGetNext_complianceLeftBoundaryConditionSucceed]*/
            /*[uStreamBufferGetNext_complianceSingleByteSucceed]*/
            /*[uStreamBufferGetNext_complianceGetFromClonedBufferSucceed]*/
            /*[uStreamBufferGetNext_complianceClonedBufferRightBoundaryConditionSucceed]*/
            *size = 0;
            USTREAMBUFFER_RESULT intermediateResult = USTREAMBUFFER_SUCCESS;
            while((node != NULL) &&
                    (*size < bufferLength) &&
                    (intermediateResult == USTREAMBUFFER_SUCCESS))
            {
                size_t totalCopySize;
                /*[uStreamBufferGetNext_complianceSucceed_2]*/
                size_t remainSize = bufferLength - *size;
                /*[uStreamBufferGetNext_complianceSucceed_1]*/
                intermediateResult = uStreamBufferGetNext(node->buffer, &buffer[*size], remainSize, &totalCopySize);
                switch(intermediateResult)
                {
                case USTREAMBUFFER_SUCCESS:
                    *size += totalCopySize;
                    /* do not **break** here.*/
                case USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION:
                    if(*size < bufferLength)
                    {
                        node = node->next;
                        instance->currentNode = node;
                        if(node != NULL)
                        {
                            intermediateResult = USTREAMBUFFER_SUCCESS;
                            (void)uStreamBufferReset(node->buffer);
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
                instance->innerCurrentPosition += *size;
                result = USTREAMBUFFER_SUCCESS;
            }
            else
            {
                /*[uStreamBufferGetNext_complianceSucceed_3]*/
                result = intermediateResult;
            }
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
        USTREAMBUFFER_MULTI_INSTANCE* instance = (USTREAMBUFFER_MULTI_INSTANCE*)uStreamBufferInterface->handle;

        /*[uStreamBufferGetRemainingSize_complianceNewBufferSucceed]*/
        /*[uStreamBufferGetRemainingSize_complianceNewBufferWithNonZeroCurrentPositionSucceed]*/
        /*[uStreamBufferGetRemainingSize_complianceClonedBufferWithNonZeroCurrentPositionSucceed]*/
        *size = instance->length - instance->innerCurrentPosition;
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
        USTREAMBUFFER_MULTI_INSTANCE* instance = (USTREAMBUFFER_MULTI_INSTANCE*)uStreamBufferInterface->handle;

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
        USTREAMBUFFER_MULTI_INSTANCE* instance = (USTREAMBUFFER_MULTI_INSTANCE*)uStreamBufferInterface->handle;
        offset_t innerPosition = position - instance->offsetDiff;
        BUFFER_LIST_NODE* newBufferListStart = instance->bufferList;

        if((innerPosition >= instance->innerCurrentPosition) ||
                (innerPosition < instance->innerFirstValidPosition))
        {
            /*[uStreamBufferRelease_complianceReleaseAfterCurrentFailed]*/
            /*[uStreamBufferRelease_complianceReleasePositionAlreayReleasedFailed]*/
            result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
        }
        else
        {
            /*[uStreamBufferRelease_complianceReleaseAllSucceed]*/
            /*[uStreamBufferRelease_complianceRunFullBufferByteByByteSucceed]*/
            /*[uStreamBufferRelease_complianceClonedBufferReleaseAllSucceed]*/
            /*[uStreamBufferRelease_complianceClonedBufferRunFullBufferByteByByteSucceed]*/
            result = USTREAMBUFFER_SUCCESS;
            offset_t lastPosition = 0;
            while((newBufferListStart != NULL) &&
                    ((result = uStreamBufferGetCurrentPosition(newBufferListStart->buffer, &lastPosition)) == USTREAMBUFFER_SUCCESS) && 
                    (innerPosition > lastPosition))
            {
                newBufferListStart = newBufferListStart->next;
            }

            if(result == USTREAMBUFFER_SUCCESS)
            {
                size_t size;
                if((result = uStreamBufferGetRemainingSize(newBufferListStart->buffer, &size)) == USTREAMBUFFER_SUCCESS)
                {
                    if((size == 0) && (innerPosition == (lastPosition - 1)))
                    {
                        newBufferListStart = newBufferListStart->next;
                    }

                    /*[uStreamBufferRelease_complianceSucceed]*/
                    /*[uStreamBufferRelease_complianceClonedBufferSucceed]*/
                    // move the first valid position.
                    instance->innerFirstValidPosition = innerPosition + (offset_t)1;

                    // release all unnecessary buffers.
                    while(instance->bufferList != newBufferListStart)
                    {
                        BUFFER_LIST_NODE* node = instance->bufferList;
                        instance->bufferList = instance->bufferList->next;
                        destroyBufferNode(node); 
                    }
                }
            }
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
        USTREAMBUFFER_MULTI_INSTANCE* instance = (USTREAMBUFFER_MULTI_INSTANCE*)uStreamBufferInterface->handle;

        if(offset > (UINT32_MAX - instance->length))
        {
            /*[uStreamBufferClone_complianceOffsetExceedSizeFailed]*/
            interfaceResult = NULL;
        }
        else
        {
            /*[uStreamBufferClone_complianceEmptyBufferSucceed]*/
            interfaceResult = createInstance();

            /*[uStreamBufferClone_complianceNoMemoryToCreateInstanceFailed]*/
            if(interfaceResult != NULL)
            {
                USTREAMBUFFER_MULTI_INSTANCE* newInstance = (USTREAMBUFFER_MULTI_INSTANCE*)interfaceResult->handle;
                /*[uStreamBufferClone_complianceNewBufferClonedWithZeroOffsetSucceed]*/
                /*[uStreamBufferClone_complianceNewBufferClonedWithOffsetSucceed]*/
                /*[uStreamBufferClone_complianceNewBufferWithNonZeroCurrentAndReleasedPositionsClonedWithOffsetSucceed]*/
                /*[uStreamBufferClone_complianceNewBufferWithNonZeroCurrentAndReleasedPositionsClonedWithNegativeOffsetSucceed]*/
                /*[uStreamBufferClone_complianceClonedBufferWithNonZeroCurrentAndReleasedPositionsClonedWithOffsetSucceed]*/
                BUFFER_LIST_NODE* nodeListToClone = instance->currentNode;
                BUFFER_LIST_NODE** insertPosition = &(newInstance->bufferList);
                bool fail = false;
                while((nodeListToClone != NULL) && (fail == false))
                {
                    size_t newBufferSize;
                    BUFFER_LIST_NODE* newNode;

                    if((newNode = createBufferNode(nodeListToClone->buffer, (offset_t)(newInstance->length))) == NULL)
                    {
                        fail = true;
                    }
                    else if(uStreamBufferGetRemainingSize(newNode->buffer, &(newBufferSize)) == USTREAMBUFFER_SUCCESS)
                    {
                        // find insertion position in the list
                        *insertPosition = newNode;
                        newInstance->length += newBufferSize;
                        insertPosition = &(newNode->next);
                    }
                    else
                    {
                        destroyBufferNode(newNode);
                        fail = true;
                    }
                    nodeListToClone = nodeListToClone->next;
                }

                if(fail == true)
                {
                    destroyInstance(interfaceResult);
                    interfaceResult = NULL;
                }
                else
                {
                    newInstance->currentNode = newInstance->bufferList;
                    newInstance->offsetDiff = offset - newInstance->innerCurrentPosition;
                }
            }
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
        /*[uStreamBufferDispose_complianceClonedInstanceDisposedFirstSucceed]*/
        /*[uStreamBufferDispose_complianceClonedInstanceDisposedSecondSucceed]*/
        /*[uStreamBufferDispose_complianceSingleInstanceSucceed]*/
        destroyInstance(uStreamBufferInterface);
        result = USTREAMBUFFER_SUCCESS;
    }

    return result;
}

USTREAMBUFFER_INTERFACE uStreamBufferMultiCreate(void)
{
    /*[uStreamBufferMultiCreate_succeed]*/
    /*[uStreamBufferMultiCreate_noMemoryToCreateInterfaceFailed]*/
    return createInstance();
}

USTREAMBUFFER_RESULT uStreamBufferMultiAppend(
        USTREAMBUFFER_INTERFACE uStreamBufferInterface,
        USTREAMBUFFER_INTERFACE uStreamBufferToAppend)
{
    USTREAMBUFFER_RESULT result;

    if(USTREAMBUFFER_IS_NOT_TYPE_OF(uStreamBufferInterface, _api))
    {
        /*[uStreamBufferMultiAppend_nullMultibufferFailed]*/
        /*[uStreamBufferMultiAppend_notMultibufferTypeFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAMBUFFER_STRING);
        result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else if(uStreamBufferToAppend == NULL)
    {
        /*[uStreamBufferMultiAppend_nullBufferToAddFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "uStreamBufferToAppend");
        result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        size_t newBufferSize;
        BUFFER_LIST_NODE* newNode;
        USTREAMBUFFER_MULTI_INSTANCE* instance = (USTREAMBUFFER_MULTI_INSTANCE*)uStreamBufferInterface->handle;

        /*[uStreamBufferMultiAppend_notEnoughMemoryFailed]*/
        /*[uStreamBufferMultiAppend_notEnoughMemoryToCloneTheBufferFailed]*/
        if((newNode = createBufferNode(uStreamBufferToAppend, (offset_t)(instance->length))) == NULL)
        {
            result = USTREAMBUFFER_OUT_OF_MEMORY_EXCEPTION;
        }
        else if((result = uStreamBufferGetRemainingSize(newNode->buffer, &(newBufferSize))) != USTREAMBUFFER_SUCCESS)
        {
            /*[uStreamBufferMultiAppend_newInnerBufferFailedOnGetRemainingSizeFailed]*/
            ULIB_CONFIG_LOG(
                ULOG_TYPE_ERROR,
                ULOG_REPORT_EXCEPTION_STRING,
                "uStreamBufferMultiAppend",
                result);
            destroyBufferNode(newNode);
        }
        else
        {
            /*[uStreamBufferMultiAppend_succeed]*/
            /* find insertion position in the list */
            BUFFER_LIST_NODE** insertPosition = &(instance->bufferList);

            while(*insertPosition != NULL)
            {
                insertPosition = &((*insertPosition)->next);
            }

            *insertPosition = newNode;
            instance->length += newBufferSize;
            if(instance->currentNode == NULL)
            {
                instance->currentNode = newNode;
            }
        }
    }

    return result;
}

