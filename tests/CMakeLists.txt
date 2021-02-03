#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. 
#See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.10)

ulib_use_permissive_rules_for_samples_and_tests()

if(${run_ulib_unit_tests})
    add_subdirectory(tests_ut/az_ulib_descriptor_ut)
    add_subdirectory(tests_ut/az_ulib_ipc_ut)
    add_subdirectory(tests_ut/az_ulib_ustream_ut)
    add_subdirectory(tests_ut/az_ulib_ucontract_ut)
    add_subdirectory(tests_ut/az_ulib_ustream_aux_ut)
endif()

if(${run_ulib_e2e_tests})
    add_subdirectory(tests_e2e/az_ulib_ipc_e2e)
    add_subdirectory(tests_e2e/az_ulib_ustream_e2e)
    add_subdirectory(tests_e2e/az_ulib_ustream_aux_e2e)
endif()

