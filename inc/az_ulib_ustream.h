// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/**
 * @file az_ulib_ustream.h
 *
 * @brief ustream implementation for local memory
 */

#ifndef AZ_ULIB_USTREAM_H
#define AZ_ULIB_USTREAM_H

#include "az_ulib_ustream_base.h"

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
 * @brief   Factory to initialize a new ustream.
 *
 *  This factory initializes a ustream that handles the content of the provided buffer. As a
 *      result, it will return an #az_ulib_ustream* with this content. The initialized ustream
 *      takes ownership of the passed memory and will release the memory with the passed
 *      #az_ulib_release_callback function when the ref count of the
 *      `ustream_control_block` goes to zero.
 *
 * @param[out]      ustream_instance        The pointer to the allocated #az_ulib_ustream struct.
 *                                          This memory must be valid from the time
 *                                          az_ulib_ustream_init() is called through
 *                                          az_ulib_ustream_dispose(). The ustream will not free
 *                                          this struct and it is the responsibility of the
 *                                          developer to make sure it is valid during the time
 *                                          frame described above. It cannot be `NULL`.
 * @param[in]       ustream_control_block   The pointer to the allocated #az_ulib_ustream_data_cb
 *                                          struct. This memory should be allocated in a way that
 *                                          it stays valid until the passed
 *                                          `control_block_release` is called at some
 *                                          (potentially) unknown time in the future. It cannot
 *                                          be `NULL`.
 * @param[in]       control_block_release   The #az_ulib_release_callback function that will be
 *                                          called to release the control block (the passed
 *                                          `ustream_control_block` parameter) once all the
 *                                          references to the ustream are diposed. If `NULL`
 *                                          is passed, the data is assumed to be constant with no
 *                                          need to be free'd. In other words, there is no need
 *                                          for notification that the memory may be released.
 *                                          As a default, developers may use the stdlib `free`
 *                                          to release malloc'd memory.
 * @param[in]       data_buffer             The `const uint8_t* const` that points to a memory
 *                                          position where the buffer starts. It cannot be `NULL`.
 * @param[in]       data_buffer_length      The `size_t` with the number of `uint8_t` in the
 *                                          provided buffer. It shall be larger than zero.
 * @param[in]       data_buffer_release     The #az_ulib_release_callback function that will be
 *                                          called to release the data once all the references to
 *                                          the ustream are disposed. If `NULL` is passed, the
 *                                          data is assumed to be constant with no need to be
 *                                          free'd. In other words, there is no need for
 *                                          notification that the memory may be released. As a
 *                                          default, developers may use the stdlib `free` to
 *                                          release malloc'd memory.
 *
 * @return The #az_result with result of the initialization.
 *      @retval #AZ_OK                        If the #az_ulib_ustream* is successfully
 *                                            initialized.
 *      @retval #AZ_ERROR_ARG                 If one of the provided parameters is
 *                                            invalid.
 */
AZ_NODISCARD az_result az_ulib_ustream_init(
    az_ulib_ustream* ustream_instance,
    az_ulib_ustream_data_cb* ustream_control_block,
    az_ulib_release_callback control_block_release,
    const uint8_t* const data_buffer,
    size_t data_buffer_length,
    az_ulib_release_callback data_buffer_release);

/**
 * @brief   Concatenate a ustream to the existing ustream.
 *
 *  The concat will effectively append a ustream at the end of the passed `ustream_instance`.
 *     To do that, the concat will copy the `ustream_instance` into a
 *     `az_ulib_ustream_multi_data_cb` and clone the `ustream_to_concat` inside the
 *     `az_ulib_ustream_multi_data_cb`. When returned, the original `ustream_instance` will
 *     point to the #az_ulib_ustream_data_cb inside of the passed `multi_data`. The data of the
 *     passed `ustream_instance` will at that point be read as if the content of the original
 *     `ustream_instance` and `ustream_to_concat` were one ustream. This means that both
 *     `ustream_instance` and `ustream_to_concat` will have to be disposed by the calling
 *     function.
 *
 * @param[in,out]  ustream_instance        The #az_ulib_ustream* with the interface of the
 *                                         ustream. It cannot be `NULL`, and it shall be a
 *                                         valid ustream.
 * @param[in]      ustream_to_concat       The #az_ulib_ustream* with the interface of the
 *                                         ustream to concat to `ustream_instance`. It cannot
 *                                         be `NULL`, and it shall be a valid ustream.
 * @param[in]      multi_data              The #az_ulib_ustream_multi_data_cb* pointing to the
 *                                         allocated multi data control block. It must be
 *                                         allocated in a way that it remains a valid address
 *                                         until the passed `multi_data_release` is invoked some
 *                                         time in the future.
 * @param[in]      multi_data_release      The #az_ulib_release_callback callback which will be
 *                                         called once the number of references to the control
 *                                         block reaches zero. It may be `NULL` if no future
 *                                         cleanup is needed.
 * @return The #az_result with the result of the `concat` operation.
 *     @retval #AZ_OK                         If the az_ulib_ustream is concatenated with
 *                                            success.
 *     @retval #AZ_ERROR_ARG                  If one of the provided parameters is invalid.
 */
AZ_NODISCARD az_result az_ulib_ustream_concat(
    az_ulib_ustream* ustream_instance,
    az_ulib_ustream* ustream_to_concat,
    az_ulib_ustream_multi_data_cb* multi_data,
    az_ulib_release_callback multi_data_release);

/**
 * @brief   Split a ustream at a given position.
 *
 *  The split will divide a given ustream into two parts, divided at the passed `split_pos`. The
 *     result of this operation will be two ustream instances. The first and original ustream
 *     will contain the data from the original first valid position up to but not including the
 *     passed `split_pos`. Its current position will be the same as it was when passed. The second
 *     ustream will contain the data from the passed `split_pos` up to the end of the
 *     `ustream_instance` before the function was called. It's current position will be the same
 *     as `split_pos`. The `split_pos` should be relative to the position returned from
 *     az_ulib_ustream_get_position() and the remaining size of `ustream_instance`.
 *
 * @param[in,out]  ustream_instance        The #az_ulib_ustream* with the interface of the
 *                                         ustream. It cannot be `NULL`, and it shall be a valid
 *                                         ustream.
 * @param[out]     ustream_instance_split  The #az_ulib_ustream* with the interface of the
 *                                         ustream which will contain the controls to the data
 *                                         after `split_pos`. It cannot be `NULL`.
 * @param[in]      split_pos               The `offset_t` at which to split the passed
 *                                         `ustream_instance`. It cannot be equal to the current
 *                                         position of `ustream_instance` or equal to the current
 *                                         position + remaining size. In other words, resulting
 *                                         ustreams of size 0 are not allowed.
 *
 * @return The #az_result with the result of the `concat` operation.
 *     @retval #AZ_OK                         If the az_ulib_ustream is split with success.
 *     @retval #AZ_ERROR_ARG                  If one of the provided parameters is invalid.
 *     @retval #AZ_ERROR_ITEM_NOT_FOUND       If the input `split_pos` is outside the allowed
 *                                            range.
 */
AZ_NODISCARD az_result az_ulib_ustream_split(
    az_ulib_ustream* ustream_instance,
    az_ulib_ustream* ustream_instance_split,
    offset_t split_pos);

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_USTREAM_H */
