#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

set(thisTestSuiteName ucontract_ut)

set(${thisTestSuiteName}_test_files
    ${thisTestSuiteName}.c
)

include_directories(${SHARED_UTIL_REAL_TEST_FOLDER})

set(${thisTestSuiteName}_c_files
    ../../ulog/ulog.c
)

set(${thisTestSuiteName}_h_files
    ../../inc/ucontract.h
)

build_c_test_artifacts(${thisTestSuiteName} ON "tests/ucontract_tests")

compile_c_test_artifacts_as(${thisTestSuiteName} C99)
