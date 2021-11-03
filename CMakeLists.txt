#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake-modules")

#Specify user options
option(UNIT_TESTING "Build unit test projects" OFF)
option(PRECONDITIONS "Build uLib with preconditions enabled" ON)
option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(LOGGING "Build uLib with logging support" ON)
option(SKIP_SAMPLES "Skip building samples (default is OFF)[if possible, they are always built]" OFF)
option(USE_INSTALLED_DEPENDENCIES "Use installed packages instead of building dependencies from submodules" OFF)
option(VALIDATE_DOCUMENTATION "set to enable the -Wdocumentation flag on clang to validate documentation.
                                If not using clang this will have no effect." OFF)

message("CONFIGURATIONS:")
if (NOT PRECONDITIONS)
  message("  -- Preconditions OFF")
  add_compile_definitions(AZ_NO_PRECONDITION_CHECKING)
else()
  message("  -- Preconditions ON")
endif()

if (NOT WARNINGS_AS_ERRORS)
  message("  -- Warnings as errors OFF")
else()
  message("  -- Warnings as errors ON")
endif()

if (NOT LOGGING)
  message("  -- Logging OFF")
  add_compile_definitions(AZ_NO_LOGGING)
else()
  message("  -- Logging ON")
endif()

if (NOT VALIDATE_DOCUMENTATION)
  message("  -- Validate documentation OFF")
else()
  message("  -- Validate documentation ON")
endif()

if (SKIP_SAMPLES)
  message("  -- Samples OFF")
else()
  message("  -- Samples ON")
endif()

if (UNIT_TESTING)
  message("  -- Testing ON")
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps)
else()
  message("  -- Testing OFF")
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/azure-core-c)

project(azure_ulib_c)
enable_testing()

set(AZURE_ULIB_C_VERSION 0.1.0)

set(CMAKE_C_STANDARD 99)

set(ULIB_RESULT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/inc)

# for gcc, we need to add no-clobbered compile opt to avoid warning about set-jump function
set(NO_CLOBBERED_WARNING "")
if (CMAKE_C_COMPILER_ID MATCHES "GNU")
  set(NO_CLOBBERED_WARNING "-Wno-clobbered")
endif()

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
endif()

#Include Errors and Warnings configuration.
include(build_all/global_compile_options.txt)

#Include helper functions
include("build_all/configs/ulib_functions.cmake")
include("build_all/configs/ulib_build_rules.cmake")

#Set the standards for C and C++
ulib_add_standards()

#Create PAL library.
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/pal)

#Add library of ulib c files
add_library(azure_ulib_c
    ${CMAKE_CURRENT_LIST_DIR}/src/az_ulib_ustream/az_ulib_ustream_aux.c
    ${CMAKE_CURRENT_LIST_DIR}/src/az_ulib_ustream/az_ulib_ustream.c
    ${CMAKE_CURRENT_LIST_DIR}/src/az_ulib_ustream_forward/az_ulib_ustream_forward.c
    ${CMAKE_CURRENT_LIST_DIR}/src/az_ulib_ipc/az_ulib_ipc.c
    ${CMAKE_CURRENT_LIST_DIR}/src/az_ulib_ipc/az_ulib_ipc_query_interface.c
    ${CMAKE_CURRENT_LIST_DIR}/src/az_ulib_ipc/az_ulib_ipc_interface_manager_interface.c
    ${CMAKE_CURRENT_LIST_DIR}/src/az_ulib_registry/az_ulib_registry.c
)

add_library(az::ulib ALIAS azure_ulib_c)

#Add include directories for this target and anyone linking against it
target_include_directories(azure_ulib_c
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/inc
        ${CMAKE_CURRENT_LIST_DIR}/config
        ${CMAKE_CURRENT_LIST_DIR}/deps/azure-core-c/inc
        ${ULIB_PAL_OS_PORT}
        ${ULIB_PAL_OS_INC}
)

set_target_properties(azure_ulib_c
    PROPERTIES
        FOLDER "uLib Library"
)

if(${VALIDATE_DOCUMENTATION})
    target_compile_options(azure_ulib_c
        PRIVATE
            $<$<C_COMPILER_ID:Clang>:-Wdocumentation>
    )
endif()

target_link_libraries(azure_ulib_c
  PUBLIC
    az::upal
    az::core
)

set(AZURE_ULIB_C_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using sharedLib lib" FORCE)

ulib_set_target_build_properties(azure_ulib_c)

# default for Unit testing with cmocka is OFF, however, this will be ON on CI and tests must
# pass before committing changes
if (UNIT_TESTING)
    set(TEST_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/tests)
    add_subdirectory(tests)
endif()

if (NOT ${SKIP_SAMPLES})
    add_subdirectory(samples)
endif()
