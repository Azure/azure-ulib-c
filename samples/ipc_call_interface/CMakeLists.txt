#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. 
#See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.2.0)

add_executable(ipc_call_interface
  ${CMAKE_CURRENT_LIST_DIR}/src/main.c
  ${CMAKE_CURRENT_LIST_DIR}/src/math.c
  ${CMAKE_CURRENT_LIST_DIR}/src/consumer.c
)

#Add include directories for this target and anyone linking against it
target_include_directories(ipc_call_interface
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

ulib_populate_sample_target(ipc_call_interface)
