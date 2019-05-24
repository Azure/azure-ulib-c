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
    USTREAM* buffer;
} BUFFER_LIST_NODE;

typedef struct USTREAM_MULTI_INSTANCE_TAG
{
    /* Inner buffer. */
    BUFFER_LIST_NODE* bufferList;
    BUFFER_LIST_NODE* currentNode;
    size_t length;

    /* Instance controls. */
    offset_t offsetDiff;
    offset_t innerCurrentPosition;
    offset_t innerFirstValidPosition;
} USTREAM_MULTI_INSTANCE;

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

static BUFFER_LIST_NODE* createBufferNode(
    USTREAM* buffer, 
    offset_t offset)
{
    BUFFER_LIST_NODE* newNode = (BUFFER_LIST_NODE*)ULIB_CONFIG_MALLOC(sizeof(BUFFER_LIST_NODE));
    if(newNode == NULL)
    {
        /*[uStreamClone_noMemoryToCreateFirstNodeFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "buffer list");
    }
    else
    {
        newNode->next = NULL;
        newNode->buffer = uStreamClone(buffer, offset);
        if(newNode->buffer == NULL)
        {
            /*[uStreamClone_noMemoryToCloneFirstNodeFailed]*/
            ULIB_CONFIG_FREE(newNode);
            newNode = NULL;
        }
    }
    return newNode;
}

static void destroyBufferNode(BUFFER_LIST_NODE* node)
{
    uStreamDispose(node->buffer);
    ULIB_CONFIG_FREE(node);
}

static void destroyFullBufferList(BUFFER_LIST_NODE* node)
{
    /*[uStreamMultiDispose_multibufferWithoutBuffersFreeAllResourcesSucceed]*/
    /*[uStreamMultiDispose_multibufferWithBuffersFreeAllResourcesSucceed]*/
    while(node != NULL)
    {
        BUFFER_LIST_NODE* temp = node;
        node = node->next;
        destroyBufferNode(temp);
    }
}

static USTREAM* createInstance(void)
{
    USTREAM* uStreamInterface = (USTREAM*)ULIB_CONFIG_MALLOC(sizeof(USTREAM));
    /*[uStreamMultiCreate_noMemoryToCreateInstanceFailed]*/
    /*[uStreamClone_noMemoryToCreateInterfaceFailed]*/
    if(uStreamInterface == NULL)
    {
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "uStreamInterface");
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ULIB_CONFIG_MALLOC(sizeof(USTREAM_MULTI_INSTANCE));
        if(instance == NULL)
        {
            /*[uStreamClone_noMemoryToCreateInstanceFailed]*/
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "uStreamInstance");
            ULIB_CONFIG_FREE(uStreamInterface);
            uStreamInterface = NULL;
        }
        else
        {
            uStreamInterface->api = &_api;
            uStreamInterface->handle = (void*)instance;

            instance->innerCurrentPosition = 0;
            instance->innerFirstValidPosition = 0;
            instance->offsetDiff = 0;
            instance->bufferList = NULL;
            instance->currentNode = NULL;
            instance->length = 0;
        }
    }
    return uStreamInterface;
}

static void destroyInstance(USTREAM* uStreamInterface)
{
    USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)uStreamInterface->handle;

    destroyFullBufferList(instance->bufferList);
    ULIB_CONFIG_FREE(instance);
    ULIB_CONFIG_FREE(uStreamInterface);
}

