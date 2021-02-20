// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from cipher v1 DL.
 *
 * Implement the code under the concrete functions.
 *
 ********************************************************************/

#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "az_ulib_ucontract.h"
#include "az_ulib_ulog.h"
#include "cipher_1_interface.h"
#include "cipher_v1i1.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Concrete implementations of the cipher commands.
 */
static az_result cipher_1_encrypt_concrete(az_ulib_model_in model_in, az_ulib_model_out model_out)
{
  az_result result;

  /*
   * ==================
   * The user code starts here.
   */
  cipher_1_encrypt_model_in* in = (cipher_1_encrypt_model_in*)model_in;
  cipher_1_encrypt_model_out* out = (cipher_1_encrypt_model_out*)model_out;

  result = cipher_v1i1_encrypt(
      in->context, in->src, in->src_size, in->dst_buffer_size, out->dst, out->dst_size);

  /*
   * The user code ends here.
   * ==================
   */

  return result;
}

static az_result cipher_1_decrypt_concrete(az_ulib_model_in model_in, az_ulib_model_out model_out)
{
  az_result result;

  /*
   * ==================
   * The user code starts here.
   */
  cipher_1_decrypt_model_in* in = (cipher_1_decrypt_model_in*)model_in;
  cipher_1_decrypt_model_out* out = (cipher_1_decrypt_model_out*)model_out;

  result = cipher_v1i1_decrypt(in->src, in->src_size, in->dst_buffer_size, out->dst, out->dst_size);

  /*
   * The user code ends here.
   * ==================
   */

  return result;
}

AZ_ULIB_DESCRIPTOR_CREATE(
    CIPHER_1_DESCRIPTOR,
    CIPHER_1_INTERFACE_NAME,
    CIPHER_1_INTERFACE_VERSION,
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND(
        CIPHER_1_INTERFACE_ENCRYPT_COMMAND_NAME,
        cipher_1_encrypt_concrete),
    AZ_ULIB_DESCRIPTOR_ADD_COMMAND(
        CIPHER_1_INTERFACE_DECRYPT_COMMAND_NAME,
        cipher_1_decrypt_concrete));

az_result publish_cipher_v1i1_interface(void)
{
  return az_ulib_ipc_publish(&CIPHER_1_DESCRIPTOR, NULL);
}

az_result unpublish_cipher_v1i1_interface(void)
{
#ifdef AZ_ULIB_CONFIG_IPC_UNPUBLISH
  return az_ulib_ipc_unpublish(&CIPHER_1_DESCRIPTOR, AZ_ULIB_NO_WAIT);
#else
  return AZ_OK;
#endif // AZ_ULIB_CONFIG_IPC_UNPUBLISH
}
