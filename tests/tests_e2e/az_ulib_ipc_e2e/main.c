// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"

int main(void) {
  size_t failed_test_count = 0;
  RUN_TEST_SUITE(az_ulib_ipc_e2e, failed_test_count);
  return failed_test_count;
}
