#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. 
#See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.10)

add_executable(ipc_telemetry
  ${CMAKE_CURRENT_LIST_DIR}/main.c
  ${CMAKE_CURRENT_LIST_DIR}/consumers/my_consumer.c
  ${CMAKE_CURRENT_LIST_DIR}/producers/sensors_1.c
)

#Add include directories for this target and anyone linking against it
target_include_directories(ipc_telemetry
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/sensors_1_interface
    ${CMAKE_CURRENT_LIST_DIR}/consumers
    ${CMAKE_CURRENT_LIST_DIR}/producers
)

ulib_populate_sample_target(ipc_telemetry)
