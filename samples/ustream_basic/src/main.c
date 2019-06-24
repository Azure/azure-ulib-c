// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "ustream.h"
#include "ulib_result.h"

#define USER_BUFFER_SIZE 5
#define USTREAM_TWO_STRING "World\r\n"

static const char ustream_one_string[] = "Hello ";
static uint8_t user_buf[USER_BUFFER_SIZE];

static void print_buffer(USTREAM* ustream)
{
    size_t returned_size;
    uint32_t printed_chars;

    //Read ustream until receive ULIB_EOF
    printf("\r\n---Printing the USTREAM---\r\n");
    while(ustream_read(ustream, user_buf, USER_BUFFER_SIZE - 1, &returned_size) == ULIB_SUCCESS)
    {
        printed_chars = 0;
        while(printed_chars < returned_size)
        {
            //Print passed data
            printed_chars += printf("%s", &(user_buf[printed_chars]));

            //Account for NULL terminator
            printed_chars++;
        }
    }
    printf("-----------EOF------------\r\n");
}

int main(void)
{
    size_t ustream_two_string_len, ustream_size;

    //Allocate second string in the heap
    ustream_two_string_len = sizeof(USTREAM_TWO_STRING);
    char* ustream_two_string = (char*)malloc(ustream_two_string_len);
    memcpy(ustream_two_string, USTREAM_TWO_STRING, ustream_two_string_len);

    //Create the first USTREAM from constant memory
    USTREAM* ustream_one = ustream_create((const uint8_t*)ustream_one_string, sizeof(ustream_one_string), NULL);
    ustream_get_remaining_size(ustream_one, &ustream_size);
    printf("Size of ustream_one: %lu\r\n", ustream_size);

    //Create the second USTREAM from the string in the heap, passing standard free function as inner free
    USTREAM* ustream_two = ustream_create((const uint8_t*)ustream_two_string, ustream_two_string_len, free);
    ustream_get_remaining_size(ustream_two, &ustream_size);
    printf("Size of ustream_two: %lu\r\n", ustream_size);

    //Append the second USTREAM to the first USTREAM
    ULIB_RESULT result = ustream_append(ustream_one, ustream_two);

    //Dispose of our instance of the second ustream (now the appended has the only instance)
    ustream_dispose(ustream_two);

    //Print the size of the appended ustream
    ustream_get_remaining_size(ustream_one, &ustream_size);
    printf("Size of ustream_one after append: %lu\r\n", ustream_size);
    (void)result;

    //Print the USTREAM contents
    print_buffer(ustream_one);

    //Dispose of the USTREAM (original ustream_one and original ustream_two)
    //At this point the memory malloc'd for ustream_two will be free'd
    ustream_dispose(ustream_one);

    return 0;
}
