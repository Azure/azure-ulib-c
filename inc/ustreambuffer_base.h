// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_INC_USTREAMBUFFER_BASE_H_
#define AZURE_ULIB_C_INC_USTREAMBUFFER_BASE_H_

/**
 * @brief uStreamBuffer Interface.
 *
 * <p> This is the definition of a heterogeneous buffer that helps the other modules in the system to
 *      expose big amount of data without use a big amount of memory. Modules in the system can expose
 *      its own data using this interface, to do that, the module shall implement the functions in the
 *      interface. This implementation shall be following the definition described in this file, not
 *      only the prototype of the header, but the behavior too.
 *
 * <p> The uStreamBuffer shall have a clear separation between the internal content and what it exposes 
 *      as an external content. The uStreamBuffer shall never expose the internal content, providing,
 *      for instance, a pointer to a internal memory position. Every exposed content shall be copied 
 *      from the internal memory to a external memory. To do that in a clear way, the uStreamBuffer 
 *      shall always work with two buffers, inner and local, following the follow definition:
 *      - @b Inner buffer - is the place where the data is stored by the implementation of the buffer
 *          interface. The Inner buffer is protected, and immutable. Users can read it by the
 *          {@link uStreamBufferGetNext} API, which will copy part of the Inner buffer to the provided
 *          external memory, called local buffer.
 *      - @b Local buffer - is the user buffer, where the {@link uStreamBufferGetNext} API will copy the
 *          required bytes from the Inner buffer. The local buffer belongs to the user of this
 *          interface, which means that the user shall allocate and free this memory, and the content
 *          of the local buffer can be changed and released.
 *
 * <i><b>Example:
 * <p> A module `Provide` whats to create a uStreamBuffer to expose a data to the module `Consumer`. 
 *      `Provider` already have the content in the HEAP, and will create a uStreamBuffer from it,
 *      passing the ownership of the content to the uStreamBuffer. Consumer will print the content
 *      of the uStramBuffer, using a local buffer of 1K. The following diagram represent this operation.
 *
 * <pre><code>
 *  +----------------+        +----------------+         +------------------+     +------------+
 *  |    Provider    |        |    Consumer    |         |   uSreamBuffer   |     |    HEAP    |
 *  +----------------+        +----------------+         +------------------+     +------------+
 *          |                         |                            |                    |
 *          |<--GetProviderContent()--+                            |                    |
 *          +-----------------------------malloc(contentSize)-------------------------->|
 *          |<--------------------------------contentPtr--------------------------------+
 *   +------+                         |                            |                    |
 *   | generate the content and store in the contentPtr            |                    |
 *   +----->|                         |                            |                    |
 *          +-----uStreamBufferCreate                              |                    |
 *          |          (contentPtr, contentSize, takeOwnership)--->|                    |
 *          |                         |                     +------+                    |
 *          |                         |                     | innerBuffer = contentPtr  |
 *          |                         |                     | innerBufferSize = contentSize
 *          |                         |                     | ownership = true          |
 *          |                         |                     +----->|                    |
 *          |<-----------------uStreamBufferInterface--------------+                    |
 *          +-uStreamBufferInterface->|                            |                    |
 *
 *  Now that the `Consumer` have the uStreamBuffer with the content, it will print it using the 
 *   iterator uStreamBufferGetNext.
 *
 *          |                         +------------------malloc(1024)------------------>|
 *          |                         |<-----------------localBuffer--------------------+
 *  .. while uStreamBufferGetNext return USTREAMBUFFER_SUCCESS ....................................
 *  :       |                         +-uStreamBufferGetNext       |                    |         :
 *  :       |                         |  (uStreamBufferInterface,  |                    |         :
 *  :       |                         |   localBuffer,             |                    |         :
 *  :       |                         |   1024)------------------->|                    |         :
 *  :       |                         |                     +------+                    |         :
 *  :       |                         |                     | copy the next 1024 bytes from the   :
 *  :       |                         |                     |  innerBuffer to the localBuffer.    :
 *  :       |                         |                     +----->|                    |         :
 *  :       |                         |<---USTREAMBUFFER_SUCCESS---+                    |         :
 *  :       |                  +------+                            |                    |         :
 *  :       |                  | use the content in the localBuffer                     |         :
 *  :       |                  +----->|                            |                    |         :
 *  ...............................................................................................
 *          |                         +---------------free(localBuffer)---------------->|
 *          |                         +-uStreamBufferDispose       |                    |
 *          |                         |  (uStreamBufferInterface)->|                    |
 *          |                         |                            +-free(innerBuffer)->|
 *          |                         |                            |                    |
 * </code></pre>
 *
 * <i><b> Heterogeneous buffer:
 * <p> Data can be stored in multiple different medias, like RAM, flash, file, or cloud. Each media
 *      has its own read requirements. A simple way to unify it is copying it all to the RAM. For
 *      example, if an HTTP package contains a header that is in the flash, with some data in the RAM
 *      and the content is in a file in the external flash, to concatenate it all in a single datagram
 *      you can allocate a single area in the RAM that fits it all, and bring all the data to this
 *      memory. The problem with this approach is the amount of memory required for that, which can be
 *      multiple times the total RAM that you have for the entire system.
 * <p> A second option to solve this problem is to make each component that needs to access this data
 *      understand each media and implement code to handle it. This approach will not require
 *      storing all data in the RAM, but will increase the size of the program itself, and
 *      is not easily portable, as different hardware will contain different media with different
 *      requirements.
 * <p> The `uStreamBuffer` resolves this problem by creating a single interface that can handle any media,
 *      exposing it as a standard iterator. Who ever wants to expose a media as a uStreamBuffer shall
 *      implement the functions described on the interface, handling all implementation details for
 *      each API. For example, the  {@link uStreamBufferGetNext} can be a simple copy of the flash to
 *      the RAM for a buffer that handles constants, or be as complex as creating a TCP/IP connection
 *      to bring the data for a buffer that handles data in the cloud.
 * <p> As a user of the `uStreamBuffer`, you can use all kinds of media in the same way, and may easily
 *      concatenate it exposing as `uStreamBuffer` that handles multiple `uStreamBuffer`.
 *
 * <i><b> Ownership:
 * <p> The uStreamBuffer is an owner-less buffer, every instance of the buffer has the same rights. They
 *      all can read the buffer content, release the parts that are not necessary anymore, and dispose
 *      it.
 * <p> Each instance of the uStreamBuffer is owned by who created it, and should never be shared by 
 *      multiple modules. When a module receives an uStreamBuffer and makes operations over it, this 
 *      module must first make a copy of the buffer, creating its own instance of it, and than make 
 *      the needed operations.
 * <p> Clone a buffer creates a new set of controls for the buffer that will share the same content of
 *      the original buffer. The content itself is a smart pointer with a {@code refCount} that
 *      controls the total number of instances.
 * <p> Disposing an instance of the buffer will decrease the {@code refCount} of this buffer. If the
 *      number of references reach 0, the buffer will destroy itself releasing all allocated memory.
 *      Not disposing an instance of the buffer will leak memory.
 * <p> Instances of the buffer can be created in 2 ways:
 *      - @b Factory - when a module exposes data using this buffer, it must create the buffer
 *          using a factory, so the operation buffer {@code Create} returns the first instance of the
 *          buffer.
 *      - @b Clone - when a module needs a copy of the buffer, it can use the {@link uStreamBufferClone}.
 *
 * <i><b> Thread safe:
 * <p> The uStreamBuffer **IS NOT** thread safe for multiple accesses over the same instance. The ownership
 *      of the instance of an uStreamBuffer shall **NOT** be shared, especially not by code that runs on 
 *      different threads. The owner thread shall create a clone of the uStreamBuffer and pass to the other 
 *      thread.
 *      For example, any `IPC` call runs on its own thread, so if the `IPC` call contains an uStreamBuffer
 *      in the model, the caller shall create a clone of the buffer and use it in the model.
 * <p> The uStreamBuffer **IS** thread safe for accesses between instances. It means that any access to a
 *      memory shared by multiple instances shall be thread safe.
 *
 * <i><b> Data retention:
 * <p> As with any buffer, this buffer shall be used to handle data that was created by one modules of the
 *      system as a result of an operation.
 * <p> This interface only exposes read functions, so once created, the content of the buffer cannot
 *      be changed by a module that is using the data. Changing the module that was copied by another
 *      module can result in a data mismatch. The owner of a buffer can change the content during the
 *      creation process.
 * <p> Users of the buffer can do a partial release of the buffer by calling {@link uStreamBufferRelease}.
 *      Calling the release does not imply that part of the memory will be immediately released. Once a
 *      buffer can handle multiple instances, a memory can only be free if all instances released it.
 *      A buffer implementation can or cannot have the ability to do partial releases, for instance, a
 *      buffer that handles a constant data stored in the flash will never release any memory on the
 *      {@link uStreamBufferRelease} API.
 * <p> Released data cannot be accessed, even if it is still available in the memory.
 *
 * <i><b> Lazy:
 * <p> The buffer can contain the full contents, or bring it into memory when required. The
 *      implementation of the {@link uStreamBufferGetNext} function can be smart enough to use the minimal
 *      amount of memory.
 *
 * <i><b> Sliding window:
 * <p> The data stored in the Inner buffer is controlled by byte position. Each byte receives a label
 *      that unique identify it on the buffer. The label is a sequential positive integer. When a new
 *      buffer is created, the first byte receives the label `0`, and the further positions are
 *      incremented by one. The byte position is defined internally in the buffer creation and is called
 *      the `Inner` position.
 * <p> The uStreamBuffer shall expose the byte label, called the `Logical` position, which can be the same as
 *      the Inner position or not. The Logical position is the one that the user of the buffer shall
 *      use to {@link uStreamBufferSeek} or {@link uStreamBufferRelease} bytes.
 * <p> The internal implementation of the buffer shall implement the conversion between the `Logical`
 *      and `Inner` positions.
 * <p> By definition, on the creation process, the `Logical` and `Inner` position shares the same value.
 * <p> When a buffer is cloned, a new label for the first byte shall be provided, this is the `Offset`
 *      of the buffer and generates a new `Logical` label for each byte. Providing an offset to a buffer
 *      can be useful in many cases, for example, to concatenate buffers the second buffer can have
 *      an offset of the end of the first buffer plus one.
 * <p> The module that is using this buffer can seek any position of the buffer between the released
 *      position and the end of the buffer. Once a position is released, all bytes from the beginning
 *      of the buffer to the released position (inclusive) became unavailable.
 * <p> Users can seek the first available byte in the buffer by calling {@link uStreamBufferReset}.
 *
 */

