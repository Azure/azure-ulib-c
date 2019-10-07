// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include "ustream.h"
#include "ulib_result.h"
#include "ulog.h"

#define USER_BUFFER_SIZE 5

static const char USTREAM_ONE_STRING[] = "Split BeforeSplit After";

static AZ_ULIB_RESULT print_ustream(AZ_USTREAM* ustream)
{
    AZ_ULIB_RESULT result;
    size_t returned_size;
    uint8_t user_buf[USER_BUFFER_SIZE] = { 0 };

    //Read ustream until receive AZIOT_ULIB_EOF
    (void)printf("\r\n------printing the ustream------\r\n");
    while((result = az_ustream_read(ustream, user_buf, USER_BUFFER_SIZE - 1, &returned_size)) == AZ_ULIB_SUCCESS)
    {
        user_buf[returned_size] = '\0';
        (void)printf("%s", user_buf);
    }
    (void)printf("\r\n-----------end of ustream------------\r\n\r\n");

    //Change return to AZ_ULIB_SUCCESS if last returned value was AZ_ULIB_EOF
    if(result == AZ_ULIB_EOF)
    {
        result = AZ_ULIB_SUCCESS;
    }
    return result;
}


/**
 * This sample creates two ustreams and concatenates them. It then prints the concatenated ustream
 * as if the two original ustreams were one.
 *      Content of ustream one: "Hello "
 *      Content of ustream two: "World\r\n"
 *      Content of concatenated ustream: "Hello World\r\n"
 * With both instances, the AZ_USTREAM lives on the stack while the control blocks use stdlib malloc for
 * allocation and stdlib free to free the memory.
 * 
 * Steps followed:
 *      1) Create the first ustream for a buffer in static memory. Print the size of the ustream.
 *      2) Create the second ustream for a buffer in the heap. Print the size of the ustream.
 *      3) Concatenate the second ustream to the first ustream. Dispose of the local instance of ustream_two.
 *              Remember we must do this to avoid memory leaks because while ustream_one is copied into the
 *              AZ_USTREAM_MULTI_DATA_CB (still one reference), ustream_two is cloned (meaning there are 
 *              now two references). We then print the size of the concatenated ustream.
 *      4) Print the concatenated ustream. Dispose of the ustream_one instance. Underneath the covers,
 *              since references to both buffers reaches zero, the free functions for the buffers and the
 *              control blocks are called and no memory is leaked.
 */

int main(void)
{
    AZ_ULIB_RESULT result;
    
    AZ_USTREAM_DATA_CB* data_cb = (AZ_USTREAM_DATA_CB*)malloc(sizeof(AZ_USTREAM_DATA_CB));
    AZ_USTREAM ustream_instance;
    az_ustream_init(&ustream_instance, data_cb, free, USTREAM_ONE_STRING, sizeof(USTREAM_ONE_STRING), NULL);
    print_ustream(&ustream_instance);
    az_ustream_reset(&ustream_instance);

    AZ_USTREAM ustream_instance_split;

    az_ustream_split(&ustream_instance, &ustream_instance_split, 12);
    print_ustream(&ustream_instance);
    print_ustream(&ustream_instance_split);

    az_ustream_dispose(&ustream_instance);
    az_ustream_dispose(&ustream_instance_split);

    return result;
}
