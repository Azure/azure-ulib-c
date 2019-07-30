#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.2)

add_standards()

usePermissiveRulesForSamplesAndTests()

set(this_sample_name http_simple_get)

set(${this_sample_name}_c_files
    ./src/main.c
	../../../src/azulib_http/azulib_http.c
)

set(${this_sample_name}_h_files
    ./inc/azulib_credential_api.h
    ./inc/azulib_http_api.h
    ./inc/azulib_message_api.h
    ./inc/azulib_option_api.h
    ./inc/azulib_transport_api.h
    ./inc/ucontract.h
    ./inc/ulib_result.h
)

build_c_sample_target(${this_sample_name} "samples/http_simple_get")
