#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

function(target_link_libraries_with_arg_prefix arg_prefix what_is_building lib)
    if(${arg_prefix} STREQUAL "debug")
        target_link_libraries(${what_is_building} debug ${lib})
    elseif(${arg_prefix} STREQUAL "optimized")
        target_link_libraries(${what_is_building} optimized ${lib})
    elseif(${arg_prefix} STREQUAL "general")
        target_link_libraries(${what_is_building} general ${lib})
    else()
        target_link_libraries(${what_is_building} ${lib})
    endif()
endfunction()

function(set_test_target_folder what_is_building ext)
    if("${what_is_building}" MATCHES ".*e2e.*")
        set_target_properties(${what_is_building}_${ext}
                   PROPERTIES
                   FOLDER "tests/E2ETests")
    else()
        set_target_properties(${what_is_building}_${ext}
                   PROPERTIES
                   FOLDER "tests/UnitTests")
    endif()
endfunction()

function(windows_unittests_add_dll what_is_building)
    link_directories(${what_is_building}_dll $ENV{VCInstallDir}UnitTest/lib)

    add_library(${what_is_building}_dll SHARED
        ${${what_is_building}_cpp_files}
        ${${what_is_building}_h_files}
        ${${what_is_building}_c_files}
        ${logging_files}
    )

    set_test_target_folder(${what_is_building} "dll")

    set(PARSING_ADDITIONAL_LIBS OFF)
    set(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
    set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER)
    set(ARG_PREFIX "none")
    foreach(f ${ARGN})
        set(skip_to_next FALSE)
        if(${f} STREQUAL "ADDITIONAL_LIBS")
            SET(PARSING_ADDITIONAL_LIBS ON)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
            set(ARG_PREFIX "none")
            #also unset all the other states
            set(skip_to_next TRUE)
        elseif(${f} STREQUAL "VALGRIND_SUPPRESSIONS_FILE")
            SET(PARSING_ADDITIONAL_LIBS OFF)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE ON)
            set(skip_to_next TRUE)
        endif()

        if(NOT skip_to_next)
            if(PARSING_ADDITIONAL_LIBS)
                if((${f} STREQUAL "debug") OR (${f} STREQUAL "optimized") OR (${f} STREQUAL "general"))
                    SET(ARG_PREFIX ${f})
                else()
                    target_link_libraries_with_arg_prefix(${ARG_PREFIX} ${what_is_building}_dll ${f})
                    set(ARG_PREFIX "none")
                endif()
            endif()
        endif()

    endforeach()

    target_include_directories(${what_is_building}_dll PUBLIC ${sharedutil_include_directories} $ENV{VCInstallDir}UnitTest/include)
    target_compile_definitions(${what_is_building}_dll PUBLIC -DCPP_UNITTEST)
    target_link_libraries(${what_is_building}_dll micromock_cpp_unittest umock_c ctest testrunnerswitcher)
endfunction()

function(windows_unittests_add_exe what_is_building)
    add_executable(${what_is_building}_exe
        ${${what_is_building}_cpp_files}
        ${${what_is_building}_h_files}
        ${${what_is_building}_c_files}
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${logging_files}
    )

    set_test_target_folder(${what_is_building} "exe")

    set(PARSING_ADDITIONAL_LIBS OFF)
    set(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
    set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER)
    set(ARG_PREFIX "none")
    foreach(f ${ARGN})
        set(skip_to_next FALSE)
        if(${f} STREQUAL "ADDITIONAL_LIBS")
            SET(PARSING_ADDITIONAL_LIBS ON)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
            set(ARG_PREFIX "none")
            #also unset all the other states
            set(skip_to_next TRUE)
        elseif(${f} STREQUAL "VALGRIND_SUPPRESSIONS_FILE")
            SET(PARSING_ADDITIONAL_LIBS OFF)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE ON)
            set(skip_to_next TRUE)
        endif()

        if(NOT skip_to_next)
            if(PARSING_ADDITIONAL_LIBS)
                if((${f} STREQUAL "debug") OR (${f} STREQUAL "optimized") OR (${f} STREQUAL "general"))
                    SET(ARG_PREFIX ${f})
                else()
                    target_link_libraries_with_arg_prefix(${ARG_PREFIX} ${what_is_building}_exe ${f})
                    set(ARG_PREFIX "none")
                endif()
            endif()
        endif()

    endforeach()

    target_compile_definitions(${what_is_building}_exe PUBLIC -DUSE_CTEST)
    target_include_directories(${what_is_building}_exe PUBLIC ${sharedutil_include_directories})
    target_link_libraries(${what_is_building}_exe micromock_ctest umock_c ctest testrunnerswitcher)
    add_test(NAME ${what_is_building} COMMAND ${what_is_building}_exe)