#include "ulib_config.h"
#include "macro_utils.h"
#include "umock_c_prod.h"

#ifdef __cplusplus
#include <cstdint>
#include <cstddef>
extern "C" {
#else
#include <stdint.h>
#include <stddef.h>
#endif /* __cplusplus */

/**
 * @brief   Handles that unique identify of the buffer interface.
 */
typedef struct USTREAMBUFFER_TAG* USTREAMBUFFER_INTERFACE;

/**
 * @brief   List of uStreamBuffer results.
 */
DEFINE_ENUM(
    USTREAMBUFFER_RESULT, 
        USTREAMBUFFER_SUCCESS                       = 0, 
        USTREAMBUFFER_OUT_OF_MEMORY_EXCEPTION       = -1,
        USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION     = -2,
        USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION    = -3,
        USTREAMBUFFER_SECURITY_EXCEPTION            = -4,
        USTREAMBUFFER_SYSTEM_EXCEPTION              = -5,
        USTREAMBUFFER_CANCELLED_EXCEPTION           = -9,
        USTREAMBUFFER_BUSY_EXCEPTION                = -12
)

/**
 * @brief   Define offset_t with the same size as size_t.
 */
typedef size_t  offset_t;

/**
 * @brief   vTable with the uStreamBuffer APIs.
 *
 * <p> Any module that exposes the uStreamBuffer shall implement the functions on this vTable.
 * <p> Any code that will use an exposed uStreamBuffer shall call the APIs using the `uStreamBuffer...`
 *      macros.
 */
typedef struct USTREAMBUFFER_API_TAG
{
    USTREAMBUFFER_RESULT(*seek)(USTREAMBUFFER_INTERFACE uStreamBufferInterface, offset_t position);
    USTREAMBUFFER_RESULT(*reset)(USTREAMBUFFER_INTERFACE uStreamBufferInterface);
    USTREAMBUFFER_RESULT(*getNext)(USTREAMBUFFER_INTERFACE uStreamBufferInterface, uint8_t* const buffer, size_t bufferLength, size_t* const size);
    USTREAMBUFFER_RESULT(*getRemainingSize)(USTREAMBUFFER_INTERFACE uStreamBufferInterface, size_t* const size);
    USTREAMBUFFER_RESULT(*getCurrentPosition)(USTREAMBUFFER_INTERFACE uStreamBufferInterface, offset_t* const position);
    USTREAMBUFFER_RESULT(*release)(USTREAMBUFFER_INTERFACE uStreamBufferInterface, offset_t position);
    USTREAMBUFFER_INTERFACE(*clone)(USTREAMBUFFER_INTERFACE uStreamBufferInterface, offset_t offset);
    USTREAMBUFFER_RESULT(*dispose)(USTREAMBUFFER_INTERFACE uStreamBufferInterface);
} USTREAMBUFFER_API;

/**
 * @brief   Interface description.
 */
typedef struct USTREAMBUFFER_TAG
{
    const USTREAMBUFFER_API* api;
    void* handle;
} USTREAMBUFFER;

/**
 * @brief   Check if a handle is the same type of the API.
 *
 * <p> It will return true if the handle is valid and it is the same type of the API. It will
 *      return false if the handle is NULL or note the correct type.
 */
#define USTREAMBUFFER_IS_NOT_TYPE_OF(handle, typeApi)   ((handle == NULL) || (handle->api != &typeApi))

/**
 * @brief   Change the current position of the buffer.
 *
 * <p> The current position is the one that will be returned in the local buffer by the next
 *      {@link uStreamBufferGetNext}. Users can call this API to go back or forward, but it cannot be exceed
 *      the end of the buffer or precede the released position.
 *
 * <p> The seek API shall follow these minimum requirements:
 *      - The buffer seek shall change the current position of the buffer.
 *      - If the provided position is out of the range of the buffer, the buffer seek shall return
 *          USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION, and will not change the current position.
 *      - If the provided position is already released, the buffer seek shall return
 *          USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION, and will not change the current position.
 *      - If the provided interface is NULL, the buffer seek shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *      - If the provided interface is not the implemented buffer type, the buffer seek shall return
 *          USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *
 * @param:  uStreamBufferInterface       The {@link USTREAMBUFFER_INTERFACE} with the interface of the buffer. It
 *                              cannot be {@code NULL}, and it shall be a valid buffer that is the
 *                              implemented buffer type.
 * @param:  position        The {@code offset_t} with the new current position in the buffer. It
 *                              shall be bigger than the released position.
 * @return: The {@link USTREAMBUFFER_RESULT} with the result of the seek operation. The results can be:
 *          - @b USTREAMBUFFER_SUCCESS - If the buffer changed the current position with success.
 *          - @b USTREAMBUFFER_BUSY_EXCEPTION - If the resource necessary for the seek operation is busy.
 *          - @b USTREAMBUFFER_CANCELLED_EXCEPTION - If the seek operation was cancelled.
 *          - @b USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION - If one of the provided parameters is invalid.
 *          - @b USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION - If the position is out of the buffer range.
 *          - @b USTREAMBUFFER_OUT_OF_MEMORY_EXCEPTION - If there is not enough memory to execute the
 *              seek operation.
 *          - @b USTREAMBUFFER_SECURITY_EXCEPTION - if the seek operation was denied for security
 *              reasons.
 *          - @b USTREAMBUFFER_SYSTEM_EXCEPTION - if the seek operation failed on the system level.
 */
#define uStreamBufferSeek( \
            /*[USTREAMBUFFER_INTERFACE]*/ uStreamBufferInterface, \
            /*[offset_t]*/ position) \
    ((uStreamBufferInterface)->api->seek( \
            (uStreamBufferInterface), \
            (position)))

