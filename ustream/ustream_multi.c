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
    BUFFER_LIST_NODE* buffer_list;
    BUFFER_LIST_NODE* current_node;
    size_t length;

    /* Instance controls. */
    offset_t offset_diff;
    offset_t inner_current_position;
    offset_t inner_first_valid_position;
} USTREAM_MULTI_INSTANCE;

static ULIB_RESULT concrete_set_position(USTREAM* ustream_interface, offset_t position);
static ULIB_RESULT concrete_reset(USTREAM* ustream_interface);
static ULIB_RESULT concrete_read(USTREAM* ustream_interface, uint8_t* const buffer, size_t buffer_length, size_t* const size);
static ULIB_RESULT concrete_get_remaining_size(USTREAM* ustream_interface, size_t* const size);
static ULIB_RESULT concrete_get_position(USTREAM* ustream_interface, offset_t* const position);
static ULIB_RESULT concrete_release(USTREAM* ustream_interface, offset_t position);
static USTREAM* concrete_clone(USTREAM* ustream_interface, offset_t offset);
static ULIB_RESULT concrete_dispose(USTREAM* ustream_interface);
static const USTREAM_INTERFACE api =
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
    USTREAM* buffer, 
    offset_t offset)
{
    BUFFER_LIST_NODE* newNode = (BUFFER_LIST_NODE*)ULIB_CONFIG_MALLOC(sizeof(BUFFER_LIST_NODE));
    if(newNode == NULL)
    {
        /*[ustream_clone_noMemoryToCreateFirstNodeFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "buffer list");
    }
    else
    {
        newNode->next = NULL;
        newNode->buffer = ustream_clone(buffer, offset);
        if(newNode->buffer == NULL)
        {
            /*[ustream_clone_noMemoryToCloneFirstNodeFailed]*/
            ULIB_CONFIG_FREE(newNode);
            newNode = NULL;
        }
    }
    return newNode;
}

static void destroy_buffer_node(BUFFER_LIST_NODE* node)
{
    ustream_dispose(node->buffer);
    ULIB_CONFIG_FREE(node);
}

static void destroy_full_buffer_list(BUFFER_LIST_NODE* node)
{
    /*[ustream_multi_dispose_multibufferWithoutBuffersFreeAllResourcesSucceed]*/
    /*[ustream_multi_dispose_multibufferWithBuffersFreeAllResourcesSucceed]*/
    while(node != NULL)
    {
        BUFFER_LIST_NODE* temp = node;
        node = node->next;
        destroy_buffer_node(temp);
    }
}

static USTREAM* create_instance(void)
{
    USTREAM* ustream_interface = (USTREAM*)ULIB_CONFIG_MALLOC(sizeof(USTREAM));
    /*[ustream_multi_create_noMemoryTocreate_instanceFailed]*/
    /*[ustream_clone_noMemoryToCreateInterfaceFailed]*/
    if(ustream_interface == NULL)
    {
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "ustream_interface");
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ULIB_CONFIG_MALLOC(sizeof(USTREAM_MULTI_INSTANCE));
        if(instance == NULL)
        {
            /*[ustream_clone_noMemoryTocreate_instanceFailed]*/
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "uStreamInstance");
            ULIB_CONFIG_FREE(ustream_interface);
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

static void destroy_instance(USTREAM* ustream_interface)
{
    USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

    destroy_full_buffer_list(instance->buffer_list);
    ULIB_CONFIG_FREE(instance);
    ULIB_CONFIG_FREE(ustream_interface);
}

static ULIB_RESULT concrete_set_position(
        USTREAM* ustream_interface, 
        offset_t position)
{
    ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[ustream_set_position_complianceNullBufferFailed]*/
        /*[ustream_set_position_complianceNonTypeOfBufferAPIFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;
        offset_t innerPosition = position - instance->offset_diff;

        if(innerPosition == instance->inner_current_position)
        {
            /*[ustream_set_position_complianceForwardToTheEndPositionSucceed]*/
            result = ULIB_SUCCESS;
        }
        else if((innerPosition > (offset_t)(instance->length)) || 
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
            /*[ustream_set_position_complianceRunFullBufferByteByByteSucceed]*/
            /*[ustream_set_position_complianceRunFullBufferByteByByteReverseOrderSucceed]*/
            /*[ustream_set_position_complianceClonedBufferBackToBeginningSucceed]*/
            /*[ustream_set_position_complianceClonedBufferBackPositionSucceed]*/
            /*[ustream_set_position_complianceClonedBufferForwardPositionSucceed]*/
            /*[ustream_set_position_complianceClonedBufferForwardToTheEndPositionSucceed]*/
            /*[ustream_set_position_complianceClonedBufferRunFullBufferByteByByteSucceed]*/
            /*[ustream_set_position_complianceClonedBufferRunFullBufferByteByByteReverseOrderSucceed]*/
            result = ULIB_SUCCESS;
            BUFFER_LIST_NODE* node = instance->buffer_list;
            BUFFER_LIST_NODE* newCurrentNode = NULL; 
            bool bypassOldCurrentNode = false;
            while((node != NULL) && (result == ULIB_SUCCESS))
            {
                if(node == instance->current_node)
                {
                    bypassOldCurrentNode = true;
                }
                
                if(newCurrentNode == NULL)
                {
                    offset_t currentPosition;
                    /*[ustream_multi_seek_innerBufferFailedInGetCurrentPositionFailed]*/
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
                instance->inner_current_position = innerPosition;
                instance->current_node = newCurrentNode;
            }
            else
            {
                if(instance->current_node != NULL)
                {
                    ULIB_RESULT rollbackResult = 
                        ustream_set_position(instance->current_node->buffer, instance->inner_current_position);
                    if(rollbackResult != ULIB_SUCCESS)
                    {
                        ULIB_CONFIG_LOG(
                            ULOG_TYPE_ERROR,
                            ULOG_REPORT_EXCEPTION_STRING,
                            "ustream_multi_seek rollback",
                            rollbackResult);
                    }
                }
                ULIB_CONFIG_LOG(
                    ULOG_TYPE_ERROR, 
                    ULOG_REPORT_EXCEPTION_STRING,
                    "ustream_multi_seek",
                    result);
            }
        }
    }

    return result;
}

