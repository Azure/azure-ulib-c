// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_action_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "az_ulib_ucontract.h"
#include "az_ulib_ulog.h"
#include "consumer.h"
#include "math.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static az_ulib_ipc ipc_handle;

/*
 * OS code.
 */
int main(void) {
  az_ulib_result result;

  (void)printf("Start ipc_call_interface sample\r\n\r\n");

  /* 
   * Create the IPC. It shall be called at the very beginning of the application. 
   * The IPC will prepare itself to receive interfaces. 
   */
  if ((result = az_ulib_ipc_init(&ipc_handle)) != AZ_ULIB_SUCCESS) {
    (void)printf("Initialize IPC failed with code %d\r\n", result);
  } else {
    /* 
     * Publish the math interface. After this point anybody can call the math methods 
     * through IPC.
     */
    if ((result = math_publish_interface()) != AZ_ULIB_SUCCESS) {
      (void)printf("Server publish math interface failed with error %d\r\n", result);
    } else {
      /*
       * Consumer will use the math interface.
       */
      client_use_math();

      /*
       * Unpublish math interface. After this point, any call to math will return
       * AZ_ULIB_NO_SUCH_ELEMENT_ERROR.
       */
      math_unpublish_interface();
    }
  }

  return 0;
}
