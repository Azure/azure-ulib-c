// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "_az_ulib_pal_flash_driver.h"
#include "az_ulib_result.h"

#include <stdint.h>

az_result _az_ulib_pal_flash_driver_write_64(uint64_t* destination_ptr, uint64_t source)
{
  *destination_ptr = source;
  return AZ_OK;
}

az_result _az_ulib_pal_flash_driver_erase(uint64_t* destination_ptr, uint32_t size)
{
  for (uint8_t* runner = (uint8_t*)destination_ptr; runner < ((uint8_t*)destination_ptr + size);
       runner++)
  {
    *runner = 0xFF;
  }
  return AZ_OK;
}

az_result _az_ulib_pal_flash_driver_open(
    _az_ulib_pal_flash_driver_control_block* flash_cb,
    uint64_t* destination_ptr)
{
  flash_cb->destination_ptr = destination_ptr;
  flash_cb->remainder_count = 0;
  return AZ_OK;
}

az_result _az_ulib_pal_flash_driver_write(
    _az_ulib_pal_flash_driver_control_block* flash_cb,
    uint8_t* source_ptr,
    uint32_t size)
{
  for (uint32_t i = 0; i < size; i++)
  {
    flash_cb->write_buffer.uint8[flash_cb->remainder_count++] = *source_ptr++;
    if (flash_cb->remainder_count == 8)
    {
      *flash_cb->destination_ptr++ = flash_cb->write_buffer.uint64;
      flash_cb->remainder_count = 0;
    }
  }
  return AZ_OK;
}

az_result _az_ulib_pal_flash_driver_close(
    _az_ulib_pal_flash_driver_control_block* flash_cb,
    uint8_t pad)
{
  if (flash_cb->remainder_count != 0)
  {
    for (uint32_t i = flash_cb->remainder_count; i < 8; i++)
    {
      flash_cb->write_buffer.uint8[i] = pad;
    }
    *flash_cb->destination_ptr = flash_cb->write_buffer.uint64;
  }
  return AZ_OK;
}
