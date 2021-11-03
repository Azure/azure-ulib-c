// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <stdio.h>

#include "az_ulib_ipc_ut.h"

int main(void)
{
  int result = 0;

  (void)printf("[==========]\r\n[ STARTING ] Running az_ulib_ipc_ut.\r\n");
  result += az_ulib_ipc_ut();
  (void)printf("[==========]\r\n[ STARTING ] Running az_ulib_descriptor_ut.\r\n");
  result += az_ulib_descriptor_ut();

  return result;
}
