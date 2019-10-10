// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file ustream.h
 * 
 * @brief ustream implementation for local memory
 */

#ifndef AZ_USTREAM_H
#define AZ_USTREAM_H

#include "ustream_base.h"

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
#include <cstdint>
#include <cstddef>
extern "C" {
#else
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#endif /* __cplusplus */

/**
 * @brief   Factory to initialize a new ustream.
 *
 *  This factory initializes a ustream that handles the content of the provided buffer. As a result,
 *      it will return an {@link AZ_USTREAM}* with this content. The initialized ustream takes ownership of the
 *      passed memory and will release the memory with the passed {@link AZ_RELEASE_CALLBACK} function when
 *      the ref count of the <tt>ustream_control_block</tt> goes to zero.
 *
 * @param[out]      ustream_instance        The pointer to the allocated #AZ_USTREAM struct. This memory must be valid from
 *                                          the time az_ustream_init() is called through az_ustream_dispose(). The ustream will not
 *                                          free this struct and it is the responsibility of the developer to make sure it is valid during
 *                                          the time frame described above. It cannot be <tt>NULL</tt>.
 * @param[in]       ustream_control_block   The pointer to the allocated #AZ_USTREAM_DATA_CB struct. This memory should be allocated in
 *                                          a way that it stays valid until the passed <tt>control_block_release</tt> is called
 *                                          at some (potentially) unknown time in the future. It cannot be <tt>NULL</tt>.
 * @param[in]       control_block_release   The {@link AZ_RELEASE_CALLBACK} function that will be called to release the
 *                                          control block (the passed <tt>ustream_control_block</tt> parameter) once all the 
 *                                          references to the ustream are diposed. If <tt>NULL</tt> is passed, the data is assumed to 
 *                                          be constant with no need to be free'd. In other words, there is no need for notification 
 *                                          that the memory may be released.
 *                                          As a default, developers may use the stdlib <tt>free</tt> to release malloc'd memory.
 * @param[in]       data_buffer             The <tt>const uint8_t* const</tt> that points to a memory position where the buffer starts.
 *                                          It cannot be <tt>NULL</tt>.
 * @param[in]       data_buffer_length      The <tt>size_t</tt> with the number of <tt>uint8_t</tt> in the provided buffer.
 *                                          It shall be larger than zero.
 * @param[in]       data_buffer_release     The {@link AZ_RELEASE_CALLBACK} function that will be called to release the data 
 *                                          once all the references to the ustream are disposed. If <tt>NULL</tt> is 
 *                                          passed, the data is assumed to be constant with no need to be free'd. In other words, 
 *                                          there is no need for notification that the memory may be released.
 *                                          As a default, developers may use the stdlib <tt>free</tt> to release malloc'd memory.
 *
 * @return The {@link AZ_ULIB_RESULT} with result of the initialization.
 *          @retval     AZ_ULIB_SUCCESS                          If the {@link AZ_USTREAM}* is successfully initialized.
 *          @retval     AZ_ULIB_ILLEGAL_ARGUMENT_EXCEPTION       If one of the provided parameters is invalid.
 */
MOCKABLE_FUNCTION(, AZ_ULIB_RESULT, az_ustream_init,
        AZ_USTREAM*, ustream_instance,
        AZ_USTREAM_DATA_CB*, ustream_control_block,
        AZ_RELEASE_CALLBACK, control_block_release,
        const uint8_t* const, data_buffer,
        size_t, data_buffer_length,
        AZ_RELEASE_CALLBACK, data_buffer_release);


/**
  * @brief   Concatenate a ustream to the existing ustream.
  *
  *  The concat will effectively append a ustream at the end of the passed <tt>ustream_instance</tt>. To do that, the 
  *     concat will copy the <tt>ustream_instance</tt> into a <tt>AZ_USTREAM_MULTI_DATA_CB</tt> and clone the 
  *     <tt>ustream_to_concat</tt> inside the <tt>AZ_USTREAM_MULTI_DATA_CB</tt>. When returned, the original 
  *     <tt>ustream_instance</tt> will point to the #AZ_USTREAM_DATA_CB inside of the passed <tt>multi_data</tt>. 
  *     The data of the passed <tt>ustream_instance</tt> will at that point be read as if the content of the orignal
  *     <tt>ustream_instance</tt> and <tt>ustream_to_concat</tt> were one ustream. This means that both <tt>ustream_instance</tt>
  *     and <tt>ustream_to_concat</tt> will have to be disposed by the calling function.
  *
  * @param[in,out]      ustream_instance        The {@link AZ_USTREAM}* with the interface of 
  *                                             the ustream. It cannot be <tt>NULL</tt>, and it shall be a valid ustream.
  * @param[in]          ustream_to_concat       The {@link AZ_USTREAM}* with the interface of 
  *                                             the ustream to concat to <tt>ustream_instance</tt>. It cannot be <tt>NULL</tt>, 
  *                                             and it shall be a valid ustream.
  * @param[in]          multi_data              The {@link AZ_USTREAM_MULTI_DATA_CB}* pointing to the allocated multi data control block.
  *                                             It must be allocated in a way that it remains a valid address until the passed
  *                                             <tt>multi_data_release</tt> is invoked some time in the future.
  * @param[in]          multi_data_release      The {@link AZ_RELEASE_CALLBACK} callback which will be called once
  *                                             the number of references to the control block reaches zero. It may be <tt>NULL</tt> if no 
  *                                             future cleanup is needed. 
  * @return The {@link AZ_ULIB_RESULT} with the result of the <tt>concat</tt> operation.
  *          @retval    AZ_ULIB_SUCCESS                If the AZ_USTREAM is concatenated with success.
  *          @retval    AZ_ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
  */
MOCKABLE_FUNCTION(, AZ_ULIB_RESULT, az_ustream_concat,
        AZ_USTREAM*, ustream_instance,
        AZ_USTREAM*, ustream_to_concat,
        AZ_USTREAM_MULTI_DATA_CB*, multi_data,
        AZ_RELEASE_CALLBACK, multi_data_release);

/**
  * @brief   Split a ustream at a given position.
  *
  *  The split will divide a given ustream into two parts, divided at a passed position. The result of this operation will be two
  *     ustream instances. The first and original ustream will contain the data from the original position up to but not including 
  *     the passed <tt>split_pos</tt>. The second ustream will contain the data from the passed <tt>split_pos</tt> up to the end of the
  *     <tt>ustream_instance</tt> before the function was called. The <tt>split_pos</tt> should be relative to the position returned
  *     from az_ustream_get_position() and the remaining size of <tt>ustream_instance</tt>.
  *
  * @param[in,out]      ustream_instance        The {@link AZ_USTREAM}* with the interface of 
  *                                             the ustream. It cannot be <tt>NULL</tt>, and it shall be a valid ustream.
  * @param[out]         ustream_instance_split  The {@link AZ_USTREAM}* with the interface of the ustream which will contain the 
  *                                             controls to the data after <tt>split_pos</tt>. It cannot be <tt>NULL</tt>.
  * @param[in]          split_pos               The position at which to split the passed <tt>ustream_instance</tt>. It cannot
  *                                             be equal to the current position of <tt>ustream_instance</tt> or equal to
  *                                             the current position + remaining size. In other words, resulting ustreams of
  *                                             size 0 are not allowed.
  * 
  * @return The {@link AZ_ULIB_RESULT} with the result of the <tt>concat</tt> operation.
  *          @retval    AZ_ULIB_SUCCESS                If the AZ_USTREAM is split with success.
  *          @retval    AZ_ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
  *          @retval    AZ_ULIB_NO_SUCH_ELEMENT_ERROR  If the input <tt>split_pos</tt> is outside the allowed range. 
  */
MOCKABLE_FUNCTION(, AZ_ULIB_RESULT, az_ustream_split,
        AZ_USTREAM*, ustream_instance,
        AZ_USTREAM*, ustream_instance_split,
        offset_t, split_pos);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZ_USTREAM_H */
