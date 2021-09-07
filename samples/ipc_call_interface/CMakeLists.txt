#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. 
#See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.10)

add_executable(ipc_call_interface
  ${CMAKE_CURRENT_LIST_DIR}/main.c
  ${CMAKE_CURRENT_LIST_DIR}/consumers/my_consumer.c
  ${CMAKE_CURRENT_LIST_DIR}/producers/key_vault_1/key_vault_1.c
  ${CMAKE_CURRENT_LIST_DIR}/producers/key_vault_2/key_vault_2.c
)

#Add include directories for this target and anyone linking against it
target_include_directories(ipc_call_interface
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/cipher_1_interface
    ${CMAKE_CURRENT_LIST_DIR}/consumers
    ${CMAKE_CURRENT_LIST_DIR}/producers/key_vault_1
    ${CMAKE_CURRENT_LIST_DIR}/producers/key_vault_2
)

ulib_populate_sample_target(ipc_call_interface)
