// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stdio.h>

#include "az_ulib_ipc_e2e.h"

int main(void)
{
  int result = 0;

  (void)printf("[==========]\r\n[ STARTING ] Running az_ulib_ipc_e2e.\r\n");
  result += az_ulib_ipc_e2e();

  return result;
}
