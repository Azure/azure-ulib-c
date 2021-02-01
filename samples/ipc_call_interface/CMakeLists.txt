#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. 
#See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.10)

add_executable(ipc_call_interface
  ${CMAKE_CURRENT_LIST_DIR}/user/main.c
  ${CMAKE_CURRENT_LIST_DIR}/user/consumer.c
  ${CMAKE_CURRENT_LIST_DIR}/user/producer.c
  ${CMAKE_CURRENT_LIST_DIR}/interfaces/math_interface.c
  ${CMAKE_CURRENT_LIST_DIR}/wrappers/math_wrapper.c
)

#Add include directories for this target and anyone linking against it
target_include_directories(ipc_call_interface
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/user
)

ulib_populate_sample_target(ipc_call_interface)
