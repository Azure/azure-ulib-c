// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_TEST_HELPERS_H
#define AZ_ULIB_TEST_HELPERS_H

#include "az_ulib_ustream_base.h"
#include "az_ulib_ustream_forward.h"

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif /* __cplusplus */

  /**
   * Check buffer
   */
  void check_buffer(
      az_ulib_ustream* ustream_instance,
      uint8_t offset,
      const uint8_t* const expected_content,
      uint8_t expected_content_length);
  /**
   * Check buffer
   */
  void check_ustream_forward_buffer(
      az_ulib_ustream_forward* ustream_forward,
      uint8_t offset,
      const uint8_t* const expected_content,
      uint8_t expected_content_length);

#ifdef __cplusplus
  {
#endif /* __cplusplus */

#endif /* AZ_ULIB_TEST_HELPERS_H */
