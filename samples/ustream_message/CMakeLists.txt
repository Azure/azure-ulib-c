#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.2.0)

add_executable(ustream_message
    ${CMAKE_CURRENT_LIST_DIR}/src/main.c
)

ulib_populate_sample_target(ustream_message)