/**
 * @brief   Changes the current position to the byte immediately after the last released position.
 *
 * <p> The current position is the one that will be returned in the local buffer by the next
 *      {@link uStreamBufferGetNext}. Reset will bring the current position to the first valid one, which
 *      is the first byte after the released position.
 *
 * <p> The reset API shall follow the following minimum requirements:
 *      - The buffer reset shall change the current position of the buffer to the first byte after the
 *          released position.
 *      - If all bytes are already released, the buffer reset shall return
 *          USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION, and will not change the current position.
 *      - If the provided interface is NULL, the buffer reset shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *      - If the provided interface is not the implemented buffer type, the buffer reset shall return
 *          USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *
 * @param:  uStreamBufferInterface       The {@link USTREAMBUFFER_INTERFACE} with the interface of the buffer. It
 *                              cannot be {@code NULL}, and it shall be a valid buffer that is the
 *                              implemented buffer type.
 * @return: The {@link USTREAMBUFFER_RESULT} with the result of the reset operation. The results can be:
 *          - @b USTREAMBUFFER_SUCCESS - If the buffer changed the current position with success.
 *          - @b USTREAMBUFFER_BUSY_EXCEPTION - If the resource necessary for the reset operation is
 *              busy.
 *          - @b USTREAMBUFFER_CANCELLED_EXCEPTION - If the reset operation was cancelled.
 *          - @b USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION - If one of the provided parameters is invalid.
 *          - @b USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION - If all previous bytes in the buffer were already
 *              released.
 *          - @b USTREAMBUFFER_OUT_OF_MEMORY_EXCEPTION - If there is not enough memory to execute the
 *              reset operation.
 *          - @b USTREAMBUFFER_SECURITY_EXCEPTION - If the reset operation was denied for security
 *              reasons.
 *          - @b USTREAMBUFFER_SYSTEM_EXCEPTION - If the reset operation failed on the system level.
 */
