#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.10)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ipc_call_interface)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ipc_hardware_update)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ipc_telemetry)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ustream_basic)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ustream_forward_basic)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ustream_split)
