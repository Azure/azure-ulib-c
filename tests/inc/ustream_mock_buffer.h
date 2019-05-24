// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_TESTS_INC_USTREAM_MOCK_BUFFER_H_
#define AZURE_ULIB_C_TESTS_INC_USTREAM_MOCK_BUFFER_H_

#include "ustream_base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

USTREAM* uStreamMockCreate(void);

void setSeekResult(ULIB_RESULT result);
void setResetResult(ULIB_RESULT result);
void setGetNextResult(ULIB_RESULT result);
void setGetRemainingSizeResult(ULIB_RESULT result);
void setGetCurrentPositionResult(ULIB_RESULT result);
void setReleaseResult(ULIB_RESULT result);
void setCloneResult(ULIB_RESULT result);
void setDisposeResult(ULIB_RESULT result);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_TESTS_INC_USTREAM_MOCK_BUFFER_H_ */
