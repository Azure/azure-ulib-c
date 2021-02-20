// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef CIPHER_V2I1_H
#define CIPHER_V2I1_H

#ifdef __cplusplus
extern "C"
{
#endif

  void cipher_v2i1_create(void);
  void cipher_v2i1_destroy(void);

  az_result cipher_v2i1_encrypt(
      uint32_t context,
      const char* const src,
      uint32_t src_size,
      uint32_t dst_buffer_size,
      char* dst,
      uint32_t* dst_size);

  az_result cipher_v2i1_decrypt(
      const char* const src,
      uint32_t src_size,
      uint32_t dst_buffer_size,
      char* dst,
      uint32_t* dst_size);

#ifdef __cplusplus
}
#endif

#endif /* CIPHER_V2I1_H */
