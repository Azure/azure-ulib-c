// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include "az_ulib_ustream.h"
#include "az_ulib_result.h"
#include "az_ulib_ulog.h"

#define USER_BUFFER_SIZE 5
#define USTREAM_TWO_STRING "World\r\n"

static const char USTREAM_ONE_STRING[] = "Hello ";

static az_ulib_result print_buffer(az_ulib_ustream* ustream)
{
    az_ulib_result result;
    size_t returned_size;
    uint8_t user_buf[USER_BUFFER_SIZE] = { 0 };

    //Read ustream until receive AZIOT_ULIB_EOF
    (void)printf("\r\n------printing the ustream------\r\n");
    while((result = az_ulib_ustream_read(ustream, user_buf, USER_BUFFER_SIZE - 1, &returned_size)) == AZ_ULIB_SUCCESS)
    {
        user_buf[returned_size] = '\0';
        (void)printf("%s", user_buf);
    }
    (void)printf("-----------end of ustream------------\r\n\r\n");

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
 * With both instances, the az_ulib_ustream lives on the stack while the control blocks use stdlib malloc for
 * allocation and stdlib free to free the memory.
 * 
 * Steps followed:
 *      1) Create the first ustream for a buffer in static memory. Print the size of the ustream.
 *      2) Create the second ustream for a buffer in the heap. Print the size of the ustream.
 *      3) Concatenate the second ustream to the first ustream. Dispose of the local instance of ustream_two.
 *              Remember we must do this to avoid memory leaks because while ustream_one is copied into the
 *              az_ulib_ustream_multi_data_cb (still one reference), ustream_two is cloned (meaning there are 
 *              now two references). We then print the size of the concatenated ustream.
 *      4) Print the concatenated ustream. Dispose of the ustream_one instance. Underneath the covers,
 *              since references to both buffers reaches zero, the free functions for the buffers and the
 *              control blocks are called and no memory is leaked.
 */

int main(void)
{
    az_ulib_result result;
    size_t ustream_two_string_len;
    char* ustream_two_string;

    //Allocate second string in the heap
    ustream_two_string_len = sizeof(USTREAM_TWO_STRING) - 1;
    if((ustream_two_string = (char*)malloc(ustream_two_string_len)) == NULL)
    {
        printf("Not enough memory for string\r\n");
        result = -1;
    }
    else
    {
        memcpy(ustream_two_string, USTREAM_TWO_STRING, ustream_two_string_len);

        //Create the first az_ulib_ustream from constant memory
        az_ulib_ustream ustream_one;
        az_ulib_ustream_data_cb* ustream_control_block_one = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
        size_t ustream_size;
        if((result = az_ulib_ustream_init(&ustream_one, ustream_control_block_one, free,
                                                (const uint8_t*)USTREAM_ONE_STRING, sizeof(USTREAM_ONE_STRING) - 1, NULL)) != AZ_ULIB_SUCCESS)
        {
            printf("Couldn't initialize ustream_one\r\n");
        }
        else if((result = az_ulib_ustream_get_remaining_size(&ustream_one, &ustream_size)) != AZ_ULIB_SUCCESS)
        {
            printf("Couldn't get ustream_one remaining size\r\n");
        }
        else
        {
            (void)printf("Size of ustream_one: %zu\r\n", ustream_size);

            //Create the second az_ulib_ustream from the string in the heap, passing standard free function as release callback
            az_ulib_ustream ustream_two;
            az_ulib_ustream_data_cb* ustream_control_block_two = (az_ulib_ustream_data_cb*)malloc(sizeof(az_ulib_ustream_data_cb));
            if((result = az_ulib_ustream_init(&ustream_two, ustream_control_block_two, free,
                                                (const uint8_t*) ustream_two_string, ustream_two_string_len, free)) != AZ_ULIB_SUCCESS)
            {
                printf("Couldn't initialize ustream_two\r\n");
            }
            else if((result = az_ulib_ustream_get_remaining_size(&ustream_two, &ustream_size)) != AZ_ULIB_SUCCESS)
            {
                printf("Couldn't get ustream_two remaining size\r\n");
            }
            else
            {
                (void)printf("Size of ustream_two: %zu\r\n", ustream_size);

                az_ulib_ustream_multi_data_cb* multi_data = (az_ulib_ustream_multi_data_cb*)malloc(sizeof(az_ulib_ustream_multi_data_cb));
                //Concat the second az_ulib_ustream to the first az_ulib_ustream
                if((result = az_ulib_ustream_concat(&ustream_one, &ustream_two, multi_data, free)) != AZ_ULIB_SUCCESS)
                {
                    printf("Couldn't concat ustream_two to ustream_one\r\n");
                }
                //Dispose of our instance of the second ustream (now the concatenated has the only instance)
                else if((result = az_ulib_ustream_dispose(&ustream_two)) != AZ_ULIB_SUCCESS)
                {
                    printf("Couldn't dispose ustream_two\r\n");
                }
                else if((result = az_ulib_ustream_get_remaining_size(&ustream_one, &ustream_size)) != AZ_ULIB_SUCCESS)
                {
                    printf("Couldn't get concatenated ustream remaining size\r\n");
                }
                else
                {
                    //Print the size of the concat ustream
                    (void)printf("Size of ustream_one after concat: %zu\r\n", ustream_size);

                    //Print the az_ulib_ustream contents
                    if((result = print_buffer(&ustream_one)) != AZ_ULIB_SUCCESS)
                    {
                        printf("Couldn't print concatenated ustream\r\n");
                    }
                    //Dispose of the az_ulib_ustream (original ustream_one and original ustream_two)
                    //At this point the memory malloc'd for ustream_two will be free'd
                    else if((result = az_ulib_ustream_dispose(&ustream_one)) != AZ_ULIB_SUCCESS)
                    {
                        printf("Couldn't dispose ustream_one\r\n");
                    }
                }
            }
        }
    }

    return result;
}
