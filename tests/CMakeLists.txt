#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. 
#See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.10)

ulib_use_permissive_rules_for_samples_and_tests()

if(${UNIT_TESTING})
    add_subdirectory(tests_ut/az_ulib_ipc_ut)
    add_subdirectory(tests_ut/az_ulib_registry_ut)
    add_subdirectory(tests_ut/az_ulib_ustream_ut)
    add_subdirectory(tests_ut/az_ulib_ustream_forward_ut)
    add_subdirectory(tests_e2e/az_ulib_ipc_e2e)
    add_subdirectory(tests_e2e/az_ulib_ustream_e2e)
    add_subdirectory(tests_e2e/az_ulib_ustream_forward_e2e)
endif()

