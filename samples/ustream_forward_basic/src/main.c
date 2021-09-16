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
static const char ustream_forward_producer_buf[] = "Hello World!\r\n";

typedef struct 
{
  offset_t offset;
  char buffer[100];
} consumer_data_cb;

static az_result my_consumer(void)
{
  AZ_ULIB_TRY
  {
    // initialize ustream_forward with producer data
    az_ulib_ustream_forward ustream_forward_instance;
    AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ustream_forward_init(
        &ustream_forward_instance,
        NULL,
        (const uint8_t*)ustream_forward_producer_buf,
        sizeof(ustream_forward_producer_buf),
        NULL));

    // initialize context
    consumer_data_cb consumer_data = { 0 };
    (void)printf("consumer_data.buffer = %s\r\n", consumer_data.buffer);

    // read direct (without memcpy) from producer to consumer buffer
    (void)printf("----- READ DIRECT ----\r\n");

    // grab az_span pointing to producer data
    az_span span = AZ_SPAN_EMPTY;
    size_t size = 0;
    AZ_ULIB_THROW_IF_AZ_ERROR(
        az_ulib_ustream_forward_read(&ustream_forward_instance, &span, &size));
    
    // write producer data to consumer buffer
    (void)snprintf(
        consumer_data.buffer + consumer_data.offset, 
        size, 
        "%s", az_span_ptr(span));

    // increase offset
    consumer_data.offset += size;

    (void)printf("consumer_data.buffer = %s\r\n", consumer_data.buffer);
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
}

    // // start ustream data transfer from blob client
    // az_span user_buffer = az_span_create(buffer[100], 100)
    // az_span output_buffer;
    // else
    // {
    //   do
    //   {
    //     // grab next buffer-full from ustream_instance
    //     if ((result = az_ulib_ustream_forward_read(&ustream_forward_instance, user_buffer, &output_buffer)))
    //         == AZ_OK) // should not use EOF
    //     {
    //       // write to flash if we have not reached the end of this chunk of data
    //       if ((hal_status = internal_flash_write((uint8_t*)address, az_span_ptr(output_buffer), az_span_size(output_buffer)))
    //           != HAL_OK)
    //       {
    //         result = result_from_hal_status(hal_status);
    //       }
    //       // increment the write address by the last write-size
    //       address += returned_size;
    //     }
    //   } while (result == AZ_OK);
    // }


int main(void)
{
  az_result result;

  if ((result = my_consumer()) != AZ_OK)
  {
    printf("my_consumer() failed\r\n");
  }

  return 0;
}
