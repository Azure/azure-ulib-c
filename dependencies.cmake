#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

if (${run_unittests})
    if(${use_installed_dependencies})
        #These need to be set for the functions included by umock-c
        if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/deps/umock-c/src")
            message(FATAL_ERROR "umock-c directory is empty. Please pull in all submodules if building unit tests.")
        endif()
        set(UMOCK_C_SRC_FOLDER "${CMAKE_CURRENT_LIST_DIR}/deps/umock-c/src")
        set(UMOCK_C_FOLDER "${CMAKE_CURRENT_LIST_DIR}/deps/umock-c")
        find_package(umock_c REQUIRED CONFIG)
    endif()
endif()
