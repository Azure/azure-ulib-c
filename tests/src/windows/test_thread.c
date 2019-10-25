// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "windows.h"
#include "test_thread.h"

TEST_THREAD_RESULT test_thread_create(THREAD_HANDLE* thread_handle, THREAD_START_FUNC func, void* arg)
{
    TEST_THREAD_RESULT result;
    if ((thread_handle == NULL) ||
        (func == NULL))
    {
        result = TEST_THREAD_INVALID_ARG;
    }
    else
    {
        *thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, 0, NULL);
        if(*thread_handle == NULL)
        {
            result = (GetLastError() == ERROR_OUTOFMEMORY) ? TEST_THREAD_NO_MEMORY : TEST_THREAD_ERROR;

        }
        else
        {
            result = TEST_THREAD_OK;
        }
    }

    return result;
}

TEST_THREAD_RESULT test_thread_join(THREAD_HANDLE thread_handle, int* res)
{
    TEST_THREAD_RESULT result = TEST_THREAD_OK;

    if(thread_handle == NULL)
    {
        result = TEST_THREAD_INVALID_ARG;
    }
    else
    {
        DWORD returnCode = WaitForSingleObject(thread_handle, INFINITE);

        if(returnCode != WAIT_OBJECT_0)
        {
            result = TEST_THREAD_ERROR;
        }
        else
        {
            if(res != NULL)
            {
                DWORD exit_code;
                if(!GetExitCodeThread(thread_handle, &exit_code))   //If thread end is signaled we need to get the Thread Exit Code;
                {
                    result = TEST_THREAD_ERROR;
                }
                else
                {
                    *res = (int)exit_code;
                }
            }
        }
        CloseHandle(thread_handle);
    }

    return result;
}

void test_thread_exit(int res)
{
    ExitThread(res);
}

void test_thread_sleep(unsigned int milliseconds)
{
    Sleep(milliseconds);
}
