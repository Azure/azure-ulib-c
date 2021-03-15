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
option(REMOVE_IPC_UNPUBLISH "Remove the ipc unpublish and all the extra code required to handle it." OFF)

# disable preconditions when it's set to OFF
if (NOT PRECONDITIONS)
  add_compile_definitions(AZ_NO_PRECONDITION_CHECKING)
endif()

if (NOT LOGGING)
  add_compile_definitions(AZ_NO_LOGGING)
endif()

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
elseif(DEFINED ENV{VCPKG_INSTALLATION_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_INSTALLATION_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
  set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

# Use the static runtime libraries when building statically for consistency with vcpkg's
# arch-windows-static triplets:
cmake_policy(SET CMP0091 NEW) # https://cmake.org/cmake/help/v3.15/variable/CMAKE_MSVC_RUNTIME_LIBRARY.html
if (NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project(azure_ulib_c)
enable_testing()

set(AZURE_ULIB_C_VERSION 0.1.0)

set(CMAKE_C_STANDARD 99)

#Include helper functions
include("build_all/configs/ulib_functions.cmake")
include("build_all/configs/ulib_build_rules.cmake")

#Set the standards for C and C++
ulib_add_standards()

#Add library of ulib c files
add_library(azure_ulib_c
    ${PROJECT_SOURCE_DIR}/src/az_ulib_ulog/az_ulib_ulog.c
    ${PROJECT_SOURCE_DIR}/src/az_ulib_ustream/az_ulib_ustream_aux.c
    ${PROJECT_SOURCE_DIR}/src/az_ulib_ustream/az_ulib_ustream.c
    ${PROJECT_SOURCE_DIR}/src/az_ulib_ipc/az_ulib_ipc.c
    ${PROJECT_SOURCE_DIR}/pal/os/src/${ULIB_PAL_OS_DIRECTORY}/az_ulib_pal_os.c
)

#Add include directories for this target and anyone linking against it
target_include_directories(azure_ulib_c
    PUBLIC
        ${PROJECT_SOURCE_DIR}/inc
        ${PROJECT_SOURCE_DIR}/config
        ${PROJECT_SOURCE_DIR}/pal/${ULIB_PAL_DIRECTORY}
        ${PROJECT_SOURCE_DIR}/pal/os/inc
        ${PROJECT_SOURCE_DIR}/pal/os/inc/${ULIB_PAL_OS_DIRECTORY}
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

if(${REMOVE_IPC_UNPUBLISH})
    target_compile_definitions(azure_ulib_c
        PUBLIC
            AZ_ULIB_CONFIG_REMOVE_UNPUBLISH
    )
endif()

target_link_libraries(azure_ulib_c
  PUBLIC
    az::core
)

set(AZURE_ULIB_C_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using sharedLib lib" FORCE)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/azure-core-c)

ulib_set_target_build_properties(azure_ulib_c)

# default for Unit testing with cmocka is OFF, however, this will be ON on CI and tests must
# pass before committing changes
if (UNIT_TESTING)
  # make generate step fail if cmocka dependency is not found
  find_package(cmocka CONFIG REQUIRED)

  # When using VCPKG, cmocka lib name is set different than when it is globally installed
  if(DEFINED ENV{VCPKG_ROOT} OR DEFINED ENV{VCPKG_INSTALLATION_ROOT})
    set(CMOCKA_LIB ${CMOCKA_LIBRARIES})
  else()
    set(CMOCKA_LIB cmocka::cmocka)
  endif()

  # for gcc, we need to add no-clobbered compile opt to avoid warning about set-jump function
  set(NO_CLOBBERED_WARNING "")
  if (CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(NO_CLOBBERED_WARNING "-Wno-clobbered")
  endif()

  set(TEST_DIRECTORY ${PROJECT_SOURCE_DIR}/tests)

  add_subdirectory(tests)
endif()

if (NOT ${SKIP_SAMPLES})
    add_subdirectory(samples)
endif()
