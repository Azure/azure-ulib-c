// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef FABRIKAN_DISPLAY_48x4_BSP_H
#define FABRIKAN_DISPLAY_48x4_BSP_H

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
extern "C"
{
#else
#include <stddef.h>
#include <stdint.h>
#endif

  void fabrikan_display_48x4_bsp_create(void);
  void fabrikan_display_48x4_bsp_destroy(void);

  void fabrikan_display_48x4_bsp_cls(void);
  void fabrikan_display_48x4_bsp_goto(int32_t x, int32_t y);
  void fabrikan_display_48x4_bsp_print(const char* buf, size_t size);
  void fabrikan_display_48x4_bsp_invalidate(void);

  int32_t fabrikan_display_48x4_bsp_get_max_x(void);
  int32_t fabrikan_display_48x4_bsp_get_max_y(void);

#ifdef __cplusplus
}
#endif

#endif /* FABRIKAN_DISPLAY_48x4_BSP_H */
