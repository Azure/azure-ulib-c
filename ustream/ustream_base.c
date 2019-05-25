// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ustream.h"

ULIB_RESULT ustream_append(
    USTREAM* uStreamInterface, 
    USTREAM* uStreamToAppend)
{
    ULIB_RESULT result;

    if((uStreamToAppend == NULL) || 
        (uStreamInterface == NULL)) 
    {
        /*[uStreamAppend_nullBufferToAddFailed]*/
        /*[uStreamAppend_nullInterfaceFailed]*/
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    /*[uStreamAppend_startingFromMultibufferWithNotEnoughMemoryFailed]*/
    else if((result = uStreamMultiAppend(uStreamInterface, uStreamToAppend)) == ULIB_ILLEGAL_ARGUMENT_ERROR)
    {
        USTREAM* newMultiBuffer = uStreamMultiCreate();
        if(newMultiBuffer == NULL)
        {
            /*[uStreamAppend_notEnoughMemoryToCreateMultibufferFailed]*/
            result = ULIB_OUT_OF_MEMORY_ERROR;
        }
        else if ((result = uStreamMultiAppend(newMultiBuffer, uStreamInterface)) != ULIB_SUCCESS)
        {
            /*[uStreamAppend_notEnoughMemoryToAppendFirstBufferFailed]*/
            ustream_dispose(newMultiBuffer);
        }
        else if ((result = uStreamMultiAppend(newMultiBuffer, uStreamToAppend)) != ULIB_SUCCESS)
        {
            /*[uStreamAppend_notEnoughMemoryToAppendSecondBufferFailed]*/
            ustream_dispose(newMultiBuffer);
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

            ustream_dispose(newMultiBuffer);
        }
    }

    return result;
}

