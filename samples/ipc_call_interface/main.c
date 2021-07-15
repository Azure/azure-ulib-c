// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "cipher_v1i1.h"
#include "cipher_v2i1.h"
#include "my_consumer.h"
#include <inttypes.h>
#include <stdio.h>

static az_ulib_ipc ipc_handle;

/*
 * OS code.
 */
int main(void)
{
  az_result result;

  (void)printf("Start ipc_call_interface sample.\r\n\r\n");

  /*
   * Create the IPC. It shall be called at the very beginning of the application.
   * The IPC will prepare itself to receive interfaces.
   */
  if ((result = az_ulib_ipc_init(&ipc_handle)) != AZ_OK)
  {
    (void)printf("Initialize IPC failed with code %" PRIi32 ".\r\n", result);
  }
  else
  {
    /* Publish cipher v1 with 1 key.
     * After this point anybody can call the cipher commands through IPC. */
    cipher_v1i1_create();
    (void)printf("\r\n");

    /* Consumer will use the cipher interface. */
    my_consumer_create();

    /* Query current interfaces. */
    my_consumer_query_interfaces();
    (void)printf("\r\n");

    /* My consumer try to use cipher to add numbers. */
    my_consumer_do_cipher(0);
    my_consumer_do_cipher(1);
    (void)printf("\r\n");

    /* Publish cipher v2 with 2 key.
     * After this point anybody can call the cipher commands through IPC. */
    cipher_v2i1_create();
    (void)printf("\r\n");

    /* Query current interfaces. */
    my_consumer_query_interfaces();
    (void)printf("\r\n");

    /* My consumer try to use cipher to add numbers. */
    my_consumer_do_cipher(0);
    my_consumer_do_cipher(1);
    (void)printf("\r\n");

    /* Consumer will stop to use the cipher interface. */
    my_consumer_destroy();
    (void)printf("\r\n");

    /* Move default to cipher v2. */
    my_consumer_change_default();

    /* Query current interfaces. */
    my_consumer_query_interfaces();
    (void)printf("\r\n");

    /* Consumer will use the cipher interface. */
    my_consumer_create();

    /* My consumer try to use cipher to add numbers. */
    my_consumer_do_cipher(0);
    my_consumer_do_cipher(1);
    (void)printf("\r\n");

    /* Unpublish cipher v1. After this point, any call to cipher will return
     * AZ_ERROR_ITEM_NOT_FOUND. */
    cipher_v1i1_destroy();
    (void)printf("\r\n");

    /* Query current interfaces. */
    my_consumer_query_interfaces();

    /* Unpublish cipher v2. After this point, any call to cipher will return
     * AZ_ERROR_ITEM_NOT_FOUND. */
    cipher_v2i1_destroy();
    (void)printf("\r\n");

    /* Consumer will stop to use the cipher interface. */
    my_consumer_destroy();
    (void)printf("\r\n");
  }

  return 0;
}
