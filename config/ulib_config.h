// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_CONFIG_H
#define AZ_ULIB_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file ulib_config.h
 * 
 * @brief Configuration options for ulib
 */

/*
 *  Define the heap 
 */

/**
 * @brief   ulib malloc
 *
 *  Defines the malloc function that the ulib shall use as its own way to dynamically allocate
 *      memory from the HEAP. For simplicity, it can be defined as the malloc(size) from the `stdlib.h`.
 */
#define AZ_ULIB_CONFIG_MALLOC(size)    malloc(size)

/**
 * @brief   ulib free
 *
 *  Defines the free function that the ulib shall use as its own way to release memory dynamic 
 *      allocated in the HEAP. For simplicity, it can be defined as the free(ptr) from the `stdlib.h`.
 */
#define AZ_ULIB_CONFIG_FREE(ptr)       free(ptr)

/*
 * Define log Function
 */

/**
 * @brief   ulib logger
 *
 *  Defines the log function that the ulib shall use as its own way to print information in the
 *      log system.
 */
#define AZ_ULIB_CONFIG_LOG(category, format, ...)  az_ulog_print(category, format, ##__VA_ARGS__)

/**
 * @brief   Maximum size of the ulib log.
 *
 *  Defines the maximum number of chars in each log line.
 */
#define AZ_ULIB_CONFIG_MAX_LOG_SIZE        256

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZ_ULIB_CONFIG_H */
