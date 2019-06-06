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
        /*[ustream_append_nullBufferToAddFailed]*/
        /*[ustream_append_nullInterfaceFailed]*/
        result = ULIB_ILLEGAL_ARGUMENT_ERROR;
    }
    /*[ustream_append_startingFromMultibufferWithNotEnoughMemoryFailed]*/
    else if((result = ustream_multi_append(ustream_interface, ustream_to_append)) == ULIB_ILLEGAL_ARGUMENT_ERROR)
    {
        USTREAM* newMultiBuffer = ustream_multi_create();
        if(newMultiBuffer == NULL)
        {
            /*[ustream_append_notEnoughMemoryToCreateMultibufferFailed]*/
            result = ULIB_OUT_OF_MEMORY_ERROR;
        }
        else if ((result = ustream_multi_append(newMultiBuffer, ustream_interface)) != ULIB_SUCCESS)
        {
            /*[ustream_append_notEnoughMemoryToAppendFirstBufferFailed]*/
            ustream_dispose(newMultiBuffer);
        }
        else if ((result = ustream_multi_append(newMultiBuffer, ustream_to_append)) != ULIB_SUCCESS)
        {
            /*[ustream_append_notEnoughMemoryToAppendSecondBufferFailed]*/
            ustream_dispose(newMultiBuffer);
        }
        else
        {
            /*[ustream_append_startFromEmptyMultibufferSucceed]*/
            /*[ustream_append_appendMultipleBuffersSucceed]*/
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

