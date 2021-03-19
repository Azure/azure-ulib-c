# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.10)

project(az_cmocka VERSION 1.1.5 LANGUAGES C)
set(CMAKE_C_STANDARD 99)

set(CMOCKA_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/cmocka/include)

# config.h checks
include(CheckCCompilerFlag)
include(cmocka/ConfigureChecks.cmake)
configure_file(cmocka/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_library (
  az_cmocka
  ${CMAKE_CURRENT_LIST_DIR}/cmocka/src/cmocka.c
)

target_include_directories (az_cmocka
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  $<BUILD_INTERFACE:${CMOCKA_INCLUDE_DIR}>
)

target_compile_options(az_cmocka PRIVATE -DHAVE_CONFIG_H)

if ( MSVC )
  target_compile_options(az_cmocka PRIVATE /wd4267 /wd4244 /wd4267 /wd4996)
else ()
  target_compile_options(az_cmocka PRIVATE -Wno-implicit-function-declaration -Wno-int-conversion)
endif()

# make sure that users can consume the project as a library.
add_library (az::cmocka ALIAS az_cmocka)
