// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_DESCRIPTOR_H
#define AZ_ULIB_DESCRIPTOR_H

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#include "az_ulib_base.h"
#include "az_ulib_capability_api.h"
#include "az_ulib_result.h"

#ifndef __cplusplus
#include <stddef.h>
#include <stdint.h>
#else
#include <cstddef>
#include <cstdint>
#endif /* __cplusplus */

#include "azure/core/_az_cfg_prefix.h"

/**
 * @file    az_ulib_descriptor_api.h
 *
 * @brief   Contains the macros to create the interface descriptor.
 *
 * This descriptor is used by the IPC to describe capabilities in the interface.
 *
 * The descriptor shall be created in compile time using these macros, which will place all the
 * content of the descriptor in the Text area (as constant), avoiding copies and access violation.
 */

/**
 * @brief   Capability descriptor.
 *
 * This structure contains the full information about a single capability in the interface. No
 * matter the type of the capability, it shall be fully described by this structure, which means
 * that this structure is a superset of the fields of all types of capabilities.
 */
typedef struct az_ulib_capability_descriptor_tag
{
  const char* name; /**<The `\0` terminated `const char *` with the capability name.*/
  const union /**<The primary function of the capability. */
  {
    const void* capability;
    const az_ulib_capability_get get;
    const az_ulib_capability_command command;
    const az_ulib_capability_command_async command_async;
  } capability_ptr_1;
  const union /**<The secondary function of the capability. */
  {
    const void* capability;
    const az_ulib_capability_set set;
    const az_ulib_capability_cancellation_callback cancel;
  } capability_ptr_2;
  const uint8_t flags; /**<This is an 8 bit flags that handles internal status of the capability. */
} az_ulib_capability_descriptor;

/**
 * @brief   Interface descriptor.
 *
 * This structure contains the full information about a collection of capability that represents the
 * interface.
 */
typedef struct az_ulib_interface_descriptor_tag
{
  const char* name; /**<The `\0` terminated `const char*` with the interface name. */
  az_ulib_version version; /**<The #az_ulib_version with the interface version. */
  uint8_t size; /**<The `uint8_t` with the number of capabilities in the interface. */
  az_ulib_capability_descriptor* capability_list; /**<The list of #az_ulib_capability_descriptor
                                                  with the capabilities in this interface. */
} az_ulib_interface_descriptor;

/**
 * @brief   Create a new Interface descriptor.
 *
 * This API creates a new description for a interface, it will create the
 * #az_ulib_interface_descriptor with the provided interface name and version, adding all the
 * capabilities.
 *
 * The descriptor will be stored in the Text area (Flash) that will be pointed by the provided
 * interface_var. In this way, no other components on the system needs to copy any of the data on
 * the descriptor.
 *
 * @param[in]   interface_var   The #az_ulib_interface_descriptor that will point to the created
 *                              descriptor.
 * @param[in]   interface_name  The `/0` terminated `const char* const` with the interface name.
 *                              It cannot be `NULL` and shall be allocated in a way that it stays
 *                              valid until the interface is unpublished at some (potentially)
 *                              unknown time in the future.
 * @param[in]   version         The #az_ulib_version with the interface version.
 * @param[in]   ...             The list of #az_ulib_capability_descriptor with the capabilities in
 *                              the interface.
 */
#define AZ_ULIB_DESCRIPTOR_CREATE(interface_var, interface_name, version, ...)        \
  static const az_ulib_capability_descriptor MU_C2(interface_var, _CAPABILITY_LIST)[] \
      = { MU_FOR_EACH_1(MU_DEFINE_ENUMERATION_CONSTANT, __VA_ARGS__) };               \
  static const az_ulib_interface_descriptor MU_C1(interface_var)                      \
      = { (interface_name),                                                           \
          (version),                                                                  \
          (uint8_t)(MU_COUNT_ARG(__VA_ARGS__) / 4),                                   \
          (az_ulib_capability_descriptor*)MU_C2(interface_var, _CAPABILITY_LIST) };

