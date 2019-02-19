#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

project(azure-ulib-c)

set(AZURE_ULIB_C_VERSION 1.0.1)

option(run_e2e_tests "set run_e2e_tests to ON to run e2e tests (default is OFF)" OFF)
option(run_unittests "set run_unittests to ON to run unittests (default is OFF)" OFF)
option(skip_samples "set skip_samples to ON to skip building samples (default is OFF)[if possible, they are always built]" OFF)
option(use_installed_dependencies "set use_installed_dependencies to ON to use installed packages instead of building dependencies from submodules" OFF)

#
#making a global variable to know if we are on linux, windows, or macosx.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(WINDOWS TRUE)
    set(ULIB_PAL_DIRECTORY "MSBUILD/X86")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
    # on Linux, enable valgrind
    # these commands (MEMORYCHECK...) need to apear BEFORE include(CTest) or they will not have any effect
    find_program(MEMORYCHECK_COMMAND valgrind)
    set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --error-exitcode=1")
    set(ULIB_PAL_DIRECTORY "GCC/LINUX")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(MACOSX TRUE)
    add_definitions(-DMACOSX)
    set(ULIB_PAL_DIRECTORY "GCC/IOS")
endif()

include (CTest)

if(${no_logging})
    add_definitions(-DNO_LOGGING)
endif()

#Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

enable_testing()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

set(AZURE_ULIB_C_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using sharedLib lib" FORCE)

#do not add or build any tests of the dependencies
set(original_run_e2e_tests ${run_e2e_tests})
set(original_run_unittests ${run_unittests})

set(run_e2e_tests OFF)
set(run_unittests OFF)

include("dependencies.cmake")

if (${original_run_unittests})
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/umock-c EXCLUDE_FROM_ALL)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/azure-ctest EXCLUDE_FROM_ALL)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/azure-c-testrunnerswitcher EXCLUDE_FROM_ALL)
endif()

set(use_cppunittest ON)
#set_platform_files(${CMAKE_CURRENT_LIST_DIR}/deps/c-utility)

set(run_e2e_tests ${original_run_e2e_tests})
set(run_unittests ${original_run_unittests})


set(azure_ulib_c_files
    ./ulog/ulog.c
    ./ustreambuffer/ustreambuffer_base.c
    ./ustreambuffer/ustreambuffer_multi.c
    ./ustreambuffer/ustreambuffer.c
    ./pal/MemMang/ulib_heap.c
)

set(azure_ulib_h_files
    ./inc/ulog.h
    ./inc/ustreambuffer_base.h
    ./inc/ustreambuffer.h
    ./config/ulib_config.h
    ./inc/ulib_heap.h
    ./pal/${ULIB_PAL_DIRECTORY}/ulib_port.h
)

include_directories(./inc)
include_directories(./config)
include_directories(./pal/${ULIB_PAL_DIRECTORY})
include_directories(./deps/umock-c/inc)

include("build_all/configs/ulib_functions.cmake")
include("build_all/configs/ulib_aux_functions.cmake")
include("build_all/configs/ulib_build_rules.cmake")

add_library(azure-ulib-c ${azure_ulib_c_files} ${azure_ulib_h_files})
setTargetBuildProperties(azure-ulib-c)
target_link_libraries(azure-ulib-c aziotsharedutil)

if (${run_unittests})
    add_subdirectory(tests)
endif()

if (NOT ${skip_samples})
    add_subdirectory(samples)
endif()

compileTargetAsC99(azure-ulib-c)
