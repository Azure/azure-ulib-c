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
        newNode->buffer = ustream_clone(buffer, offset);
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
    ustream_dispose(node->buffer);
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
    USTREAM* ustream_interface = (USTREAM*)ULIB_CONFIG_MALLOC(sizeof(USTREAM));
    /*[uStreamMultiCreate_noMemoryToCreateInstanceFailed]*/
    /*[uStreamClone_noMemoryToCreateInterfaceFailed]*/
    if(ustream_interface == NULL)
    {
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "ustream_interface");
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ULIB_CONFIG_MALLOC(sizeof(USTREAM_MULTI_INSTANCE));
        if(instance == NULL)
        {
            /*[uStreamClone_noMemoryToCreateInstanceFailed]*/
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "uStreamInstance");
            ULIB_CONFIG_FREE(ustream_interface);
            ustream_interface = NULL;
        }
        else
        {
            ustream_interface->api = &_api;
            ustream_interface->handle = (void*)instance;

            instance->innerCurrentPosition = 0;
            instance->innerFirstValidPosition = 0;
            instance->offsetDiff = 0;
            instance->bufferList = NULL;
            instance->currentNode = NULL;
            instance->length = 0;
        }
    }
    return ustream_interface;
}

static void destroyInstance(USTREAM* ustream_interface)
{
    USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

    destroyFullBufferList(instance->bufferList);
    ULIB_CONFIG_FREE(instance);
    ULIB_CONFIG_FREE(ustream_interface);
}

