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

        git clone https://github.com/Azure/azure-ulib-c --recursive


3. Ensure the uLib builds correctly.

   - Create an output directory for your build artifacts (in this example, we named it `build`, but you can pick any name).
        ```bash
          mkdir build
        ```
   - Navigate to that newly created directory.
        ```bash
          cd build
        ```
   - Run `cmake` pointing to the sources at the root of the repo to generate the builds files.
        ```bash
          cmake ..
        ```
   - Launch the underlying build system to compile the libraries.
        ```bash
          cmake --build .
        ```
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

For example:
  - to build uLib with samples and test

    ```bash
      cmake .. -DUNIT_TESTING:BOOL=ON
    ```

  - to build clean uLib

    ```bash
      cmake .. -DSKIP_SAMPLES:BOOL=ON -DPRECONDITIONS:BOOL=OFF
    ```

### Development Environment

Project contains files to work on Windows, Mac or Linux based OS.

**Note** For any environment variables set to use with CMake, the environment variables must be set
BEFORE the first cmake generation command (`cmake ..`). The environment variables will NOT be picked up
if you have already generated the build files, set environment variables, and then regenerate. In that
case, you must either delete the `CMakeCache.txt` file or delete the folder in which you are generating build
files and start again.

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
