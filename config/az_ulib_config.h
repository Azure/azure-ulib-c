// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_CONFIG_H
#define AZ_ULIB_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file az_ulib_config.h
 *
 * @brief Configuration options for ulib
 */

/*
 * Define log Function
 */

/**
 * @brief   ulib logger
 *
 * Defines the log function that the ulib shall use as its own way to print information in the
 * log system.
 */
#define AZ_ULIB_CONFIG_LOG(category, format, ...) az_ulog_print(category, format, ##__VA_ARGS__)

/**
 * @brief   Maximum size of the ulib log.
 *
 * Defines the maximum number of chars in each log line.
 */
#define AZ_ULIB_CONFIG_MAX_LOG_SIZE 256

/**
 * @brief   IPC public API shall validate the contract
 *
 * This definition enables the argument check on all IPC public APIs. This check will verify if the
 * provided arguments follow the contract defined in the documentation.
 *
 * Commenting this definition, the IPC public APIs will not test any of the received arguments.
 */
#define AZ_ULIB_CONFIG_IPC_VALIDATE_CONTRACT

/**
 * @brief   Maximum number of interfaces published in the IPC.
 *
 * Defines the maximum number of interfaces that the system can publish on the IPC. Increasing this
 * number will increase the amount of memory reserved to the IPC.
 *
 * The IPC will reserve memory to store an `_az_ulib_ipc_interface_list` for the maximum number of
 * interfaces defined by this constant.
 */
#define AZ_ULIB_CONFIG_MAX_IPC_INTERFACE 10

/**
 * @brief   Maximum interface instances.
 *
 * Defines the maximum number of instances of each interface. The instance is the handle returned by
 * az_ulib_ipc_get_interface() or az_ulib_ipc_try_get_interface(). This value does not affect the
 * amount of memory that the IPC will use. It just impose a limit on the interface `ref_count`.
 */
#define AZ_ULIB_CONFIG_MAX_IPC_INSTANCES 20

#ifndef AZ_ULIB_CONFIG_REMOVE_UNPUBLISH
/**
 * @brief   Enable unpublish on IPC.
 *
 * @note    Comment this line will:
 *            - Improve performance.
 *            - Reduce memory by 2 longs per IPC interface.
 *            - Remove the API az_ulib_ipc_unpublish.
 *
 * To allow users to unpublish interfaces in the IPC, it is necessary to add a flag to avoid an
 * interface to be unpublished if at least one of its methods are in execution at that time. So, if
 * the system doesn't need to unpublish interfaces, this controls can be removed, saving memory
 * space and improving the az_ulib_ipc_call performance.
 *
 * @note  **To avoid conflicts in the linker, instead of comment this line, define
 *        AZ_ULIB_CONFIG_REMOVE_UNPUBLISH as part of the make file that will build the project. 
 *        For cmake, use the option -Dremove_ipc_unpublish.**
 */
#define AZ_ULIB_CONFIG_IPC_UNPUBLISH
#endif /*AZ_ULIB_CONFIG_REMOVE_UNPUBLISH*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZ_ULIB_CONFIG_H */
