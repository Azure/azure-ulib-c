#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. 
#See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.10)

add_executable(az_ulib_ipc_ut
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/az_ulib_ipc_ut.c
)

ulib_populate_test_target(az_ulib_ipc_ut)
