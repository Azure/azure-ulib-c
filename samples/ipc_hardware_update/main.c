// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "az_ulib_ipc_api.h"
#include "az_ulib_registry_api.h"
#include "az_ulib_result.h"
#include "contoso_display_200401.h"
#include "contoso_display_480401.h"
#include "my_consumer.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

static az_ulib_ipc_control_block ipc_control_block;

#define REGISTRY_PAGE_SIZE 0x800

/* Static memory to store registry information. Because we run this samples in Linux and Windows,
 * instead of store the Registry in a Flash, we store it in the RAM. */
static uint8_t registry_buffer[REGISTRY_PAGE_SIZE * 2];
static uint8_t registry_informarmation_buffer[REGISTRY_PAGE_SIZE];

#define __REGISTRY_START (registry_buffer[0])
#define __REGISTRY_END (registry_buffer[(REGISTRY_PAGE_SIZE * 2)])
#define __REGISTRYINFO_START (registry_informarmation_buffer[0])
#define __REGISTRYINFO_END (registry_informarmation_buffer[REGISTRY_PAGE_SIZE])

static const az_ulib_registry_control_block registry_cb
    = { .registry_start = (void*)(&__REGISTRY_START),
        .registry_end = (void*)(&__REGISTRY_END),
        .registry_info_start = (void*)(&__REGISTRYINFO_START),
        .registry_info_end = (void*)(&__REGISTRYINFO_END),
        .page_size = REGISTRY_PAGE_SIZE };

/*
 * OS code.
 */
int main(void)
{
  az_result result;

  (void)printf("Start ipc_call_interface sample.\r\n\r\n");

  /* Start Registry. */
  az_ulib_registry_init(&registry_cb);
  az_ulib_registry_clean_all();

  /*
   * Create the IPC. It shall be called at the very beginning of the application.
   * The IPC will prepare itself to receive interfaces.
   */
  if ((result = az_ulib_ipc_init(&ipc_control_block)) != AZ_OK)
  {
    (void)printf("Initialize IPC failed with code %" PRIi32 ".\r\n", result);
  }
  else
  {
    /* Contoso publish display interface.
     * After this point anybody can call the display commands through IPC. */
    contoso_display_200401_create();
    (void)printf("\r\n");

    /* Consumer will use the display interface in a new thread. */
    my_consumer_create();
    (void)printf("\r\n");

    /* Give some time to consumer to play with the display. */
    az_pal_os_sleep(5000);
    (void)printf("\r\n");

    /* Unpublish display interface. After this point, any call to display will return
     * AZ_ERROR_ITEM_NOT_FOUND. */
    contoso_display_200401_destroy();
    (void)printf("\r\n");

    /* My consumer try to use display. */
    az_pal_os_sleep(2000); // It will fail because the Contoso handle is not available anymore.
    (void)printf("\r\n");

    /* Contoso publish another display interface.
     * After this point anybody can call the display commands through IPC. */
    contoso_display_480401_create();
    (void)printf("\r\n");

    /* Give some more time to consumer to play with the display. */
    az_pal_os_sleep(5000);
    (void)printf("\r\n");

    /* Unpublish display interface. After this point, any call to display will return
     * AZ_ERROR_ITEM_NOT_FOUND. */
    contoso_display_480401_destroy();
    (void)printf("\r\n");

    /* Contoso publish display interface again.
     * After this point anybody can call the display commands through IPC. */
    contoso_display_200401_create();
    (void)printf("\r\n");

    /* Give some more time to consumer to play with the display. */
    az_pal_os_sleep(5000);
    (void)printf("\r\n");

    /* Unpublish display interface. After this point, any call to display will return
     * AZ_ERROR_ITEM_NOT_FOUND. */
    contoso_display_200401_destroy();
    (void)printf("\r\n");

    /* Destroy consumer. */
    my_consumer_destroy();
    (void)printf("\r\n");
  }

  az_ulib_registry_deinit();

  return 0;
}
