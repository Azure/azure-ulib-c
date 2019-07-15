#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.2)

ulib_add_standards()

ulib_use_permissive_rules_for_samples_and_tests()

set(this_sample_name ustream_basic)

set(${this_sample_name}_c_files
    ./src/main.c
)

ulib_build_c_sample_target(${this_sample_name} "samples/ustream_basic")
