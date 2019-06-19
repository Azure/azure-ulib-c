#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.2)

usePermissiveRulesForSamplesAndTests()

include_directories(./inc)
include_directories(${CMAKE_SOURCE_DIR}/inc)
include_directories(${CMAKE_SOURCE_DIR}/config)
include_directories(${CMAKE_SOURCE_DIR}/deps/azure-macro-utils-c/inc)
include_directories(${CMAKE_SOURCE_DIR}/deps/umock-c/inc)
include_directories(${CMAKE_SOURCE_DIR}/pal/${ULIB_PAL_DIRECTORY})

if(${run_ulib_unit_tests})
    add_subdirectory(ucontract_ut)
    add_subdirectory(ustream_base_ut)
    add_subdirectory(ustream_const_ut)
    add_subdirectory(ustream_multi_ut)
    add_subdirectory(ustream_no_ownership_ut)
    add_subdirectory(ustream_ownership_ut)
endif()
