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

static const char CONST_STRING_POST[] = "POST: ";
static const char CONST_STRING_HOST[] = "Host: ";
static const char CONST_STRING_CONTENT_LENGTH[] = "Content-Length: ";
static const char CONST_STRING_CONTENT_TYPE[] = "Content-Type: text/plain";
static const char CONST_STRING_HTTP_VERSION[] = "HTTP/1.1";

static const char USER_STRING_HOST[] = "contoso-storage-account.blob.core.windows.net\r\n";

static const char USER_STRING_PAYLOAD[] =
"\r\n\r\nFar out in the uncharted backwaters of the unfashionable end of the western spiral \
arm of the Galaxy lies a small unregarded yellow sun. Orbiting this at a distance of \
roughly ninety-two million miles is an utterly insignificant little blue green planet\
whose ape-descended life forms are so amazingly primitive that they still think digital.\r\n";

static const int32_t PAYLOAD_SIZE = sizeof(USER_STRING_PAYLOAD);

static size_t total_alloc = 0;

static void* malloc_track(size_t size_to_alloc)
{
    total_alloc += size_to_alloc;
    return malloc(size_to_alloc);
}

static AZIOT_ULIB_RESULT print_ustream(AZIOT_USTREAM * ustream)
{
    AZIOT_ULIB_RESULT result;
    size_t returned_size;
    uint8_t user_buf[USER_BUFFER_SIZE] = { 0 };
    uint32_t printed_chars;
    uint32_t ustream_read_iterations = 0;

    //Read ustream until receive AZIOT_ULIB_EOF
    (void)printf("\r\n------Printing the Header------\r\n");
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
    (void)printf("-----------End of Header------------\r\n\r\n");
    // (void)printf("aziot_ustream_read was called %i times\r\n", ustream_read_iterations);

    //Change return to AZIOT_ULIB_SUCCESS if last returned value was AZIOT_ULIB_EOF
    if(result == AZIOT_ULIB_EOF)
    {
        result = AZIOT_ULIB_SUCCESS;
    }

    return result;
}

static void concat_ustream(AZIOT_USTREAM* ustream_one, AZIOT_USTREAM* ustream_two)
{
    AZIOT_USTREAM_INNER_BUFFER* inner_buffer = (AZIOT_USTREAM_INNER_BUFFER*)malloc_track(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_USTREAM_MULTI_DATA* multi_data = (AZIOT_USTREAM_MULTI_DATA*)malloc_track(sizeof(AZIOT_USTREAM_MULTI_DATA));
    aziot_ustream_concat(ustream_one, ustream_two, inner_buffer, free, multi_data, free);
}

static void create_ustream_from_string(AZIOT_USTREAM* ustream, const char* input_str, size_t input_str_size, AZIOT_RELEASE_CALLBACK rel_callback)
{
    AZIOT_USTREAM_INNER_BUFFER* inner_buffer = (AZIOT_USTREAM_INNER_BUFFER*)malloc_track(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    aziot_ustream_init(ustream, inner_buffer, free,
                                     (const uint8_t *)input_str, input_str_size, rel_callback);
}

static AZIOT_USTREAM* create_post_header_ustream(AZIOT_USTREAM* ustream, const char* host, size_t host_size)
{
    create_ustream_from_string(ustream, CONST_STRING_HOST, sizeof(CONST_STRING_HOST), NULL);

    AZIOT_USTREAM host_ustream;
    create_ustream_from_string(&host_ustream, host, host_size, NULL);
    concat_ustream(ustream, &host_ustream);

    AZIOT_USTREAM content_type_ustream;
    create_ustream_from_string(&content_type_ustream, CONST_STRING_CONTENT_TYPE, sizeof(CONST_STRING_CONTENT_TYPE), NULL);
    concat_ustream(ustream, &content_type_ustream);

    aziot_ustream_dispose(&content_type_ustream);
    aziot_ustream_dispose(&host_ustream);
}

int main(void)
{
    //Create header and payload headers
    AZIOT_USTREAM header_ustream;
    AZIOT_USTREAM payload_ustream;
    create_post_header_ustream(&header_ustream, USER_STRING_HOST, sizeof(USER_STRING_HOST));
    create_ustream_from_string(&payload_ustream, USER_STRING_PAYLOAD, sizeof(USER_STRING_PAYLOAD), NULL);

    //Concat the two together
    concat_ustream(&header_ustream, &payload_ustream);
    aziot_ustream_dispose(&payload_ustream);

    //Get size of both
    size_t request_size;
    aziot_ustream_get_remaining_size(&header_ustream, &request_size);
    printf("Request size is %zu bytes.\r\n", request_size);

    print_ustream(&header_ustream);

    aziot_ustream_dispose(&header_ustream);

    printf("Allocatetd %zu bytes to manage %zu bytes\r\n", total_alloc, request_size);

    return 0;
}
