// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "contoso_display_20x4_bsp.h"
#include "az_ulib_result.h"

#include <memory.h>
#include <stddef.h>
#include <stdio.h>

#define MAX_X (size_t)20
#define MAX_Y (size_t)4

int32_t contoso_display_20x4_bsp_get_max_x(void) { return MAX_X; }
int32_t contoso_display_20x4_bsp_get_max_y(void) { return MAX_Y; }

typedef struct display_cb_tag
{
  int32_t x;
  int32_t y;
  char mem[MAX_Y][MAX_X];
} display_cb;

static display_cb cb;

static void print_line(uint32_t line_number)
{
  char line[MAX_X + 1];

  memcpy(line, cb.mem[line_number], MAX_X);
  line[MAX_X] = '\0';
  (void)printf("        |%s|\r\n", line);
}

void contoso_display_20x4_bsp_invalidate(void)
{
  (void)printf("        +Contoso emulator----+\r\n");
  for (uint32_t i = 0; i < MAX_Y; i++)
  {
    print_line(i);
  }
  (void)printf("        +--------------------+\r\n");
}

void contoso_display_20x4_bsp_cls(void)
{
  for (uint32_t line_number = 0; line_number < MAX_Y; line_number++)
  {
    memset(cb.mem[line_number], ' ', MAX_X);
  }
}

void contoso_display_20x4_bsp_goto(int32_t x, int32_t y)
{
  cb.x = x;
  cb.y = y;
}

void contoso_display_20x4_bsp_print(const char* buf, size_t size)
{
  uint32_t pos_x;
  uint32_t pos_y;
  bool shall_print = true;
  if (cb.x < 0)
  {
    pos_x = (uint32_t)(-cb.x);
    if (size > pos_x)
    {
      buf += pos_x;
      size -= pos_x;
      pos_x = 0;
    }
    else
    {
      shall_print = false;
    }
  }
  else
  {
    pos_x = (uint32_t)(cb.x);
  }

  if (cb.y < 0)
  {
    pos_y = 0;
    shall_print = false;
  }
  else
  {
    pos_y = (uint32_t)cb.y;
  }

  if (shall_print && (pos_x < MAX_X) && (pos_y < MAX_Y))
  {
    size_t eol = MAX_X - pos_x;
    size_t copy_lenght = (size < eol) ? size : eol;

    memcpy(&(cb.mem[pos_y][pos_x]), buf, copy_lenght);
  }
}

void contoso_display_20x4_bsp_create(void)
{
  cb.x = 0;
  cb.y = 0;
}

void contoso_display_20x4_bsp_destroy(void) {}
