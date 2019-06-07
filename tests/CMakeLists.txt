#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.12)

use_permissive_rules_for_samples_and_tests()

include_directories(./inc)

add_unittest_directory(ucontract_ut)
add_unittest_directory(ustream_base_ut)
add_unittest_directory(ustream_const_ut)
add_unittest_directory(ustream_multi_ut)
add_unittest_directory(ustream_no_ownership_ut)
add_unittest_directory(ustream_ownership_ut)
