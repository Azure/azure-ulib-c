#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

set(this_test_suite_name ustream_no_ownership_ut)

set(${this_test_suite_name}_test_files
    ${this_test_suite_name}.c
)

include_directories(${SHARED_UTIL_REAL_TEST_FOLDER})

set(${this_test_suite_name}_c_files
    ../../ulog/ulog.c
    ../../ustream/ustream.c
    ../../ustream/ustream_base.c
    ../../ustream/ustream_multi.c
    ../src/ustream_mock_buffer.c
)

set(${this_test_suite_name}_h_files
	../../inc/ulog.h
	../../inc/ustream.h
	../../inc/ustream_base.h
    ../inc/ustream_compliance_ut.h
    ../inc/ustream_ctest_aux.h
    ../inc/ustream_mock_buffer.h
)

build_c_test_artifacts(${this_test_suite_name} ON "tests/ustream_no_ownership_tests")

compile_c_test_artifacts_as(${this_test_suite_name} C99)
