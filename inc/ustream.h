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
 * @brief   Factory to create a new uStream.
 *
 *  This factory creates a uStream that handles the content of the provided buffer. As a result,
 *      it will return a {@link USTREAM}* with this content. The new uStream can keep 
 *      or not keep the ownership of the memory pointed to by the <tt>buffer</tt> depending on the state of the 
 *      boolean <tt>take_ownership</tt>. It can be:
 *
 *  - <b>true</b>   To inform the new uStream that it shall take ownership of the memory. This 
 *                  means that the uStream will copy only the reference of the memory, use it directly 
 *                  as its own inner buffer, and will free the memory when the uStream is disposed. 
 *                  The function that called this factory shall not release the provided buffer or directly 
 *                  change its content since this buffer now belongs to the uStream.
 *
 *  - <b>false</b>  To inform the new uStream that it shall not take the ownership of the memory.
 *                  This means that the uStream shall allocate its own memory and copy the content of
 *                  the provided buffer to this internal memory. The function that called this factory with
 *                  <tt>take_ownership = false</tt> may release or change the original buffer. When the 
 *                  uStream is disposed, it will only release the memory allocated to copy the original
 *                  buffer.
 *
 * @param[in]  buffer           The <tt>const uint8_t* const</tt> that points to a memory position where the buffer starts.
 *                              It cannot be <tt>NULL</tt>.
 * @param[in]  buffer_length    The <tt>size_t</tt> with the number of <tt>uint8_t</tt> in the provided buffer.
 * @param[in]  take_ownership   The <tt>bool</tt> that indicates if the factory shall or shall not take ownership of the buffer.
 *                              If <tt>true</tt> the uStream will take ownership of the memory allocated for the
 *                              provided buffer. If <tt>false</tt> the uStream will make a copy of the content of
 *                              the provided buffer in its own memory.
 *
 * @return The {@link USTREAM}* with the uStream interface.
 *          @retval not-NULL    If the uStream was created with success.
 *          @retval NULL        If there is no memory to create the new uStream.
 */
MOCKABLE_FUNCTION(, USTREAM*, ustream_create,
        const uint8_t* const, buffer,
        size_t, buffer_length,
        bool, take_ownership);

/**
 * @brief   Factory to create a new uStream using constant content, which cannot be changed or released.
 *
 *  This factory creates a uStream that manages content in constant memory. In this case, 
 *      this API will not copy the content to a internal buffer, or release the memory when the uStream 
 *      is disposed. To create a new uStream, a valid pointer to a constant memory address shall be provided, 
 *      and the number of bytes to handle. The factory will use the reference of the provided buffer as its
 *      own inner buffer.
 *
 * @param[in]   buffer          The <tt>const uint8_t* const</tt> that points to a memory position where the buffer starts.
 *                              It cannot be <tt>NULL</tt>.
 * @param[in]   buffer_length   The <tt>size_tt</tt> with the number of <tt>uint8_t</tt> in the provided buffer. It cannot be zero.
 *
 * @return The {@link USTREAM}* with the uStream interface.
 *          @retval not-NULL    If the uStream was created with success.
 *          @retval NULL        If there is no memory to create the new uStream.
 */
MOCKABLE_FUNCTION(, USTREAM*, ustream_const_create,
    const uint8_t* const, buffer,
    size_t, buffer_length);

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
 * @return The {@link USTREAM}* with the uStreamMulti interface.
 *          @retval not-NULL    If the uStreamMulti was created with success.
 *          @retval NULL        If there is no memory to create the new uStreamMulti.
 */
MOCKABLE_FUNCTION(, USTREAM*, ustream_multi_create);

/**
 * @brief   Append an existent uStream to the uStreamMulti.
 *
 *  Append will add a new uStream at the end of the list of uStreams in the uStreamMulti. To maintain
 *      the immutability, every copy of the uStreamMulti will create an independent instance of it, which
 *      means that appending a new uStream in a current instance of the uStreamMulti will not affect the
 *      other instances.
 *
 * @param[in,out]   ustream_interface   The {@link USTREAM}* with the handle of the uStreamMulti.
 *                                      It cannot be <tt>NULL</tt> and it shall be a valid uStream that is a
 *                                      type of uStreamMulti.
 * @param[in]       ustream_to_append   The {@link USTREAM}* with the handle of the uStream to add
 *                                      to the uStreamMulti. It cannot be <tt>NULL</tt>.
 *
 * @return The {@link ULIB_RESULT} with the result of the append operation.
 *          @retval     ULIB_SUCCESS                If the uStreamMulti appended the provided uStream with success.
 *          @retval     ULIB_ILLEGAL_ARGUMENT_ERROR If the one of the provided parameters is invalid.
 *          @retval     ULIB_OUT_OF_MEMORY_ERROR    If there is no memory to append the uStream.
 */
MOCKABLE_FUNCTION(, ULIB_RESULT, ustream_multi_append,
    USTREAM*, ustream_interface,
    USTREAM*, ustream_to_append);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_USTREAM_H_ */
