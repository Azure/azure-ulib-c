# Azure uLib C - PREVIEW

[![Build Status](https://azure-iot-sdks.visualstudio.com/azure-iot-stuben/_apis/build/status/azure-iot-ulib/azure-iot-ulib-build?branchName=master)](https://azure-iot-sdks.visualstudio.com/azure-iot-stuben/_build/latest?definitionId=176&branchName=master)

**THIS REPO IS CURRENTLY IN PREVIEW. THE API'S ARE NOT FINAL AND ARE SUBJECT TO CHANGE WITHOUT NOTICE.**

The intention of this repository is to provide developers useful tools to make developing for small, limited-resource devices easier.

This repository contains the following:

* [**uStream**](https://azure.github.io/azure-ulib-c/ustream__base_8h.html): avoid copying memory and expose large amounts of data without using a large amount of memory. Samples can be found [here](https://github.com/Azure/azure-ulib-c/tree/master/samples).
* [**uLog**](https://azure.github.io/azure-ulib-c/ulog_8h.html): a small logging system with constant strings for reduced memory footprint.

## Reference

Documentation for API's are automatically generated with [doxygen](http://www.doxygen.nl/). You can find the generated documentation
on the [Github Page](https://azure.github.io/azure-ulib-c) for this repo.

## Development Machine Requirements

This repo is built using cmake. The minimum requirement is version 3.10. You can download and install the latest cmake version [here](https://cmake.org/).

## Building the Repo

1. Clone the repo into the directory of your choosing with the following command

    ```bash
    git clone --recursive https://github.com/Azure/azure-ulib-c
    ```

2. From here you can either use build scripts we have provided in `build_all\` or elect to build by yourself using the commands detailed in the following Command Line section.

### Scripts

1. Find the script for the OS on your dev machine and run it. By default, the tests will be compiled and then run at the end of the script.

### Command Line

1. Create the cmake directory to put all of your build files:

    ```bash
    mkdir cmake
    ```

2. cd into the cmake directory:

    ```bash
    cd cmake
    ```

3. Run cmake. Here you have the option of building the tests as well. If you would like to build the tests, run:

    ```bash
    cmake .. -Drun_ulib_unit_tests:BOOL=ON -Drun_ulib_e2e_tests:BOOL=ON
    ```

    Otherwise, to just build the source files, run:

    ```bash
    cmake ..
    ```

4. Build with cmake

    ```bash
    cmake --build .
    ```

5. If you would like to run the tests and added the option from step 3, you can run the following:

    ```bash
    ctest -C "debug" -V
    ```

    The -C option chooses the build configuration to test and the -V turns on verbose output from the tests.

## Creating Your Own uStream

Using the specification listed [here](https://azure.github.io/azure-ulib-c/ustream__base_8h.html) in the
documentation, you can create your own implementation of a ustream. In order to ensure that it behaves
according to the specification, we have created tests which you can run against your implementation. 
You can use `ustream_ut_sample.c` and `ustream_e2e_sample.c` in [tests_sample](tests/tests_sample) as a
reference. `TODO`'s have been placed in the sample files to help guide what you need to implement.

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit [Microsoft CLA](https://cla.microsoft.com).

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
