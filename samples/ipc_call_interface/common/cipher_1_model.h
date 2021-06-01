// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from cipher v1 DL and shall not be
 * modified.
 ********************************************************************/

#ifndef CIPHER_1_MODEL_H
#define CIPHER_1_MODEL_H

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

/*
 * interface definition
 */
#define CIPHER_1_INTERFACE_NAME "cipher"
#define CIPHER_1_INTERFACE_VERSION 1
#define CIPHER_1_CAPABILITY_SIZE 4

/*
 * Define encrypt command on cipher interface.
 */
#define CIPHER_1_ENCRYPT_COMMAND (az_ulib_capability_index)0
#define CIPHER_1_ENCRYPT_COMMAND_NAME "encrypt"
#define CIPHER_1_ENCRYPT_ALGORITHM_NAME "algorithm"
#define CIPHER_1_ENCRYPT_SRC_NAME "src"
#define CIPHER_1_ENCRYPT_DEST_NAME "dest"
  typedef struct
  {
    uint32_t algorithm;
    az_span src;
  } cipher_1_encrypt_model_in;
  typedef struct
  {
    az_span* dest;
  } cipher_1_encrypt_model_out;

/*
 * Define decrypt command on cipher interface.
 */
#define CIPHER_1_DECRYPT_COMMAND (az_ulib_capability_index)1
#define CIPHER_1_DECRYPT_COMMAND_NAME "decrypt"
#define CIPHER_1_DECRYPT_SRC_NAME "src"
#define CIPHER_1_DECRYPT_DEST_NAME "dest"
  typedef struct
  {
    az_span src;
  } cipher_1_decrypt_model_in;
  typedef struct
  {
    az_span* dest;
  } cipher_1_decrypt_model_out;

/*
 * Define alpha property on cipher interface.
 */
#define CIPHER_1_ALPHA_PROPERTY (az_ulib_capability_index)2
#define CIPHER_1_ALPHA_PROPERTY_NAME "alpha"
  typedef int8_t cipher_1_alpha_model;

/*
 * Define delta property on cipher interface.
 */
#define CIPHER_1_DELTA_PROPERTY (az_ulib_capability_index)3
#define CIPHER_1_DELTA_PROPERTY_NAME "delta"
  typedef uint32_t cipher_1_delta_model;

#ifdef __cplusplus
}
#endif

#endif /* CIPHER_1_MODEL_H */