static USTREAM_RESULT concreteSeek(
        USTREAM* uStreamInterface, 
        offset_t position)
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
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)uStreamInterface->handle;
        offset_t innerPosition = position - instance->offsetDiff;

        if(innerPosition == instance->innerCurrentPosition)
        {
            /*[uStreamSeek_complianceForwardToTheEndPositionSucceed]*/
            result = USTREAM_SUCCESS;
        }
        else if((innerPosition > (offset_t)(instance->length)) || 
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
            /*[uStreamSeek_complianceRunFullBufferByteByByteSucceed]*/
            /*[uStreamSeek_complianceRunFullBufferByteByByteReverseOrderSucceed]*/
            /*[uStreamSeek_complianceClonedBufferBackToBeginningSucceed]*/
            /*[uStreamSeek_complianceClonedBufferBackPositionSucceed]*/
            /*[uStreamSeek_complianceClonedBufferForwardPositionSucceed]*/
            /*[uStreamSeek_complianceClonedBufferForwardToTheEndPositionSucceed]*/
            /*[uStreamSeek_complianceClonedBufferRunFullBufferByteByByteSucceed]*/
            /*[uStreamSeek_complianceClonedBufferRunFullBufferByteByByteReverseOrderSucceed]*/
            result = USTREAM_SUCCESS;
            BUFFER_LIST_NODE* node = instance->bufferList;
            BUFFER_LIST_NODE* newCurrentNode = NULL; 
            bool bypassOldCurrentNode = false;
            while((node != NULL) && (result == USTREAM_SUCCESS))
            {
                if(node == instance->currentNode)
                {
                    bypassOldCurrentNode = true;
                }
                
                if(newCurrentNode == NULL)
                {
                    offset_t currentPosition;
                    /*[uStreamMultiSeek_innerBufferFailedInGetCurrentPositionFailed]*/
                    if((result = uStreamGetCurrentPosition(node->buffer, &currentPosition)) == USTREAM_SUCCESS)
                    {
                        size_t size;
                        if((result = uStreamGetRemainingSize(node->buffer, &size)) == USTREAM_SUCCESS)
                        {
                            if((currentPosition + size) > innerPosition)
                            {
                                newCurrentNode = node;
                                result = uStreamSeek(node->buffer, innerPosition);
                            }
                            else
                            {
                                result = uStreamSeek(node->buffer, (currentPosition + (offset_t)size - (offset_t)1));
                            }
                        }
                    }
                }
                else
                {
                    (void)uStreamReset(node->buffer);
                    if(bypassOldCurrentNode == true)
                    {
                        break;
                    }
                }
                node = node->next;
            } 

            if(result == USTREAM_SUCCESS)
            {
                instance->innerCurrentPosition = innerPosition;
                instance->currentNode = newCurrentNode;
            }
            else
            {
                if(instance->currentNode != NULL)
                {
                    USTREAM_RESULT rollbackResult = 
                        uStreamSeek(instance->currentNode->buffer, instance->innerCurrentPosition);
                    if(rollbackResult != USTREAM_SUCCESS)
                    {
                        ULIB_CONFIG_LOG(
                            ULOG_TYPE_ERROR,
                            ULOG_REPORT_EXCEPTION_STRING,
                            "uStreamMultiSeek rollback",
                            rollbackResult);
                    }
                }
                ULIB_CONFIG_LOG(
                    ULOG_TYPE_ERROR, 
                    ULOG_REPORT_EXCEPTION_STRING,
                    "uStreamMultiSeek",
                    result);
            }
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
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)uStreamInterface->handle;

        /*[uStreamReset_complianceBackToBeginningSucceed]*/
        /*[uStreamReset_complianceBackPositionSucceed]*/
        result = concreteSeek(uStreamInterface, 
                (instance->innerFirstValidPosition + instance->offsetDiff));
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
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)uStreamInterface->handle;

        BUFFER_LIST_NODE* node = instance->currentNode;
        if(node == NULL)
        {
            *size = 0;
            result = USTREAM_NO_SUCH_ELEMENT_EXCEPTION;
        }
        else
        {
            /*[uStreamGetNext_complianceSingleBufferSucceed]*/
            /*[uStreamGetNext_complianceRightBoundaryConditionSucceed]*/
            /*[uStreamGetNext_complianceBoundaryConditionSucceed]*/
            /*[uStreamGetNext_complianceLeftBoundaryConditionSucceed]*/
            /*[uStreamGetNext_complianceSingleByteSucceed]*/
            /*[uStreamGetNext_complianceGetFromClonedBufferSucceed]*/
            /*[uStreamGetNext_complianceClonedBufferRightBoundaryConditionSucceed]*/
            *size = 0;
            USTREAM_RESULT intermediateResult = USTREAM_SUCCESS;
            while((node != NULL) &&
                    (*size < bufferLength) &&
                    (intermediateResult == USTREAM_SUCCESS))
            {
                size_t totalCopySize;
                /*[uStreamGetNext_complianceSucceed_2]*/
                size_t remainSize = bufferLength - *size;
                /*[uStreamGetNext_complianceSucceed_1]*/
                intermediateResult = uStreamGetNext(node->buffer, &buffer[*size], remainSize, &totalCopySize);
                switch(intermediateResult)
                {
                case USTREAM_SUCCESS:
                    *size += totalCopySize;
                    /* do not **break** here.*/
                case USTREAM_NO_SUCH_ELEMENT_EXCEPTION:
                    if(*size < bufferLength)
                    {
                        node = node->next;
                        instance->currentNode = node;
                        if(node != NULL)
                        {
                            intermediateResult = USTREAM_SUCCESS;
                            (void)uStreamReset(node->buffer);
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
                result = USTREAM_SUCCESS;
            }
            else
            {
                /*[uStreamGetNext_complianceSucceed_3]*/
                result = intermediateResult;
            }
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
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)uStreamInterface->handle;

        /*[uStreamGetRemainingSize_complianceNewBufferSucceed]*/
        /*[uStreamGetRemainingSize_complianceNewBufferWithNonZeroCurrentPositionSucceed]*/
        /*[uStreamGetRemainingSize_complianceClonedBufferWithNonZeroCurrentPositionSucceed]*/
        *size = instance->length - instance->innerCurrentPosition;
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
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)uStreamInterface->handle;

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
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)uStreamInterface->handle;
        offset_t innerPosition = position - instance->offsetDiff;
        BUFFER_LIST_NODE* newBufferListStart = instance->bufferList;

        if((innerPosition >= instance->innerCurrentPosition) ||
                (innerPosition < instance->innerFirstValidPosition))
        {
            /*[uStreamRelease_complianceReleaseAfterCurrentFailed]*/
            /*[uStreamRelease_complianceReleasePositionAlreayReleasedFailed]*/
            result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
        }
        else
        {
            /*[uStreamRelease_complianceReleaseAllSucceed]*/
            /*[uStreamRelease_complianceRunFullBufferByteByByteSucceed]*/
            /*[uStreamRelease_complianceClonedBufferReleaseAllSucceed]*/
            /*[uStreamRelease_complianceClonedBufferRunFullBufferByteByByteSucceed]*/
            result = USTREAM_SUCCESS;
            offset_t lastPosition = 0;
            while((newBufferListStart != NULL) &&
                    ((result = uStreamGetCurrentPosition(newBufferListStart->buffer, &lastPosition)) == USTREAM_SUCCESS) && 
                    (innerPosition > lastPosition))
            {
                newBufferListStart = newBufferListStart->next;
            }

            if(result == USTREAM_SUCCESS)
            {
                size_t size;
                if((result = uStreamGetRemainingSize(newBufferListStart->buffer, &size)) == USTREAM_SUCCESS)
                {
                    if((size == 0) && (innerPosition == (lastPosition - 1)))
                    {
                        newBufferListStart = newBufferListStart->next;
                    }

                    /*[uStreamRelease_complianceSucceed]*/
                    /*[uStreamRelease_complianceClonedBufferSucceed]*/
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
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)uStreamInterface->handle;

        if(offset > (UINT32_MAX - instance->length))
        {
            /*[uStreamClone_complianceOffsetExceedSizeFailed]*/
            interfaceResult = NULL;
        }
        else
        {
            /*[uStreamClone_complianceEmptyBufferSucceed]*/
            interfaceResult = createInstance();

            /*[uStreamClone_complianceNoMemoryToCreateInstanceFailed]*/
            if(interfaceResult != NULL)
            {
                USTREAM_MULTI_INSTANCE* newInstance = (USTREAM_MULTI_INSTANCE*)interfaceResult->handle;
                /*[uStreamClone_complianceNewBufferClonedWithZeroOffsetSucceed]*/
                /*[uStreamClone_complianceNewBufferClonedWithOffsetSucceed]*/
                /*[uStreamClone_complianceNewBufferWithNonZeroCurrentAndReleasedPositionsClonedWithOffsetSucceed]*/
                /*[uStreamClone_complianceNewBufferWithNonZeroCurrentAndReleasedPositionsClonedWithNegativeOffsetSucceed]*/
                /*[uStreamClone_complianceClonedBufferWithNonZeroCurrentAndReleasedPositionsClonedWithOffsetSucceed]*/
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
                    else if(uStreamGetRemainingSize(newNode->buffer, &(newBufferSize)) == USTREAM_SUCCESS)
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
        /*[uStreamDispose_complianceClonedInstanceDisposedFirstSucceed]*/
        /*[uStreamDispose_complianceClonedInstanceDisposedSecondSucceed]*/
        /*[uStreamDispose_complianceSingleInstanceSucceed]*/
        destroyInstance(uStreamInterface);
        result = USTREAM_SUCCESS;
    }

    return result;
}

USTREAM* uStreamMultiCreate(void)
{
    /*[uStreamMultiCreate_succeed]*/
    /*[uStreamMultiCreate_noMemoryToCreateInterfaceFailed]*/
    return createInstance();
}

USTREAM_RESULT uStreamMultiAppend(
        USTREAM* uStreamInterface,
        USTREAM* uStreamToAppend)
{
    USTREAM_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(uStreamInterface, _api))
    {
        /*[uStreamMultiAppend_nullMultibufferFailed]*/
        /*[uStreamMultiAppend_notMultibufferTypeFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else if(uStreamToAppend == NULL)
    {
        /*[uStreamMultiAppend_nullBufferToAddFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "uStreamToAppend");
        result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    else
    {
        size_t newBufferSize;
        BUFFER_LIST_NODE* newNode;
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)uStreamInterface->handle;

        /*[uStreamMultiAppend_notEnoughMemoryFailed]*/
        /*[uStreamMultiAppend_notEnoughMemoryToCloneTheBufferFailed]*/
        if((newNode = createBufferNode(uStreamToAppend, (offset_t)(instance->length))) == NULL)
        {
            result = USTREAM_OUT_OF_MEMORY_EXCEPTION;
        }
        else if((result = uStreamGetRemainingSize(newNode->buffer, &(newBufferSize))) != USTREAM_SUCCESS)
        {
            /*[uStreamMultiAppend_newInnerBufferFailedOnGetRemainingSizeFailed]*/
            ULIB_CONFIG_LOG(
                ULOG_TYPE_ERROR,
                ULOG_REPORT_EXCEPTION_STRING,
                "uStreamMultiAppend",
                result);
            destroyBufferNode(newNode);
        }
        else
        {
            /*[uStreamMultiAppend_succeed]*/
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

