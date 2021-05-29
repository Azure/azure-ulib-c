// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_DESCRIPTOR_API_H
#define AZ_ULIB_DESCRIPTOR_API_H

#include "az_ulib_base.h"
#include "az_ulib_capability_api.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"

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
  struct
  {
    /** The `\0` terminated `const az_span` with the capability name. */
    const az_span name;

    /** The primary function of the capability. */
    const union
    {
      const az_ulib_capability_get get;
      const az_ulib_capability_command command;
      const az_ulib_capability_command_async command_async;
    } capability_ptr_1;

    /** The secondary function of the capability. */
    const union
    {
      const az_ulib_capability_set set;
      const az_ulib_capability_cancellation_callback cancel;
    } capability_ptr_2;

    /** The primary span wrapper of the capability. */
    const union
    {
      const az_ulib_capability_get_span_wrapper get;
      const az_ulib_capability_command_span_wrapper command;
      const az_ulib_capability_command_async_span_wrapper command_async;
    } span_wrapper_ptr_1;

    /** The secondary span wrapper of the capability. */
    const union
    {
      const az_ulib_capability_set_span_wrapper set;
    } span_wrapper_ptr_2;

    /** This is an 8 bit flag that handles the internal status of the capability. */
    const uint8_t flags;

  } _internal;
} az_ulib_capability_descriptor;

/**
 * @brief   Interface descriptor.
 *
 * This structure contains the full information about a collection of capability that represents the
 * interface.
 */
typedef struct az_ulib_interface_descriptor_tag
{
  struct
  {
    /** The `\0` terminated `const az_span` with the interface name. */
    const az_span name;

    /** The #az_ulib_version with the interface version. */
    const az_ulib_version version;

    /** The `uint8_t` with the number of capabilities in the interface. */
    const az_ulib_capability_index size;

    /** The list of #az_ulib_capability_descriptor with the capabilities in this interface. */
    const az_ulib_capability_descriptor* capability_list;

  } _internal;
} az_ulib_interface_descriptor;

/**
 * @brief Interface handle.
 */
typedef void* az_ulib_ipc_interface_handle;

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
 * @param[in]   interface_name    The `/0` terminated `const char* const` with the interface name.
 *                                It cannot be `NULL` and shall be allocated in a way that it stays
 *                                valid until the interface is unpublished at some (potentially)
 *                                unknown time in the future.
 * @param[in]   interface_version The #az_ulib_version with the interface version.
 * @param[in]   capability_size   The number of #az_ulib_capability_descriptor in the interface.
 * @param[in]   capabilities      The list of #az_ulib_capability_descriptor in the interface.
 * @return The #az_ulib_interface_descriptor with the description of the interface.
 */
#define AZ_ULIB_DESCRIPTOR_CREATE(                                    \
    interface_name, interface_version, capability_size, capabilities) \
  {                                                                   \
    ._internal                                                        \
        = {.name = AZ_SPAN_LITERAL_FROM_STR(interface_name),          \
           .version = interface_version,                              \
           .size = capability_size,                                   \
           .capability_list = capabilities }                          \
  }

/**
 * @brief   Add property to the interface descriptor.
 *
 * Populate a new [*property* capability](#AZ_ULIB_CAPABILITY_TYPE_PROPERTY) to add to the
 * interface.
 *
 * @param[in] property_name     The `/0` terminated `const char* const` with the property name.
 *                              It cannot be `NULL` and shall be allocated in a way that it
 *                              stays valid until the interface is unpublished at some
 *                              (potentially) unknown time in the future.
 * @param[in] get_concrete      The function pointer to #az_ulib_capability_get with the
 *                              implementation of the get command for the property. The get
 *                              command shall be valid until the interface is unpublished at
 *                              some (potentially) unknown time in the future.
 * @param[in] set_concrete      The function pointer to #az_ulib_capability_set with the
 *                              implementation of the set command for the property. The set
 *                              command shall be valid until the interface is unpublished at
 *                              some (potentially) unknown time in the future.
 * @param[in] get_span_wrapper  The function pointer to #az_ulib_capability_get_span_wrapper
 *                              with the wrapper for the get command using strings in `az_span`.
 * @param[in] set_span_wrapper  The function pointer to #az_ulib_capability_set_span_wrapper
 *                              with the wrapper for the set command using strings in `az_span`.
 * @return The #az_ulib_capability_descriptor with the property.
 */
#define AZ_ULIB_DESCRIPTOR_ADD_PROPERTY(                                                          \
    property_name, get_concrete, set_concrete, get_span_wrapper, set_span_wrapper)                \
  {                                                                                               \
    ._internal                                                                                    \
        = {.name = AZ_SPAN_LITERAL_FROM_STR(property_name),                                       \
           .capability_ptr_1.get = (const az_ulib_capability_get)get_concrete,                    \
           .capability_ptr_2.set = (const az_ulib_capability_set)set_concrete,                    \
           .span_wrapper_ptr_1.get = (const az_ulib_capability_get_span_wrapper)get_span_wrapper, \
           .span_wrapper_ptr_2.set = (const az_ulib_capability_set_span_wrapper)set_span_wrapper, \
           .flags = (uint8_t)(AZ_ULIB_CAPABILITY_TYPE_PROPERTY) }                                 \
  }

