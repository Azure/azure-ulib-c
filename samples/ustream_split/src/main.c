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
#define SPLIT_POSITION 12

static const char USTREAM_ONE_STRING[] = "Split BeforeSplit After";

static az_ulib_result print_ustream(az_ulib_ustream* ustream)
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
    (void)printf("\r\n-----------end of ustream------------\r\n\r\n");

    //Change return to AZ_ULIB_SUCCESS if last returned value was AZ_ULIB_EOF
    if(result == AZ_ULIB_EOF)
    {
        result = AZ_ULIB_SUCCESS;
    }
    return result;
}

/**
 * This sample creates a ustream and then splits it into two ustreams at a desired position.
 * The following steps are followed:
 *      1. Create the ustream and print the ustream to the console. It needs to be reset
 *          before splitting so that the current position is restored to the beginning.
 *      2. Split the ustream at the desired position.
 *      3. Print each ustream to the console.
 *      4. Dispose of the two ustreams.
 */
int main(void)
{
    az_ulib_result result;

    az_ulib_ustream_data_cb *data_cb;
    if((data_cb = (az_ulib_ustream_data_cb *)malloc(sizeof(az_ulib_ustream_data_cb))) != NULL)
    {
        az_ulib_ustream ustream_instance;
        if((result = az_ulib_ustream_init(&ustream_instance, data_cb, free, (const uint8_t*)USTREAM_ONE_STRING, 
                                                        sizeof(USTREAM_ONE_STRING), NULL)) != AZ_ULIB_SUCCESS)
        {
            printf("Could not initialize ustream_instance\r\n");
        }
        else if((result = print_ustream(&ustream_instance)) != AZ_ULIB_SUCCESS)
        {
            az_ulib_ustream_dispose(&ustream_instance);
            printf("Could not print the original ustream_instance\r\n");
        }
        else if((result = az_ulib_ustream_reset(&ustream_instance)) != AZ_ULIB_SUCCESS)
        {
            az_ulib_ustream_dispose(&ustream_instance);
            printf("Could not reset ustream_instance\r\n");
        }
        else
        {
            az_ulib_ustream ustream_instance_split;

            if((result = az_ulib_ustream_split(&ustream_instance, &ustream_instance_split, SPLIT_POSITION)) != AZ_ULIB_SUCCESS)
            {
                printf("Could not split ustream_instance\r\n");
            }
            else
            {
                if((result = print_ustream(&ustream_instance)) != AZ_ULIB_SUCCESS)
                {
                    printf("Could not print the split ustream_instance\r\n");
                }
                else if((result = print_ustream(&ustream_instance_split)) != AZ_ULIB_SUCCESS)
                {
                    printf("Could not print ustream_instance_split\r\n");
                }

                if((result = az_ulib_ustream_dispose(&ustream_instance_split)) != AZ_ULIB_SUCCESS)
                {
                    printf("Could not dispose of ustream_instance_split\r\n");
                }
            }

            if((result = az_ulib_ustream_dispose(&ustream_instance)) != AZ_ULIB_SUCCESS)
            {
                printf("Could not dispose of ustream_instance\r\n");
            }
        }
    }
    else
    {
        result = AZ_ULIB_OUT_OF_MEMORY_ERROR;
    }

    return result;
}
