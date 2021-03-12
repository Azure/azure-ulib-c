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

#Build Linux unit tests with valgrind if enabled
function(ulib_c_linux_valgrind target_name)

    #Setup valgrind if applicable and add separate test for valgrind/helgrind
    if(${RUN_VALGRIND})
        find_program(VALGRIND_FOUND NAMES valgrind)
        if(${VALGRIND_FOUND} STREQUAL VALGRIND_FOUND-NOTFOUND)
            message(WARNING "RUN_VALGRIND was TRUE, but valgrind was not found - there will be no tests run under valgrind")
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
            ${PROJECT_SOURCE_DIR}/tests/src/az_ulib_ustream_mock_buffer.c
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
    )

    set_target_properties(${target_name}
        PROPERTIES
            FOLDER "uLib Samples"
    )

endfunction()
