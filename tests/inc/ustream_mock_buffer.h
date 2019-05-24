// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_TESTS_INC_USTREAM_MOCK_BUFFER_H_
#define AZURE_ULIB_C_TESTS_INC_USTREAM_MOCK_BUFFER_H_

#include "ustream_base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

USTREAM* uStreamMockCreate(void);

void setSeekResult(USTREAM_RESULT result);
void setResetResult(USTREAM_RESULT result);
void setGetNextResult(USTREAM_RESULT result);
void setGetRemainingSizeResult(USTREAM_RESULT result);
void setGetCurrentPositionResult(USTREAM_RESULT result);
void setReleaseResult(USTREAM_RESULT result);
void setCloneResult(USTREAM_RESULT result);
void setDisposeResult(USTREAM_RESULT result);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_TESTS_INC_USTREAM_MOCK_BUFFER_H_ */
