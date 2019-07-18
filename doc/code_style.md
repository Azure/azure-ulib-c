# Azure MCU C code style convention

## Overview
This document aims at capturing the engineering practices used in the Azure MCU team. It should be a reference for the team members, and it can serve as a ramp up tool on the team practices. It is important that all developers and contributors follow the same code style to make the code more readable, and maintainable.

## References
1.    [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
2.    [ISO/IEC 9899 (c99)](http://www.open-std.org/jtc1/sc22/WG14/www/docs/n1256.pdf)
3.    [Security Development Lifecycle (SDL) Banned Function Calls](https://docs.microsoft.com/en-us/previous-versions/bb288454(v=msdn.10))
4.    [Doxygen](http://www.doxygen.nl/index.html)
5.    [Coding Horror](https://blog.codinghorror.com/new-programming-jargon/)
6.    [Best practices for cloud applications](https://docs.microsoft.com/en-us/azure/architecture/best-practices/api-design)

## Updating this document
In order to improve our engineering process, the following steps should be followed when something needs to be changed in this document:
* The change should be proposed as a PR.
* If generally agreed, and no blocking vote, then the change should be incorporated, and a new version published.

## Directory tree organization
The repo should follow the structure:

```
repo_name
    build_all
    config
        repo_name_config.h
    deps
        dep_1
        dep_2
        dep_n
    inc
        internal
    pal
    sample
        sample_1
        sample_2
        sample_n
    tests
        inc
        src
        lib_1_ut
        lib_2_ut
        lib_n_ut
        test_a_e2e
        test_b_e2e
        test_c_lh
        test_d_lh
    src
        lib_1
        lib_2
        lib_n
    LICENSE
    README.md
```

Where:
* **build_all**: Contains the scripts to build the project.
* **config**: Contains the user configuration, it contains a default config that the user should copy and modify to create its own set of configurations.
* **deps**: Contains the repo dependencies, normally github submodules.
* **inc**: Contains the includes of the repo, all files on the root are the public API and should be part of the LTS (Long Term Support). The files in the `internal` are the ones that contains the signature of methods and data structs that are not part of the public API and Microsoft can change it without any previous note. The `internal` should not be part of the include path, include an internal file should always contains the internal path in the name, (ex: `#include “internal/bar_non_public.h”`).
* **pal**: Contains the files with the port specific code.
* **sample**: Contains samples and tutorials that users will use as reference to learn how to use the code in the repo.
* **tests**: Contains the files with the tests for the libs (target code in the repo), each lib must have **ut** (unit test), it is very recommend to add **e2e** (end to end test), and **lh** (longhaul test) as well.
* **LICENSE**: Is the file that describe the license of the code in the repo.
* **README.md**: Is the file with the generic description of the repo, it can contain links for the lib documentation

## Naming standard
As a standard, all names on C should be a composition of the same name in C++, that will concatenate `namespace`, `class_name`, and `function_name`, exceptions may happen if the meaning of the name is different in C and C++. So, the C++ code:
```cpp
namespace aziot
{
    class credential
    {
    public:
        credential();
        ~credential();
        void do_something(void);
    };
}
```

will result in the C code:
```c
void aziot_credential_init(AZIOT_CREDENTIAL* credential)
{
    // constructor implementation.
}

void aziot_credential_deinit(AZIOT_CREDENTIAL* credential)
{
    // destructor implementation.
}

void aziot_credential_do_something(AZIOT_CREDENTIAL* credential)
{
    // do something implementation.
}
```

Each team in Microsoft should define its own namespace, and use it on all of its naming to avoid naming conflict. The usage of this naming will be detailed below on this document.

## Filename
All filenames should be full lowercase to avoid casing issues across platforms.

A source (`.c`/`.cpp`) filename should be made of the namespace, followed by the class name. 
* *Example*: `aziot_credential.c`
 
Words in filenames should be separated by `_`. 

The public API should be clearly identified with the namespace followed by the class name and the name `api` 
* *Example*: `aziot_credential_api.h`

An include (`.h`) that is not part of the public API should be placed in the `internal` directory, and shall not contains the namespace on the name. The path `internal` should not be included as standard include directory.
* *Example*: `internal/credential.h`

The file name should use characters `a` to `z`, `0` to `9`, `_` (underscore) as name separator, and one `.` before the file extension. All other characters should be avoided. 

Filenames should be concise enough, but they should convey the information of what the file contains. 

**Bad**: `ih.c`, `the_iot_hub_client_thingie.c` 

**Good**: `aziot_memorymanager_byte_malloc.c`

## General C conventions
The following apply to C language:
* The code should follow the C99 standard [[3]](http://www.open-std.org/jtc1/sc22/WG14/www/docs/n1256.pdf). 
* All public API should validate all arguments, these includes static functions that are passed for the purpose of callbacks to other modules. If, for performance reasons, we need a flavor without argument validation, we should have two versions: one with argument validation and another without it.
* No static function should ever have to verify the validity of its arguments. Except static functions used for callbacks, as mentioned in the previous item.
* If a function has a return value that can indicate a failure, it should be checked and logerror'd (including static functions). The return value should not be ignored.
* Variables should be defined as close as possible to the point where they are actually used.
* Variables that are intended to hold the function’s return value should not be initialized (does not predict a return value or set a default value), so that the compiler can warn about paths where the return value is not set.
* Returns in the middle of the function should be avoided. Each function should contain only one return at the end of the function. Returns are allowed at the very beginning of the function to validate the arguments, but no code can be placed before this validation (use CONTRACT macros for that). 
* Global variables should be strongly avoided.
* No shared responsibility about setting variables.	 Example:
    ```c
    INT a = 6;
    some_function(&a);     /* shared responsibility of setting "a" between */
                           /*     main function and some_function*/
    ```
* Functions should write to out arguments only when they succeed.
* All data conversion should be explicitly identified by a cast operator. Avoid any implicit cast. 
* Avoid misuse of types, for example, using an integer as a Boolean. 
* Avoid casting `const` (constant) variable to not `const` variable. 
* Make single responsibility per function. Functions should be small to fulfill only one responsibility. A function with a few hundred lines of code usually implies multiple responsibilities.

## Minor style C convention
* Single brackets per line, no missing brackets, no “Egyptian” brackets [[5]](https://blog.codinghorror.com/new-programming-jargon/).
* Use 4 spaces instead of tab.
* Keep lines small. Limit the number of characters per line. In some tools, including Github, it is hard to visualize the full content of lines with 100+ characters.
* Avoid “voidly type”. Create strong type to represent entities instead of using `void*`.
* Avoid “Baklava code” [[5]](https://blog.codinghorror.com/new-programming-jargon/). Avoid useless layers on the design.
* Only expose what is really necessary in the public API. It is difficult to remove or change a public API, so we should make clear the limits between the public API and the internal implementation details or structures.

## Copyright
All files should start with the copyright information:

```c
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
```

## Constants and macros

Macros should be avoided wherever possible; C functions should be used instead to help the compiler help detect defects earlier.

Magic numbers should be avoided. All constants should be defined using C macros (`#define`).

Magic strings should be avoided. Strings can be defined as macros (`#define`) or constants (`static const char[]`).

Constants should be defined at the beginning of the file, and the name should be full uppercase with underline separator. Unless other order makes more sense, they should also be sorted alphabetically for ease of reading when number of items increases:
```c
#define MAX_QUEUE_SIZE 256f
static const char PROGRAM_FILE_FORMAT[] = "c:/program_file/%s_%s";
```
Macros should be named in full uppercase with underline separators:
```c
#define COUNT_OF(a) (sizeof(a) / sizeof((a)[0]))
```

## Function naming

All functions should use full lowercase name for the functions. The words should be separated by `_`. 
```c
int64_t bar_add(int32_t a, int32_t b);
```
All functions in the public API should start with the namespace followed by the class name and the function name. The definition of the function should be placed in the *xx_api.h* file of the module (for the below example, the *aziot_barclass_api.h*). 
```c
AZIOT_RESULT aziot_barclass_init(AZIOT_BARCLASS* my_barclass, 
        const char* const name, uint32_t entry, void* buffer_start, 
        size_t buffer_size, uint8_t priority);
```
A public API that should be implemented in multiple flavors, should have the flavors with the same name of the public API preceded by `_`. For the flavor without argument validation, the name should be the same of the public API followed by `_no_test`. For example, the two flavors of `aziot_credential_create_from_connection_string` should be: 
```c
/**
 * @brief   Create a credential component from connection string.
 */
static inline AZ_IOT_ULIB_RESULT aziot_credential_create_from_connection_string(
    AZIOT_CREDENTIAL* credential,
    const char* const connection_string)
{
#ifdef AZIOT_CREDENTIAL_VALIDATE_ARGUMENTS
    return _aziot_credential_create_from_connection_string(
        credential,
        connection_string);
#else
    return _aziot_credential_create_from_connection_string_no_test(
        credential,
        connection_string);
#endif // AZIOT_CREDENTIAL_VALIDATE_ARGUMENTS
}
```
All functions that are not `static` but not part of the public API should start with `_` followed by the namespace, the class name, and the function name. The definition of the function should be placed in the header file of the module, inside of the `internal` directory. **These function should be strongly avoided**. 

All functions whose purpose is only inside of the source code (`.c`/`.cpp`) should be set as `static`. The static function should contain only the function name, without any prefix. For example:
```c
static int64_t sum(int32_t a, int32_t b)
{
    return (int64_t)a + (int64_t)b;
}
```

## Variable naming
All variables should use full lowercase name. The words should be separated by `_`.
Global variables should start with `g_` followed by the namespace, and the variable name. **Global variables should be strongly avoided**. 
```c
uint32_t g_aziot_byte_counter;
```
All variables that are not `static` but not part of the public API should start with `_` followed by the namespace, the class name, and the variable name. The definition of the variable should be placed in the header file of the module, inside of the `internal` directory. **These variable should be strongly avoided**. 
```c
uint32_t _aziot_bar_byte_counter;
```
All variables whose propose is only inside of the source code (`.c`/`.cpp`) should be set as `static`. The static variable should not have any prefix, only the variable name.
```c
static uint32_t byte_counter = 10;
```
Arguments and local variable names should use a fully lowercase name separated by `_`, without any prefix.
```c
static AZIOT_BAR_RESULT call_procedure(
    const char* const name, 
    const void* const procedure_model_in, 
    const void* procedure_model_out);
{
    uint32_t byte_counter = 10;
    ...
}
```

## Structures
Structures should be defined with typedef, the structure name should be uppercase with `_` separator, 

### *Internal structures*
Internal structures should be placed in the source code file (`.c`, `.cpp`) and may contains only the structure name, and end with `_TAG`, the same name should be used as the type name, but without the `_TAG`.
```c
typedef struct HEADER_DATA_TAG
{
    AZIOT_BAR_DEVICE* bar_device;
    AZIOT_BAR_SCHEMA_MODEL bar_schema_model;
    size_t size;
    uint8_t* data;
} HEADER_DATA;
```
### *Internal structure with public size*
To avoid memory allocation, `C` APIs may require pre-allocated memory for some internal structures. To do that, the caller shall know the size of the structure. So, this is a privet structure with public size.

As any privet structure, its definition should be placed in the source code file (`.c`, `.cpp`). For example, the `AZIOT_HTTP` may be have its own control block defined in the *aziot_http.c* file as: 
```c
typedef struct HTTP_CONTROL_BLOCK_TAG
{
    AZIOT_TRANSPORT_ENDPOINT* endpoint;
    AZIOT_TRANSPORT_STATE logic_state;
    AZIOT_CREDENTIAL* credential;
    AZIOT_I_TRANSPORT_CLIENT* clients[AZIOT_ULIB_CONFIG_HTTP_MAX_CLIENT];
    ...
} HTTP_CONTROL_BLOCK;
```
To create a strong type with the correct size, a dummy structure will be placed in the API header (`.h`), the public name in the header should start with the namespace followed by the class name, and the type name, the content of the public structure should contains a `dummy` array with the size of the internal structure. For the above example, the following code should be in the *aziot_http_api.h*.
```c
#define AZIOT_HTTP_CONTROL_BLOCK_SIZE    \
( \
    sizeof(void*) + \
    sizeof(AZIOT_TRANSPORT_STATE) + \
    sizeof(void*) + \
    (sizeof(void*) * AZIOT_ULIB_CONFIG_HTTP_MAX_CLIENT) \
    ... \
)

typedef struct AZIOT_HTTP_TAG
{
    uint8_t dummy[AZIOT_HTTP_CONTROL_BLOCK_SIZE];
} AZIOT_HTTP;
```
To make sure that the internal structure fits the dummy external structure, on the source code, immediately after the structure definition, a check should be added. A simple way to check the structure size on compilation time may be created with the following macro:
```c
#define AZIOT_CONTRACT_REQUEST_STRUCT_SIZE(tag, size) \
    typedef char MU_C2(tag, _check_size)[(sizeof(tag) == MU_C1(size) ? 1 : -1)]
```
Both `MU_C1` and `MU_C2` came from *azure_macro_utils/macro_utils.h*. On the above example, in the *aziot_http.c*, we can check the `HTTP_CONTROL_BLOCK_TAG` size with the following code:
```c
// Check if the size of the dummy AZ_HTTP fits the HTTP.
AZIOT_CONTRACT_REQUEST_STRUCT_SIZE(HTTP_CONTROL_BLOCK, AZIOT_HTTP_CONTROL_BLOCK_SIZE);
```
User shall use the public definition of the structure to create it, and pass to the public API that should cast to the internal definition to access the internal fields.

### *Public structures*
Public structures name should start with the namespace followed by the class name, the type name and `_TAG`. The type shall have the same name,but without the `_TAG`. The structure shall be defined in the API header, for example, an aziot http class expose a data structure with the connection statistics, in this case, the following definition should be in *aziot_http_api.h*:
```c
typedef struct AZIOT_HTTP_STATISTICS_TAG
{
    uint32_t average_tx_message_per_second;
    uint32_t average_rx_message_per_second;
    uint32_t average_tx_message_size;
    uint32_t average_rx_message_size;
    ...
} AZIOT_HTTP_STATISTICS;
```

## Others
### *Callbacks*
Functions that request a callback with a context should ordering the callback first and than the callback context. For example:
```c
AZIOT_RESULT aziot_client_send_async(AZIOT_CLIENT* client, 
        AZIOT_MESSAGE* message, AZIOT_RELEASE_CALLBACK release_message, 
        AZIOT_CLIENT_RESULT_CALLBACK callback, AZIOT_CLIENT_RESULT_CONTEXT context)
```

All callbacks that has context should receive the context as the first argument. For example:
```c
static void on_client_result(AZIOT_CLIENT_RESULT_CONTEXT context, 
        AZIOT_RESULT result, AZIOT_MESSAGE_RECEIVED message)
```

### *Operator `this`*
The handle of an object, which is equivalent to the `this` pointer in the C++, should be the first argument in all the functions that belongs to an abject.
```c
AZIOT_RESULT aziot_client_init(AZIOT_CLIENT* client);
AZIOT_RESULT aziot_client_deinit(AZIOT_CLIENT* client);
AZIOT_RESULT aziot_client_send_async(AZIOT_CLIENT* client, 
        AZIOT_MESSAGE* message, AZIOT_RELEASE_CALLBACK release_message, 
        AZIOT_CLIENT_RESULT_CALLBACK callback, AZIOT_CLIENT_RESULT_CONTEXT context);
```

### *Memory policy*
Clients should avoid any usage of heap. If it is necessary, the system shall provide the means for the customer to decide each function to call to `allocate` and `free` memory. This option should be in a config file that can be replaced by the the developer. It should contains the following definitions:
```c
/**
 * @brief   uLib malloc
 *
 *  Defines the malloc function that the ulib shall use as its own way to dynamically allocate
 *      memory from the HEAP. For simplicity, it can be defined as the malloc(size) from the `stdlib.h`.
 */
#define AZIOT_ULIB_CONFIG_MALLOC(size)    malloc(size)

/**
 * @brief   uLib free
 *
 *  Defines the free function that the ulib shall use as its own way to release memory dynamic 
 *      allocated in the HEAP. For simplicity, it can be defined as the free(ptr) from the `stdlib.h`.
 */
#define AZIOT_ULIB_CONFIG_FREE(ptr)       free(ptr)
```

#### Memory for the control block
The memory to all the control block should be allocated by the user of the API, and provided as part of the argument. On this way, the user may decide to allocate it on the .BSS, Stack, or Heap. 

#### Passing pre-allocate buffer
To avoid multiple copies of the same content, functions that receive a buffer may require a pointer to a release function. The object that receives the buffer may keep it for the time frame that is needed, when it is not necessary anymore, it shall call the release function. The release function should came after the buffer in the list of arguments, and the name should be `release_` following you the buffer name. For example:
```c
AZIOT_RESULT aziot_client_send_async(AZIOT_CLIENT* client, 
        AZIOT_MESSAGE* message, AZIOT_RELEASE_CALLBACK release_message, 
        AZIOT_CLIENT_RESULT_CALLBACK callback, AZIOT_CLIENT_RESULT_CONTEXT context);
```
The signature of the release function should be:
```c
void release(void* ptr); 
```

## Headers
The final **MSDN** documentation will be auto generated based on the header comments in the code. As a standard, all headers should be written using **Doxygen** [[4]](http://www.doxygen.nl/index.html).

### *Functions*
The header of a function should precede the function declaration in the header file (`.h`). All function headers should contain, at least:
* **@brief**: Contains a small description of the function. Other paragraphs can be added with more details about the function.
* **@section: Visibility**: Explicitly defines if the function is part of the public API or not.
* **@param**: Contains the name of the parameter and a description of it. It should contain details about the parameter, like if it is input or output, the memory ownership, any implicit type (ex: CHAR* should be a null terminated string), and the exception conditions (ex: a VOID* cannot be NULL). The header should contain one `@param` per argument in the function.
* **@return**: Contains the expected return. It can contain implicit typed (ex: BAR_RESULT with the error code). If applicable, the possible results should be listed, like `true` in the condition `a`, and `false` in the condition `b`, or a list of possible errors that it can return.
```c
/**
 * @brief: Allocate block of memory
 *
 * @section: Description
 *   This service allocates a memory block from the specified memory
 *     area. The actual size of the memory block is determined by the
 *     requested size, plus the header and padding. 
 *
 * @section: Allowed from
 *   Initialization, threads, timers, and ISRs
 *
 * @section: Preemption allowed
 *   Yes
 *
 * @section: Visibility
 *   Public API
 *
 * @example:
 * <pre><code>
 *   BAR_HEAP my_heap;
 *   uint8_t* memory_ptr;
 *   uint32_t status;
 *
 *   // Allocate a memory block from my_heap. Assume that the
 *   //   heap has already been created with a call to
 *   //   bar_heap_create.
 *   status = bar_heap_malloc(&my_heap, (void **) &memory_ptr, 100);
 *
 *   // If status equals SUCCESS, memory_ptr contains the
 *   //   address of the allocated block of memory.
 * </code></pre>
 *
 * @see: bar_heap_create
 * @see: bar_heap_delete
 * @see: bar_heap_info_get
 * @see: bar_heap_free
 *
 * @param:  heap_ptr        [IN] Pointer to a previously created memory heap.
 *                            It cannot be NULL.
 * @param:  block_ptr       [OUT] Pointer to a destination memory. On
 *                            successful allocation, the address of the 
 *                            allocated memory block is placed where this
 *                            parameter points. It cannot be NULL.
 * @param:  memory_size     [IN] Size of the block to be allocated in number 
 *                            of bytes. It shall be bigger than 0.
 * @return: The BAR_RESULT with the error code
 *   -@b SUCCESS          (0x00)   Successful memory block allocation.
 *   -@b NO_MEMORY        (0x10)   Service was unable to allocate a block
 *                                  of memory.
 *   -@b HEAP_ERROR       (0x02)   Invalid heap pointer.
 *   -@b PTR_ERROR        (0x03)   Invalid pointer to destination pointer.
 *   -@b SIZE_ERROR       (0x04)   Invalid memory size.
 */
```

### *Types (Structures, enumerators, strong types, etc)*
The header of a type should precede the type declaration. All type headers should contain, at least:
* **@brief**: Contains a small description of the type. Other paragraphs can be added with more details about the type 
* **@section: Visibility**: Explicitly defines if the type is part of the public API (`Public API`) or not (`Internal only`).
```c
/**
 * @brief: vTable with the micro stream public APIs.
 *
 * @section: Description
 *   This vTable contains the micro stream functions that 
 *     is part or the public interface of the stream. Any 
 *     producer that exposes the data as a micro stream 
 *     shall implement the function on this vTable. Any 
 *     consumer that will use the content of the exposed 
 *     micro stream shall call the `sx_` public APIs.
 *
 * @section: Visibility
 *   Public API
 */
```

### *Internals*
All header files in the `internal` directory should be documented as “`Internal only`” immediately after the Microsoft Copyright. The comment should contain the following information: 
```c
/**
 * @section: Visibility
 *    Internal only
 * 
 *  Never use the definitions on this file directly, this is **NOT** part 
 *    of the public API. The content of this file and its implementation 
 *    can change without notice.
 *
 * @see: xx_api.h
 */
```

Optionally, declarations placed in the internal header file may be explicitly documented as internal, and point to the public API for that purpose. The documentation with the usage restriction should be part of the section `Visibility`, and should contain, at least, the comment `Internal only`, for example:
```c
/**
 * @brief: Heap control block structure.  
 *
 * @section: Description
 *   Control block for heap control.
 *
 * @section: Visibility
 *    Internal only
 * 
 *  Never use this struct directly, this is **NOT** part of the public API.
 *  This struct can change without notice. 
 *      - To create a new heap, please use {@link bar_heap_create}.
 *      - To get the heap information, please use {@link bar_heap_info_get}. 
 *      - For any other heap operation, please use one of the public API
 *          `bar_heap` on {@link bar_api.h}
 */
```

## Test and requirements
All components must be totally tested, it is highly recommended that the code have 100% of test coverage.

All code should contain, at least, requirements, unit tests, end-to-end tests, and samples. The requirements description should be placed in the unit test file, on top of the test function that verifies the requirement. The unit test name should be placed in the code as a comment, together with the code that implements that functionality. For example:

### *On the code:*
```c
void foo_tcp_manager_destroy(TCP_HANDLE handle)
{
    if(handle == NULL)
    {
        /*[foo_tcp_manager_destroy_does_nothing_on_null_handle]*/
        LogError("handle cannot be NULL");
    }
    else
    {
        TCP_INSTANCE* instance = (TCP_INSTANCE*)handle;

        /*[foo_tcp_manager_destroy_succeed_on_free_all_resources]*/
        netif_remove(&(instance->lpc_netif));
        free(instance);
    }
}
```
### *On the unit test file:*
```c
/* If the provided TCP_HANDLE is NULL, the foo_tcp_manager_destroy shall do nothing. */
TEST_FUNCTION(foo_tcp_manager_destroy_does_nothing_on_null_handle)
{
    ///arrange

    ///act
    foo_tcp_manager_destroy(NULL);

    ///assert

    ///cleanup
}

/* The foo_tcp_manager_destroy shall free all resources allocated by the tcpip. */
TEST_FUNCTION(foo_tcp_manager_destroy_succeed_on_free_all_resources)
{
    ///arrange
    TCP_HANDLE handle = foo_tcp_manager_create();
    umock_c_reset_all_calls();
    STRICT_EXPECTED_CALL(netif_remove(IGNORED_PTR_ARG));
    STRICT_EXPECTED_CALL(free(handle));

    ///act
    foo_tcp_manager_destroy(handle);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    ///cleanup
}
```

If a single unit test tests more than one requirement, it should be sequentially enumerated in the UT file, and the same number should be added to the test name in the code comment. For example:

### *On the UT file:*
```c
/*[1]The foo_tcp_manager_create shall create a new instance of the TCP_INSTANCE 
        and return it as TCP_HANDLE.*/
/*[2]The foo_tcp_manager_create shall initialize the tcpip thread.*/
/*[3]The foo_tcp_manager_create shall initialize the netif with default 
        gateway, ip address, and net mask by calling netif_add.*/
/*[4]The foo_tcp_manager_create shall set the netif defaults by calling 
        netif_set_default and netif_set_up.*/
/*[5]If dhcp is enabled, the foo_tcp_manager_create shall start it by calling dhcp_start.*/
TEST_FUNCTION(foo_tcp_manager_create_createAndReturnInstanceSucceed)
{
    ...
}
```
### *On the code:*
```c
    /*[foo_tcp_manager_create_createAndReturnInstanceSucceed_1]*/
    TCP_INSTANCE* instance = (TCP_INSTANCE*)malloc(sizeof(TCP_INSTANCE));
```

No memory leaks (always run samples under **valgrind/vld**. Unit tests and e2e tests should be automatically **valgrind** verified at the gate).

## Code review

All code should be reviewed and approved by at least **2** other developers before being pushed to the master branch. If one developer does not approve the code, it should not be pushed, even if 2 other developers approve it.

The code review should follow the guidelines in this document. Any deviations from these guidelines should be properly justified. The developer should justify why it is necessary to not follow a rule. 
