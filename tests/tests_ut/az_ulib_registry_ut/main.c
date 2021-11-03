// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include <stdio.h>

#include "az_ulib_registry_ut.h"

int main(void)
{
  int result = 0;

  (void)printf("[==========]\r\n[ STARTING ] Running az_ulib_registry_ut.\r\n");
  result += az_ulib_registry_ut();

  return result;
}
