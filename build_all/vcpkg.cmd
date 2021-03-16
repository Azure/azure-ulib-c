@REM Copyright (c) Microsoft. All rights reserved.
@REM Licensed under the MIT license. See LICENSE file in the project root for full license information.

@REM Add this environment variables to link this VCPKG folder with cmocka:
set VCPKG_DEFAULT_TRIPLET=x64-windows-static
set VCPKG_ROOT=%~dp0..\vcpkg

@setlocal EnableExtensions EnableDelayedExpansion
@echo off

@REM Clone vcpkg:
git clone https://github.com/Microsoft/vcpkg.git %VCPKG_ROOT%

@REM (consider this path as PATH_TO_VCPKG)
cd %VCPKG_ROOT%

@REM Checkout the vcpkg commit from the vcpkg-commit.txt file (link above)
git checkout tags/2020.06

@REM build vcpkg (remove .bat on Linux/Mac)
call .\bootstrap-vcpkg.bat

@REM install dependencies (remove .exe in Linux/Mac) and update triplet
.\vcpkg.exe install --triplet %VCPKG_DEFAULT_TRIPLET% cmocka
