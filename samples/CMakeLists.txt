#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.2)

include_directories(${CMAKE_SOURCE_DIR}/inc)
include_directories(${CMAKE_SOURCE_DIR}/config)
include_directories(${CMAKE_SOURCE_DIR}/deps/azure-macro-utils-c/inc)
include_directories(${CMAKE_SOURCE_DIR}/deps/umock-c/inc)
include_directories(${CMAKE_SOURCE_DIR}/pal/${ULIB_PAL_DIRECTORY})

