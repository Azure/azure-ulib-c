// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_DESCRIPTOR_H
#define AZ_ULIB_DESCRIPTOR_H

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#include "az_ulib_action_api.h"
#include "az_ulib_base.h"
#include "az_ulib_result.h"

#ifndef __cplusplus
#include <stddef.h>
#include <stdint.h>
#else
#include <cstddef>
#include <cstdint>
extern "C" {
#endif /* __cplusplus */

/**
 * @file    az_ulib_descriptor_api.h
 *
 * @brief   Contains the macros to create the interface descriptor.
 *
 * This descriptor is used by the IPC to describe actions in the interface.
 *
 * The descriptor shall be created in compile time using these macros, which will place all the
 * content of the descriptor in the Text area (as constant), avoiding copies and access violation.
 */

/**
 * @brief   Action descriptor.
 *
 * This structure contains the full information about a single action in the interface. No matter
 * the type of the action, it shall be fully described by this structure, which means that this
 * structure is a superset of the fields of all types of actions.
 */
typedef struct az_ulib_action_descriptor_tag {
  const char* name; /**<The `\0` terminated `const char *` with the action name.*/
  const union /**<The primary function of the action. */
  {
    const void* action;
    const az_ulib_action_get get;
    const az_ulib_action_method method;
    const az_ulib_action_method_async method_async;
  } action_ptr_1;
  const union /**<The secondary function of the action. */
  {
    const void* action;
    const az_ulib_action_set set;
    const az_ulib_action_cancellation_callback cancel;
  } action_ptr_2;
  const uint8_t flags; /**<This is an 8 bit flags that handles internal status of the action. */
} az_ulib_action_descriptor;

/**
 * @brief   Interface descriptor.
 *
 * This structure contains the full information about a collection of action that represents the
 * interface.
 */
typedef struct az_ulib_interface_descriptor_tag {
  const char* name; /**<The `\0` terminated `const char*` with the interface name. */
  az_ulib_version version; /**<The #az_ulib_version with the interface version. */
  uint8_t size; /**<The `uint8_t` with the number of actions in the interface. */
  az_ulib_action_descriptor* action_list; /**<The list of #az_ulib_action_descriptor with the
                                          actions in this interface. */
} az_ulib_interface_descriptor;

/**
 * @brief   Create a new Interface descriptor.
 *
 * This API creates a new description for a interface, it will create the
 * #az_ulib_interface_descriptor with the provided interface name and version, adding all the
 * actions.
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
 * @param[in]   ...             The list of #az_ulib_action_descriptor with the actions in the
 *                              interface.
 */
#define AZ_ULIB_DESCRIPTOR_CREATE(interface_var, interface_name, version, ...) \
  static const az_ulib_action_descriptor MU_C2(interface_var, _ACTION_LIST)[] \
      = { MU_FOR_EACH_1(MU_DEFINE_ENUMERATION_CONSTANT, __VA_ARGS__) }; \
  static const az_ulib_interface_descriptor MU_C1(interface_var) \
      = { (interface_name), \
          (version), \
          (uint8_t)(MU_COUNT_ARG(__VA_ARGS__) / 4), \
          (az_ulib_action_descriptor*)MU_C2(interface_var, _ACTION_LIST) };

/**
 * @brief   Add property to the interface descriptor.
 *
 * Populate a new *property* action descriptor to add to the interface. On the interface context, a
 * `property` is a data exposed by its `get` and `set` function.
 *
 * @param[in]   name        The `/0` terminated `const char* const` with the property name. It
 *                          cannot be `NULL` and shall be allocated in a way that it stays valid
 *                          until the interface is unpublished at some (potentially) unknown time
 *                          in the future.
 * @param[in]   get         The function pointer to #az_ulib_action_get with the implementation
 *                          of the get method for the property. The get method shall be valid
 *                          until the interface is unpublished at some (potentially) unknown time
 *                          in the future.
 * @param[in]   set         The function pointer to #az_ulib_action_set with the implementation
 *                          of the set method for the property. The set method shall be valid
 *                          until the interface is unpublished at some (potentially) unknown time
 *                          in the future.
 * @return The #az_ulib_action_descriptor with the property.
 */
#define AZ_ULIB_DESCRIPTOR_ADD_PROPERTY(name, get, set) \
  { \
    (name), { (const void*)(get) }, { (const void*)(set) }, \
        (uint8_t)(AZ_ULIB_ACTION_TYPE_PROPERTY) \
  }

/**
 * @brief   Add a synchronous method to the interface descriptor.
 *
 * Populate a new *synchronous method* action descriptor to add to the interface. On the interface
 * context, a synchronous method is a blocking call method, which executes all the needed
 * instructions at once, and return the result at the end of the execution.
 *
 * @param[in]   name        The `/0` terminated `const char* const` with the method name. It
 *                          cannot be `NULL` and shall be allocated in a way that it stays valid
 *                          until the interface is unpublished at some (potentially) unknown time
 *                          in the future.
 * @param[in]   method      The function pointer to #az_ulib_action_method with the implementation
 *                          of the synchronous method. The method shall be valid until the
 *                          interface is unpublished at some (potentially) unknown time in the
 *                          future.
 * @return The #az_ulib_action_descriptor with the method.
 */
#define AZ_ULIB_DESCRIPTOR_ADD_METHOD(name, method) \
  { \
    (name), { (const void*)(method) }, { (const void*)NULL }, \
        (uint8_t)(AZ_ULIB_ACTION_TYPE_METHOD) \
  }

/**
 * @brief   Add an asynchronous method to the interface descriptor.
 *
 * Populate a new *asynchronous method* action descriptor to add to the interface. On the interface
 * context, an asynchronous method is a call to a method that will start a hardware operation and
 * return before its conclusion. In some point in the future, a hardware interruption (IRQ) will
 * wake the CPU with the result of this operation, at this point, the asynchronous method will
 * conclude the operation firing a callback with the result.
 *
 * The hardware operation may be cancellable, so, the API allows the developer to provide a cancel
 * function.
 *
 * @param[in]   name            The `/0` terminated `const char* const` with the method name. It
 *                              cannot be `NULL` and shall be allocated in a way that it stays
 *                              valid until the interface is unpublished at some (potentially)
 *                              unknown time in the future.
 * @param[in]   method_async    The function pointer to #az_ulib_action_method_async with the
 *                              implementation of the asynchronous method. The method shall be
 *                              valid until the interface is unpublished at some (potentially)
 *                              unknown time in the future.
 * @param[in]   cancel          The function pointer to #az_ulib_action_cancellation_callback
 *                              with the implementation of the function to cancel the asynchronous
 *                              method. It can be `NULL` if the method does not allow any
 *                              cancellation. If provided, the cancel shall be valid until the
 *                              interface is unpublished at some (potentially) unknown time in the
 *                              future.
 * @return The #az_ulib_action_descriptor with the method.
 */
#define AZ_ULIB_DESCRIPTOR_ADD_METHOD_ASYNC(name, method_async, cancel) \
  { \
    (name), { (const void*)(method_async) }, { (const void*)(cancel) }, \
        (uint8_t)(AZ_ULIB_ACTION_TYPE_METHOD_ASYNC) \
  }

/**
 * @brief   Add an event to the interface descriptor.
 *
 * Populate a new *event* action descriptor to add to the interface. On the interface context, a
 * event is a subscribed base notification, where other components in the system can subscribe to
 * be notified when certain event happened.
 *
 * @param[in]   name        The `/0` terminated `const char* const` with the event name. It cannot
 *                          be `NULL` and shall be allocated in a way that it stays valid until the
 *                          interface is unpublished at some (potentially) unknown time in the
 *                          future.
 * @return The #az_ulib_action_descriptor with the method.
 */
#define AZ_ULIB_DESCRIPTOR_ADD_EVENT(name) \
  { (name), { (const void*)(NULL) }, { (const void*)(NULL) }, (uint8_t)(AZ_ULIB_ACTION_TYPE_EVENT) }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZ_ULIB_DESCRIPTOR_H */
