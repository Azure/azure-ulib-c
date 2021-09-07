// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "contoso_display_480401.h"
#include "az_ulib_result.h"
#include "display_1_capabilities.h"
#include "display_1_model.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>

#define CONTOSO_PACKAGE_NAME "contoso"
#define CONTOSO_PACKAGE_VERSION 480401

#define MAX_X (size_t)48
#define MAX_Y (size_t)4

static const az_ulib_interface_descriptor DISPLAY_1_DESCRIPTOR = AZ_ULIB_DESCRIPTOR_CREATE(
    CONTOSO_PACKAGE_NAME,
    CONTOSO_PACKAGE_VERSION,
    DISPLAY_1_INTERFACE_NAME,
    DISPLAY_1_INTERFACE_VERSION,
    DISPLAY_1_CAPABILITIES);

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

static az_result display_1_max_x_concrete(
    const display_1_max_x_model* const in,
    display_1_max_x_model* out)
{
  if (in != NULL)
  {
    return AZ_ERROR_NOT_SUPPORTED;
  }

  *out = MAX_X;

  return AZ_OK;
}

static az_result display_1_max_y_concrete(
    const display_1_max_y_model* const in,
    display_1_max_y_model* out)
{
  if (in != NULL)
  {
    return AZ_ERROR_NOT_SUPPORTED;
  }

  *out = MAX_Y;

  return AZ_OK;
}

static az_result display_1_invalidate_concrete(
    const display_1_invalidate_model_in* const in,
    display_1_invalidate_model_out* out)
{
  (void)in;
  (void)out;

  (void)printf("        +Contoso display emulator------------------------+\r\n");
  for (uint32_t i = 0; i < MAX_Y; i++)
  {
    print_line(i);
  }
  (void)printf("        +------------------------------------------------+\r\n");

  return AZ_OK;
}

static az_result display_1_cls_concrete(
    const display_1_cls_model_in* const in,
    display_1_cls_model_out* out)
{
  (void)in;
  (void)out;

  for (uint32_t line_number = 0; line_number < MAX_Y; line_number++)
  {
    memset(cb.mem[line_number], ' ', MAX_X);
  }

  return AZ_OK;
}

static az_result display_1_print_concrete(
    const display_1_print_model_in* const in,
    display_1_print_model_out* out)
{
  (void)out;

  uint32_t pos_x;
  uint32_t pos_y;
  const char* buf = in->buffer;
  size_t size = in->size;
  bool shall_print = true;

  cb.x = in->x;
  cb.y = in->y;

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

  return AZ_OK;
}

void contoso_display_480401_create(void)
{
  az_result result;

  cb.x = 0;
  cb.y = 0;

  if ((result = az_ulib_ipc_publish(&DISPLAY_1_DESCRIPTOR)) != AZ_OK)
  {
    (void)printf(
        "Contoso published display 48x4 interface failed with error %" PRIi32 "\r\n", result);
  }
  else
  {
    (void)printf("Contoso published display 48x4 interface with success\r\n");
  }
}

void contoso_display_480401_destroy(void)
{
  az_result result;

  if ((result = az_ulib_ipc_unpublish(&DISPLAY_1_DESCRIPTOR, AZ_ULIB_WAIT_FOREVER)) != AZ_OK)
  {
    (void)printf(
        "Contoso unpublished display 48x4 interface failed with error %" PRIi32 "\r\n", result);
  }
  else
  {
    (void)printf("Destroy Contoso producer for display 48x4.\r\n");
  }
}