static ULIB_RESULT concrete_reset(USTREAM* ustream_interface)
{
    ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[ustream_reset_complianceNullBufferFailed]*/
        /*[ustream_reset_complianceNonTypeOfBufferAPIFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

        /*[ustream_reset_complianceBackToBeginningSucceed]*/
        /*[ustream_reset_complianceBackPositionSucceed]*/
        result = concrete_set_position(ustream_interface, 
                (instance->inner_first_valid_position + instance->offset_diff));
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

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
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

        BUFFER_LIST_NODE* node = instance->current_node;
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
                        instance->current_node = node;
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
                instance->inner_current_position += *size;
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

static ULIB_RESULT concrete_get_remaining_size(USTREAM* ustream_interface, size_t* const size)
{
    ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[ustream_get_remaining_size_complianceNullBufferFailed]*/
        /*[ustream_get_remaining_size_complianceBufferIsNotTypeOfBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if(size == NULL)
    {
        /*[ustream_get_remaining_size_complianceNullSizeFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "size");
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

        /*[ustream_get_remaining_size_complianceNewBufferSucceed]*/
        /*[ustream_get_remaining_size_complianceNewBufferWithNonZeroCurrentPositionSucceed]*/
        /*[ustream_get_remaining_size_complianceClonedBufferWithNonZeroCurrentPositionSucceed]*/
        *size = instance->length - instance->inner_current_position;
        result = ULIB_SUCCESS;
    }

    return result;
}

static ULIB_RESULT concrete_get_position(USTREAM* ustream_interface, offset_t* const position)
{
    ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[ustream_get_position_complianceNullBufferFailed]*/
        /*[ustream_get_position_complianceBufferIsNotTypeOfBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if(position == NULL)
    {
        /*[ustream_get_position_complianceNullPositionFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "position");
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

        /*[ustream_get_position_complianceNewBufferSucceed]*/
        /*[ustream_get_position_complianceNewBufferWithNonZeroCurrentPositionSucceed]*/
        /*[ustream_get_position_complianceClonedBufferWithNonZeroCurrentPositionSucceed]*/
        *position = instance->inner_current_position + instance->offset_diff;
        result = ULIB_SUCCESS;
    }

    return result;
}