endfunction()

#this function takes more than the 1 mandatory argument (what_is_building)
#the parameters are separated by "known" separators
#for example, ADDITIONAL_LIBS starts a list of needed libraries
function(linux_unittests_add_exe what_is_building)
    add_executable(${what_is_building}_exe
        ${${what_is_building}_cpp_files}
        ${${what_is_building}_h_files}
        ${${what_is_building}_c_files}
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${logging_files}
    )

    set_test_target_folder(${what_is_building} "exe")

    target_compile_definitions(${what_is_building}_exe PUBLIC -DUSE_CTEST)
    target_include_directories(${what_is_building}_exe PUBLIC ${sharedutil_include_directories})

    #this part detects
    #   - the additional libraries that might be needed.
    #     additional libraries are started by ADDITIONAL_LIBS parameter and ended by any other known parameter (or end of variable arguments)
    #   - a valgrind suppression file (VALGRIND_SUPPRESSIONS_FILE) for memcheck
    #     the file name follows immediately after

    set(PARSING_ADDITIONAL_LIBS OFF)
    set(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
    set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER)
    set(ARG_PREFIX "none")
    foreach(f ${ARGN})
        set(skip_to_next FALSE)
        if(${f} STREQUAL "ADDITIONAL_LIBS")
            SET(PARSING_ADDITIONAL_LIBS ON)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
            set(ARG_PREFIX "none")
            set(skip_to_next TRUE)
            #also unset all the other states

        elseif(${f} STREQUAL "VALGRIND_SUPPRESSIONS_FILE")
            SET(PARSING_ADDITIONAL_LIBS OFF)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE ON)
            set(skip_to_next TRUE)
        else()

        if(NOT skip_to_next)
            if(PARSING_ADDITIONAL_LIBS)
                if((${f} STREQUAL "debug") OR (${f} STREQUAL "optimized") OR (${f} STREQUAL "general"))
                    SET(ARG_PREFIX ${f})
                else()
                    target_link_libraries_with_arg_prefix(${ARG_PREFIX} ${what_is_building}_exe ${f})
                    set(ARG_PREFIX "none")
                endif()
            elseif(PARSING_VALGRIND_SUPPRESSIONS_FILE)
                set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER "--suppressions=${f}")
            endif()
        endif()

    endforeach()

    target_link_libraries(${what_is_building}_exe micromock_ctest umock_c ctest)

    add_test(NAME ${what_is_building} COMMAND $<TARGET_FILE:${what_is_building}_exe>)

    if(${run_valgrind})
        find_program(VALGRIND_FOUND NAMES valgrind)
        if(${VALGRIND_FOUND} STREQUAL VALGRIND_FOUND-NOTFOUND)
            message(WARNING "run_valgrind was TRUE, but valgrind was not found - there will be no tests run under valgrind")
        else()
            add_test(NAME ${what_is_building}_valgrind COMMAND valgrind                 --num-callers=100 --error-exitcode=1 --leak-check=full --track-origins=yes ${VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER} $<TARGET_FILE:${what_is_building}_exe>)
            add_test(NAME ${what_is_building}_helgrind COMMAND valgrind --tool=helgrind --num-callers=100 --error-exitcode=1 ${VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER} $<TARGET_FILE:${what_is_building}_exe>)
            add_test(NAME ${what_is_building}_drd      COMMAND valgrind --tool=drd      --num-callers=100 --error-exitcode=1 ${VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER} $<TARGET_FILE:${what_is_building}_exe>)
        endif()
    endif()
endfunction()

#function(build_test_artifacts what_is_building use_gballoc)
#
    ##the first argument is what is building
    ##the second argument is whether the tests should be build with gballoc #defines or not
    ##the following arguments are a list of libraries to link with
#
    #if(${use_gballoc})
        #add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
    #else()
    #endif()
