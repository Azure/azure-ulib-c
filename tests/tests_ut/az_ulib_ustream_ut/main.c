// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stdio.h>

#include "az_ulib_ustream_ut.h"

int main(void)
{
  int result = 0;

  (void)printf("[==========]\r\n[ STARTING ] Running az_ulib_ustream_ut.\r\n");
  result += az_ulib_ustream_ut();
  (void)printf("[==========]\r\n[ STARTING ] Running az_ulib_ustream_aux_ut.\r\n");
  result += az_ulib_ustream_aux_ut();

  return result;
}