static ULIB_RESULT concrete_release(USTREAM* ustream_interface, offset_t position)
{
    ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[ustream_release_complianceNullBufferFailed]*/
        /*[ustream_release_complianceNonTypeOfBufferAPIFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;
        offset_t innerPosition = position - instance->offset_diff;
        BUFFER_LIST_NODE* newBufferListStart = instance->buffer_list;

        if((innerPosition >= instance->inner_current_position) ||
                (innerPosition < instance->inner_first_valid_position))
        {
            /*[ustream_release_complianceReleaseAfterCurrentFailed]*/
            /*[ustream_release_complianceReleasePositionAlreayReleasedFailed]*/
            result = ULIB_ILLEGAL_ARGUMENT_ERROR;
        }
        else
        {
            /*[ustream_release_complianceReleaseAllSucceed]*/
            /*[ustream_release_complianceRunFullBufferByteByByteSucceed]*/
            /*[ustream_release_complianceClonedBufferReleaseAllSucceed]*/
            /*[ustream_release_complianceClonedBufferRunFullBufferByteByByteSucceed]*/
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

                    /*[ustream_release_complianceSucceed]*/
                    /*[ustream_release_complianceClonedBufferSucceed]*/
                    // move the first valid position.
                    instance->inner_first_valid_position = innerPosition + (offset_t)1;

                    // release all unnecessary buffers.
                    while(instance->buffer_list != newBufferListStart)
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

static USTREAM* concrete_clone(USTREAM* ustream_interface, offset_t offset)
{
    USTREAM* interfaceResult;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[ustream_clone_complianceNullBufferFailed]*/
        /*[ustream_clone_complianceBufferIsNotTypeOfBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        interfaceResult = NULL;
    }
    else
    {
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

        if(offset > (UINT32_MAX - instance->length))
        {
            /*[ustream_clone_complianceOffsetExceedSizeFailed]*/
            interfaceResult = NULL;
        }
        else
        {
            /*[ustream_clone_complianceEmptyBufferSucceed]*/
            interfaceResult = create_instance();

            /*[ustream_clone_complianceNoMemoryTocreate_instanceFailed]*/
            if(interfaceResult != NULL)
            {
                USTREAM_MULTI_INSTANCE* newInstance = (USTREAM_MULTI_INSTANCE*)interfaceResult->handle;
                /*[ustream_clone_complianceNewBufferClonedWithZeroOffsetSucceed]*/
                /*[ustream_clone_complianceNewBufferClonedWithOffsetSucceed]*/
                /*[ustream_clone_complianceNewBufferWithNonZeroCurrentAndReleasedPositionsClonedWithOffsetSucceed]*/
                /*[ustream_clone_complianceNewBufferWithNonZeroCurrentAndReleasedPositionsClonedWithNegativeOffsetSucceed]*/
                /*[ustream_clone_complianceClonedBufferWithNonZeroCurrentAndReleasedPositionsClonedWithOffsetSucceed]*/
                BUFFER_LIST_NODE* nodeListToClone = instance->current_node;
                BUFFER_LIST_NODE** insertPosition = &(newInstance->buffer_list);
                bool fail = false;
                while((nodeListToClone != NULL) && (fail == false))
                {
                    size_t newBufferSize;
                    BUFFER_LIST_NODE* newNode;

                    if((newNode = create_buffer_node(nodeListToClone->buffer, (offset_t)(newInstance->length))) == NULL)
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
                        destroy_buffer_node(newNode);
                        fail = true;
                    }
                    nodeListToClone = nodeListToClone->next;
                }

                if(fail == true)
                {
                    destroy_instance(interfaceResult);
                    interfaceResult = NULL;
                }
                else
                {
                    newInstance->current_node = newInstance->buffer_list;
                    newInstance->offset_diff = offset - newInstance->inner_current_position;
                }
            }
        }
    }

    return interfaceResult;
}

static ULIB_RESULT concrete_dispose(USTREAM* ustream_interface)
{
    ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[ustream_dispose_complianceNullBufferFailed]*/
        /*[ustream_dispose_complianceBufferIsNotTypeOfBufferFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        /*[ustream_dispose_complianceClonedInstanceDisposedFirstSucceed]*/
        /*[ustream_dispose_complianceClonedInstanceDisposedSecondSucceed]*/
        /*[ustream_dispose_complianceSingleInstanceSucceed]*/
        destroy_instance(ustream_interface);
        result = ULIB_SUCCESS;
    }

    return result;
}

USTREAM* ustream_multi_create(void)
{
    /*[ustream_multi_create_succeed]*/
    /*[ustream_multi_create_noMemoryToCreateInterfaceFailed]*/
    return create_instance();
}

ULIB_RESULT ustream_multi_append(
        USTREAM* ustream_interface,
        USTREAM* ustream_to_append)
{
    ULIB_RESULT result;

    if(USTREAM_IS_NOT_TYPE_OF(ustream_interface, api))
    {
        /*[ustream_multi_append_nullMultibufferFailed]*/
        /*[ustream_multi_append_notMultibufferTypeFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_TYPE_OF_USTREAM_STRING);
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else if(ustream_to_append == NULL)
    {
        /*[ustream_multi_append_nullBufferToAddFailed]*/
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REQUIRE_NOT_NULL_STRING, "ustream_to_append");
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    else
    {
        size_t newBufferSize;
        BUFFER_LIST_NODE* newNode;
        USTREAM_MULTI_INSTANCE* instance = (USTREAM_MULTI_INSTANCE*)ustream_interface->handle;

        /*[ustream_multi_append_notEnoughMemoryFailed]*/
        /*[ustream_multi_append_notEnoughMemoryToCloneTheBufferFailed]*/
        if((newNode = create_buffer_node(ustream_to_append, (offset_t)(instance->length))) == NULL)
        {
            result = ULIB_OUT_OF_MEMORY_ERROR;
        }
        else if((result = ustream_get_remaining_size(newNode->buffer, &(newBufferSize))) != ULIB_SUCCESS)
        {
            /*[ustream_multi_append_newInnerBufferFailedOnGetRemainingSizeFailed]*/
            ULIB_CONFIG_LOG(
                ULOG_TYPE_ERROR,
                ULOG_REPORT_EXCEPTION_STRING,
                "ustream_multi_append",
                result);
            destroy_buffer_node(newNode);
        }
        else
        {
            /*[ustream_multi_append_succeed]*/
            /* find insertion position in the list */
            BUFFER_LIST_NODE** insertPosition = &(instance->buffer_list);

            while(*insertPosition != NULL)
            {
                insertPosition = &((*insertPosition)->next);
            }

            *insertPosition = newNode;
            instance->length += newBufferSize;
            if(instance->current_node == NULL)
            {
                instance->current_node = newNode;
            }
        }
    }

    return result;
}

