// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_TEST_THREAD_H
#define AZ_ULIB_TEST_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*THREAD_START_FUNC)(void*);

typedef enum TEST_THREAD_RESULT_TAG
{
    TEST_THREAD_OK,
    TEST_THREAD_INVALID_ARG,
    TEST_THREAD_NO_MEMORY,
    TEST_THREAD_ERROR
} TEST_THREAD_RESULT;

typedef void* THREAD_HANDLE;

TEST_THREAD_RESULT test_thread_create(THREAD_HANDLE* thread_handle, THREAD_START_FUNC func, void* arg);

TEST_THREAD_RESULT test_thread_join(THREAD_HANDLE thread_handle, int* res);

void test_thread_exit(int res);

void test_thread_sleep(unsigned int milliseconds);

#ifdef __cplusplus
}
#endif

#endif /* AZ_ULIB_TEST_THREAD_H */
