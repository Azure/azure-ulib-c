// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_TESTS_INC_USTREAMBUFFER_MOCK_BUFFER_H_
#define AZURE_ULIB_C_TESTS_INC_USTREAMBUFFER_MOCK_BUFFER_H_

#include "ustreambuffer_base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

USTREAMBUFFER_INTERFACE uStreamBufferMockCreate(void);

void setSeekResult(USTREAMBUFFER_RESULT result);
void setResetResult(USTREAMBUFFER_RESULT result);
void setGetNextResult(USTREAMBUFFER_RESULT result);
void setGetRemainingSizeResult(USTREAMBUFFER_RESULT result);
void setGetCurrentPositionResult(USTREAMBUFFER_RESULT result);
void setReleaseResult(USTREAMBUFFER_RESULT result);
void setCloneResult(USTREAMBUFFER_RESULT result);
void setDisposeResult(USTREAMBUFFER_RESULT result);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_TESTS_INC_USTREAMBUFFER_MOCK_BUFFER_H_ */
