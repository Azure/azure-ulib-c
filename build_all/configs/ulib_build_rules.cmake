#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Build with -fPIC always
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

option(RUN_VALGRIND "set RUN_VALGRIND to ON if tests are to be run under valgrind/helgrind/drd. Default is OFF" OFF)
option(compileOption_C "passes a string to the command line of the C compiler" OFF)
option(compileOption_CXX "passes a string to the command line of the C++ compiler" OFF)

# Relax warnings when applicable
function(ulib_use_permissive_rules_for_samples_and_tests)
    if(NOT MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable  -Wno-unused-function -Wno-missing-braces -Wno-strict-aliasing")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable  -Wno-unused-function -Wno-missing-braces -Wno-strict-aliasing")
        if(NOT APPLE AND NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-but-set-variable -Wno-clobbered")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-but-set-variable -Wno-clobbered")
        endif()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" PARENT_SCOPE)
    endif()
endfunction()

# Global variable to know if we are on linux, windows, or macosx.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(ULIB_PAL_DIRECTORY "MSBUILD/X86")
    set(ULIB_PAL_OS_DIRECTORY "windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Enable valgrind on Linux
    find_program(MEMORYCHECK_COMMAND valgrind)
    set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --error-exitcode=1")
    set(ULIB_PAL_DIRECTORY "GCC/LINUX")
    set(ULIB_PAL_OS_DIRECTORY "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_definitions(-DMACOSX)
    set(ULIB_PAL_DIRECTORY "GCC/IOS")
    set(ULIB_PAL_OS_DIRECTORY "linux")
endif()

# System-specific compiler flags
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    if(WINCE) # Be lax with WEC 2013 compiler
        add_definitions(-DWIN32) #WEC 2013
        # Don't treat warning as errors for WEC 2013. WEC 2013 uses older compiler version
        add_definitions(/WX-)
    else()
        # Make warning as error
        add_definitions(/WX)
    endif()
else()
    # Make warning as error
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    if(NOT(IN_OPENWRT OR APPLE))
        set(CMAKE_C_FLAGS "-D_POSIX_C_SOURCE=200112L ${CMAKE_C_FLAGS}")
    endif()
endif()

#Detect and set target architecture
include(CheckSymbolExists)
function(ulib_detect_architecture symbol arch)
    if(NOT DEFINED ARCHITECTURE OR ARCHITECTURE STREQUAL "")
        set(CMAKE_REQUIRED_QUIET 1)
        check_symbol_exists("${symbol}" "" ARCHITECTURE_${arch})
        unset(CMAKE_REQUIRED_QUIET)

        # The output variable needs to be unique across invocations otherwise
        # CMake's crazy scope rules will keep it defined
        if(ARCHITECTURE_${arch})
            set(ARCHITECTURE "${arch}" PARENT_SCOPE)
            set(ARCHITECTURE_${arch} 1 PARENT_SCOPE)
            add_definitions(-DARCHITECTURE_${arch}=1)
        endif()
    endif()
endfunction()

if(MSVC)
    ulib_detect_architecture("_M_AMD64" x86_64)
    ulib_detect_architecture("_M_IX86" x86)
    ulib_detect_architecture("_M_ARM" ARM)
else()
    ulib_detect_architecture("__x86_64__" x86_64)
    ulib_detect_architecture("__i386__" x86)
    ulib_detect_architecture("__arm__" ARM)
endif()
if (NOT DEFINED ARCHITECTURE OR ARCHITECTURE STREQUAL "")
    set(ARCHITECTURE "GENERIC")
endif()
message(STATUS "target architecture: ${ARCHITECTURE}")

#if any compiler has a command line switch called "OFF" then it will need special care
if(NOT "${compileOption_C}" STREQUAL "OFF")
    set(CMAKE_C_FLAGS "${compileOption_C} ${CMAKE_C_FLAGS}")
endif()

if(NOT "${compileOption_CXX}" STREQUAL "OFF")
    set(CMAKE_CXX_FLAGS "${compileOption_CXX} ${CMAKE_CXX_FLAGS}")
endif()

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" CXX_FLAG_CXX11)

# For targets which set warning switches as project properties (e.g. XCode)
function(ulib_set_target_build_properties stbp_target)
    if(XCODE)
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_BLOCK_CAPTURE_AUTORELEASING "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_BOOL_CONVERSION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_COMMA "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_CONSTANT_CONVERSION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_DEPRECATED_OBJC_IMPLEMENTATIONS "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_EMPTY_BODY "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_ENUM_CONVERSION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_INFINITE_RECURSION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_INT_CONVERSION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_NON_LITERAL_NULL_CONVERSION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_OBJC_IMPLICIT_RETAIN_SELF "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_OBJC_LITERAL_CONVERSION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_OBJC_ROOT_CLASS "YES_ERROR")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_RANGE_LOOP_ANALYSIS "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_STRICT_PROTOTYPES "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_SUSPICIOUS_MOVE "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN_UNREACHABLE_CODE "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_CLANG_WARN__DUPLICATE_METHOD_MATCH "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_64_TO_32_BIT_CONVERSION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_ABOUT_RETURN_TYPE "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNDECLARED_SELECTOR "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNINITIALIZED_AUTOS "YES_AGGRESSIVE")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNUSED_FUNCTION "YES")
        set_target_properties(${stbp_target} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_UNUSED_VARIABLE "YES")
    endif()
endfunction()

