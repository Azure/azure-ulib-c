// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_USTREAM_FORWARD_MOCK_BUFFER_H
#define AZ_ULIB_USTREAM_FORWARD_MOCK_BUFFER_H

#include "az_ulib_ustream_forward_base.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

  az_ulib_ustream_forward* ustream_forward_mock_create(void);

  void reset_mock_buffer(void);
  void set_concurrency_ustream(void);
  void set_delay_return_value(uint32_t delay);

  void set_flush_result(az_result result);
  void set_read_result(az_result result);
  void set_get_size_result(az_result result);
  void set_dispose_result(az_result result);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZ_ULIB_USTREAM_FORWARD_MOCK_BUFFER_H */
