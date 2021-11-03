// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <stdio.h>

#include "az_ulib_ipc_e2e.h"

int main(void)
{
  int result = 0;

  (void)printf("[==========]\r\n[ STARTING ] Running az_ulib_ipc_e2e.\r\n");
  result += az_ulib_ipc_e2e();

  return result;
}