/**
 * @brief   Add property to the interface descriptor.
 *
 * Populate a new [*property* capability](#AZ_ULIB_CAPABILITY_TYPE_PROPERTY) to add to the
 * interface.
 *
 * @param[in]   name        The `/0` terminated `const char* const` with the property name. It
 *                          cannot be `NULL` and shall be allocated in a way that it stays valid
 *                          until the interface is unpublished at some (potentially) unknown time
 *                          in the future.
 * @param[in]   get         The function pointer to #az_ulib_capability_get with the implementation
 *                          of the get command for the property. The get command shall be valid
 *                          until the interface is unpublished at some (potentially) unknown time
 *                          in the future.
 * @param[in]   set         The function pointer to #az_ulib_capability_set with the implementation
 *                          of the set command for the property. The set command shall be valid
 *                          until the interface is unpublished at some (potentially) unknown time
 *                          in the future.
 * @return The #az_ulib_capability_descriptor with the property.
 */
#define AZ_ULIB_DESCRIPTOR_ADD_PROPERTY(name, get, set)     \
  {                                                         \
    (name), { (const void*)(get) }, { (const void*)(set) }, \
        (uint8_t)(AZ_ULIB_CAPABILITY_TYPE_PROPERTY)         \
  }

/**
 * @brief   Add a synchronous command to the interface descriptor.
 *
 * Populate a new [*synchronous command* capability](#AZ_ULIB_CAPABILITY_TYPE_COMMAND) to add
 * to the interface.
 *
 * @param[in]   name        The `/0` terminated `const char* const` with the command name. It
 *                          cannot be `NULL` and shall be allocated in a way that it stays valid
 *                          until the interface is unpublished at some (potentially) unknown time
 *                          in the future.
 * @param[in]   command      The function pointer to #az_ulib_capability_command with the
 *                          implementation of the synchronous command. The command shall be valid
 *                          until the interface is unpublished at some (potentially) unknown time
 *                          in the future.
 * @return The #az_ulib_capability_descriptor with the command.
 */
#define AZ_ULIB_DESCRIPTOR_ADD_COMMAND(name, command)          \
  {                                                            \
    (name), { (const void*)(command) }, { (const void*)NULL }, \
        (uint8_t)(AZ_ULIB_CAPABILITY_TYPE_COMMAND)             \
  }

/**
 * @brief   Add an asynchronous command to the interface descriptor.
 *
 * Populate a new [*asynchronous command* capability](#AZ_ULIB_CAPABILITY_TYPE_COMMAND_ASYNC) to
 * add to the interface.
 *
 * @param[in]   name            The `/0` terminated `const char* const` with the command name. It
 *                              cannot be `NULL` and shall be allocated in a way that it stays
 *                              valid until the interface is unpublished at some (potentially)
 *                              unknown time in the future.
 * @param[in]   command_async   The function pointer to #az_ulib_capability_command_async with the
 *                              implementation of the asynchronous command. The command shall be
 *                              valid until the interface is unpublished at some (potentially)
 *                              unknown time in the future.
 * @param[in]   cancel          The function pointer to #az_ulib_capability_cancellation_callback
 *                              with the implementation of the function to cancel the asynchronous
 *                              command. It can be `NULL` if the command does not allow any
 *                              cancellation. If provided, the cancel shall be valid until the
 *                              interface is unpublished at some (potentially) unknown time in the
 *                              future.
 * @return The #az_ulib_capability_descriptor with the command async.
 */
#define AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(name, command_async, cancel) \
  {                                                                       \
    (name), { (const void*)(command_async) }, { (const void*)(cancel) },  \
        (uint8_t)(AZ_ULIB_CAPABILITY_TYPE_COMMAND_ASYNC)                  \
  }

/**
 * @brief   Add a telemetry to the interface descriptor.
 *
 * Populate a new [*telemetry* capability](#AZ_ULIB_CAPABILITY_TYPE_TELEMETRY) to add to the
 * interface.
 *
 * @param[in]   name        The `/0` terminated `const char* const` with the telemetry name. It
 *                          cannot be `NULL` and shall be allocated in a way that it stays valid
 *                          until the interface is unpublished at some (potentially) unknown time
 *                          in the future.
 * @return The #az_ulib_capability_descriptor with the telemetry.
 */
#define AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(name)                \
  {                                                           \
    (name), { (const void*)(NULL) }, { (const void*)(NULL) }, \
        (uint8_t)(AZ_ULIB_CAPABILITY_TYPE_TELEMETRY)          \
  }

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_DESCRIPTOR_H */