#define uStreamBufferReset( \
            /*[USTREAMBUFFER_INTERFACE]*/ uStreamBufferInterface) \
    ((uStreamBufferInterface)->api->reset( \
            (uStreamBufferInterface)))

/**
 * @brief   Gets the next portion of the buffer starting at the current position.
 *
 * <p> The {@code uStreamBufferGetNext} API will copy the contents of the Inner buffer to the local buffer
 *      starting at the current position. The local buffer is the one referenced by the parameter
 *      `buffer`, and with the maximum size bufferLength.
 * <p> The buffer is defined as a {@code uint8_t*} and can represent any sequence of data. Pay
 *      special attention, if the data is a string, the buffer will still copy it as a sequence of
 *      {@code uint8_t}, and will **NOT** put any terminator at the end of the string. The size of 
 *      the content copied in the buffer will be returned by the {@code uStreamBufferGetNext} in 
 *      the parameter `size`.
 *
 * <p> The getNext API shall follow the following minimum requirements:
 *      - The getNext shall copy the contents of the Inner buffers to the provided local buffer.
 *      - If the contents of the Inner buffer is bigger than the `bufferLength`, the getNext shall
 *          limit the copy size up to the bufferLength.
 *      - The getNext shall return the number of valid {@code uint8_t} values in the local buffer in 
 *          the provided `size`.
 *      - If there is no more content to return, the getNext shall return
 *          USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION, size shall receive 0, and will not change the contents
 *          of the local buffer.
 *      - If the provided bufferLength is zero, the getNext shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *      - If the provided interface is NULL, the getNext shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *      - If the provided interface is not the implemented buffer type, the getNext shall return
 *          USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *      - If the provided local buffer is NULL, the getNext shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *      - If the provided return size pointer is NULL, the getNext shall return
 *          USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION and will not change the local buffer contents or the
 *          current position of the buffer.
 *
 * @param:  uStreamBufferInterface       The {@link USTREAMBUFFER_INTERFACE} with the interface of the buffer. It
 *                              cannot be {@code NULL}, and it shall be a valid buffer that is the
 *                              implemented buffer type.
 * @param:  buffer          The {@code uint8_t* const} that points to the local buffer. It cannot be {@code NULL}.
 * @param:  bufferLength    The {@code size_t} with the size of the local buffer. It shall be
 *                              bigger than 0.
 * @param:  size            The {@code size_t* const} that points to the place where the getNext shall store
 *                              the number of valid {@code uint8_t} values in the local buffer. It cannot be {@code NULL}.
 * @return: The {@link USTREAMBUFFER_RESULT} with the result of the getNext operation. The results can be:
 *          - @b USTREAMBUFFER_SUCCESS - If the buffer copied the content of the Inner buffer to the local one
 *              with success.
 *          - @b USTREAMBUFFER_BUSY_EXCEPTION - If the resource necessary to copy the buffer content is busy.
 *          - @b USTREAMBUFFER_CANCELLED_EXCEPTION - If the copy of the content was cancelled.
 *          - @b USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION - If one of the provided parameters is invalid.
 *          - @b USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION - If there is no more {@code uint8_t} values in the 
 *              Inner buffer to copy.
 *          - @b USTREAMBUFFER_OUT_OF_MEMORY_EXCEPTION - If there is not enough memory to execute the copy.
 *          - @b USTREAMBUFFER_SECURITY_EXCEPTION - If the get next was denied for security reasons.
 *          - @b USTREAMBUFFER_SYSTEM_EXCEPTION - If the get next operation failed on the system level.
 */
