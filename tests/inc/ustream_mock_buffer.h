// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_TESTS_INC_USTREAM_MOCK_BUFFER_H_
#define AZURE_ULIB_C_TESTS_INC_USTREAM_MOCK_BUFFER_H_

#include "ustream_base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

USTREAM* ustream_mock_create(void);

void set_set_position_result(AZIOT_ULIB_RESULT result);
void set_reset_result(AZIOT_ULIB_RESULT result);
void set_read_result(AZIOT_ULIB_RESULT result);
void set_get_remaining_size_result(AZIOT_ULIB_RESULT result);
void set_get_position_result(AZIOT_ULIB_RESULT result);
void set_release_result(AZIOT_ULIB_RESULT result);
void set_clone_result(AZIOT_ULIB_RESULT result);
void set_dispose_result(AZIOT_ULIB_RESULT result);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_TESTS_INC_USTREAM_MOCK_BUFFER_H_ */
