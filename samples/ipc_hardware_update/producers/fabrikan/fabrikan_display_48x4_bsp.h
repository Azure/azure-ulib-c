// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef FABRIKAN_DISPLAY_48x4_BSP_H
#define FABRIKAN_DISPLAY_48x4_BSP_H

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

  void fabrikan_display_48x4_bsp_create(void);
  void fabrikan_display_48x4_bsp_destroy(void);

  void fabrikan_display_48x4_bsp_cls(void);
  void fabrikan_display_48x4_bsp_goto(int32_t x, int32_t y);
  void fabrikan_display_48x4_bsp_print(const char* buf, int32_t size);
  void fabrikan_display_48x4_bsp_invalidate(void);

#ifdef __cplusplus
}
#endif

#endif /* FABRIKAN_DISPLAY_48x4_BSP_H */
