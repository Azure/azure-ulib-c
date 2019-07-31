// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZULIB_USTREAM_MOCK_BUFFER_H
#define AZULIB_USTREAM_MOCK_BUFFER_H

#include "ustream_base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

AZULIB_USTREAM* ustream_mock_create(void);

void set_set_position_result(AZULIB_RESULT result);
void set_reset_result(AZULIB_RESULT result);
void set_read_result(AZULIB_RESULT result);
void set_get_remaining_size_result(AZULIB_RESULT result);
void set_get_position_result(AZULIB_RESULT result);
void set_release_result(AZULIB_RESULT result);
void set_clone_result(AZULIB_RESULT result);
void set_dispose_result(AZULIB_RESULT result);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZULIB_USTREAM_MOCK_BUFFER_H */
