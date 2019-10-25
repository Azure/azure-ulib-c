// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_action_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "consumer.h"
#include "math.h"
#include "math_tlb.h"
#include "ucontract.h"
#include "ulib_result.h"
#include "ulog.h"
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
  AZ_ULIB_RESULT result;

  (void)printf("Start ipc_call_interface sample\r\n\r\n");

  if ((result = az_ulib_ipc_init(&ipc_handle)) != AZ_ULIB_SUCCESS) {
    (void)printf("Initialize IPC failed with code %d\r\n", result);
  } else {
    if ((result = math_publish_interface()) != AZ_ULIB_SUCCESS) {
      (void)printf("Server publish math interface failed with error %d\r\n", result);
    } else {
      client_use_math();
      math_unpublish_interface();
    }
  }

  return 0;
}
