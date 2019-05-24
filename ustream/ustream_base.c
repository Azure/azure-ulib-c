// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ustream.h"

USTREAM_RESULT uStreamAppend(
    USTREAM* uStreamInterface, 
    USTREAM* uStreamToAppend)
{
    USTREAM_RESULT result;

    if((uStreamToAppend == NULL) || 
        (uStreamInterface == NULL)) 
    {
        /*[uStreamAppend_nullBufferToAddFailed]*/
        /*[uStreamAppend_nullInterfaceFailed]*/
        result = USTREAM_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    /*[uStreamAppend_startingFromMultibufferWithNotEnoughMemoryFailed]*/
    else if((result = uStreamMultiAppend(uStreamInterface, uStreamToAppend)) == USTREAM_ILLEGAL_ARGUMENT_EXCEPTION)
    {
        USTREAM* newMultiBuffer = uStreamMultiCreate();
        if(newMultiBuffer == NULL)
        {
            /*[uStreamAppend_notEnoughMemoryToCreateMultibufferFailed]*/
            result = USTREAM_OUT_OF_MEMORY_EXCEPTION;
        }
        else if ((result = uStreamMultiAppend(newMultiBuffer, uStreamInterface)) != USTREAM_SUCCESS)
        {
            /*[uStreamAppend_notEnoughMemoryToAppendFirstBufferFailed]*/
            uStreamDispose(newMultiBuffer);
        }
        else if ((result = uStreamMultiAppend(newMultiBuffer, uStreamToAppend)) != USTREAM_SUCCESS)
        {
            /*[uStreamAppend_notEnoughMemoryToAppendSecondBufferFailed]*/
            uStreamDispose(newMultiBuffer);
        }
        else
        {
            /*[uStreamAppend_startFromEmptyMultibufferSucceed]*/
            /*[uStreamAppend_appendMultipleBuffersSucceed]*/
            USTREAM aux;
            aux.api = uStreamInterface->api;
            aux.handle = uStreamInterface->handle;

            uStreamInterface->api = newMultiBuffer->api;
            uStreamInterface->handle = newMultiBuffer->handle;

            newMultiBuffer->api = aux.api;
            newMultiBuffer->handle = aux.handle;

            uStreamDispose(newMultiBuffer);
        }
    }

    return result;
}