/**
 * @brief   Add a synchronous command to the interface descriptor.
 *
 * Populate a new [*synchronous command* capability](#AZ_ULIB_CAPABILITY_TYPE_COMMAND) to add
 * to the interface.
 *
 * @param[in] command_name          The `/0` terminated `const char* const` with the command name.
 *                                  It cannot be `NULL` and shall be allocated in a way that it
 *                                  stays valid until the interface is unpublished at some
 *                                  (potentially) unknown time in the future.
 * @param[in] command_concrete      The function pointer to #az_ulib_capability_command with the
 *                                  implementation of the synchronous command. The command shall be
 *                                  valid until the interface is unpublished at some (potentially)
 *                                  unknown time in the future.
 * @param[in] command_span_wrapper  The function pointer to #az_ulib_capability_command_span_wrapper
 *                                  with the wrapper for the command using strings in `az_span`.
 * @return The #az_ulib_capability_descriptor with the command.
 */
#define AZ_ULIB_DESCRIPTOR_ADD_COMMAND(command_name, command_concrete, command_span_wrapper) \
  {                                                                                          \
    ._internal                                                                               \
        = {.name = AZ_SPAN_LITERAL_FROM_STR(command_name),                                   \
           .capability_ptr_1.command = (const az_ulib_capability_command)command_concrete,   \
           .span_wrapper_ptr_1.command                                                       \
           = (const az_ulib_capability_command_span_wrapper)command_span_wrapper,            \
           .flags = (uint8_t)(AZ_ULIB_CAPABILITY_TYPE_COMMAND) }                             \
  }

/**
 * @brief   Add an asynchronous command to the interface descriptor.
 *
 * Populate a new [*asynchronous command* capability](#AZ_ULIB_CAPABILITY_TYPE_COMMAND_ASYNC) to
 * add to the interface.
 *
 * @param[in] command_name          The `/0` terminated `const char* const` with the command name.
 *                                  It cannot be `NULL` and shall be allocated in a way that it
 *                                  stays valid until the interface is unpublished at some
 *                                  (potentially) unknown time in the future.
 * @param[in] command_concrete      The function pointer to #az_ulib_capability_command_async with
 *                                  the implementation of the asynchronous command. The command
 *                                  shall be valid until the interface is unpublished at some
 *                                  (potentially) unknown time in the future.
 * @param[in] command_span_wrapper  The function pointer to
 *                                  #az_ulib_capability_command_async_span_wrapper with the wrapper
 *                                  for the command using strings in `az_span`.
 * @param[in] cancel_concrete       The function pointer to
 *                                  #az_ulib_capability_cancellation_callback with the
 *                                  implementation of the function to cancel the asynchronous
 *                                  command. It can be `NULL` if the command does not allow any
 *                                  cancellation. If provided, the cancel shall be valid until the
 *                                  interface is unpublished at some (potentially) unknown time in
 *                                  the future.
 * @return The #az_ulib_capability_descriptor with the command async.
 */
#define AZ_ULIB_DESCRIPTOR_ADD_COMMAND_ASYNC(                                                     \
    command_name, command_concrete, command_span_wrapper, cancel_concrete)                        \
  {                                                                                               \
    ._internal = {                                                                                \
      .name = AZ_SPAN_LITERAL_FROM_STR(command_name),                                             \
      .capability_ptr_1.command_async = (const az_ulib_capability_command_async)command_concrete, \
      .capability_ptr_2.cancel = (const az_ulib_capability_cancellation_callback)cancel_concrete, \
      .span_wrapper_ptr_1.command_async                                                           \
      = (const az_ulib_capability_command_async_span_wrapper)command_span_wrapper,                \
      .flags = (uint8_t)(AZ_ULIB_CAPABILITY_TYPE_COMMAND_ASYNC)                                   \
    }                                                                                             \
  }

/**
 * @brief   Add a telemetry to the interface descriptor.
 *
 * Populate a new [*telemetry* capability](#AZ_ULIB_CAPABILITY_TYPE_TELEMETRY) to add to the
 * interface.
 *
 * @param[in] telemetry_name    The `/0` terminated `const char* const` with the telemetry name. It
 *                              cannot be `NULL` and shall be allocated in a way that it stays valid
 *                              until the interface is unpublished at some (potentially) unknown
 *                              time in the future.
 * @return The #az_ulib_capability_descriptor with the telemetry.
 */
#define AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(telemetry_name)           \
  {                                                                \
    ._internal                                                     \
        = {.name = AZ_SPAN_LITERAL_FROM_STR(telemetry_name),       \
           .flags = (uint8_t)(AZ_ULIB_CAPABILITY_TYPE_TELEMETRY) } \
  }

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_DESCRIPTOR_API_H */
