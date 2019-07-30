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

static AZULIB_ULIB_RESULT print_buffer(AZULIB_USTREAM* ustream)
{
    AZULIB_ULIB_RESULT result;
    size_t returned_size;
    uint8_t user_buf[USER_BUFFER_SIZE] = { 0 };
    uint32_t printed_chars;
    uint32_t ustream_read_iterations = 0;

    //Read ustream until receive AZULIB_ULIB_EOF
    (void)printf("\r\n---Printing the AZULIB_USTREAM---\r\n");
    while((result = azulib_ustream_read(ustream, user_buf, USER_BUFFER_SIZE - 1, &returned_size)) == AZULIB_ULIB_SUCCESS)
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
    (void)printf("azulib_ustream_read was called %i times\r\n", ustream_read_iterations);

    //Change return to AZULIB_ULIB_SUCCESS if last returned value was AZULIB_ULIB_EOF
    if(result == AZULIB_ULIB_EOF)
    {
        result = AZULIB_ULIB_SUCCESS;
    }

    return result;
}

int main(void)
{
    AZULIB_ULIB_RESULT result;
    size_t ustream_two_string_len;
    char* ustream_two_string;

    //Allocate second string in the heap
    ustream_two_string_len = sizeof(USTREAM_TWO_STRING);
    if((ustream_two_string = (char*)malloc(ustream_two_string_len)) == NULL)
    {
        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_OUT_OF_MEMORY_STRING, "string");
        result = AZULIB_ULIB_OUT_OF_MEMORY_ERROR;
    }
    else
    {
        memcpy(ustream_two_string, USTREAM_TWO_STRING, ustream_two_string_len);

        //Create the first AZULIB_USTREAM from constant memory
        AZULIB_USTREAM* ustream_one;
        size_t ustream_size;
        if((ustream_one = azulib_ustream_create((const uint8_t*)USTREAM_ONE_STRING, sizeof(USTREAM_ONE_STRING), NULL)) == NULL)
        {
            AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REPORT_EXCEPTION_STRING, "azulib_ustream_create", AZULIB_ULIB_SYSTEM_ERROR);
            result = AZULIB_ULIB_SYSTEM_ERROR;
        }
        else if((result = azulib_ustream_get_remaining_size(ustream_one, &ustream_size)) != AZULIB_ULIB_SUCCESS)
        {
            AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REPORT_EXCEPTION_STRING, "azulib_ustream_get_remaining_size", result);
        }
        else
        {
            (void)printf("Size of ustream_one: %lu\r\n", ustream_size);

            //Create the second AZULIB_USTREAM from the string in the heap, passing standard free function as release callback
            AZULIB_USTREAM* ustream_two;
            if((ustream_two = azulib_ustream_create((const uint8_t*)ustream_two_string, ustream_two_string_len, free)) == NULL)
            {
                AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REPORT_EXCEPTION_STRING, "azulib_ustream_create", AZULIB_ULIB_SYSTEM_ERROR);
                result = AZULIB_ULIB_SYSTEM_ERROR;
            }
            else if((result = azulib_ustream_get_remaining_size(ustream_two, &ustream_size)) != AZULIB_ULIB_SUCCESS)
            {
                AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REPORT_EXCEPTION_STRING, "azulib_ustream_get_remaining_size", result);
            }
            else
            {
                (void)printf("Size of ustream_two: %lu\r\n", ustream_size);

                //Append the second AZULIB_USTREAM to the first AZULIB_USTREAM
                if((result = azulib_ustream_append(ustream_one, ustream_two)) != AZULIB_ULIB_SUCCESS)
                {
                    AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REPORT_EXCEPTION_STRING, "azulib_ustream_append", result);
                }
                //Dispose of our instance of the second ustream (now the appended has the only instance)
                else if((result = azulib_ustream_dispose(ustream_two)) != AZULIB_ULIB_SUCCESS)
                {
                    AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REPORT_EXCEPTION_STRING, "azulib_ustream_dispose", result);
                }
                else if((result = azulib_ustream_get_remaining_size(ustream_one, &ustream_size)) != AZULIB_ULIB_SUCCESS)
                {
                    AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REPORT_EXCEPTION_STRING, "azulib_ustream_get_remaining_size", result);
                }
                else
                {
                    //Print the size of the appended ustream
                    (void)printf("Size of ustream_one after append: %lu\r\n", ustream_size);

                    //Print the AZULIB_USTREAM contents
                    if((result = print_buffer(ustream_one)) != AZULIB_ULIB_SUCCESS)
                    {
                        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REPORT_EXCEPTION_STRING, "print_buffer", result);
                    }
                    //Dispose of the AZULIB_USTREAM (original ustream_one and original ustream_two)
                    //At this point the memory malloc'd for ustream_two will be free'd
                    else if((result = azulib_ustream_dispose(ustream_one)) != AZULIB_ULIB_SUCCESS)
                    {
                        AZULIB_ULIB_CONFIG_LOG(AZULIB_ULOG_TYPE_ERROR, AZULIB_ULOG_REPORT_EXCEPTION_STRING, "azulib_ustream_dispose", result);
                    }
                }
            }
        }
    }

    return result;
}
