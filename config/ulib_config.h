// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_CONFIG_ULIB_CONFIG_H_
#define AZURE_ULIB_C_CONFIG_ULIB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Define the heap 
 */

/**
 * @brief   uLib malloc.
 *
 * <p> Defines the malloc function that the ulib shall use as its own way to dynamically allocate
 *      memory from the HEAP. For simplicity, it can be defined as the malloc(size) from the `stdlib.h`.
 */
#define ULIB_CONFIG_MALLOC(size)    uLibMalloc(size)

/**
 * @brief   uLib free.
 *
 * <p> Defines the free function that the ulib shall use as its own way to release memory dynamic 
 *      allocated in the HEAP. For simplicity, it can be defined as the free(ptr) from the `stdlib.h`.
 */
#define ULIB_CONFIG_FREE(ptr)       uLibFree(ptr)

/**
 * Define log Function
 */

/**
 * @brief   uLib logger.
 *
 * <p> Defines the log function that the ulib shall use as its own way to print information in the
 *      log system.
 */
#define ULIB_CONFIG_LOG(category, format, ...)  ULIB_PRINT(category, format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_CONFIG_ULIB_CONFIG_H_ */
