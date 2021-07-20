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
static char ustream_forward_consumer_buf[13] = { 0 };
static const char* ustream_forward_producer_buf = "Hello World!";

void push_callback(const az_span* const buffer, az_ulib_callback_context push_callback_context);
az_result my_consumer(void);

typedef struct ustream_forward_basic_context
{
   offset_t offset;
} consumer_context;

void push_callback(const az_span* const buffer, az_ulib_callback_context push_callback_context)
{
    // handle buffer
    az_span_to_str(ustream_forward_consumer_buf, sizeof(ustream_forward_consumer_buf)/sizeof(char), *buffer);
    // adjust offset
    consumer_context* push_context = (consumer_context*)push_callback_context;    
    push_context->offset += (offset_t)az_span_size(*buffer);
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
            (const uint8_t*)ustream_forward_producer_buf,
            strlen(ustream_forward_producer_buf),
            NULL));
    
        // initialize context
        consumer_context my_consumer_context;
        my_consumer_context.offset = START_MEMORY_OFFSET;
        az_ulib_callback_context callback_context;
        callback_context = (void*)&my_consumer_context;
        
        (void)printf("ustream_forward_consumer_buf = %s\r\n", ustream_forward_consumer_buf);
        (void)printf("my_consumer_context.offset = %zu\r\n", my_consumer_context.offset);
        
        // flush from producer to consumer buffer
        (void)printf("----- FLUSH ----\r\n");
        AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ustream_forward_flush(
            &ustream_forward_instance,
            push_callback,
            callback_context));

        consumer_context* result_consumer_context = (consumer_context*)callback_context;
        (void)printf("ustream_forward_consumer_buf = %s\r\n", ustream_forward_consumer_buf);
        (void)printf("my_consumer_context.offset= %zu \r\n", result_consumer_context->offset);
    }
    AZ_ULIB_CATCH(...) {}

    return AZ_OK;
}

int main(void)
{
    az_result result;

    if ((result = my_consumer()) != AZ_OK)
    {
      printf("my_consumer() failed\r\n");
    }

    return 0;
}
