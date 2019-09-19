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

static const char USER_STRING_HOST[] = "https://contoso.blob.core.windows.net/container/superSecrets.jpg?\
sp=r&st=2019-09-19T18:40:04Z&se=2019-09-20T02:40:04Z&spr=https&\
sv=2018-03-28&sig=Gd2xwG4mBoYYXw6GmptRs82FiFMSsj5DldUF5Z%2Fc0yk%3D&sr=b";

int main(void)
{
    AZIOT_USTREAM message_ustream;
    AZIOT_USTREAM_INNER_BUFFER *inner_buffer = (AZIOT_USTREAM_INNER_BUFFER *)malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER));

    aziot_ustream_init(&message_ustream, inner_buffer, free, USER_STRING_HOST, sizeof(USER_STRING_HOST), NULL);

    size_t returned_size;
    aziot_ustream_get_remaining_size(&message_ustream, &returned_size);
    printf("\r\nsize of the request is %zu bytes\r\n", returned_size);

    printf("size of request copied ten times = %zu bytes\r\n", sizeof(USER_STRING_HOST) * 10);

    printf("ustream dynamic memory allocation = %zu bytes\r\n", sizeof(AZIOT_USTREAM_INNER_BUFFER));
    printf("ustream instances that would be on the stack = %zu bytes\r\n", sizeof(AZIOT_USTREAM));

    aziot_print_ustream(&message_ustream);

    return 0;
}
