// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef CONTOSO_DISPLAY_20x4_BSP_H
#define CONTOSO_DISPLAY_20x4_BSP_H

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
extern "C"
{
#else
#include <stddef.h>
#include <stdint.h>
#endif

  void contoso_display_20x4_bsp_create(void);
  void contoso_display_20x4_bsp_destroy(void);

  void contoso_display_20x4_bsp_cls(void);
  void contoso_display_20x4_bsp_goto(int32_t x, int32_t y);
  void contoso_display_20x4_bsp_print(const char* buf, size_t size);
  void contoso_display_20x4_bsp_invalidate(void);

  int32_t contoso_display_20x4_bsp_get_max_x(void);
  int32_t contoso_display_20x4_bsp_get_max_y(void);

#ifdef __cplusplus
}
#endif

#endif /* CONTOSO_DISPLAY_20x4_BSP_H */