#
    ##setting #defines
    #if(WIN32)
        #add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    #else()
    #endif()
#
    ##setting includes
    #set(sharedutil_include_directories ${sharedutil_include_directories} ${TESTRUNNERSWITCHER_INCLUDES} ${CTEST_INCLUDES} ${UMOCK_C_INCLUDES} ${AZURE_C_SHARED_UTILITY_INCLUDES})
    #set(sharedutil_include_directories ${sharedutil_include_directories} ${MICROMOCK_INC_FOLDER} ${TESTRUNNERSWITCHER_INC_FOLDER} ${CTEST_INC_FOLDER} ${SAL_INC_FOLDER} ${UMOCK_C_INC_FOLDER} ${UMOCK_C_SRC_FOLDER})
    #if(WIN32)
    #else()
        #include_directories(${sharedutil_include_directories})
    #endif()
#
    ##setting logging_files
    #if(DEFINED UMOCK_C_SRC_FOLDER)
        #set(logging_files ${XLOGGING_C_FILE} ${LOGGING_C_FILE})
    #elseif(DEFINED UMOCK_C_FOLDER)
        #set(logging_files ${XLOGGING_C_FILE} ${LOGGING_C_FILE})
    #else()
        #message(FATAL_ERROR "No Shared Utility folder defined for src.")
    #endif()
#
    ##setting output type
    #if(WIN32)
        #if(
            #(("${what_is_building}" MATCHES ".*ut.*") AND ${run_unittests}) OR
            #(("${what_is_building}" MATCHES ".*e2e.*") AND ${run_e2e_tests}) OR
            #(("${what_is_building}" MATCHES ".*int.*") AND ${run_int_tests})
        #)
                #windows_unittests_add_exe(${what_is_building} ${ARGN})
                #if (${use_cppunittest})
                    #windows_unittests_add_dll(${what_is_building} ${ARGN})
                #endif()
        #else()
            #if(
                #(("${what_is_building}" MATCHES ".*e2e.*") AND ${nuget_e2e_tests})
            #)
                #windows_unittests_add_exe(${what_is_building}_nuget ${ARGN})
            #else()
                ##do nothing
            #endif()
        #endif()
    #else()
        #if(
            #(("${what_is_building}" MATCHES ".*ut.*") AND ${run_unittests}) OR
            #(("${what_is_building}" MATCHES ".*e2e.*") AND ${run_e2e_tests}) OR
            #(("${what_is_building}" MATCHES ".*int.*") AND ${run_int_tests})
        #)
            #linux_unittests_add_exe(${what_is_building} ${ARGN})
        #endif()
    #endif()
#endfunction(build_test_artifacts)


