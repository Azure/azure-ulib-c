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
    /** The `az_span` with the capability name. */
    const az_span name;

    /** Pointer to the capability function to call. */
    const az_ulib_capability capability_ptr;

    /** The primary span wrapper of the capability. */
    const az_ulib_capability_span_wrapper capability_span_wrapper;
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
    /** The `az_span` with the package name. */
    const az_span pkg_name;

    /** The #az_ulib_version with the package version. */
    const az_ulib_version pkg_version;

    /** The `az_span` with the interface name. */
    const az_span intf_name;

    /** The #az_ulib_version with the interface version. */
    const az_ulib_version intf_version;

    /** The #az_ulib_capability_index with the number of capabilities in the interface. */
    const az_ulib_capability_index size;

    /** The list of #az_ulib_capability_descriptor with the capabilities in this interface. */
    const az_ulib_capability_descriptor* capability_list;

  } _internal;
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
 * @param[in]   package_name      The `\0` terminated `const char* const` with the package name.
 *                                It cannot be `NULL` and shall be allocated in a way that it stays
 *                                valid until the interface is unpublished at some (potentially)
 *                                unknown time in the future.
 * @param[in]   package_version   The #az_ulib_version with the package version. It cannot be `0`.
 * @param[in]   interface_name    The `\0` terminated `const char* const` with the interface name.
 *                                It cannot be `NULL` and shall be allocated in a way that it stays
 *                                valid until the interface is unpublished at some (potentially)
 *                                unknown time in the future.
 * @param[in]   interface_version The #az_ulib_version with the interface version. It cannot be `0`.
 * @param[in]   capabilities      The list of #az_ulib_capability_descriptor in the interface.
 * @return The #az_ulib_interface_descriptor with the description of the interface.
 */
#define AZ_ULIB_DESCRIPTOR_CREATE(                                                  \
    package_name, package_version, interface_name, interface_version, capabilities) \
  {                                                                                 \
    ._internal                                                                      \
        = {.pkg_name = AZ_SPAN_LITERAL_FROM_STR(package_name),                      \
           .pkg_version = package_version,                                          \
           .intf_name = AZ_SPAN_LITERAL_FROM_STR(interface_name),                   \
           .intf_version = interface_version,                                       \
           .size = sizeof(capabilities) / sizeof(az_ulib_capability_descriptor),    \
           .capability_list = capabilities }                                        \
  }

/**
 * @brief   Add a synchronous capability to the interface descriptor.
 *
 * @param[in] capability_name     The `\0` terminated `const char* const` with the capability name.
 *                                It cannot be `NULL` and shall be allocated in a way that it
 *                                stays valid until the interface is unpublished at some
 *                                (potentially) unknown time in the future.
 * @param[in] concrete            The function pointer to #az_ulib_capability with the
 *                                implementation of the synchronous capability call. The capability
 *                                shall be valid until the interface is unpublished at some
 *                                (potentially) unknown time in the future.
 * @param[in] span_wrapper        The function pointer to #az_ulib_capability_span_wrapper
 *                                with the wrapper for the capability using strings in `az_span`.
 * @return The #az_ulib_capability_descriptor with the capability.
 */
#define AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(capability_name, concrete, span_wrapper)          \
  {                                                                                         \
    ._internal                                                                              \
        = {.name = AZ_SPAN_LITERAL_FROM_STR(capability_name),                               \
           .capability_ptr = (const az_ulib_capability)concrete,                            \
           .capability_span_wrapper = (const az_ulib_capability_span_wrapper)span_wrapper } \
  }

/**
 * @brief   Add a telemetry to the interface descriptor.
 *
 * @param[in] telemetry_name    The `\0` terminated `const char* const` with the telemetry name. It
 *                              cannot be `NULL` and shall be allocated in a way that it stays valid
 *                              until the interface is unpublished at some (potentially) unknown
 *                              time in the future.
 * @return The #az_ulib_capability_descriptor with the telemetry.
 */
#define AZ_ULIB_DESCRIPTOR_ADD_TELEMETRY(telemetry_name) \
  AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(telemetry_name, NULL, NULL)

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_DESCRIPTOR_API_H */
