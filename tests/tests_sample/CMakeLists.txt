#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.2.0)

#This cmake is an example of what you would need to compile and run your tests.
#Add any additional .c files here that are needed to run your tests.
add_executable(ustream_ut_sample
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/ustream_ut.c
)

#Links to appropriate libraries and adds to ctest
ulib_populate_test_target(ustream_ut_sample)