function(c_windows_unittests_add_dll what_is_building folder)
    link_directories(${what_is_building}_dll $ENV{VCInstall_dir}UnitTest/lib)

    add_library(${what_is_building}_testsonly_lib STATIC
            ${${what_is_building}_test_files}
    )
    SET(VAR 1)
    foreach(file ${${what_is_building}_test_files})
        set_source_files_properties(${file} PROPERTIES COMPILE_FLAGS -DCPPUNITTEST_SYMBOL=some_symbol_for_cppunittest_${VAR})
        MATH(EXPR VAR "${VAR}+1")
    endforeach()

    set_target_properties(${what_is_building}_testsonly_lib
               PROPERTIES
               FOLDER ${folder} )

    target_include_directories(${what_is_building}_testsonly_lib PUBLIC ${sharedutil_include_directories} $ENV{VCInstallDir}UnitTest/include)
    target_compile_definitions(${what_is_building}_testsonly_lib PUBLIC -DCPP_UNITTEST)
    target_compile_options(${what_is_building}_testsonly_lib PUBLIC /TP /EHsc)

    add_library(${what_is_building}_dll SHARED
        ${${what_is_building}_cpp_files}
        ${${what_is_building}_h_files}
        ${${what_is_building}_c_files}
        ${logging_files}
    )

    set_target_properties(${what_is_building}_dll
               PROPERTIES
               FOLDER ${folder})

    set_source_files_properties(${${what_is_building}_c_files} ${logging_files}
               PROPERTIES
               COMPILE_FLAGS /TC)

    set_source_files_properties(${${what_is_building}_cpp_files}
               PROPERTIES
               COMPILE_FLAGS /TP)

    target_link_libraries(${what_is_building}_dll umock_c ctest testrunnerswitcher ${what_is_building}_testsonly_lib )

    set(PARSING_ADDITIONAL_LIBS OFF)
    set(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
    set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER)
    set(ARG_PREFIX "none")
    foreach(f ${ARGN})
        set(skip_to_next FALSE)
        if(${f} STREQUAL "ADDITIONAL_LIBS")
            SET(PARSING_ADDITIONAL_LIBS ON)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
            set(ARG_PREFIX "none")
            #also unset all the other states
            set(skip_to_next TRUE)
        elseif(${f} STREQUAL "VALGRIND_SUPPRESSIONS_FILE")
            SET(PARSING_ADDITIONAL_LIBS OFF)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE ON)
            set(skip_to_next TRUE)
        endif()

        if(NOT skip_to_next)
            if(PARSING_ADDITIONAL_LIBS)
                if((${f} STREQUAL "debug") OR (${f} STREQUAL "optimized") OR (${f} STREQUAL "general"))
                    SET(ARG_PREFIX ${f})
                else()
                    target_link_libraries_with_arg_prefix(${ARG_PREFIX} ${what_is_building}_dll ${f})
                    target_link_libraries_with_arg_prefix(${ARG_PREFIX} ${what_is_building}_testsonly_lib ${f})
                    set(ARG_PREFIX "none")
                endif()
            endif()
        endif()

    endforeach()

    SET(SPACES " ")
    SET(VAR 1)
    foreach(file ${${what_is_building}_test_files})
        # for x64 the underscore is not needed
        if (ARCHITECTURE STREQUAL "x86_64" OR ARCHITECTURE STREQUAL "ARM")
            set_property(TARGET ${what_is_building}_dll APPEND_STRING PROPERTY LINK_FLAGS ${SPACES}/INCLUDE:"some_symbol_for_cppunittest_${VAR}")
        else()
            set_property(TARGET ${what_is_building}_dll APPEND_STRING PROPERTY LINK_FLAGS ${SPACES}/INCLUDE:"_some_symbol_for_cppunittest_${VAR}")
        endif()
        MATH(EXPR VAR "${VAR}+1")
    endforeach()
endfunction()

function(c_windows_unittests_add_exe what_is_building folder)
    add_executable(${what_is_building}_exe
        ${${what_is_building}_test_files}
        ${${what_is_building}_cpp_files}
        ${${what_is_building}_h_files}
        ${${what_is_building}_c_files}
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${logging_files}
    )
    set_target_properties(${what_is_building}_exe
               PROPERTIES
               FOLDER ${folder})

    target_compile_definitions(${what_is_building}_exe PUBLIC -DUSE_CTEST)
    target_include_directories(${what_is_building}_exe PUBLIC ${sharedutil_include_directories})
    target_link_libraries(${what_is_building}_exe umock_c ctest testrunnerswitcher)

    set(PARSING_ADDITIONAL_LIBS OFF)
    set(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
    set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER)
    set(ARG_PREFIX "none")
    foreach(f ${ARGN})
        set(skip_to_next FALSE)
        if(${f} STREQUAL "ADDITIONAL_LIBS")
            SET(PARSING_ADDITIONAL_LIBS ON)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
            set(ARG_PREFIX "none")
            #also unset all the other states
            set(skip_to_next TRUE)
        elseif(${f} STREQUAL "VALGRIND_SUPPRESSIONS_FILE")
            SET(PARSING_ADDITIONAL_LIBS OFF)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE ON)
            set(skip_to_next TRUE)
        endif()

        if(NOT skip_to_next)
            if(PARSING_ADDITIONAL_LIBS)
                if((${f} STREQUAL "debug") OR (${f} STREQUAL "optimized") OR (${f} STREQUAL "general"))
                    SET(ARG_PREFIX ${f})
                else()
                    target_link_libraries_with_arg_prefix(${ARG_PREFIX} ${what_is_building}_exe ${f})
                    set(ARG_PREFIX "none")
                endif()
            endif()
        endif()

    endforeach()

    add_test(NAME ${what_is_building} COMMAND ${what_is_building}_exe)
endfunction()

