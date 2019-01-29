// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_INC_USTREAMBUFFER_H_
#define AZURE_ULIB_C_INC_USTREAMBUFFER_H_

#include "ustreambuffer_base.h"

#include "macro_utils.h"
#include "umock_c_prod.h"

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
 * @brief   Factory to create a new uStreamBuffer.
 *
 * <p> This factory creates a uStreamBuffer that handles the content of the provided buffer. As a result,
 *      it will return a {@link USTREAMBUFFER_INTERFACE} with this content. The new uStreamBuffer can keep 
 *      or not the ownership of the memory pointed by the {@code *buffer} depending of the state of the 
 *      boolean {@code takeOwnership}. It can be:
 *
 *          -@b true - To inform the new uStreamBuffer that it shall take the ownership of the memory, it 
 *                  means that the uStreamBuffer will copy only the reference of the memory, use it directly 
 *                  as its own inner buffer, and will free the memory when the uStreamBuffer is disposed. 
 *                  The function that called this factory shall not release the provided buffer or directly 
 *                  change its content, this buffer now belongs to the uStreamBuffer.
 *          -@b false - To inform the new uStreamBuffer that it shall not take the ownership of the memory,
 *                  it means that the uStreamBuffer shall allocate its own memory and copy the content of
 *                  the provided buffer to this internal memory. the function that called this factory with
 *                  {@code takeOwnership = false} may release or change the original buffer. When the 
 *                  uStreamBuffer is disposed, it will only release the memory allocated to copy the original
 *                  buffer.
 *
 * @param:  buffer          The {@code const uint8_t* const} that points to a memory position where the buffer starts.
 *                              It cannot be {@code NULL}.
 * @param:  bufferLength    The {@code size_t} with the number of {@code uint8_t} in the provided buffer.
 * @param:  takeOwnership   The {@code bool} that indicates if the factory shall take or not the ownership of the buffer.
 *                              If {@code true} the uStreamBuffer will take the ownership of the memory allocated for the
 *                              provided buffer. If {@code false} the uStreamBuffer will make a copy of the content of
 *                              the provided buffer in its own memory.
 * @return: The {@link USTREAMBUFFER_INTERFACE} with the uStreamBuffer interface. The results can be:
 *          - @b not NULL - If the uStreamBuffer was created with success.
 *          - @b NULL - If there is no memory to create the new uStreamBuffer.
 */
MOCKABLE_FUNCTION(, USTREAMBUFFER_INTERFACE, uStreamBufferCreate,
        const uint8_t* const, buffer,
        size_t, bufferLength,
        bool, takeOwnership);

/**
 * @brief   Factory to create a new uStreamBuffer using a constant content, which cannot be changed or released.
 *
 * <p> This factory creates a uStreamBuffer that manage a content in a constant memory, in this case, 
 *      this API will not copy the content to a internal buffer, or release the memory when the uStreamBuffer 
 *      is disposed. To create a new uStreamBuffer, a valid pointer to a constant memory shall be provided, 
 *      and the number of bytes to handle. The factory will use the reference of the provided buffer as its
 *      own inner buffer.
 *
 * @param:  buffer          The {@code const uint8_t* const} that points to a memory position where the buffer starts.
 *                              It cannot be {@code NULL}.
 * @param:  bufferLength    The {@code size_t} with the number of {@code uint8_t} in the provided buffer.
 * @return: The {@link USTREAMBUFFER_INTERFACE} with the uStreamBuffer interface. The results can be:
 *          - @b not NULL - If the uStreamBuffer was created with success.
 *          - @b NULL - If there is no memory to create the new uStreamBuffer.
 */
MOCKABLE_FUNCTION(, USTREAMBUFFER_INTERFACE, uStreamBufferConstCreate,
    const uint8_t* const, buffer,
    size_t, bufferLength);

/**
 * @brief   Factory to create a new uStreamBufferMulti.
 *
 * <p> uStreamBufferMulti is a buffer that handles multiple heterogeneous uStreamBuffers and exposes 
 *      it with a single interface. The sequence of the buffers is determined by the appended sequence,
 *      first in first out.
 * <p> Note that the {@code getNext()} API will return the content of the appended buffers as if
 *      it was a single buffer, there is nothing that identify where one buffer stops and the
 *      next one starts.
 * <p> Copy a uStreamBufferMulti creates a new instance that have the same content of the original one
 *      at the moment where the copy was made. Any change in the original uStreamBufferMulti
 *      will **NOT** be reflected in its copies, and changes in the copies will not change the
 *      original uStreamBufferMulti as well.
 * <p> The uStreamBufferMulti creates its own copy of each appended buffer, it will enforce the immutable
 *      characteristic of the uStreamBuffer.
 * <p> Because a uStreamBufferMulti expose an uStreamBuffer interface, it can be appended to another
 *      uStreamBufferMulti.
 * <p> Release part of a uStreamBufferMulti will dispose the appended buffers that is not necessary anymore,
 *      and dispose the uStreamBufferMulti will dispose all appended buffers.
 *
 * @return: The {@link USTREAMBUFFER_INTERFACE} with the uStreamBufferMulti interface. The results can be:
 *          - @b not NULL - If the uStreamBufferMulti was created with success.
 *          - @b NULL - If there is no memory to create the new uStreamBufferMulti.
 */
MOCKABLE_FUNCTION(, USTREAMBUFFER_INTERFACE, uStreamBufferMultiCreate);

/**
 * @brief   Append an existent buffer to the uStreamBufferMulti.
 *
 * <p> Append will add a new buffer at the end of the list of buffers in the uStreamBufferMulti. To maintain
 *      the immutability, every copy of the uStreamBufferMulti will create a independent instance of it, which
 *      means that append a new buffer in a current instance of the uStreamBufferMulti will not affect the
 *      other instances.
 *
 * @param:  uStreamBufferInterface  The {@link USTREAMBUFFER_INTERFACE} with the handle of the uStreamBufferMulti.
 *                                  It cannot be {@code NULL}, and it shall be a valid buffer that is
 *                                  type of uStreamBufferMulti.
 * @param:  uStreamBufferToAppend   The {@link USTREAMBUFFER_INTERFACE} with the handle of the buffer to add
 *                                  to the uStreamBufferMulti. It cannot be {@code NULL}.
 * @return: The {@link USTREAMBUFFER_RESULT} with the result of the append operation. The results can be:
 *          - @b USTREAMBUFFER_SUCCESS - If the uStreamBufferMulti appended the provided buffer with success.
 *          - @b USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION - If the one of the provided parameters is invalid.
 *          - @b USTREAMBUFFER_OUT_OF_MEMORY_EXCEPTION - If there is no memory to append the buffer.
 */
MOCKABLE_FUNCTION(, USTREAMBUFFER_RESULT, uStreamBufferMultiAppend,
    USTREAMBUFFER_INTERFACE, uStreamBufferInterface,
    USTREAMBUFFER_INTERFACE, uStreamBufferToAppend);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_USTREAMBUFFER_H_ */
