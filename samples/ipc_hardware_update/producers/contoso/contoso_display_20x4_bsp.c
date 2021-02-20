// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_result.h"
#include <memory.h>
#include <stdio.h>

#define MAX_X 20
#define MAX_Y 4

typedef struct display_cb_tag
{
  int32_t x;
  int32_t y;
  char mem[MAX_Y][MAX_X];
} display_cb;

static display_cb cb;

static void print_line(int32_t line_number)
{
  char line[MAX_X + 1];

  memcpy(line, cb.mem[line_number], MAX_X);
  line[MAX_X] = '\0';
  (void)printf("        |%s|\r\n", line);
}

void contoso_display_20x4_bsp_invalidate(void)
{
  (void)printf("        +Contoso emulator----+\r\n");
  for (int i = 0; i < MAX_Y; i++)
  {
    print_line(i);
  }
  (void)printf("        +--------------------+\r\n");
}

void contoso_display_20x4_bsp_cls(void)
{
  for (int line_number = 0; line_number < MAX_Y; line_number++)
  {
    memset(cb.mem[line_number], ' ', MAX_X);
  }
}

void contoso_display_20x4_bsp_goto(int32_t x, int32_t y)
{
  cb.x = x;
  cb.y = y;
}

void contoso_display_20x4_bsp_print(const char* buf, int32_t size)
{
  if ((cb.x < MAX_X) && (cb.y < MAX_Y))
  {
    int32_t eol = MAX_X - cb.x;
    int32_t copy_lenght = (size < eol) ? size : eol;

    memcpy(&(cb.mem[cb.y][cb.x]), buf, copy_lenght);
  }
}

void contoso_display_20x4_bsp_create(void)
{
  cb.x = 0;
  cb.y = 0;
}

void contoso_display_20x4_bsp_destroy(void) {}
