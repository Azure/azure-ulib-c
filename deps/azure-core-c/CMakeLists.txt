# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.10)

# Read in az_version.h
#   The build will fail with an error if the file is not found or improperly formatted
set(AZ_SDK_VERSION_H_FILE "${CMAKE_CURRENT_LIST_DIR}/inc/azure/core/az_version.h")

if(EXISTS ${AZ_SDK_VERSION_H_FILE})
    file (STRINGS ${AZ_SDK_VERSION_H_FILE} VERSION_H_CONTENT)
    message(STATUS "Read version from ${AZ_SDK_VERSION_H_FILE}")

    #Find and read the AZ_SDK_VERSION_STRING define
    string(REGEX MATCHALL "#define AZ_SDK_VERSION_STRING \"([0-9]+)\.([0-9]+)\.([0-9]+)(\-[^\"\-]+)?\"" VERSION_PARTS ${VERSION_H_CONTENT})

    #Ensure we matched as expected.
    #   MAJOR.AZ_VERSION_MINOR.PATCH are required.
    #   PRERELEASE is optional.
    if(NOT CMAKE_MATCH_1 AND NOT CMAKE_MATCH_2 AND NOT CMAKE_MATCH_3)
        message(FATAL_ERROR "Unexpected version format in ${AZ_SDK_VERSION_H_FILE}")
    endif()
    set(AZ_SDK_VERSION_MAJOR ${CMAKE_MATCH_1} )
    set(AZ_SDK_VERSION_MINOR ${CMAKE_MATCH_2} )
    set(AZ_SDK_VERSION_PATCH ${CMAKE_MATCH_3} )
    if(CMAKE_MATCH_4)
        #Trim off the "-" from the start of pre-release portion
        # https://cmake.org/cmake/help/v3.10/command/string.html?highlight=substring#substring
        string(SUBSTRING ${CMAKE_MATCH_4} 1 -1 PRERELEASE_SUBSTRING )
        set(AZ_SDK_VERSION_PRERELEASE ${PRERELEASE_SUBSTRING})
    endif()
    message(STATUS "AZ_SDK_VERSION_MAJOR " ${AZ_SDK_VERSION_MAJOR})
    message(STATUS "AZ_SDK_VERSION_MAJOR " ${AZ_SDK_VERSION_MINOR})
    message(STATUS "AZ_SDK_VERSION_PATCH " ${AZ_SDK_VERSION_PATCH})
    message(STATUS "AZ_SDK_VERSION_PRERELEASE " ${AZ_SDK_VERSION_PRERELEASE})
else()
  message(FATAL_ERROR "Missing Version file ${AZ_SDK_VERSION_H_FILE}")
endif()

if(NOT ${USE_INSTALLED_DEPENDENCIES})
  project (az_core LANGUAGES C VERSION ${AZ_SDK_VERSION_MAJOR}.${AZ_SDK_VERSION_MINOR}.${AZ_SDK_VERSION_PATCH})
  set(CMAKE_C_STANDARD 99)

  add_library (
    az_core
    ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_context.c
    ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_http_pipeline.c
    ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_http_policy.c
    ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_http_policy_logging.c
    ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_http_policy_retry.c
    ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_http_request.c
    ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_http_response.c
    ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_json_reader.c
    ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_json_token.c
    ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_json_writer.c
    ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_log.c
    ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_precondition.c
    ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_span.c
  )

  target_include_directories (az_core
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
    $<INSTALL_INTERFACE:include/az_core>
  )

  # include test internal headers
  target_include_directories(az_core
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/tests/core/inc>
  )

  target_link_libraries(az_core
    PUBLIC
      ${PAL}
  )

  # make sure that users can consume the project as a library.
  add_library (az::core ALIAS az_core)
endif()

project (az_core_pic LANGUAGES C VERSION ${AZ_SDK_VERSION_MAJOR}.${AZ_SDK_VERSION_MINOR}.${AZ_SDK_VERSION_PATCH})
set(CMAKE_C_STANDARD 99)

add_library (
  az_core_pic
  ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_context.c
  ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_http_pipeline.c
  ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_http_policy.c
  ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_http_policy_logging.c
  ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_http_policy_retry.c
  ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_http_request.c
  ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_http_response.c
  ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_json_reader.c
  ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_json_token.c
  ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_json_writer.c
  ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_log.c
  ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_precondition.c
  ${CMAKE_CURRENT_LIST_DIR}/src/azure/core/az_span.c
)

target_include_directories (az_core_pic
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
  $<INSTALL_INTERFACE:include/az_core>
)

target_compile_options(az_core_pic
    PRIVATE
        -fpie -fno-jump-tables ${PIC_FLAGS})

# include test internal headers
target_include_directories(az_core_pic
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/tests/core/inc>
)

target_link_libraries(az_core_pic
  PUBLIC
    ${PAL}
)

# make sure that users can consume the project as a library.
add_library (az::core_pic ALIAS az_core_pic)