static ULIB_RESULT concreteSeek(
        USTREAM* ustream_interface, 
        offset_t position)
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
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;
        offset_t innerPosition = position - instance->offsetDiff;

        if(innerPosition == instance->innerCurrentPosition)
        {
            /*[ustream_set_position_complianceForwardToTheEndPositionSucceed]*/
            result = ULIB_SUCCESS;
        }
        else if((innerPosition > (offset_t)(instance->length)) || 
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
            /*[ustream_set_position_complianceRunFullBufferByteByByteSucceed]*/
            /*[ustream_set_position_complianceRunFullBufferByteByByteReverseOrderSucceed]*/
            /*[ustream_set_position_complianceClonedBufferBackToBeginningSucceed]*/
            /*[ustream_set_position_complianceClonedBufferBackPositionSucceed]*/
            /*[ustream_set_position_complianceClonedBufferForwardPositionSucceed]*/
            /*[ustream_set_position_complianceClonedBufferForwardToTheEndPositionSucceed]*/
            /*[ustream_set_position_complianceClonedBufferRunFullBufferByteByByteSucceed]*/
            /*[ustream_set_position_complianceClonedBufferRunFullBufferByteByByteReverseOrderSucceed]*/
            result = ULIB_SUCCESS;
            BUFFER_LIST_NODE* node = instance->bufferList;
            BUFFER_LIST_NODE* newCurrentNode = NULL; 
            bool bypassOldCurrentNode = false;
            while((node != NULL) && (result == ULIB_SUCCESS))
            {
                if(node == instance->currentNode)
                {
                    bypassOldCurrentNode = true;
                }
                
                if(newCurrentNode == NULL)
                {
                    offset_t currentPosition;
                    /*[uStreamMultiSeek_innerBufferFailedInGetCurrentPositionFailed]*/
                    if((result = ustream_get_position(node->buffer, &currentPosition)) == ULIB_SUCCESS)
                    {
                        size_t size;
                        if((result = ustream_get_remaining_size(node->buffer, &size)) == ULIB_SUCCESS)
                        {
                            if((currentPosition + size) > innerPosition)
                            {
                                newCurrentNode = node;
                                result = ustream_set_position(node->buffer, innerPosition);
                            }
                            else
                            {
                                result = ustream_set_position(node->buffer, (currentPosition + (offset_t)size - (offset_t)1));
                            }
                        }
                    }
                }
                else
                {
                    (void)ustream_reset(node->buffer);
                    if(bypassOldCurrentNode == true)
                    {
                        break;
                    }
                }
                node = node->next;
            } 

            if(result == ULIB_SUCCESS)
            {
                instance->innerCurrentPosition = innerPosition;
                instance->currentNode = newCurrentNode;
            }
            else
            {
                if(instance->currentNode != NULL)
                {
                    ULIB_RESULT rollbackResult = 
                        ustream_set_position(instance->currentNode->buffer, instance->innerCurrentPosition);
                    if(rollbackResult != ULIB_SUCCESS)
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

static ULIB_RESULT concreteReset(USTREAM* ustream_interface)
{
    ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, _api))
    {
        /*[uStreamReset_complianceNullBufferFailed]*/
        /*[uStreamReset_complianceNonTypeOfBufferAPIFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

        /*[uStreamReset_complianceBackToBeginningSucceed]*/
        /*[uStreamReset_complianceBackPositionSucceed]*/
        result = concreteSeek(ustream_interface, 
                (instance->innerFirstValidPosition + instance->offsetDiff));
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
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

        BUFFER_LIST_NODE* node = instance->currentNode;
        if(node == NULL)
        {
            *size = 0;
            result = ULIB_EOF;
        }
        else
        {
            /*[ustream_read_complianceSingleBufferSucceed]*/
            /*[ustream_read_complianceRightBoundaryConditionSucceed]*/
            /*[ustream_read_complianceBoundaryConditionSucceed]*/
            /*[ustream_read_complianceLeftBoundaryConditionSucceed]*/
            /*[ustream_read_complianceSingleByteSucceed]*/
            /*[ustream_read_complianceGetFromClonedBufferSucceed]*/
            /*[ustream_read_complianceClonedBufferRightBoundaryConditionSucceed]*/
            *size = 0;
            ULIB_RESULT intermediateResult = ULIB_SUCCESS;
            while((node != NULL) &&
                    (*size < buffer_length) &&
                    (intermediateResult == ULIB_SUCCESS))
            {
                size_t totalCopySize;
                /*[ustream_read_complianceSucceed_2]*/
                size_t remainSize = buffer_length - *size;
                /*[ustream_read_complianceSucceed_1]*/
                intermediateResult = ustream_read(node->buffer, &buffer[*size], remainSize, &totalCopySize);
                switch(intermediateResult)
                {
                case ULIB_SUCCESS:
                    *size += totalCopySize;
                    /* do not **break** here.*/
                case ULIB_EOF:
                    if(*size < buffer_length)
                    {
                        node = node->next;
                        instance->currentNode = node;
                        if(node != NULL)
                        {
                            intermediateResult = ULIB_SUCCESS;
                            (void)ustream_reset(node->buffer);
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
                result = ULIB_SUCCESS;
            }
            else
            {
                /*[ustream_read_complianceSucceed_3]*/
                result = intermediateResult;
            }
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
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

        /*[uStreamGetRemainingSize_complianceNewBufferSucceed]*/
        /*[uStreamGetRemainingSize_complianceNewBufferWithNonZeroCurrentPositionSucceed]*/
        /*[uStreamGetRemainingSize_complianceClonedBufferWithNonZeroCurrentPositionSucceed]*/
        *size = instance->length - instance->innerCurrentPosition;
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
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

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
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;
        offset_t innerPosition = position - instance->offsetDiff;
        BUFFER_LIST_NODE* newBufferListStart = instance->bufferList;

        if((innerPosition >= instance->innerCurrentPosition) ||
                (innerPosition < instance->innerFirstValidPosition))
        {
            /*[uStreamRelease_complianceReleaseAfterCurrentFailed]*/
            /*[uStreamRelease_complianceReleasePositionAlreayReleasedFailed]*/
            result = ULIB_ILLEGAL_ARGUMENT_ERROR;
        }
        else
        {
            /*[uStreamRelease_complianceReleaseAllSucceed]*/
            /*[uStreamRelease_complianceRunFullBufferByteByByteSucceed]*/
            /*[uStreamRelease_complianceClonedBufferReleaseAllSucceed]*/
            /*[uStreamRelease_complianceClonedBufferRunFullBufferByteByByteSucceed]*/
            result = ULIB_SUCCESS;
            offset_t lastPosition = 0;
            while((newBufferListStart != NULL) &&
                    ((result = ustream_get_position(newBufferListStart->buffer, &lastPosition)) == ULIB_SUCCESS) && 
                    (innerPosition > lastPosition))
            {
                newBufferListStart = newBufferListStart->next;
            }

            if(result == ULIB_SUCCESS)
            {
                size_t size;
                if((result = ustream_get_remaining_size(newBufferListStart->buffer, &size)) == ULIB_SUCCESS)
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
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

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
                    else if(ustream_get_remaining_size(newNode->buffer, &(newBufferSize)) == ULIB_SUCCESS)
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
        /*[uStreamDispose_complianceClonedInstanceDisposedFirstSucceed]*/
        /*[uStreamDispose_complianceClonedInstanceDisposedSecondSucceed]*/
        /*[uStreamDispose_complianceSingleInstanceSucceed]*/
        destroyInstance(ustream_interface);
        result = ULIB_SUCCESS;
    }

    return result;
}

USTREAM* ustream_multi_create(void)
{
    /*[uStreamMultiCreate_succeed]*/
    /*[uStreamMultiCreate_noMemoryToCreateInterfaceFailed]*/
    return createInstance();
}

ULIB_RESULT ustream_multi_append(
        USTREAM* ustream_interface,
        USTREAM* ustream_to_append)
{
    ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, _api))
    {
        /*[uStreamMultiAppend_nullMultibufferFailed]*/
        /*[uStreamMultiAppend_notMultibufferTypeFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if(ustream_to_append == NULL)
    {
        /*[uStreamMultiAppend_nullBufferToAddFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "ustream_to_append");
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        size_t newBufferSize;
        BUFFER_LIST_NODE* newNode;
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

        /*[uStreamMultiAppend_notEnoughMemoryFailed]*/
        /*[uStreamMultiAppend_notEnoughMemoryToCloneTheBufferFailed]*/
        if((newNode = createBufferNode(ustream_to_append, (offset_t)(instance->length))) == NULL)
        {
            result = ULIB_OUT_OF_MEMORY_ERROR;
        }
        else if((result = ustream_get_remaining_size(newNode->buffer, &(newBufferSize))) != ULIB_SUCCESS)
        {
            /*[uStreamMultiAppend_newInnerBufferFailedOnGetRemainingSizeFailed]*/
            ULIB_CONFIG_LOG(
                ULOG_TYPE_ERROR,
                ULOG_REPORT_EXCEPTION_STRING,
                "ustream_multi_append",
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