#define uStreamBufferGetNext( \
            /*[USTREAMBUFFER_INTERFACE]*/ uStreamBufferInterface, \
            /*[uint8_t* const]*/ buffer, \
            /*[size_t]*/ bufferLength, \
            /*[size_t* const]*/ size) \
    ((uStreamBufferInterface)->api->getNext( \
            (uStreamBufferInterface), \
            (buffer), \
            (bufferLength), \
            (size)))

/**
 * @brief   Returns the remaining size of the buffer.
 *
 * <p> This API returns the number of bytes between the current position and the end of the buffer.
 *
 * <p> The getRemainingSize API shall follow the following minimum requirements:
 *      - The getRemainingSize shall return the number of bytes between the current position and the
 *          end of the buffer.
 *      - If the provided interface is NULL, the getRemainingSize shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *      - If the provided interface is not the implemented buffer type, the getRemainingSize shall
 *          return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *      - If the provided size is NULL, the getRemainingSize shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *
 * @param:  uStreamBufferInterface       The {@link USTREAMBUFFER_INTERFACE} with the interface of the buffer. It
 *                              cannot be {@code NULL}, and it shall be a valid buffer that is the
 *                              implemented buffer type.
 * @param:  size            The {@code size_t* const} to returns the remaining number of {@code uint8_t} values 
 *                              copied to the buffer. It cannot be {@code NULL}.
 * @return: The {@link USTREAMBUFFER_RESULT} with the result of the getRemainingSize operation. The results can be:
 *          - @b USTREAMBUFFER_SUCCESS - If it succeeded to get the remaining size of the buffer.
 *          - @b USTREAMBUFFER_BUSY_EXCEPTION - If the resource necessary to the get the remain size of
 *              the buffer is busy.
 *          - @b USTREAMBUFFER_CANCELLED_EXCEPTION - If the get remaining size was cancelled.
 *          - @b USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION - If one of the provided parameters is invalid.
 *          - @b USTREAMBUFFER_OUT_OF_MEMORY_EXCEPTION - If there is not enough memory to execute the get
 *              remaining size operation.
 *          - @b USTREAMBUFFER_SECURITY_EXCEPTION - If the get remaining size was denied for security reasons.
 *          - @b USTREAMBUFFER_SYSTEM_EXCEPTION - If the get remaining size operation failed on the
 *              system level.
 */