#this function takes more than the 1 mandatory argument (what_is_building)
#the parameters are separated by "known" separators
#for example, ADDITIONAL_LIBS starts a list of needed libraries
function(c_linux_unittests_add_exe what_is_building folder)

    add_executable(${what_is_building}_exe
        ${${what_is_building}_test_files}
        ${${what_is_building}_cpp_files}
        ${${what_is_building}_h_files}
        ${${what_is_building}_c_files}
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${logging_files}
    )
    set_target_properties(${what_is_building}_exe
               PROPERTIES
               FOLDER ${folder})

    target_compile_definitions(${what_is_building}_exe PUBLIC -DUSE_CTEST)
    target_include_directories(${what_is_building}_exe PUBLIC ${sharedutil_include_directories})

    #this part detects
    #       - the additional libraries that might be needed.
    #     additional libraries are started by ADDITIONAL_LIBS parameter and ended by any other known parameter (or end of variable arguments)
    #   - a valgrind suppression file (VALGRIND_SUPPRESSIONS_FILE) for memcheck
    #     the file name follows immediately after

    set(PARSING_ADDITIONAL_LIBS OFF)
    set(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
    set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER)
    set(ARG_PREFIX "none")
    foreach(f ${ARGN})
        set(skip_to_next FALSE)
        if(${f} STREQUAL "ADDITIONAL_LIBS")
            SET(PARSING_ADDITIONAL_LIBS ON)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE OFF)
            set(ARG_PREFIX "none")
            #also unset all the other states
            set(skip_to_next TRUE)
        elseif(${f} STREQUAL "VALGRIND_SUPPRESSIONS_FILE")
            SET(PARSING_ADDITIONAL_LIBS OFF)
            SET(PARSING_VALGRIND_SUPPRESSIONS_FILE ON)
            set(skip_to_next TRUE)
        endif()

        if(NOT skip_to_next)
            if(PARSING_ADDITIONAL_LIBS)
                if((${f} STREQUAL "debug") OR (${f} STREQUAL "optimized") OR (${f} STREQUAL "general"))
                    SET(ARG_PREFIX ${f})
                else()
                    target_link_libraries_with_arg_prefix(${ARG_PREFIX} ${what_is_building}_exe ${f})
                    set(ARG_PREFIX "none")
                endif()
            elseif(PARSING_VALGRIND_SUPPRESSIONS_FILE)
                set(VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER "--suppressions=${f}")
            endif()
        endif()

    endforeach()

    target_link_libraries(${what_is_building}_exe umock_c ctest m)

    add_test(NAME ${what_is_building} COMMAND $<TARGET_FILE:${what_is_building}_exe>)

    if(${run_valgrind})
        find_program(VALGRIND_FOUND NAMES valgrind)
        if(${VALGRIND_FOUND} STREQUAL VALGRIND_FOUND-NOTFOUND)
            message(WARNING "run_valgrind was TRUE, but valgrind was not found - there will be no tests run under valgrind")
        else()
            add_test(NAME ${what_is_building}_valgrind COMMAND valgrind                 --gen-suppressions=all --num-callers=100 --error-exitcode=1 --leak-check=full --track-origins=yes ${VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER} $<TARGET_FILE:${what_is_building}_exe>)
            add_test(NAME ${what_is_building}_helgrind COMMAND valgrind --tool=helgrind --gen-suppressions=all --num-callers=100 --error-exitcode=1 ${VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER} $<TARGET_FILE:${what_is_building}_exe>)
            add_test(NAME ${what_is_building}_drd      COMMAND valgrind --tool=drd      --gen-suppressions=all --num-callers=100 --error-exitcode=1 ${VALGRIND_SUPPRESSIONS_FILE_EXTRA_PARAMETER} $<TARGET_FILE:${what_is_building}_exe>)
        endif()
    endif()
endfunction()

