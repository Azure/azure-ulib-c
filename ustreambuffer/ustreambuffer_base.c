// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ustreambuffer.h"

USTREAMBUFFER_RESULT uStreamBufferAppend(
    USTREAMBUFFER_INTERFACE uStreamBufferInterface, 
    USTREAMBUFFER_INTERFACE uStreamBufferToAppend)
{
    USTREAMBUFFER_RESULT result;

    if((uStreamBufferToAppend == NULL) || 
        (uStreamBufferInterface == NULL)) 
    {
        /*[uStreamBufferAppend_nullBufferToAddFailed]*/
        /*[uStreamBufferAppend_nullInterfaceFailed]*/
        result = USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    /*[uStreamBufferAppend_startingFromMultibufferWithNotEnoughMemoryFailed]*/
    else if((result = uStreamBufferMultiAppend(uStreamBufferInterface, uStreamBufferToAppend)) == USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION)
    {
        USTREAMBUFFER_INTERFACE newMultiBuffer = uStreamBufferMultiCreate();
        if(newMultiBuffer == NULL)
        {
            /*[uStreamBufferAppend_notEnoughMemoryToCreateMultibufferFailed]*/
            result = USTREAMBUFFER_OUT_OF_MEMORY_EXCEPTION;
        }
        else if ((result = uStreamBufferMultiAppend(newMultiBuffer, uStreamBufferInterface)) != USTREAMBUFFER_SUCCESS)
        {
            /*[uStreamBufferAppend_notEnoughMemoryToAppendFirstBufferFailed]*/
            uStreamBufferDispose(newMultiBuffer);
        }
        else if ((result = uStreamBufferMultiAppend(newMultiBuffer, uStreamBufferToAppend)) != USTREAMBUFFER_SUCCESS)
        {
            /*[uStreamBufferAppend_notEnoughMemoryToAppendSecondBufferFailed]*/
            uStreamBufferDispose(newMultiBuffer);
        }
        else
        {
            /*[uStreamBufferAppend_startFromEmptyMultibufferSucceed]*/
            /*[uStreamBufferAppend_appendMultipleBuffersSucceed]*/
            USTREAMBUFFER aux;
            aux.api = uStreamBufferInterface->api;
            aux.handle = uStreamBufferInterface->handle;

            uStreamBufferInterface->api = newMultiBuffer->api;
            uStreamBufferInterface->handle = newMultiBuffer->handle;

            newMultiBuffer->api = aux.api;
            newMultiBuffer->handle = aux.handle;

            uStreamBufferDispose(newMultiBuffer);
        }
    }

    return result;
}