#define uStreamBufferGetRemainingSize( \
            /*[USTREAMBUFFER_INTERFACE]*/ uStreamBufferInterface, \
            /*[size_t* const]*/ size) \
    ((uStreamBufferInterface)->api->getRemainingSize( \
            (uStreamBufferInterface), \
            (size)))

/**
 * @brief   Returns the current position in the buffer.
 *
 * <p> This API returns the logical current position.
 *
 * <p> The getCurrentPosition API shall follow the following minimum requirements:
 *      - The getCurrentPosition shall return the logical current position of the buffer.
 *      - If the provided interface is NULL, the getCurrentPosition shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *      - If the provided interface is not the implemented buffer type, the getCurrentPosition
 *          shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *      - If the provided position is NULL, the getCurrentPosition shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *
 * @param:  uStreamBufferInterface  The {@link USTREAMBUFFER_INTERFACE} with the interface of the buffer. It
 *                              cannot be {@code NULL}, and it shall be a valid buffer that is the
 *                              implemented buffer type.
 * @param:  position                The {@code offset_t* const} to returns the logical current position in the
 *                              buffer. It cannot be {@code NULL}.
 * @return: The {@link USTREAMBUFFER_RESULT} with the result of the getCurrentPosition operation. The results can be:
 *          - @b USTREAMBUFFER_SUCCESS - If it provided the logical current position of the buffer.
 *          - @b USTREAMBUFFER_BUSY_EXCEPTION - If the resource necessary for the getting the logical current
 *              position is busy.
 *          - @b USTREAMBUFFER_CANCELLED_EXCEPTION - If the get logical current position was cancelled.
 *          - @b USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION - If one of the provided parameters is invalid.
 *          - @b USTREAMBUFFER_OUT_OF_MEMORY_EXCEPTION - If there is not enough memory to execute the get
 *              logical current position operation.
 *          - @b USTREAMBUFFER_SECURITY_EXCEPTION - If the get logical current position was denied for
 *              security reasons.
 *          - @b USTREAMBUFFER_SYSTEM_EXCEPTION - If the get logical current position operation failed on
 *              the system level.
 */
#define uStreamBufferGetCurrentPosition( \
            /*[USTREAMBUFFER_INTERFACE]*/ uStreamBufferInterface, \
            /*[offset_t* const]*/ position) \
    ((uStreamBufferInterface)->api->getCurrentPosition( \
            (uStreamBufferInterface), \
            (position)))

/**
 * @brief   Releases all the resources related to the content of the buffer before and including the released position.
 *
 * <p> Calling this API will notify the buffer that the user will not need its content from the start
 *      to {@code position} (inclusive). It means that the implementation of the buffer can dispose
 *      any resources allocated to control and contain this part of the buffer.
 * <p> It is up to the implementation to decide to use the release position. For example, if the
 *      buffer is a string in the Flash, it do not make sense to release control over it.
 * <p> The provided position shall be the logical position, and it shall be between the logical first
 *      position of the buffer and the logical current position minus one. For example, the following
 *      code releases all bytes from the start to the last received position:
 *
 * <pre><code>
 * offset_t pos;
 * if(uStreamBufferGetCurrentPosition(myBuffer, pos) == USTREAMBUFFER_SUCCESS)
 * {
 *     uStreamBufferRelease(myBuffer, pos - 1);
 * }
 * </code></pre>
 *
 * <p> The release API shall follow the following requirements:
 *      - The release shall dispose all resources necessary to handle the content of buffer before and 
 *          including the release position.
 *      - If the release position is after the current position or the buffer size, the release shall
 *          return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION, and do not release any resource.
 *      - If the release position is already released, the release shall return
 *          USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION, and do not release any resource.
 *      - If the provided interface is NULL, the release shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *      - If the provided interface is not the implemented buffer type, the release shall return
 *          USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *
 * @param:  uStreamBufferInterface       The {@link USTREAMBUFFER_INTERFACE} with the interface of the buffer. It
 *                              cannot be {@code NULL}, and it shall be a valid buffer that is the
 *                              implemented buffer type.
 * @param:  position        The {@code offset_t} with the position in the buffer to release. The
 *                              buffer will release the {@code uint8_t} on the position and all {@code uint8_t} 
 *                              before the position. It shall be bigger than 0.
 * @return: The {@link USTREAMBUFFER_RESULT} with the result of the release operation. The results can be:
 *          - @b USTREAMBUFFER_SUCCESS - If the buffer releases the position with success.
 *          - @b USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION - If one of the provided parameters is invalid.
 *          - @b USTREAMBUFFER_NO_SUCH_ELEMENT_EXCEPTION - If the position is already released.
 *          - @b USTREAMBUFFER_SYSTEM_EXCEPTION - If the release operation failed on the system level.
 */
