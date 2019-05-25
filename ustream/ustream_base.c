// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ustream.h"

ULIB_RESULT ustream_append(
    USTREAM* ustream_interface, 
    USTREAM* ustream_to_append)
{
    ULIB_RESULT result;

    if((ustream_to_append == NULL) || 
        (ustream_interface == NULL)) 
    {
        /*[uStreamAppend_nullBufferToAddFailed]*/
        /*[uStreamAppend_nullInterfaceFailed]*/
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    /*[uStreamAppend_startingFromMultibufferWithNotEnoughMemoryFailed]*/
    else if((result = uStreamMultiAppend(ustream_interface, ustream_to_append)) == ULIB_ILLEGAL_ARGUMENT_ERROR)
    {
        USTREAM* newMultiBuffer = uStreamMultiCreate();
        if(newMultiBuffer == NULL)
        {
            /*[uStreamAppend_notEnoughMemoryToCreateMultibufferFailed]*/
            result = ULIB_OUT_OF_MEMORY_ERROR;
        }
        else if ((result = uStreamMultiAppend(newMultiBuffer, ustream_interface)) != ULIB_SUCCESS)
        {
            /*[uStreamAppend_notEnoughMemoryToAppendFirstBufferFailed]*/
            ustream_dispose(newMultiBuffer);
        }
        else if ((result = uStreamMultiAppend(newMultiBuffer, ustream_to_append)) != ULIB_SUCCESS)
        {
            /*[uStreamAppend_notEnoughMemoryToAppendSecondBufferFailed]*/
            ustream_dispose(newMultiBuffer);
        }
        else
        {
            /*[uStreamAppend_startFromEmptyMultibufferSucceed]*/
            /*[uStreamAppend_appendMultipleBuffersSucceed]*/
            USTREAM aux;
            aux.api = ustream_interface->api;
            aux.handle = ustream_interface->handle;

            ustream_interface->api = newMultiBuffer->api;
            ustream_interface->handle = newMultiBuffer->handle;

            newMultiBuffer->api = aux.api;
            newMultiBuffer->handle = aux.handle;

            ustream_dispose(newMultiBuffer);
        }
    }

    return result;
}

