#!/bin/bash
# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.
#

set -e

script_dir=$(cd "$(dirname "$0")" && pwd)
build_root=$(cd "${script_dir}/.." && pwd)
log_dir=$build_root
make_install=
build_folder=$build_root"/cmake/azure_ulib_c"
run_ut=ON

if [ -n "$1" ]; then
    if [ "$1" == "--disable-ut" ]; then
        run_ut=OFF
    fi
fi

rm -r -f $build_folder
mkdir -p $build_folder
pushd $build_folder
cmake ../.. -Drun_ulib_unit_tests:bool=$run_ut -Drun_valgrind:BOOL=ON
cmake --build . -- --jobs=$(nproc)
ctest -C "debug" -V

popd
:
