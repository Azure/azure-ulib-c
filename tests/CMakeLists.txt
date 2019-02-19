#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.12)

usePermissiveRulesForSamplesAndTests()

include_directories(./inc)

add_unittest_directory(ucontract_ut)
add_unittest_directory(ustreambuffer_base_ut)
add_unittest_directory(ustreambuffer_const_ut)
add_unittest_directory(ustreambuffer_multi_ut)
add_unittest_directory(ustreambuffer_no_ownership_ut)
add_unittest_directory(ustreambuffer_ownership_ut)
