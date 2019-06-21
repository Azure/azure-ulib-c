#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.2)

usePermissiveRulesForSamplesAndTests()

if(${run_ulib_unit_tests})
    add_subdirectory(ucontract_ut)
    add_subdirectory(ustream_base_ut)
    add_subdirectory(ustream_const_ut)
    add_subdirectory(ustream_multi_ut)
    add_subdirectory(ustream_no_ownership_ut)
    add_subdirectory(ustream_ownership_ut)
endif()
