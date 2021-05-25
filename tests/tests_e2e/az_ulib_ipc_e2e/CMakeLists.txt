#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. 
#See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.10)

project(az_ulib_ipc_e2e)

include(AddCMockaTest)

add_cmocka_test(az_ulib_ipc_e2e SOURCES
                main.c
                az_ulib_ipc_e2e.c
                ${TEST_DIRECTORY}/src/az_ulib_test_my_interface.c
                ${TEST_DIRECTORY}/src/${ULIB_PAL_OS_DIRECTORY}/az_ulib_test_thread.c
                COMPILE_OPTIONS ${DEFAULT_C_COMPILE_FLAGS} ${NO_CLOBBERED_WARNING}
                LINK_LIBRARIES ${CMOCKA_LIBRARIES} azure_ulib_c ${PAL} az::cmocka
                LINK_OPTIONS ${WRAP_FUNCTIONS}  
                # include cmoka headers and private folder headers
                INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/deps/cmocka/include ${CMAKE_SOURCE_DIR}/inc/ ${CMAKE_SOURCE_DIR}/tests/inc/
                )

add_cmocka_test_environment(az_ulib_ipc_e2e)
