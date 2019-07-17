// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file ustream.h
 */

#ifndef AZURE_ULIB_C_INC_USTREAM_H_
#define AZURE_ULIB_C_INC_USTREAM_H_

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
 * @brief   Signature of the function to release the inner buffer
 * 
 * @param[in]   void*       void pointer to memory that needs to be free'd
 * 
 * @return  void
 */
typedef void (*AZIOT_USTREAM_BUFFER_RELEASE_CALLBACK)(void*);

/**
 * @brief   Factory to create a new uStream.
 *
 *  This factory creates a uStream that handles the content of the provided buffer. As a result,
 *      it will return a {@link AZIOT_USTREAM}* with this content. The created uStream takes ownership of the
 *      passed memory and will release the memory with the passed #AZIOT_USTREAM_BUFFER_RELEASE_CALLBACK function when
 *      the ref count goes to zero.
 *
 * @param[in]   buffer          The <tt>const uint8_t* const</tt> that points to a memory position where the buffer starts.
 *                              It cannot be <tt>NULL</tt>.
 * @param[in]   buffer_length   The <tt>size_t</tt> with the number of <tt>uint8_t</tt> in the provided buffer.
 * @param[in]   release_buffer  The #AZIOT_USTREAM_BUFFER_RELEASE_CALLBACK function that will be called for the inner 
 *                              buffer once all the references to the uStream are released. If <tt>NULL</tt> is 
 *                              passed, the data is assumed to be constant with no need to be free'd. In other words, 
 *                              there is no need for notification that the memory can be released.
 *                              As a default, users may use the standard <tt>free</tt> to release malloc'd memory.
 *
 * @return The {@link AZIOT_USTREAM}* with the uStream interface.
 *          @retval not-NULL    If the uStream was created with success.
 *          @retval NULL        If there is no memory to create the new uStream.
 */
MOCKABLE_FUNCTION(, AZIOT_USTREAM*, aziot_ustream_create,
        const uint8_t* const, buffer,
        size_t, buffer_length,
        AZIOT_USTREAM_BUFFER_RELEASE_CALLBACK, release_buffer);

/**
 * @brief   Factory to create a new uStreamMulti.
 *
 *  uStreamMulti is a uStream that handles multiple heterogeneous uStreams and exposes 
 *      it with a single interface. The sequence of the uStreams is determined by the appended sequence:
 *      first in first out.
 *
 *  Note that the <tt>read()</tt> API will return the content of the appended uStreams as if
 *      it was a single uStream. There is nothing that identifies where one uStream stops and the
 *      next one starts.
 *
 *  Cloning a uStreamMulti creates a new instance that has the same content of the original one
 *      at the moment where the clone was made. Any change in the original uStreamMulti
 *      will <b>NOT</b> be reflected in its clone, and changes in the clone will not change the
 *      original uStreamMulti as well.
 *
 *  The uStreamMulti clones each appended uStream. It will enforce the immutable
 *      characteristic of the uStream.
 *
 *  Because a uStreamMulti exposes a uStream interface, it can be appended to another
 *      uStreamMulti.
 *
 *  Releasing part of a uStreamMulti will dispose the appended uStreams that are not necessary anymore,
 *      and the disposal of the uStreamMulti will dispose all appended uStreams.
 *
 * @return The {@link AZIOT_USTREAM}* with the uStreamMulti interface.
 *          @retval not-NULL    If the uStreamMulti was created with success.
 *          @retval NULL        If there is no memory to create the new uStreamMulti.
 */
MOCKABLE_FUNCTION(, AZIOT_USTREAM*, aziot_ustream_multi_create);

/**
 * @brief   Append an existing uStream to the uStreamMulti.
 *
 *  Append will add a new uStream at the end of the list of uStreams in the uStreamMulti. To maintain
 *      the immutability, every copy of the uStreamMulti will create an independent instance of it, which
 *      means that appending a new uStream in a current instance of the uStreamMulti will not affect the
 *      other instances.
 *
 * @param[in,out]   ustream_interface   The {@link AZIOT_USTREAM}* with the handle of the uStreamMulti.
 *                                      It cannot be <tt>NULL</tt> and it shall be a valid uStream that is a
 *                                      type of uStreamMulti.
 * @param[in]       ustream_to_append   The {@link AZIOT_USTREAM}* with the handle of the uStream to add
 *                                      to the uStreamMulti. It cannot be <tt>NULL</tt>.
 *
 * @return The {@link AZIOT_ULIB_RESULT} with the result of the append operation.
 *          @retval     AZIOT_ULIB_SUCCESS                If the <tt>ustream_to_append</tt> was appended with success.
 *          @retval     AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
 *          @retval     AZIOT_ULIB_OUT_OF_MEMORY_ERROR    If there is no memory to append the uStream.
 */
MOCKABLE_FUNCTION(, AZIOT_ULIB_RESULT, aziot_ustream_multi_append,
    AZIOT_USTREAM*, ustream_interface,
    AZIOT_USTREAM*, ustream_to_append);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_USTREAM_H_ */
