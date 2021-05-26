// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from display v1 DL and shall not be
 * modified.
 ********************************************************************/

#ifndef DISPLAY_1_WRAPPER_H
#define DISPLAY_1_WRAPPER_H

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"
#include "display_1_model.h"

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

  /*
   * display class constructor.
   */
  static inline az_result display_1_create(az_ulib_ipc_interface_handle handle)
  {
    return az_ulib_ipc_try_get_interface(
        AZ_SPAN_FROM_STR(DISPLAY_1_INTERFACE_NAME),
        DISPLAY_1_INTERFACE_VERSION,
        AZ_ULIB_VERSION_EQUALS_TO,
        handle);
  }

  /*
   * display class destructor.
   */
  static inline void display_1_destroy(az_ulib_ipc_interface_handle handle)
  {
    az_result result = az_ulib_ipc_release_interface(handle);
    (void)result;
  }

  /*
   * Azure Callable Wrapper for display cls.
   */
  static inline az_result display_1_cls(az_ulib_ipc_interface_handle handle)
  {
    // Call
    return az_ulib_ipc_call(handle, DISPLAY_1_CLS_COMMAND, NULL, NULL);
  }

  /*
   * Azure Callable Wrapper for display print.
   */
  static inline az_result display_1_print(
      az_ulib_ipc_interface_handle handle,
      int32_t x,
      int32_t y,
      const char* buf,
      size_t size)
  {
    // Marshalling
    display_1_print_model_in in = { .x = x, .y = y, .buffer = buf, .size = size };

    // Call
    return az_ulib_ipc_call(handle, DISPLAY_1_PRINT_COMMAND, &in, NULL);
  }

  /*
   * Azure Callable Wrapper for display invalidate.
   */
  static inline az_result display_1_invalidate(az_ulib_ipc_interface_handle handle)
  {
    // Call
    return az_ulib_ipc_call(handle, DISPLAY_1_INVALIDATE_COMMAND, NULL, NULL);
  }

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_1_WRAPPER_H */
