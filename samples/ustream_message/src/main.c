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

static const char USER_STRING_HOST[] = "contoso-storage-account.blob.core.windows.net";

int main(void)
{
    AZIOT_USTREAM message_ustream;
    AZIOT_USTREAM_INNER_BUFFER *inner_buffer = (AZIOT_USTREAM_INNER_BUFFER *)malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));
    AZIOT_USTREAM_MESSAGE *message = (AZIOT_USTREAM_MESSAGE *)malloc(sizeof(AZIOT_USTREAM_MESSAGE));

    aziot_ustream_message_init(message, USER_STRING_HOST, AZIOT_ULIB_MESSAGE_VERB_GET);

    aziot_ustream_message_add_option(message, AZIOT_ULIB_MESSAGE_OPTION_VERSION, "2018-03-28", sizeof("2018-03-28"));

    aziot_ustream_from_message(&message_ustream, inner_buffer, free, message, free);

    size_t returned_size;
    aziot_ustream_get_remaining_size(ustream, &returned_size);
    printf("Size of the message is %zu bytes\r\n", returned_size);

    aziot_print_ustream(&message_ustream);

    return 0;
}
