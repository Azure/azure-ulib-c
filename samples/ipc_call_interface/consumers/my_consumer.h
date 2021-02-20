// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef MY_CONSUMER_H
#define MY_CONSUMER_H

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

  void my_consumer_create(void);
  void my_consumer_destroy(void);
  void my_consumer_do_cipher(uint32_t context);

#ifdef __cplusplus
}
#endif

#endif /* MY_CONSUMER_H */
