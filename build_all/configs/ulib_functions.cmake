#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#Macro to set C and Cpp standards for a given target
macro(ulib_add_standards)
    set(CMAKE_C_STANDARD                99)
    set(CMAKE_C_STANDARD_REQUIRED       OFF)
    set(CMAKE_C_EXTENSIONS              OFF)

    set(CMAKE_CXX_STANDARD              11)
    set(CMAKE_CXX_STANDARD_REQUIRED     OFF)
    set(CMAKE_CXX_EXTENSIONS            OFF)
endmacro()

#Add windows unittest dll
function(ulib_c_windows_unittests_add_dll what_is_building folder)

    link_directories(${what_is_building}_dll $ENV{VCInstallDir}UnitTest/lib)

    #Add static library target
    add_library(${what_is_building}_testsonly_lib STATIC
            ${${what_is_building}_test_files}
    )

    #Set the output folder for the target
    set_target_properties(${what_is_building}_testsonly_lib
               PROPERTIES
               FOLDER ${folder} )

    target_include_directories(${what_is_building}_testsonly_lib PUBLIC ${sharedutil_include_directories} $ENV{VCInstallDir}UnitTest/include)
    target_compile_definitions(${what_is_building}_testsonly_lib PUBLIC CPP_UNITTEST)
    target_compile_options(${what_is_building}_testsonly_lib PUBLIC /TP /EHsc)

    #Add shared/dynamic libary target
    add_library(${what_is_building}_dll SHARED
        ${${what_is_building}_cpp_files}
        ${${what_is_building}_c_files}
    )

    #Set the output folder for the target
    set_target_properties(${what_is_building}_dll
               PROPERTIES
               FOLDER ${folder})

    #Mark C source files as "C source code"
    set_source_files_properties(${${what_is_building}_c_files}
               PROPERTIES
               COMPILE_FLAGS /TC)

    #Mark Cpp source files as "C++ source code"
    set_source_files_properties(${${what_is_building}_cpp_files}
               PROPERTIES
               COMPILE_FLAGS /TP)

    #Link necessary libs to dll library
    target_link_libraries(${what_is_building}_dll  
                PRIVATE umock_c ctest testrunnerswitcher ${what_is_building}_testsonly_lib
    )

endfunction()

#Build Linux unit tests with valgrind if enabled
function(ulib_c_linux_valgrind target_name)

    #Setup valgrind if applicable and add separate test for valgrind/helgrind
    if(${run_valgrind})
        find_program(VALGRIND_FOUND NAMES valgrind)
        if(${VALGRIND_FOUND} STREQUAL VALGRIND_FOUND-NOTFOUND)
            message(WARNING "run_valgrind was TRUE, but valgrind was not found - there will be no tests run under valgrind")
        else()
            add_test(NAME ${target_name}_valgrind COMMAND valgrind                 --gen-suppressions=all --num-callers=100 --error-exitcode=1 --leak-check=full --track-origins=yes $<TARGET_FILE:${target_name}>)
            add_test(NAME ${target_name}_helgrind COMMAND valgrind --tool=helgrind --gen-suppressions=all --num-callers=100 --error-exitcode=1 $<TARGET_FILE:${target_name}>)
            add_test(NAME ${target_name}_drd      COMMAND valgrind --tool=drd      --gen-suppressions=all --num-callers=100 --error-exitcode=1 $<TARGET_FILE:${target_name}>)
        endif()
    endif()

endfunction()

function(ulib_populate_test_target target_name)

    #Include repo directories
    target_sources(${target_name}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/tests/src/ustream_mock_buffer.c
            ${PROJECT_SOURCE_DIR}/tests/src/${ULIB_PAL_OS_DIRECTORY}/az_ulib_test_thread.c
    )

    target_include_directories(${target_name}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/inc
            ${PROJECT_SOURCE_DIR}/config
            ${PROJECT_SOURCE_DIR}/tests/inc
            ${PROJECT_SOURCE_DIR}/pal/${ULIB_PAL_DIRECTORY}
            ${PROJECT_SOURCE_DIR}/pal/os/inc
            ${PROJECT_SOURCE_DIR}/pal/os/inc/${ULIB_PAL_OS_DIRECTORY}
    )

    target_link_libraries(${target_name}
        PRIVATE
            azure_ulib_c
            testrunnerswitcher
            ctest
            umock_c
            azure_macro_utils_c
            $<$<STREQUAL:"${ULIB_PAL_OS_DIRECTORY}","linux">:pthread>
    )

    target_compile_definitions(${target_name}
        PUBLIC
            USE_CTEST
    )

    set_target_properties(${target_name}
        PROPERTIES
            FOLDER "uLib Tests"
    )

    #Create Windows/Linux test executables
    if(WIN32)
        if (${use_cppunittest})
            # ulib_c_windows_unittests_add_dll(${target_name} ${folder} ${ARGN})
        endif()
    else()
        ulib_c_linux_valgrind(${target_name})
    endif()

    add_test(NAME ${target_name} COMMAND ${target_name})

endfunction()

#Build sample
function(ulib_populate_sample_target target_name)

    #Define executable's compile definitions, linked libraries, and include dirs
    target_include_directories(${target_name}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/inc
            ${PROJECT_SOURCE_DIR}/config
            ${PROJECT_SOURCE_DIR}/pal/${ULIB_PAL_DIRECTORY}
            ${PROJECT_SOURCE_DIR}/pal/os/inc
            ${PROJECT_SOURCE_DIR}/pal/os/inc/${ULIB_PAL_OS_DIRECTORY}
    )

    target_link_libraries(${target_name}
        PRIVATE
            azure_ulib_c
            azure_macro_utils_c
            umock_c
    )

    set_target_properties(${target_name}
        PROPERTIES
            FOLDER "uLib Samples"
    )

endfunction()
