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
#define USTREAM_TWO_STRING "World\r\n"

static const char USTREAM_ONE_STRING[] = "Hello ";
static uint8_t user_buf[USER_BUFFER_SIZE];

static ULIB_RESULT print_buffer(USTREAM* ustream)
{
    ULIB_RESULT result;
    size_t returned_size;
    uint32_t printed_chars;
    uint32_t ustream_read_iterations = 0;

    //Read ustream until receive ULIB_EOF
    (void)printf("\r\n---Printing the USTREAM---\r\n");
    while((result = ustream_read(ustream, user_buf, USER_BUFFER_SIZE - 1, &returned_size)) == ULIB_SUCCESS)
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
    (void)printf("ustream_read was called %i times\r\n", ustream_read_iterations);

    //Change return to ULIB_SUCCESS if last returned value was ULIB_EOF
    if(result == ULIB_EOF)
    {
        result = ULIB_SUCCESS;
    }

    return result;
}

int main(void)
{
    ULIB_RESULT result;
    size_t ustream_two_string_len;
    char* ustream_two_string;

    //Allocate second string in the heap
    ustream_two_string_len = sizeof(USTREAM_TWO_STRING);
    if((ustream_two_string = (char*)malloc(ustream_two_string_len)) == NULL)
    {
        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_OUT_OF_MEMORY_STRING, "string");
        result = ULIB_OUT_OF_MEMORY_ERROR;
    }
    else
    {
        memcpy(ustream_two_string, USTREAM_TWO_STRING, ustream_two_string_len);

        //Create the first USTREAM from constant memory
        USTREAM* ustream_one;
        size_t ustream_size;
        if((ustream_one = ustream_create((const uint8_t*)USTREAM_ONE_STRING, sizeof(USTREAM_ONE_STRING), NULL)) == NULL)
        {
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REPORT_EXCEPTION_STRING, "ustream_create", ULIB_SYSTEM_ERROR);
            result = ULIB_SYSTEM_ERROR;
        }
        else if((result = ustream_get_remaining_size(ustream_one, &ustream_size)) != ULIB_SUCCESS)
        {
            ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REPORT_EXCEPTION_STRING, "ustream_get_remaining_size", result);
        }
        else
        {
            (void)printf("Size of ustream_one: %lu\r\n", ustream_size);

            //Create the second USTREAM from the string in the heap, passing standard free function as inner free
            USTREAM* ustream_two;
            if((ustream_two = ustream_create((const uint8_t*)ustream_two_string, ustream_two_string_len, free)) == NULL)
            {
                ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REPORT_EXCEPTION_STRING, "ustream_create", ULIB_SYSTEM_ERROR);
                result = ULIB_SYSTEM_ERROR;
            }
            else if((result = ustream_get_remaining_size(ustream_two, &ustream_size)) != ULIB_SUCCESS)
            {
                ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REPORT_EXCEPTION_STRING, "ustream_get_remaining_size", result);
            }
            else
            {
                (void)printf("Size of ustream_two: %lu\r\n", ustream_size);

                //Append the second USTREAM to the first USTREAM
                if(result = ustream_append(ustream_one, ustream_two) != ULIB_SUCCESS)
                {
                    ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REPORT_EXCEPTION_STRING, "ustream_append", result);
                }
                //Dispose of our instance of the second ustream (now the appended has the only instance)
                else if((result = ustream_dispose(ustream_two)) != ULIB_SUCCESS)
                {
                    ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REPORT_EXCEPTION_STRING, "ustream_dispose", result);
                }
                else if((result = ustream_get_remaining_size(ustream_one, &ustream_size)) != ULIB_SUCCESS)
                {
                    ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REPORT_EXCEPTION_STRING, "ustream_get_remaining_size", result);
                }
                else
                {
                    //Print the size of the appended ustream
                    (void)printf("Size of ustream_one after append: %lu\r\n", ustream_size);

                    //Print the USTREAM contents
                    if(result = print_buffer(ustream_one) != ULIB_SUCCESS)
                    {
                        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REPORT_EXCEPTION_STRING, "print_buffer", result);
                    }
                    //Dispose of the USTREAM (original ustream_one and original ustream_two)
                    //At this point the memory malloc'd for ustream_two will be free'd
                    else if((result = ustream_dispose(ustream_one)) != ULIB_SUCCESS)
                    {
                        ULIB_CONFIG_LOG(ULOG_TYPE_ERROR, ULOG_REPORT_EXCEPTION_STRING, "ustream_dispose", result);
                    }
                }
            }
        }
    }

    return result;
}
