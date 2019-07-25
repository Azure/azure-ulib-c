// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZIOT_ULIB_CONFIG_H
#define AZIOT_ULIB_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file ulib_config.h
 */

/*
 *  Define the heap 
 */

/**
 * @brief   uLib malloc
 *
 *  Defines the malloc function that the ulib shall use as its own way to dynamically allocate
 *      memory from the HEAP. For simplicity, it can be defined as the malloc(size) from the `stdlib.h`.
 */
#define AZIOT_ULIB_CONFIG_MALLOC(size)    aziot_ulib_malloc(size)

/**
 * @brief   uLib free
 *
 *  Defines the free function that the ulib shall use as its own way to release memory dynamic 
 *      allocated in the HEAP. For simplicity, it can be defined as the free(ptr) from the `stdlib.h`.
 */
#define AZIOT_ULIB_CONFIG_FREE(ptr)       aziot_ulib_free(ptr)

/*
 * Define log Function
 */

/**
 * @brief   uLib logger
 *
 *  Defines the log function that the ulib shall use as its own way to print information in the
 *      log system.
 */
#define AZIOT_ULIB_CONFIG_LOG(category, format, ...)  aziot_ulog_print(category, format, ##__VA_ARGS__)

/**
 * @brief   Maximum size of the uLib log.
 *
 *  Defines the maximum number of chars in each log line.
 */
#define AZIOT_ULIB_CONFIG_MAX_LOG_SIZE        256

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZIOT_ULIB_CONFIG_H */