#define uStreamBufferRelease( \
            /*[USTREAMBUFFER_INTERFACE]*/ uStreamBufferInterface, \
            /*[offset_t]*/ position) \
    ((uStreamBufferInterface)->api->release( \
            (uStreamBufferInterface), \
            (position)))

/**
 * @brief   Creates a new instance of the buffer and return it.
 *
 * <p> Clone a buffer will create a new instance of the buffer that share the same content of the
 *      original one. The clone shall not copy the content of the buffer, but only add a reference to
 *      it.
 * <p> Both, the start position and the current position of the cloned buffer will be the current
 *      position of the original buffer. The logical position of it will be determined by the provided
 *      offset.
 *
 * <i><b>Example 1:
 * <p> Consider a buffer with 1500 bytes, that was created from the factory, with `Logical` and `Inner`
 *      positions with label `0`. After some operations, 1000 bytes was read (from 0 to 999), so, the
 *      current position is `1000`, and 200 bytes was released (from 0 to 199), so the released
 *      position is `199`.
 * <p> For the following examples, the positions is represented by {@code [Logical, Inner]}.
 *
 * <pre><code>
 * Original buffer:
 *
 *  |<- start [0, 0]    |<- released [199, 199]           |<- current [1000, 1000]            |<- end [1499, 1499]
 *  |-------------------+---------------------------------+-----------------------------------|
 *
 *
 * Clone this original buffer with offset 0 will result in the follow buffer:
 *
 *  released [-1, 999] ->||<- start, current [0, 1000]        |<- end [499, 1499]
 *                       ||-----------------------------------|
 *
 *
 * Clone the same original buffer with offset 100 will result in the follow buffer:
 *
 *  released [99, 999] ->||<- start, current [100, 1000]      |<- end [599, 1499]
 *                       ||-----------------------------------|
 * </code></pre>
 *
 * <i><b>Example 2:
 * <p> Consider a buffer with 5000 bytes, that was created from the factory, with `Logical` and `Inner`
 *      positions with label `0`. After some operations, 250 bytes was read (from 0 to 249), so, the
 *      current position is `250`, and no release was made, so the released position is still `-1`.
 * <p> For the following examples, the positions is represented by {@code [Logical, Inner]}.
 *
 * <pre><code>
 * Original buffer:
 *
 *  released [-1, 0] ->||<- start [0, 0]    |<- current [250, 250]                                |<- end [4999, 4999]
 *                     ||-------------------+-----------------------------------------------------|
 *
 *
 * Clone this original buffer with offset 10000 will result in the follow buffer:
 *
 *  released [9999, 249] ->||<- start, current [10000, 250]        |<- end [14749, 4999]
 *                         ||--------------------------------------|
 *
 * </code></pre>
 *
 * <i><b>Example 3:
 * <p> From the previews cloned buffer, after some operations, the Logical current position is moved
 *      to `11000`, and the Logical released position is `10499`.
 * <p> For the following examples, the positions is represented by {@code [Logical, Inner]}.
 *
 * <pre><code>
 * Previews cloned buffer:
 *
 *  |<- start [10000, 250]    |<- released [10499, 749]    |<- current [11000, 1250]           |<- end [14749, 4999]
 *  |-------------------------+----------------------------+-----------------------------------|
 *
 * Clone this cloned buffer with offset 0 will result in the follow buffer:
 *
 *  released [-1, 1249] ->||<- start, current [0, 1250]           |<- end [3749, 4999]
 *                        ||--------------------------------------|
 *
 * </code></pre>
 *
 * <p> **NOTE:**
 *      - From the point of view of an uStreamBuffer user, the `Inner` position never matters, it will
 *          always use the `Logical` position for all operations.
 *      - If the position is not important to the caller, make the offset equals `0` is a safe option.
 *
 * <p> The clone API shall follow the following minimum requirements:
 *      - The clone shall return a buffer with the same content of the original buffer.
 *      - If the provided interface is NULL, the clone shall return NULL.
 *      - If the provided interface is not type of the implemented buffer, the clone shall return NULL.
 *      - If there is not enough memory to control the new buffer, the clone shall return NULL.
 *      - If the provided offset plus the buffer size is bigger than the maximum size_t, the clone
 *          shall return NULL.
 *      - The cloned buffer shall not interfere in the instance of the original buffer and vice versa.
 *
 * @param:  uStreamBufferInterface       The {@link USTREAMBUFFER_INTERFACE} with the interface of the buffer.
 *                              It cannot be {@code NULL}, and it shall be a valid buffer that is
 *                              type of the implemented buffer.
 * @param:  offset          The {@code offset_t} with the `Logical` position of the first byte in
 *                              the cloned buffer.
 * @return: The {@link USTREAMBUFFER_INTERFACE} with the result of the clone operation. The results can be:
 *          - @b not NULL - If the buffer was copied with success.
 *          - @b NULL - If one of the provided parameters is invalid or there is not enough memory to
 *              control the new buffer.
 */
