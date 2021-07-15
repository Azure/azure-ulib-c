// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_result.h"
#include "az_ulib_ustream_forward.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define START_MEMORY_OFFSET 0
static char ustream_forward_consumer_buf[15];
static const char* ustream_forward_producer_buf = "Hello World!\r\n";

typedef struct ustream_forward_consumer_context
{
    uint8_t* offset;
} consumer_context;

void push_callback(const az_span* const buffer, az_context* push_callback_context)
{
    // handle buffer
    az_span_to_str(ustream_forward_consumer_buf, sizeof(ustream_forward_consumer_buf)/sizeof(char), *buffer);

    // adjust offset
    consumer_context* push_context = (consumer_context*)push_callback_context->_internal.value;    
    push_context->offset += az_span_size(*buffer);
}

az_result my_consumer(void)
{
    AZ_ULIB_TRY
    {
        // initialize ustream_forward with producer data
        az_ulib_ustream_forward ustream_forward_instance;
        az_ulib_ustream_forward_data_cb ustream_forward_data_cb;
        AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ustream_forward_init(
            &ustream_forward_instance,
            &ustream_forward_data_cb,
            NULL,
            (uint8_t*)ustream_forward_producer_buf,
            strlen(ustream_forward_producer_buf),
            NULL));
    
        // initialize context
        consumer_context my_consumer_context;
        my_consumer_context.offset = START_MEMORY_OFFSET;
        az_context my_az_context;
        my_az_context._internal.value = (void*)&my_consumer_context;

        // flush from producer to consumer buffer
        AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ustream_forward_flush(
            &ustream_forward_instance,
            push_callback,
            &my_az_context));
        (void)printf("ustream_forward_consumer_buf = %s\r\n", ustream_forward_consumer_buf);
    }
    AZ_ULIB_CATCH(...) {}


    return AZ_OK;
}

int main(void)
{

    AZ_ULIB_TRY
    {
        AZ_ULIB_THROW_IF_AZ_ERROR(my_consumer());
    }
    AZ_ULIB_CATCH(...) {}

    return AZ_ULIB_TRY_RESULT;
}