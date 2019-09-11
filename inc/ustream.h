// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file ustream.h
 * 
 * @brief uStream implementation for local memory
 */

#ifndef AZIOT_USTREAM_H
#define AZIOT_USTREAM_H

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
 * @brief   Factory to initialize a new uStream.
 *
 *  This factory initializes a uStream that handles the content of the provided buffer. As a result,
 *      it will return an {@link AZIOT_USTREAM}* with this content. The initialized uStream takes ownership of the
 *      passed memory and will release the memory with the passed #AZIOT_USTREAM_DATA_RELEASE function when
 *      the ref count goes to zero.
 *
 * @param[in,out]   ustream_instance        The pointer to the allocated #AZIOT_USTREAM struct. This memory must be valid from
 *                                          the time aziot_ustream_init() is called through aziot_ustream_release(). The uStream will not
 *                                          free this struct and it is the responsibility of the developer to make sure it is valid during
 *                                          the time frame described above.
 * @param[in]       ustream_inner_buffer    The pointer to the allocated #AZIOT_USTREAM_INNER_BUFFER struct. This memory should be allocated in
 *                                          a way that it stays valid until the passed #AZIOT_USTREAM_INNER_BUFFER_RELEASE is called
 *                                          at some (potentially) unknown time in the future.
 * @param[in]       inner_buffer_release    The #AZIOT_USTREAM_INNER_BUFFER_RELEASE function that will be called for the inner 
 *                                          buffer control block (the passed <tt>ustream_inner_buffer</tt> parameter) once all the 
 *                                          references to the uStream are released. If <tt>NULL</tt> is passed, the data is assumed to 
 *                                          be constant with no need to be free'd. In other words, there is no need for notification 
 *                                          that the memory may be released.
 *                                          As a default, users may use the standard <tt>free</tt> to release malloc'd memory.
 * @param[in]       data_buffer             The <tt>const uint8_t* const</tt> that points to a memory position where the buffer starts.
 *                                          It cannot be <tt>NULL</tt>.
 * @param[in]       data_buffer_length      The <tt>size_t</tt> with the number of <tt>uint8_t</tt> in the provided buffer.
 * @param[in]       data_buffer_release     The #AZIOT_USTREAM_DATA_RELEASE function that will be called for the inner 
 *                                          buffer once all the references to the uStream are released. If <tt>NULL</tt> is 
 *                                          passed, the data is assumed to be constant with no need to be free'd. In other words, 
 *                                          there is no need for notification that the memory may be released.
 *                                          As a default, users may use the standard <tt>free</tt> to release malloc'd memory.
 *
 * @return The {@link AZIOT_ULIB_RESULT} with result of the initialization.
 *          @retval     AZIOT_ULIB_SUCCESS                          If the {@link AZIOT_USTREAM}* is successfully initialized.
 *          @retval     AZIOT_ULIB_ILLEGAL_ARGUMENT_EXCEPTION       If one of the provided parameters is invalid.
 */
MOCKABLE_FUNCTION(, AZIOT_ULIB_RESULT, aziot_ustream_init,
        AZIOT_USTREAM*, ustream_instance,
        AZIOT_USTREAM_INNER_BUFFER*, ustream_inner_buffer,
        AZIOT_USTREAM_INNER_BUFFER_RELEASE, inner_buffer_release,
        const uint8_t* const, data_buffer,
        size_t, data_buffer_length,
        AZIOT_USTREAM_DATA_RELEASE, data_buffer_release);


/**
  * @brief   Append a uStream to the existing uStream.
  *
  *  The append will add a uStream at the end of the current one. To do that, the append will copy 
  *         the <tt>ustream_interface</tt> into a <tt>USTREAM_MULTI_INSTANCE</tt>, and append a clone of <tt>ustream_to_append</tt>
  *         inside the <tt>USTREAM_MULTI_INSTANCE</tt>.
  *
  *  The <tt>aziot_ustream_concat</tt> API shall follow the following minimum requirements:
  *      - The <tt>append</tt> shall append <tt>ustream_to_append</tt> to the end of <tt>ustream_interface</tt>.
  *      - If <tt>ustream_interface</tt> is <tt>NULL</tt>, the <tt>append</tt> shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
  *      - If <tt>ustream_to_append</tt> is <tt>NULL</tt>, the <tt>append</tt> shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
  *      - If <tt>inner_buffer</tt> is <tt>NULL</tt>, the <tt>append</tt> shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
  *      - If <tt>multi_data</tt> is <tt>NULL</tt>, the <tt>append</tt> shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
  *
  * @param[in, out]     ustream_interface       The {@link AZIOT_USTREAM}* with the interface of 
  *                                             the uStream. It cannot be <tt>NULL</tt>, and it shall be a valid uStream.
  * @param[in]          ustream_to_append       The {@link AZIOT_USTREAM}* with the interface of 
  *                                             the uStream to be appended to <tt>ustream_interface</tt>. It cannot be <tt>NULL</tt>, 
  *                                             and it shall be a valid uStream.
  * @param[in]          inner_buffer            The pointer to the {@link AZIOT_USTREAM_INNER_BUFFER} struct.
  *                                             It must be allocated in way that it remains a valid address until
  *                                             the passed {@link AZIOT_USTREAM_INNER_BUFFER_RELEASE} callback is invoked some
  *                                             time in the future. It may be <tt>NULL</tt> if no future cleanup is needed.
  * @param[in]          inner_buffer_release    The {@link AZIOT_USTREAM_INNER_BUFFER_RELEASE} callback which will be called once
  *                                             the number of references to the inner buffer reaches zero.
  * @param[in]          multi_data              The {@link AZIOT_USTREAM_MULTI_DATA}* pointing to the allocated data. It must be allocated
  *                                             in a way that it remains a valid address until the passed {@link AZIOT_USTREAM_DATA_RELEASE}
  *                                             callback is invoked some time in the future.
  * @param[in]          multi_data_release      The {@link AZIOT_USTREAM_DATA_RELEASE} callback which will be called once
  *                                             the number of references to the inner buffer reaches zero. It may be <tt>NULL</tt> if no 
  *                                             future cleanup is needed. 
  * @return The {@link AZIOT_ULIB_RESULT} with the result of the <tt>append</tt> operation.
  *          @retval    AZIOT_ULIB_SUCCESS                If the AZIOT_USTREAM is appended with success.
  *          @retval    AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
  */
MOCKABLE_FUNCTION(, AZIOT_ULIB_RESULT, aziot_ustream_concat,
        AZIOT_USTREAM*, ustream_interface,
        AZIOT_USTREAM*, ustream_to_append,
        AZIOT_USTREAM_INNER_BUFFER*, inner_buffer,
        AZIOT_USTREAM_INNER_BUFFER_RELEASE, inner_buffer_release,
        AZIOT_USTREAM_MULTI_DATA*, multi_data,
        AZIOT_USTREAM_DATA_RELEASE, multi_data_release);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZIOT_USTREAM_H */
