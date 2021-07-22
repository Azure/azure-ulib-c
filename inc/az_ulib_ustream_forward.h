// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/**
 * @file az_ulib_ustream_forward.h
 *
 * @brief ustream_forward implementation for local memory
 */

#ifndef AZ_ULIB_ustream_forward_H
#define AZ_ULIB_ustream_forward_H

#include "az_ulib_ustream_forward_base.h"

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#else
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#endif /* __cplusplus */

#include "azure/core/_az_cfg_prefix.h"

/**
 * @brief   Factory to initialize a new ustream_forward.
 *
 *  This factory initializes a ustream_forward that handles the content of the provided buffer. As a
 *      result, it will return an #az_ulib_ustream_forward* with this content. The initialized 
 *      ustream_forward takes ownership of the passed memory and will release the memory with the passed
 *      #az_ulib_release_callback function when the caller goes out of scope.
 *
 * @param[out]      ustream_forward             The pointer to the allocated #az_ulib_ustream_forward 
 *                                              struct. This memory must be valid from the time
 *                                              az_ulib_ustream_forward_init() is called through
 *                                              az_ulib_ustream_forward_dispose(). The ustream_forward 
 *                                              will not free this struct and it is the responsibility 
 *                                              of the developer to make sure it is valid during the 
 *                                              time frame described above. It cannot be `NULL`.
 * @param[in]       ustream_forward_release     The #az_ulib_release_callback function that will be
 *                                              called to release the ustream_forward block (the passed
 *                                              `ustream_forward` parameter) once all the references
 *                                              to the ustream_forward are disposed. If `NULL`
 *                                              is passed, the data is assumed to be constant with no
 *                                              need to be freed. In other words, there is no need
 *                                              for notification that the memory may be released.
 *                                              As a default, developers may use the stdlib `free`
 *                                              to release malloc'd memory.
 * @param[in]       data_buffer                 The `const uint8_t* const` that points to a memory
 *                                              position where the buffer starts.
 * @param[in]       data_buffer_length          The `size_t` with the number of `uint8_t` in the
 *                                              provided buffer.
 * @param[in]       data_buffer_release         The #az_ulib_release_callback function that will be
 *                                              called to release the data once all the references to
 *                                              the ustream_forward are disposed. If `NULL` is passed, 
 *                                              the data is assumed to be constant with no need to be
 *                                              free'd. In other words, there is no need for
 *                                              notification that the memory may be released. As a
 *                                              default, developers may use the stdlib `free` to
 *                                              release malloc'd memory.
 * 
 * @pre \p          ustream_forward             shall not be `NULL`.
 * @pre \p          data_buffer                 shall not be `NULL`.
 * @pre \p          data_buffer_length          shall be greater than `0`.
 * 
 * @return The #az_result with result of the initialization.
 *      @retval #AZ_OK                          If the #az_ulib_ustream_forward* is successfully
 *                                              initialized.
 */
AZ_NODISCARD az_result az_ulib_ustream_forward_init(
    az_ulib_ustream_forward* ustream_forward,
    az_ulib_release_callback ustream_forward_release,
    const uint8_t* const data_buffer,
    size_t data_buffer_length,
    az_ulib_release_callback data_buffer_release);

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_ustream_forward_H */
