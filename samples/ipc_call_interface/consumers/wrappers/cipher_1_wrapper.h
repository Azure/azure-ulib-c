// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from cipher v1 DL and shall not be
 * modified.
 ********************************************************************/

#ifndef CIPHER_1_WRAPPER_H
#define CIPHER_1_WRAPPER_H

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"
#include "cipher_1_interface.h"

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

  /*
   * cipher class constructor.
   */
  static inline az_result cipher_1_create(az_ulib_ipc_interface_handle handle)
  {
    return az_ulib_ipc_try_get_interface(
        AZ_SPAN_FROM_STR(CIPHER_1_INTERFACE_NAME),
        CIPHER_1_INTERFACE_VERSION,
        AZ_ULIB_VERSION_EQUALS_TO,
        handle);
  }

  /*
   * cipher class destructor.
   */
  static inline void cipher_1_destroy(az_ulib_ipc_interface_handle handle)
  {
    az_result result = az_ulib_ipc_release_interface(handle);
    (void)result;
  }

  /*
   * Azure Callable Wrapper for cipher encrypt.
   */
  static inline az_result cipher_1_encrypt(
      az_ulib_ipc_interface_handle handle,
      uint32_t context,
      az_span src,
      az_span* dest)
  {
    // Marshalling
    cipher_1_encrypt_model_in in = { .context = context, .src = src };
    cipher_1_encrypt_model_out out = { .dest = dest };

    // Call
    return az_ulib_ipc_call(handle, CIPHER_1_INTERFACE_ENCRYPT_COMMAND, &in, &out);
  }

  /*
   * Azure Callable Wrapper for cipher subtract.
   */
  static inline az_result cipher_1_decrypt(
      az_ulib_ipc_interface_handle handle,
      az_span src,
      az_span* dest)
  {
    // Marshalling
    cipher_1_decrypt_model_in in = { .src = src };
    cipher_1_decrypt_model_out out = { .dest = dest };

    // Call
    return az_ulib_ipc_call(handle, CIPHER_1_INTERFACE_DECRYPT_COMMAND, &in, &out);
  }

#ifdef __cplusplus
}
#endif

#endif /* CIPHER_1_WRAPPER_H */
