// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include "ustream.h"
#include "ustream_message.h"
#include "ulib_heap.h"
#include "ulib_result.h"
#include "ulog.h"

#define USER_BUFFER_SIZE 1000

static const char USER_STRING_HOST[] = "contoso-storage-account.blob.core.windows.net";

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

    size_t size_of_message;
    aziot_ustream_get_remaining_size(ustream, &size_of_message);
    printf("Size of the message is %zu bytes\r\n", size_of_message);

    //Read ustream until receive AZIOT_ULIB_EOF
    (void)printf("\r\n------Printing the Header------\r\n");
    while((result = aziot_ustream_read(ustream, user_buf, USER_BUFFER_SIZE - 1, &returned_size)) == AZIOT_ULIB_SUCCESS)
    {
        //Set char after last returned to NULL
        user_buf[returned_size] = '\0';
        //Print passed data
        (void)printf("%s", user_buf);
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

int main(void)
{
    AZIOT_USTREAM message_ustream;
    AZIOT_USTREAM_INNER_BUFFER *inner_buffer = (AZIOT_USTREAM_INNER_BUFFER *)malloc_track(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_USTREAM_MESSAGE *message = (AZIOT_USTREAM_MESSAGE *)malloc_track(sizeof(AZIOT_USTREAM_MESSAGE));

    aziot_ustream_message_init(message, USER_STRING_HOST, AZIOT_ULIB_MESSAGE_VERB_GET);

    aziot_ustream_message_add_option(message, AZIOT_ULIB_MESSAGE_OPTION_CONTENT_TYPE, "text/plain", 11);

    aziot_ustream_from_message(&message_ustream, inner_buffer, free, message, free);

    print_ustream(&message_ustream);

    return 0;
}