function(build_c_test_artifacts what_is_building use_gballoc folder)

    #the first argument is what is building
    #the second argument is whether the tests should be build with gballoc #defines or not
    #the following arguments are a list of libraries to link with

    if(${use_gballoc})
        add_definitions(-DGB_MEASURE_MEMORY_FOR_THIS -DGB_DEBUG_ALLOC)
    else()
    endif()

    #setting #defines
    if(WIN32)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    else()
    endif()

    #setting includes
    set(sharedutil_include_directories ${sharedutil_include_directories} ${TESTRUNNERSWITCHER_INCLUDES} ${CTEST_INCLUDES} ${UMOCK_C_INCLUDES} )
    set(sharedutil_include_directories ${sharedutil_include_directories} ${MICROMOCK_INC_FOLDER} ${UMOCK_C_INC_FOLDER} ${TESTRUNNERSWITCHER_INC_FOLDER} ${CTEST_INC_FOLDER})
    if(WIN32)
    else()
        include_directories(${sharedutil_include_directories})
    endif()

    #setting logging_files
    if(DEFINED UMOCK_C_SRC_FOLDER)
        set(logging_files ${XLOGGING_C_FILE} ${LOGGING_C_FILE})
    elseif(DEFINED UMOCK_C_FOLDER)
        set(logging_files ${XLOGGING_C_FILE} ${LOGGING_C_FILE})
    else()
        message(FATAL_ERROR "No Shared Utility folder defined for includes/src.")
    endif()

    #setting output type
    if(WIN32)
        if(
            (("${what_is_building}" MATCHES ".*ut.*") AND ${run_unittests}) OR
            (("${what_is_building}" MATCHES ".*e2e.*") AND ${run_e2e_tests}) OR
            (("${what_is_building}" MATCHES ".*int.*") AND ${run_int_tests})
        )
            if (${use_cppunittest})
                c_windows_unittests_add_dll(${what_is_building} ${folder} ${ARGN})
            endif()
            c_windows_unittests_add_exe(${what_is_building} ${folder} ${ARGN})
        else()
            if(
                (("${what_is_building}" MATCHES ".*e2e.*") AND ${nuget_e2e_tests})
            )
                c_windows_unittests_add_exe(${what_is_building}_nuget ${folder} ${ARGN})
            else()
                #do nothing
            endif()
        endif()
    else()
        if(
            (("${what_is_building}" MATCHES ".*ut.*") AND ${run_unittests}) OR
            (("${what_is_building}" MATCHES ".*e2e.*") AND ${run_e2e_tests}) OR
            (("${what_is_building}" MATCHES ".*int.*") AND ${run_int_tests})
        )
            c_linux_unittests_add_exe(${what_is_building} ${folder} ${ARGN})
        endif()
    endif()
endfunction()

function(compile_c_test_artifacts_as what_is_building compile_as_what)

    if(WIN32)
        if(
            (("${what_is_building}" MATCHES ".*ut.*") AND ${run_unittests}) OR
            (("${what_is_building}" MATCHES ".*e2e.*") AND ${run_e2e_tests}) OR
            (("${what_is_building}" MATCHES ".*int.*") AND ${run_int_tests})
        )
            if (${use_cppunittest})
                if(${compile_as_what} STREQUAL "C99")
                    compile_target_as_c99(${what_is_building}_dll)
                    compile_target_as_c99(${what_is_building}_testsonly_lib)
                endif()
                if(${compile_as_what} STREQUAL "C11")
                    compile_target_as_c11(${what_is_building}_dll)
                    compile_target_as_c11(${what_is_building}_testsonly_lib)
                endif()
            endif()
            if(${compile_as_what} STREQUAL "C99")
                compile_target_as_c99(${what_is_building}_exe)
            endif()
            if(${compile_as_what} STREQUAL "C11")
                compile_target_as_c11(${what_is_building}_exe)
            endif()
        else()
            if(
                (("${what_is_building}" MATCHES ".*e2e.*") AND ${nuget_e2e_tests})
            )
                if(${compile_as_what} STREQUAL "C99")
                    compile_target_as_c99(${what_is_building}_exe)
                endif()
                if(${compile_as_what} STREQUAL "C11")
                    compile_target_as_c11(${what_is_building}_exe)
                endif()
            else()
                #do nothing
            endif()
        endif()
    else()
        if(
            (("${what_is_building}" MATCHES ".*ut.*") AND ${run_unittests}) OR
            (("${what_is_building}" MATCHES ".*e2e.*") AND ${run_e2e_tests}) OR
            (("${what_is_building}" MATCHES ".*int.*") AND ${run_int_tests})
        )
            if(${compile_as_what} STREQUAL "C99")
                compile_target_as_c99(${what_is_building}_exe)
            endif()
            if(${compile_as_what} STREQUAL "C11")
                compile_target_as_c11(${what_is_building}_exe)
            endif()
        endif()
    endif()
endfunction()
