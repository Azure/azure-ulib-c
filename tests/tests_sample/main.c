// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stdio.h>

#include "my_ustream_test.h"

int main(void)
{
  int result = 0;

  (void)printf("[==========]\r\n[ STARTING ] Running my_ustream_ut.\r\n");
  result += my_ustream_ut();
  (void)printf("[==========]\r\n[ STARTING ] Running my_ustream_e2e.\r\n");
  result += my_ustream_e2e();

  return result;
}
