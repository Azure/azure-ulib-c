#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. 
#See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.10)

add_executable(ipc_call_interface
  ${CMAKE_CURRENT_LIST_DIR}/main.c
  ${CMAKE_CURRENT_LIST_DIR}/consumers/my_consumer.c
  ${CMAKE_CURRENT_LIST_DIR}/producers/cipher_v1i1/cipher_v1i1.c
  ${CMAKE_CURRENT_LIST_DIR}/producers/cipher_v1i1/interfaces/cipher_v1i1_interface.c
  ${CMAKE_CURRENT_LIST_DIR}/producers/cipher_v2i1/cipher_v2i1.c
  ${CMAKE_CURRENT_LIST_DIR}/producers/cipher_v2i1/interfaces/cipher_v2i1_interface.c
)

#Add include directories for this target and anyone linking against it
target_include_directories(ipc_call_interface
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/common
    ${CMAKE_CURRENT_LIST_DIR}/consumers
    ${CMAKE_CURRENT_LIST_DIR}/producers/cipher_v1i1
    ${CMAKE_CURRENT_LIST_DIR}/producers/cipher_v2i1
)

ulib_populate_sample_target(ipc_call_interface)
