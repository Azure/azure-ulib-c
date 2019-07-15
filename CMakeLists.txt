#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.2)

project(azure_ulib_c)

set(AZURE_ULIB_C_VERSION 1.0.1)

#Include helper functions
include("build_all/configs/ulib_functions.cmake")
include("build_all/configs/ulib_build_rules.cmake")

#Set the standards for C and C++
ulib_add_standards()

#Specify user options
option(run_ulib_e2e_tests "set run_ulib_e2e_tests to ON to run e2e tests (default is OFF)" OFF)
option(run_ulib_unit_tests "set run_ulib_unit_tests to ON to run unittests (default is OFF)" OFF)
option(skip_samples "set skip_samples to ON to skip building samples (default is OFF)[if possible, they are always built]" OFF)
option(use_installed_dependencies "set use_installed_dependencies to ON to use installed packages instead of building dependencies from submodules" OFF)

#Set ulib c files
set(azure_ulib_c_files
    ${PROJECT_SOURCE_DIR}/pal/MemMang/ulib_heap.c
    ${PROJECT_SOURCE_DIR}/ulog/ulog.c
    ${PROJECT_SOURCE_DIR}/ustream/ustream_base.c
    ${PROJECT_SOURCE_DIR}/ustream/ustream_multi.c
    ${PROJECT_SOURCE_DIR}/ustream/ustream.c
)

#Add library of ulib c files
add_library(azure_ulib_c ${azure_ulib_c_files})

#Set the include directories for this project
set(azure_ulib_h_dir
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/config
    ${PROJECT_SOURCE_DIR}/pal/${ULIB_PAL_DIRECTORY}
)

#Add include directories for this target and anyone linking against it
target_include_directories(azure_ulib_c PUBLIC ${azure_ulib_h_dir})
target_include_directories(azure_ulib_c PRIVATE ./deps/umock-c/inc ./deps/azure-macro-utils-c/inc)

set(AZURE_ULIB_C_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using sharedLib lib" FORCE)

#Add deps folders and turn off any tests for them
set(run_e2e_tests OFF)
set(run_unittests OFF)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/umock-c EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/azure-ctest EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/azure-macro-utils-c EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/azure-c-testrunnerswitcher EXCLUDE_FROM_ALL)

ulib_set_target_build_properties(azure_ulib_c)

if (${run_ulib_unit_tests})
    add_subdirectory(tests)
endif()

if (NOT ${skip_samples})
    add_subdirectory(samples)
endif()
