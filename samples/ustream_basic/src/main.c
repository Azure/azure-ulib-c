// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include "ustream.h"
#include "ulib_heap.h"
#include "ulib_result.h"
#include "ulog.h"

#define USER_BUFFER_SIZE 5
#define USTREAM_TWO_STRING "World\r\n"

static const char USTREAM_ONE_STRING[] = "Hello ";

static AZIOT_ULIB_RESULT print_buffer(AZIOT_USTREAM* ustream)
{
    AZIOT_ULIB_RESULT result;
    size_t returned_size;
    uint8_t user_buf[USER_BUFFER_SIZE] = { 0 };
    uint32_t printed_chars;
    uint32_t ustream_read_iterations = 0;

    //Read ustream until receive AZIOT_ULIB_EOF
    (void)printf("\r\n---Printing the AZIOT_USTREAM---\r\n");
    while((result = aziot_ustream_read(ustream, user_buf, USER_BUFFER_SIZE - 1, &returned_size)) == AZIOT_ULIB_SUCCESS)
    {
        printed_chars = 0;
        while(printed_chars < returned_size)
        {
            //Print passed data
            printed_chars += printf("%s", &(user_buf[printed_chars]));

            //Account for NULL terminator
            printed_chars++;
        }
        ustream_read_iterations++;
    }
    (void)printf("-----------EOF------------\r\n");
    (void)printf("aziot_ustream_read was called %i times\r\n", ustream_read_iterations);

    //Change return to AZIOT_ULIB_SUCCESS if last returned value was AZIOT_ULIB_EOF
    if(result == AZIOT_ULIB_EOF)
    {
        result = AZIOT_ULIB_SUCCESS;
    }

    return result;
}

int main(void)
{
    AZIOT_ULIB_RESULT result;
    size_t ustream_two_string_len;
    char* ustream_two_string;

    //Allocate second string in the heap
    ustream_two_string_len = sizeof(USTREAM_TWO_STRING);
    if((ustream_two_string = (char*)malloc(ustream_two_string_len)) == NULL)
    {
        printf("Not enough memory for string\r\n");
        result = -1;
    }
    else
    {
        memcpy(ustream_two_string, USTREAM_TWO_STRING, ustream_two_string_len);

        //Create the first AZIOT_USTREAM from constant memory
        AZIOT_USTREAM ustream_one;
        AZIOT_USTREAM_INNER_BUFFER* ustream_inner_buffer_one = (AZIOT_USTREAM_INNER_BUFFER*)malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
        size_t ustream_size;
        if((result = aziot_ustream_init(&ustream_one, ustream_inner_buffer_one, free,
                                                (const uint8_t*)USTREAM_ONE_STRING, sizeof(USTREAM_ONE_STRING), NULL)) != AZIOT_ULIB_SUCCESS)
        {
            printf("Couldn't initialize ustream_one\r\n");
        }
        else if((result = aziot_ustream_get_remaining_size((AZIOT_USTREAM*)&ustream_one, &ustream_size)) != AZIOT_ULIB_SUCCESS)
        {
            printf("Couldn't get ustream_one remaining size\r\n");
        }
        else
        {
            (void)printf("Size of ustream_one: %zu\r\n", ustream_size);

            //Create the second AZIOT_USTREAM from the string in the heap, passing standard free function as release callback
            AZIOT_USTREAM ustream_two;
            AZIOT_USTREAM_INNER_BUFFER* ustream_inner_buffer_two = (AZIOT_USTREAM_INNER_BUFFER*)malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
            if((result = aziot_ustream_init(&ustream_two, ustream_inner_buffer_two, free,
                                                (const uint8_t*) ustream_two_string, ustream_two_string_len, free)) != AZIOT_ULIB_SUCCESS)
            {
                printf("Couldn't initialize ustream_two\r\n");
            }
            else if((result = aziot_ustream_get_remaining_size((AZIOT_USTREAM*)&ustream_two, &ustream_size)) != AZIOT_ULIB_SUCCESS)
            {
                printf("Couldn't get ustream_two remaining size\r\n");
            }
            else
            {
                (void)printf("Size of ustream_two: %zu\r\n", ustream_size);

                AZIOT_USTREAM_INNER_BUFFER* multi_inner_buffer = (AZIOT_USTREAM_INNER_BUFFER*)malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
                AZIOT_USTREAM_MULTI_DATA* multi_data = (AZIOT_USTREAM_MULTI_DATA*)malloc(sizeof(AZIOT_USTREAM_MULTI_DATA));
                //Concat the second AZIOT_USTREAM to the first AZIOT_USTREAM
                if((result = aziot_ustream_concat(&ustream_one, &ustream_two, multi_inner_buffer, free, multi_data, free)) != AZIOT_ULIB_SUCCESS)
                {
                    printf("Couldn't concat ustream_two to ustream_one\r\n");
                }
                //Dispose of our instance of the second ustream (now the concatenated has the only instance)
                else if((result = aziot_ustream_dispose((AZIOT_USTREAM*)&ustream_two)) != AZIOT_ULIB_SUCCESS)
                {
                    printf("Couldn't dispose ustream_two\r\n");
                }
                else if((result = aziot_ustream_get_remaining_size(&ustream_one, &ustream_size)) != AZIOT_ULIB_SUCCESS)
                {
                    printf("Couldn't get concatenated ustream remaining size\r\n");
                }
                else
                {
                    //Print the size of the concat ustream
                    (void)printf("Size of ustream_one after concat: %zu\r\n", ustream_size);

                    //Print the AZIOT_USTREAM contents
                    if((result = print_buffer(&ustream_one)) != AZIOT_ULIB_SUCCESS)
                    {
                        printf("Couldn't print concatenated ustream\r\n");
                    }
                    //Dispose of the AZIOT_USTREAM (original ustream_one and original ustream_two)
                    //At this point the memory malloc'd for ustream_two will be free'd
                    else if((result = aziot_ustream_dispose(&ustream_one)) != AZIOT_ULIB_SUCCESS)
                    {
                        printf("Couldn't dispose ustream_one\r\n");
                    }
                }
            }
        }
    }

    return result;
}
