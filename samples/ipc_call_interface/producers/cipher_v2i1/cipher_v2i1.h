// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef CIPHER_V2I1_H
#define CIPHER_V2I1_H

#include "az_ulib_result.h"
#include "azure/az_core.h"

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

  void cipher_v2i1_create(void);
  void cipher_v2i1_destroy(void);

  int8_t cipher_v2i1_alpha_get(void);
  void cipher_v2i1_alpha_set(const int8_t val);

  uint32_t cipher_v2i1_delta_get(void);
  void cipher_v2i1_delta_set(const uint32_t val);

  az_result cipher_v2i1_encrypt(uint32_t algorithm, az_span src, az_span* dest);

  az_result cipher_v2i1_decrypt(az_span src, az_span* dest);

#ifdef __cplusplus
}
#endif

#endif /* CIPHER_V2I1_H */
