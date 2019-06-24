#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.2)

set(this_test_suite_name ucontract_ut)

set(${this_test_suite_name}_test_files
    ${this_test_suite_name}.c
)

set(${this_test_suite_name}_c_files
    ../src/ustream_mock_buffer.c
    ../../ulog/ulog.c
    ../../ustream/ustream.c
    ../../ustream/ustream_base.c
    ../../ustream/ustream_multi.c
)

build_c_test_target(${this_test_suite_name} "tests/ucontract_tests")
