#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

set(thisTestSuiteName ustream_ownership_ut)

set(${thisTestSuiteName}_test_files
    ${thisTestSuiteName}.c
)

include_directories(${SHARED_UTIL_REAL_TEST_FOLDER})

set(${thisTestSuiteName}_c_files
    ../../ulog/ulog.c
    ../../ustream/ustream.c
    ../src/ustream_mock_buffer.c
)

set(${thisTestSuiteName}_h_files
	../../inc/ulog.h
	../../inc/ustream.h
	../../inc/ustream_base.h
    ../inc/ustream_compliance_ut.h
    ../inc/ustream_ctest_aux.h
    ../inc/ustream_mock_buffer.h
)

build_c_test_artifacts(${thisTestSuiteName} ON "tests/ustream_ownership_tests")

compile_c_test_artifacts_as(${thisTestSuiteName} C99)
