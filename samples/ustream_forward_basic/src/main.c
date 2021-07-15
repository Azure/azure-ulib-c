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

static const char ustream_forward_consumer_buf[15];
static const char* ustream_forward_producer_buf = "Hello World!\r\n";

// // here is an example of how the push_callback can be used
// struct consumer_context
// {
//    uint8_t* address;
// };

// void push_callback(const az_span *const buffer, az_context* push_callback_context)
// {
//    // consumer can handle buffer ... ex:
//    consumer_context* push_context = (consumer_context*)push_callback_context->_internal.value;
//    flash_write(push_context->address, buffer, az_span_size(buffer));
//    // the consumer is responsible for changing the offset of the write address in the event
//    // that the provider's flush implementation must make multiple external calls to access the 
//    // entirety of the data
//    push_context->address += push_context->size;
// }

// az_result my_consumer(void)
// {
//    az_ulib_ustream_forward* ustream_forward_instance;
//    az_ulib_ustream_forward_data_cb* ustream_forward_data_cb;
//    az_ulib_ustream_init(ustream_forward_instance,
//      ustream_forward_data_cb,
//      ...);
   
//    consumer_context* my_context;
//    my_context->address = START_MEMORY_ADDRESS;
   
//    az_context* my_az_context;
//    my_az_context->_internal.value = (void*)my_context;
//    az_ulib_ustream_forward_flush(ustream_forward_instance,
//        &push_callback,
//        my_az_context);
//    return AZ_OK;
// }

int main(void)
{
    return 0;
}