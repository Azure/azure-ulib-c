#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. 
#See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.10)

add_executable(ipc_hardware_update
  ${CMAKE_CURRENT_LIST_DIR}/main.c
  ${CMAKE_CURRENT_LIST_DIR}/consumers/my_consumer.c
  ${CMAKE_CURRENT_LIST_DIR}/producers/contoso.200401/contoso_display_200401.c
  ${CMAKE_CURRENT_LIST_DIR}/producers/contoso.480401/contoso_display_480401.c
)

#Add include directories for this target and anyone linking against it
target_include_directories(ipc_hardware_update
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/display_1_interface
    ${CMAKE_CURRENT_LIST_DIR}/consumers
    ${CMAKE_CURRENT_LIST_DIR}/producers/contoso.200401
    ${CMAKE_CURRENT_LIST_DIR}/producers/contoso.480401
)

ulib_populate_sample_target(ipc_hardware_update)
