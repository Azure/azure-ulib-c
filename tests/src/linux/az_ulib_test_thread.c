// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "az_ulib_test_thread.h"

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>

#include <pthread.h>
#include <time.h>

typedef struct THREAD_INSTANCE_TAG
{
    pthread_t Pthread_handle;
    THREAD_START_FUNC ThreadStartFunc;
    void* Arg;
} THREAD_INSTANCE;

static void* ThreadWrapper(void* threadInstanceArg)
{
    THREAD_INSTANCE* threadInstance = (THREAD_INSTANCE*)threadInstanceArg;
    int result = threadInstance->ThreadStartFunc(threadInstance->Arg);
    return (void*)(intptr_t)result;
}

TEST_THREAD_RESULT test_thread_create(THREAD_HANDLE* threadHandle, THREAD_START_FUNC func, void* arg)
{
    TEST_THREAD_RESULT result;

    if ((threadHandle == NULL) ||
        (func == NULL))
    {
        result = TEST_THREAD_INVALID_ARG;
    }
    else
    {
        THREAD_INSTANCE* threadInstance = malloc(sizeof(THREAD_INSTANCE));
        if (threadInstance == NULL)
        {
            result = TEST_THREAD_NO_MEMORY;
        }
        else
        {
            threadInstance->ThreadStartFunc = func;
            threadInstance->Arg = arg;
            int createResult = pthread_create(&threadInstance->Pthread_handle, NULL, ThreadWrapper, threadInstance);
            switch (createResult)
            {
            default:
                free(threadInstance);

                result = TEST_THREAD_ERROR;
                break;

            case 0:
                *threadHandle = threadInstance;
                result = TEST_THREAD_OK;
                break;

            case EAGAIN:
                free(threadInstance);

                result = TEST_THREAD_NO_MEMORY;
                break;
            }
        }
    }

    return result;
}

TEST_THREAD_RESULT test_thread_join(THREAD_HANDLE thread_handle, int* res)
{
    TEST_THREAD_RESULT result;

    THREAD_INSTANCE* thread_instance = (THREAD_INSTANCE*)thread_handle;
    if(thread_instance == NULL)
    {
        result = TEST_THREAD_INVALID_ARG;
    }
    else
    {
        void* threadResult;
        if(pthread_join(thread_instance->Pthread_handle, &threadResult) != 0)
        {
            result = TEST_THREAD_ERROR;
        }
        else
        {
            if(res != NULL)
            {
                *res = (int)(intptr_t)threadResult;
            }

            result = TEST_THREAD_OK;
        }

        free(thread_instance);
    }

    return result;
}

void test_thread_exit(int res)
{
    pthread_exit((void*)(intptr_t)res);
}

void test_thread_sleep(unsigned int milliseconds)
{
    time_t seconds = milliseconds / 1000;
    long nsRemainder = (milliseconds % 1000) * 1000000;
    struct timespec timeToSleep = { seconds, nsRemainder };
    (void)nanosleep(&timeToSleep, NULL);
}
