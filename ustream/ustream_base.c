// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "ustream.h"

extern inline ULIB_RESULT ustream_set_position(USTREAM* ustream_interface, offset_t position);
extern inline ULIB_RESULT ustream_reset(USTREAM* ustream_interface);
extern inline ULIB_RESULT ustream_read(USTREAM* ustream_interface, uint8_t* const buffer, size_t buffer_length, size_t* const size);
extern inline ULIB_RESULT ustream_get_remaining_size(USTREAM* ustream_interface, size_t* const size);
extern inline ULIB_RESULT ustream_get_position(USTREAM* ustream_interface, offset_t* const position);
extern inline ULIB_RESULT ustream_release(USTREAM* ustream_interface, offset_t position);
extern inline USTREAM* ustream_clone(USTREAM* ustream_interface, offset_t offset);
extern inline ULIB_RESULT ustream_dispose(USTREAM* ustream_interface);

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
    else if((result = ustream_multi_append(ustream_interface, ustream_to_append)) == ULIB_ILLEGAL_ARGUMENT_ERROR)
    {
        USTREAM* newMultiBuffer = ustream_multi_create();
        if(newMultiBuffer == NULL)
        {
            /*[uStreamAppend_notEnoughMemoryToCreateMultibufferFailed]*/
            result = ULIB_OUT_OF_MEMORY_ERROR;
        }
        else if ((result = ustream_multi_append(newMultiBuffer, ustream_interface)) != ULIB_SUCCESS)
        {
            /*[uStreamAppend_notEnoughMemoryToAppendFirstBufferFailed]*/
            ustream_dispose(newMultiBuffer);
        }
        else if ((result = ustream_multi_append(newMultiBuffer, ustream_to_append)) != ULIB_SUCCESS)
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

