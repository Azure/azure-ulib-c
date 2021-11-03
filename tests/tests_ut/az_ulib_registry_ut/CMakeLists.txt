# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.10)

set(TARGET az_ulib_registry_ut)

# Define the Project
project(${TARGET} C ASM)

include(AddCMockaTest)

add_cmocka_test(${TARGET} SOURCES
                main.c
                az_ulib_registry_ut.c
                ${TEST_DIRECTORY}/src/az_ulib_test_my_interface.c
                COMPILE_OPTIONS ${DEFAULT_C_COMPILE_FLAGS} ${NO_CLOBBERED_WARNING}
                LINK_LIBRARIES ${CMOCKA_LIBRARIES} azure_ulib_c ${PAL} az::cmocka
                LINK_OPTIONS ${WRAP_FUNCTIONS}  
                # include cmoka headers and private folder headers
                INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/deps/cmocka/include ${CMAKE_SOURCE_DIR}/inc/ ${CMAKE_SOURCE_DIR}/tests/inc/
                )

add_cmocka_test_environment(${TARGET})
