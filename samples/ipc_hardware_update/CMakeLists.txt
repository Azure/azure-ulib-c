#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. 
#See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.10)

add_executable(ipc_hardware_update
  ${CMAKE_CURRENT_LIST_DIR}/main.c
  ${CMAKE_CURRENT_LIST_DIR}/consumers/my_consumer.c
  ${CMAKE_CURRENT_LIST_DIR}/producers/contoso/contoso_display_20x4_1.c
  ${CMAKE_CURRENT_LIST_DIR}/producers/contoso/contoso_display_20x4_bsp.c
  ${CMAKE_CURRENT_LIST_DIR}/producers/contoso/interfaces/contoso_display_20x4_1_interface.c
  ${CMAKE_CURRENT_LIST_DIR}/producers/fabrikan/fabrikan_display_48x4_1.c
  ${CMAKE_CURRENT_LIST_DIR}/producers/fabrikan/fabrikan_display_48x4_bsp.c
  ${CMAKE_CURRENT_LIST_DIR}/producers/fabrikan/interfaces/fabrikan_display_48x4_1_interface.c
)

#Add include directories for this target and anyone linking against it
target_include_directories(ipc_hardware_update
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/common
    ${CMAKE_CURRENT_LIST_DIR}/consumers
    ${CMAKE_CURRENT_LIST_DIR}/producers/contoso
    ${CMAKE_CURRENT_LIST_DIR}/producers/fabrikan
)

ulib_populate_sample_target(ipc_hardware_update)
