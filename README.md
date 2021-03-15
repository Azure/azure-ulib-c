# Azure uLib C - PREVIEW

[![Build Status](https://azure-iot-sdks.visualstudio.com/azure-iot-stuben/_apis/build/status/azure-iot-ulib/azure-iot-ulib-build?branchName=master)](https://azure-iot-sdks.visualstudio.com/azure-iot-stuben/_build/latest?definitionId=176&branchName=master)

**THIS REPO IS CURRENTLY IN PREVIEW. THE API'S ARE NOT FINAL AND ARE SUBJECT TO CHANGE WITHOUT NOTICE.**

The intention of this repository is to provide developers useful tools to make developing for small, limited-resource devices easier.

This repository contains the following:

* [**uStream**](https://azure.github.io/azure-ulib-c/ustream__base_8h.html): avoid copying memory and expose large amounts of data without using a large amount of memory. Samples can be found [here](https://github.com/Azure/azure-ulib-c/tree/master/samples).
* [**uLog**](https://azure.github.io/azure-ulib-c/ulog_8h.html): a small logging system with constant strings for reduced memory footprint.

## Table of Contents

- [Azure uLib C](#azure-ulib-c)
  - [Table of Contents](#table-of-contents)
  - [Documentation](#documentation)
  - [The GitHub Repository](#the-gitHub-repository)
    - [Structure](#structure)
    - [Master Branch](#master-branch)
    - [Release Branches and Release Tagging](#release-branches-and-release-tagging)
  - [Getting Started Using the uLib](#getting-started-using-the-ulib)
    - [CMake](#cmake)
    - [CMake Options](#cmake-options)
    - [Development Environment](#development-environment)
    - [Windows](#windows)
    - [Linux](#linux)
    - [Mac](#mac)
  - [Creating Your Own uStream](#creating-your-own-ustream)
  - [Contributing](#contributing)
    - [Reporting Security Issues and Security Bugs](#reporting-security-issues-and-security-bugs)
    - [License](#license)

## Documentation

We use [doxygen](https://www.doxygen.nl) to generate documentation for source code. You can find the generated, versioned documentation [here](https://azure.github.io/azure-ulib-c).

## The GitHub Repository

To get help with the uLib:

- File a [Github Issue](https://github.com/Azure/azure-ulib-c/issues/new/choose).

### Structure

This repo is structured with two priorities:

1. Separation of services/features to make it easier to find relevant information and resources.
2. Simplified source file structuring to easily integrate features into a user's project.

&nbsp;&nbsp;&nbsp;&nbsp;`/docs` - documentation for each service (iot, etc)<br>
&nbsp;&nbsp;&nbsp;&nbsp;`/inc` - include directory - can be singularly included in your project to resolve all headers<br>
&nbsp;&nbsp;&nbsp;&nbsp;`/samples` - samples for each service<br>
&nbsp;&nbsp;&nbsp;&nbsp;`/src` - source files for each service<br>
&nbsp;&nbsp;&nbsp;&nbsp;`/tests` - tests for each service<br>

For instructions on how to consume the libraries via CMake, please see [here](#cmake). For instructions on how consume the source code in an IDE, command line, or other build systems, please see [here](#source-files-ide-command-line-etc).

### Master Branch

The master branch has the most recent code with new features and bug fixes. This repo is under construction, so it does **not** represent the General Availability (**GA**) release of the uLib.

### Release Branches and Release Tagging

When we make an official release, we will create a unique git tag containing the name and version to mark the commit. We'll use this tag for servicing via hotfix branches as well as debugging the code for a particular beta or stable release version.

## Getting Started Using the uLib

The uLib can be conveniently consumed either via CMake or other non-CMake methods (IDE workspaces, command line, and others).

### CMake

1. Install the required prerequisites:
   - [CMake](https://cmake.org/download/) version 3.10 or later
   - C compiler: [MSVC](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019), [gcc](https://gcc.gnu.org/) or [clang](https://clang.llvm.org/) are recommended
   - [git](https://git-scm.com/downloads) to clone our Azure uLib repository with the desired tag

2. Clone our Azure uLib repository, optionally using the desired version tag.

        git clone https://github.com/Azure/azure-ulib-c


3. Ensure the uLib builds correctly.

   - Create an output directory for your build artifacts (in this example, we named it `build`, but you can pick any name).

          mkdir build

   - Navigate to that newly created directory.

          cd build

   - Run `cmake` pointing to the sources at the root of the repo to generate the builds files.

          cmake ..

   - Launch the underlying build system to compile the libraries.

          cmake --build .

   This results in building uLib as a static library file, placed in the output directory you created. It creates a few samples in that you can directly run from the samples directory.

### CMake Options

By default, when building the project with no options, the following static libraries are generated:

- ``Libraries``:
  - az_core
    - az_span, az_http, az_json, etc.
  - azure_ulib_c
    - az_ustream, az_ipc, etc.

The following CMake options are available for adding/removing project features.

<table>
<tr>
<td>Option</td>
<td>Description</td>
<td>Default Value</td>
</tr>
<tr>
<td>UNIT_TESTING</td>
<td>Generates Unit Test for compilation. When turning this option ON, cmocka is a required dependency for compilation.<br>After Compiling, use `ctest` to run Unit Test.</td>
<td>OFF</td>
</tr>
<tr>
<td>PRECONDITIONS</td>
<td>Turning this option OFF would remove all method contracts. This is typically for shipping libraries for production to make it as optimized as possible.</td>
<td>ON</td>
</tr>
<tr>
<td>LOGGING</td>
<td>Build uLib with logging support.</td>
<td>ON</td>
</tr>
<tr>
<td>SKIP_SAMPLES</td>
<td>When turning ON, the compiler will not build the samples.</td>
<td>OFF</td>
</tr>
<tr>
</table>

### Development Environment

Project contains files to work on Windows, Mac or Linux based OS.

**Note** For any environment variables set to use with CMake, the environment variables must be set
BEFORE the first cmake generation command (`cmake ..`). The environment variables will NOT be picked up
if you have already generated the build files, set environment variables, and then regenerate. In that
case, you must either delete the `CMakeCache.txt` file or delete the folder in which you are generating build
files and start again.

### Windows

vcpkg is the easiest way to have dependencies installed. It downloads packages sources, headers and build libraries for whatever TRIPLET is set up (platform/arq).
VCPKG maintains any installed package inside its own folder, allowing to have multiple vcpkg folder with different dependencies installed on each. This is also great because you don't have to install dependencies globally on your system.

Follow next steps to install VCPKG and have it linked to cmake. The vcpkg repository is checked out at the commit in [vcpkg-commit.txt](https://github.com/Azure/azure-sdk-for-c/blob/master/eng/vcpkg-commit.txt). Azure uLib code in this version is known to work at that vcpkg commit.

```bash
# Clone vcpkg:
git clone https://github.com/Microsoft/vcpkg.git
# (consider this path as PATH_TO_VCPKG)
cd vcpkg
# Checkout the vcpkg commit from the vcpkg-commit.txt file (link above)
git checkout <vcpkg commit>

# build vcpkg (remove .bat on Linux/Mac)
.\bootstrap-vcpkg.bat
# install dependencies (remove .exe in Linux/Mac) and update triplet
.\vcpkg.exe install --triplet x64-windows-static cmocka
# Add this environment variables to link this VCPKG folder with cmake:
# VCPKG_DEFAULT_TRIPLET=x64-windows-static
# VCPKG_ROOT=PATH_TO_VCPKG (replace PATH_TO_VCPKG for where vcpkg is installed)
```

> Note: Setting up a development environment in windows without VCPKG is not supported. It requires installing all dev-dependencies globally and manually setting cmake files to link each of them.

Follow next steps to build project from command prompt:

```bash
# cd to project folder
cd azure-ulib-c
# create a new folder to generate cmake files for building (i.e. build)
mkdir build
cd build
# generate files
# cmake will automatically detect what C compiler is used by system by default and will generate files for it
cmake ..
# compile files. Cmake would call compiler and linker to generate libs
cmake --build .
```

> Note: The steps above would compile and generate the default output for azure-ulib-c which includes static libraries and samples. See section [CMake Options](#cmake-options)

#### Visual Studio 2019

Open project folder with Visual Studio. If VCPKG has been previously installed and set up like mentioned [above](#vcpkg). Everything will be ready to build.
Right after opening project, Visual Studio will read cmake files and generate cache files automatically.

### Linux

#### VCPKG

VCPKG can be used to download packages sources, headers and build libraries for whatever TRIPLET is set up (platform/architecture).
VCPKG maintains any installed package inside its own folder, allowing to have multiple vcpkg folder with different dependencies installed on each. This is also great because you don't have to install dependencies globally on your system.

Follow next steps to install VCPKG and have it linked to cmake. The vcpkg repository is checked out at the commit in [vcpkg-commit.txt](https://github.com/Azure/azure-sdk-for-c/blob/master/eng/vcpkg-commit.txt). Azure uLib code in this version is known to work at that vcpkg commit.

```bash
# Clone vcpkg:
git clone https://github.com/Microsoft/vcpkg.git
# (consider this path as PATH_TO_VCPKG)
cd vcpkg
# Checkout the vcpkg commit from the vcpkg-commit.txt file (link above)
git checkout <vcpkg commit>

# build vcpkg
./bootstrap-vcpkg.sh
./vcpkg install --triplet x64-linux cmocka
export VCPKG_DEFAULT_TRIPLET=x64-linux
export VCPKG_ROOT=PATH_TO_VCPKG #replace PATH_TO_VCPKG for where vcpkg is installed
```

#### Build

```bash
# cd to project folder
cd azure-ulib-c
# create a new folder to generate cmake files for building (i.e. build)
mkdir build
cd build
# generate files
# cmake will automatically detect what C compiler is used by system by default and will generate files for it
cmake ..
# compile files. Cmake would call compiler and linker to generate libs
make
```

> Note: The steps above would compile and generate the default output for azure-ulib-c which includes static libraries and samples. See section [CMake Options](#cmake-options)

### Mac

#### VCPKG

VCPKG can be used to download packages sources, headers and build libraries for whatever TRIPLET is set up (platform/architecture).
VCPKG maintains any installed package inside its own folder, allowing to have multiple vcpkg folder with different dependencies installed on each. This is also great because you don't have to install dependencies globally on your system.

First, ensure that you have the latest `gcc` installed:

    brew update
    brew upgrade
    brew info gcc
    brew install gcc
    brew cleanup

Follow next steps to install VCPKG and have it linked to cmake. The vcpkg repository is checked out at the commit in [vcpkg-commit.txt](https://github.com/Azure/azure-sdk-for-c/blob/master/eng/vcpkg-commit.txt). Azure uLib code in this version is known to work at that vcpkg commit.

```bash
# Clone vcpkg:
git clone https://github.com/Microsoft/vcpkg.git
# (consider this path as PATH_TO_VCPKG)
cd vcpkg
# Checkout the vcpkg commit from the vcpkg-commit.txt file (link above)
git checkout <vcpkg commit>

# build vcpkg
./bootstrap-vcpkg.sh
./vcpkg install --triplet x64-osx cmocka
export VCPKG_DEFAULT_TRIPLET=x64-osx
export VCPKG_ROOT=PATH_TO_VCPKG #replace PATH_TO_VCPKG for where vcpkg is installed
```

#### Build

```bash
# cd to project folder
cd azure-ulib-c
# create a new folder to generate cmake files for building (i.e. build)
mkdir build
cd build
# generate files
# cmake will automatically detect what C compiler is used by system by default and will generate files for it
cmake ..
# compile files. Cmake would call compiler and linker to generate libs
make
```

> Note: The steps above would compile and generate the default output for azure-ulib-c which includes static libraries and samples. See section [CMake Options](#cmake-options)

## Creating Your Own uStream

Using the specification listed [here](https://azure.github.io/azure-ulib-c/ustream__base_8h.html) in the
documentation, you can create your own implementation of a ustream. In order to ensure that it behaves
according to the specification, we have created tests which you can run against your implementation. 
You can use `ustream_ut_sample.c` and `ustream_e2e_sample.c` in [tests_sample](tests/tests_sample) as a
reference. `TODO`'s have been placed in the sample files to help guide what you need to implement.

## Contributing

For details on contributing to this repository, see the [contributing guide](https://github.com/Azure/azure-sdk-for-c/blob/master/CONTRIBUTING.md).

This project welcomes contributions and suggestions. Most contributions require you to agree to a Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us the rights to use your contribution. For details, visit [https://cla.microsoft.com](https://cla.microsoft.com).

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions provided by the bot. You will only need to do this once across all repositories using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact
[opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

### Reporting Security Issues and Security Bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

### License

Azure uLib C is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-c/blob/master/LICENSE) license.