#define uStreamBufferClone( \
            /*[USTREAMBUFFER_INTERFACE]*/ uStreamBufferInterface, \
            /*[offset_t]*/ offset) \
    ((uStreamBufferInterface)->api->clone( \
            (uStreamBufferInterface), \
            (offset)))

/**
 * @brief   Release all the resources allocated to control the instance of the buffer.
 *
 * <p> The dispose will release the instance of the buffer and decrement the reference of the buffer.
 *          If there is no more references to the buffer, the dispose will release all recources
 *          allocated to control the buffer.
 *
 * <p> The release API shall follow the following requirements:
 *      - The dispose shall free all allocated resources for the instance of the buffer.
 *      - If there is no more instances of the buffer, the dispose shall release all allocated
 *          resources to control the buffer.
 *      - If the provided interface is NULL, the dispose shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *      - If the provided interface is not type of the implemented buffer, the dispose shall return
 *          USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
 *
 * @param:  uStreamBufferInterface       The {@link USTREAMBUFFER_INTERFACE} with the interface of the buffer. It
 *                              cannot be {@code NULL}, and it shall be a valid buffer that is type
 *                              of the implemented buffer.
 * @return: The {@link USTREAMBUFFER_RESULT} with the result of the dispose operation. The results can be:
 *          - @b USTREAMBUFFER_SUCCESS - If the instance of the buffer was disposed with success.
 *          - @b USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION - If one of the provided parameters is invalid.
 */
#define uStreamBufferDispose( \
            /*[USTREAMBUFFER_INTERFACE]*/ uStreamBufferInterface) \
    ((uStreamBufferInterface)->api->dispose( \
            (uStreamBufferInterface)))

 /**
  * @brief   Append a uStreamBuffer to the existing buffer.
  *
  * <p> The append will add a buffer at the end of the current one. To do that, the append will convert 
  *         the current buffer in a {@link uStreamBuffer}, and append a clone of the provided buffer
  *         on it. If the current buffer is already an instance of {@link uStreamBuffer}, this API
  *         will only append the new buffer.
  *
  * <p> The Append API shall follow the following requirements:
  *      - The Append shall append the provided buffer at the end of the current one.
  *      - If current buffer is not a multibuffer, the Append shall convert the current buffer in a multibuffer.
  *      - If the provided interface is NULL, the Append shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
  *      - If the provided buffer to add is NULL, the Append shall return USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION.
  *      - If there is not enough memory to append the buffer, the Append shall return 
  *         USTREAMBUFFER_OUT_OF_MEMORY_EXCEPTION.
  *
  * @param:  uStreamBufferInterface       The {@link USTREAMBUFFER_INTERFACE} with the interface of 
  *                             the buffer. It cannot be {@code NULL}, and it shall be a valid buffer.
  * @param:  uStreamBufferToAppend        The {@link USTREAMBUFFER_INTERFACE} with the interface of 
  *                             the buffer to be appended to the original buffer. It cannot be {@code NULL}, 
  *                             and it shall be a valid buffer.
  * @return: The {@link USTREAMBUFFER_RESULT} with the result of the Append operation. The results can be:
  *          - @b USTREAMBUFFER_SUCCESS - If the buffer was appended with success.
  *          - @b USTREAMBUFFER_ILLEGAL_ARGUMENT_EXCEPTION - If one of the provided parameters is invalid.
  *          - @b USTREAMBUFFER_OUT_OF_MEMORY_EXCEPTION - If there is no memory to append the buffer.
  */
MOCKABLE_FUNCTION( , 
    USTREAMBUFFER_RESULT, uStreamBufferAppend,
    USTREAMBUFFER_INTERFACE, uStreamBufferInterface, 
    USTREAMBUFFER_INTERFACE, uStreamBufferToAppend);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_USTREAMBUFFER_BASE_H_ */
