// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_CONFIG_ULIB_CONFIG_H_
#define AZURE_ULIB_C_CONFIG_ULIB_CONFIG_H_

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
#define ULIB_CONFIG_MALLOC(size)    ulib_malloc(size)

/**
 * @brief   uLib free
 *
 *  Defines the free function that the ulib shall use as its own way to release memory dynamic 
 *      allocated in the HEAP. For simplicity, it can be defined as the free(ptr) from the `stdlib.h`.
 */
#define ULIB_CONFIG_FREE(ptr)       ulib_free(ptr)

/*
 * Define log Function
 */

/**
 * @brief   uLib logger
 *
 *  Defines the log function that the ulib shall use as its own way to print information in the
 *      log system.
 */
#define ULIB_CONFIG_LOG(category, format, ...)  ulog_print(category, format, ##__VA_ARGS__)

/**
 * @brief   Maximum size of the uLib log.
 *
 *  Defines the maximum number of chars in each log line.
 */
#define ULIB_CONFIG_MAX_LOG_SIZE        256

/**
 * @brief   HTTP public API shall validate the arguments
 *
 *  Commenting this definition, the HTTP public APIs will not test any of the received arguments
 */
#define ULIB_CONFIG_HTTP_VALIDATE_ARGUMENTS

/**
 * @brief   Maximum number of clients attached to the HTTP.
 */
#define ULIB_CONFIG_HTTP_MAX_CLIENT 3

 /**
  * @brief   mESSAGE public API shall validate the arguments
  *
  *  Commenting this definition, the mESSAGE public APIs will not test any of the received arguments
  */
#define ULIB_CONFIG_MESSAGE_VALIDATE_ARGUMENTS

/**
 * @brief   Maximum size of the channel name in a message.
 */ 
#define ULIB_CONFIG_MESSAGE_MAX_CHANNEL_NAME_SIZE   50

/**
 * @brief   Maximum number of properties in a message.
 */
#define ULIB_CONFIG_MESSAGE_MAX_PROPERTIES      5

/**
 * @brief   Maximum size of a property in a message.
 * 
 * This size shall include the pair key:value as a string plus the terminator `/0`.
 * 
 * Example of properties are:
 * * <tt>Last-Modified: Sat, 20 Nov 2004 07:16:26 GMT</tt>
 * * <tt>ETag: "10000000565a5-2c-3e94b66c2e680"</tt>
 */
#define ULIB_CONFIG_MESSAGE_MAX_PROPERTY_SIZE   50

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_CONFIG_ULIB_CONFIG_H_ */
