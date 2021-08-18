// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_USTREAM_MOCK_BUFFER_H
#define AZ_ULIB_USTREAM_MOCK_BUFFER_H

#include "az_ulib_ustream_base.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

  az_ulib_ustream* ustream_mock_create(void);

  void reset_mock_buffer(void);
  void set_concurrency_ustream(void);
  void set_delay_return_value(uint32_t delay);

  void set_set_position_result(az_result result);
  void set_reset_result(az_result result);
  void set_read_result(az_result result);
  void set_get_remaining_size_result(az_result result);
  void set_get_position_result(az_result result);
  void set_release_result(az_result result);
  void set_clone_result(az_result result);
  void set_dispose_result(az_result result);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZ_ULIB_USTREAM_MOCK_BUFFER_H */
