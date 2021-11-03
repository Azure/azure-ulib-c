# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.10)

set(TARGET azure_upal_c)

# Define the Project
project(${TARGET} C ASM)

#Add library of upal c files
add_library(${TARGET}
    src/os/${ULIB_PAL_OS_DIRECTORY}/az_ulib_pal_os.c
    src/${ULIB_PAL_DIRECTORY}/az_ulib_pal_flash_driver.c
)

add_library(az::upal ALIAS ${TARGET})

#Add include directories for this target and anyone linking against it
target_include_directories(${TARGET}
    PRIVATE
        ${ULIB_RESULT_DIRECTORY}
        ${ULIB_PAL_OS_PORT}
        ${ULIB_PAL_OS_INC}
    PUBLIC
        inc
        inc/common
        inc/${ULIB_PAL_DIRECTORY}
        inc/os
        inc/os/${ULIB_PAL_OS_DIRECTORY}
)

target_link_libraries(${TARGET}
  PUBLIC
    az::core
)
