// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef USTREAM_MOCK_BUFFER_H
#define USTREAM_MOCK_BUFFER_H

#include <stdbool.h>
#include "ustream_base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

AZ_USTREAM* ustream_mock_create(void);

void reset_mock_buffer(void);
void set_concurrency_ustream(void);
void set_delay_return_value(uint32_t delay);

void set_set_position_result(AZ_ULIB_RESULT result);
void set_reset_result(AZ_ULIB_RESULT result);
void set_read_result(AZ_ULIB_RESULT result);
void set_get_remaining_size_result(AZ_ULIB_RESULT result);
void set_get_position_result(AZ_ULIB_RESULT result);
void set_release_result(AZ_ULIB_RESULT result);
void set_clone_result(AZ_ULIB_RESULT result);
void set_dispose_result(AZ_ULIB_RESULT result);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* USTREAM_MOCK_BUFFER_H */
