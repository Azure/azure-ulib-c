#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.2.0)

project(azure_ulib_c)

set(AZURE_ULIB_C_VERSION 0.1.0)

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

if(${run_ulib_e2e_tests} OR ${run_ulib_unit_tests})
    include(CTest)
endif()

#Add library of ulib c files
add_library(azure_ulib_c
    ${PROJECT_SOURCE_DIR}/ulog/ulog.c
    ${PROJECT_SOURCE_DIR}/ustream/ustream_aux.c
    ${PROJECT_SOURCE_DIR}/ustream/ustream.c
    ${PROJECT_SOURCE_DIR}/src/az_ulib_ipc/az_ulib_ipc.c
    ${PROJECT_SOURCE_DIR}/pal/os/src/${ULIB_PAL_OS_DIRECTORY}/az_pal_os.c
)

#Add include directories for this target and anyone linking against it
target_include_directories(azure_ulib_c
    PUBLIC
        ${PROJECT_SOURCE_DIR}/inc
        ${PROJECT_SOURCE_DIR}/config
        ${PROJECT_SOURCE_DIR}/pal/${ULIB_PAL_DIRECTORY}
        ${PROJECT_SOURCE_DIR}/pal/os/inc
        ${PROJECT_SOURCE_DIR}/pal/os/inc/${ULIB_PAL_OS_DIRECTORY}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/deps/umock-c/inc
        ${PROJECT_SOURCE_DIR}/deps/azure-macro-utils-c/inc
)

set_target_properties(azure_ulib_c
    PROPERTIES
        FOLDER "uLib Library"
)

target_compile_options(azure_ulib_c
    PRIVATE
        $<$<C_COMPILER_ID:Clang>:-Wdocumentation>
)

set(AZURE_ULIB_C_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using sharedLib lib" FORCE)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/azure-macro-utils-c EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/umock-c EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/azure-ctest EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/azure-c-testrunnerswitcher EXCLUDE_FROM_ALL)

ulib_set_target_build_properties(azure_ulib_c)

if (${BUILD_TESTING})
    add_subdirectory(tests)
endif()

if (NOT ${skip_samples})
    add_subdirectory(samples)
endif()
