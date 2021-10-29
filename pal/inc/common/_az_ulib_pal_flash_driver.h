// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 *
 * @brief   [**INTERNAL ONLY**]Flash driver.
 *
 * This is a driver to write data into FLASH memory. Users should have a thorough understanding of
 * their device's FLASH layout, usage patterns, and limitations. Misuse of the FLASH API may cause
 * catastrophic and unrecoverable failure.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure DCF's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the DCF which would break your code.
 */

#ifndef _az_ULIB_PAL_FLASH_DRIVER_H
#define _az_ULIB_PAL_FLASH_DRIVER_H

#include "az_ulib_result.h"

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

  /**
   * @brief   [**INTERNAL ONLY**]Control block to write in the Flash.
   *
   * The driver to write data into the flash memory.
   */
  typedef struct
  {
    /** Buffer to concatenate 8 byte in a uint64. */
    union
    {
      /** Buffer in uint64. */
      uint64_t uint64;

      /** Buffer in 8 x uint8. */
      uint8_t uint8[8];
    } write_buffer;

    /** Number of reminder bytes in the write_buffer. */
    uint32_t remainder_count;

    /** Flash address to write the data. */
    uint64_t* destination_ptr;
  } _az_ulib_pal_flash_driver_control_block;

  /**
   * @brief [**INTERNAL ONLY**]Write 64 bits in the flash.
   *
   * This is a standalone function to write 64 bits, or a doubleword, into the internal MCU flash.
   * This function assumes that the flash is already erased.
   *
   * @param[in]     destination_ptr       The pointer to `uint64_t` to write the 64 bits.
   * @param[in]     value                 The `uint64_t` with the value to write.
   *
   * @return The #az_result with the result of the write in the flash.
   *      @retval #AZ_OK                        If write in the flash was successful.
   *      @retval #AZ_ERROR_ULIB_SYSTEM         If there are generic error from the HAL layer.
   *      @retval #AZ_ERROR_ULIB_BUSY           If the HAL layer is busy.
   *      @retval #AZ_ERROR_ULIB_TIMEOUT        If the HAL layer is throw a timeout error.
   */
  az_result _az_ulib_pal_flash_driver_write_64(uint64_t* destination_ptr, uint64_t value);

  /**
   * @brief [**INTERNAL ONLY**]Erase multiple bytes in the flash.
   *
   * This is a standalone function to erase pages of flash starting from the `destination_ptr`, and
   * as many pages as `size` bytes may need. This function will align the `destination_ptr` and
   * `size` to the pages in the MCU flash.
   *
   * To keep the page alignment, this API may erase bytes before `destination_ptr` and the amount of
   * bytes may be bigger than `size`.
   *
   * @param[in]     destination_ptr       The pointer to `uint64_t` to erase.
   * @param[in]     size                  The `uint32_t` with the number of bytes to erase.
   *
   * @return The #az_result with the result of the erase flash.
   *      @retval #AZ_OK                        If erase flash was successful.
   *      @retval #AZ_ERROR_ULIB_SYSTEM         If there are generic error from the HAL layer.
   *      @retval #AZ_ERROR_ULIB_BUSY           If the HAL layer is busy.
   *      @retval #AZ_ERROR_ULIB_TIMEOUT        If the HAL layer is throw a timeout error.
   */
  az_result _az_ulib_pal_flash_driver_erase(uint64_t* destination_ptr, uint32_t size);

  /**
   * @brief [**INTERNAL ONLY**]Open flash to write on it.
   *
   * This function initialize the flash_cb, preparing the flash to write a buffer larger than 64
   * bits. For buffers smaller than 64 bits, refer to _az_ulib_pal_flash_driver_write_64(). This
   * function *assumes* that the flash is already erased. See _az_ulib_pal_flash_driver_erase() to
   * erase the flash.
   *
   * @param[in,out] flash_cb              The pointer to #_az_ulib_pal_flash_driver_control_block
   *                                      with the flash driver control block. It cannot be NULL.
   * @param[in]     destination_ptr       The pointer to `uint64_t` to write the data.
   *
   * @return The #az_result with the result of the open flash.
   *      @retval #AZ_OK                        If open flash was successful.
   */
  az_result _az_ulib_pal_flash_driver_open(
      _az_ulib_pal_flash_driver_control_block* flash_cb,
      uint64_t* destination_ptr);

  /**
   * @brief [**INTERNAL ONLY**]Write data to flash.
   *
   * This function write data in the flash controlled by the flash_cb. See
   * _az_ulib_pal_flash_driver_open() to initialize the flash_cb.
   *
   * @param[in,out] flash_cb              The pointer to #_az_ulib_pal_flash_driver_control_block
   *                                      with the flash driver control block. It cannot be NULL.
   * @param[in]     source_ptr            The pointer to `uint8_t` to with the source data to write.
   * @param[in]     size                  The `uint32_t` to with the number of bytes to write.
   *
   * @return The #az_result with the result of the write to flash.
   *      @retval #AZ_OK                        If write to flash was successful.
   *      @retval #AZ_ERROR_ULIB_SYSTEM         If there are generic error from the HAL layer.
   *      @retval #AZ_ERROR_ULIB_BUSY           If the HAL layer is busy.
   *      @retval #AZ_ERROR_ULIB_TIMEOUT        If the HAL layer is throw a timeout error.
   */
  az_result _az_ulib_pal_flash_driver_write(
      _az_ulib_pal_flash_driver_control_block* flash_cb,
      uint8_t* source_ptr,
      uint32_t size);

  /**
   * @brief [**INTERNAL ONLY**]Close the write data to flash.
   *
   * This function write any pending data in the flash controlled by the flash_cb and close the
   * flash_cb.
   *
   * @param[in,out] flash_cb              The pointer to #_az_ulib_pal_flash_driver_control_block
   *                                      with the flash driver control block. It cannot be NULL.
   * @param[in]     pad                   The `uint8_t` to pad the end of the memory.
   *
   * @return The #az_result with the result of the close flash.
   *      @retval #AZ_OK                        If close flash was successful.
   *      @retval #AZ_ERROR_ULIB_SYSTEM         If there are generic error from the HAL layer.
   *      @retval #AZ_ERROR_ULIB_BUSY           If the HAL layer is busy.
   *      @retval #AZ_ERROR_ULIB_TIMEOUT        If the HAL layer is throw a timeout error.
   */
  az_result _az_ulib_pal_flash_driver_close(
      _az_ulib_pal_flash_driver_control_block* flash_cb,
      uint8_t pad);

#ifdef __cplusplus
}
#endif

#endif /* _az_ULIB_PAL_FLASH_DRIVER_H */
