// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_INC_USTREAM_BASE_H_
#define AZURE_ULIB_C_INC_USTREAM_BASE_H_

/**
 * @brief uStream Interface.
 *
 * <p> This is the definition of a heterogeneous buffer that helps other modules in a system
 *      expose large amounts of data without using a large amount of memory. Modules in the system can expose
 *      their own data using this interface. To do that, the module shall implement the functions in the
 *      interface. This implementation shall follow the definition described in this file, which includes
 *      not only the prototype of the header, but the behavior as well.
 * <p> uStream defines a provider-consumer interface when:
 *      - @b Provider - is the module of code that handles data to be exposed. This module implements
 *          the uStream interface to expose the data to the consumer.
 *      - @b Consumer - is the module of code that will use the data exposed by the provider.
 *
 * <p> The uStream shall have a clear separation between the internal content (provider domain)
 *      and what it exposes as external content (consumer domain). The uStream shall never expose 
 *      the internal content (ex: providing a pointer to a internal memory position). All 
 *      exposed content shall be copied from the internal data source to some given external memory. To do
 *      that in a clear way, the uStream shall always work with the concept of two buffers, the 
 *      `data source` and the `local buffer`, adhering to the following definition:
 *      - @b Data source - is the place where the data is stored by the implementation of the buffer
 *          interface. The data source is in the provider domain, and it shall be protected, immutable, 
 *          and non volatile. Consumers can read the data from the data source by the calling
 *          the {@link ustream_read} API, which will copy a snapshot of the data to the
 *          provided external memory, called local buffer.
 *      - @b Local buffer - is the consumer domain buffer, where the {@link ustream_read} API 
 *          will copy the required bytes from the data source. The local buffer belongs to the consumer 
 *          of this interface, which means that the consumer shall allocate and free this memory, and the
 *          content of the local buffer can be changed and released.
 *
 * <i><b>Example:
 * <p> A provider wants to create a uStream to expose data to the consumer. The provider will 
 *      store the content in the HEAP, and will create a uStream from it, passing the ownership of 
 *      the content to the uStream. Consumer will print the content of the uStream, using a 
 *      local buffer of 1K. The following diagram represents this operation.
 *
 * <pre><code>
 *  +----------------+        +----------------+         +------------------+     +------------+
 *  |    Provider    |        |    Consumer    |         |      uStream     |     |    HEAP    |
 *  +----------------+        +----------------+         +------------------+     +------------+
 *          |                         |                            |                    |
 *          |<--GetProviderContent()--+                            |                    |
 *          +----------------------------malloc(content_size)-------------------------->|
 *          |<--------------------------------content_ptr-------------------------------+
 *   +------+                         |                            |                    |
 *   | generate the content and store in the content_ptr           |                    |
 *   +----->|                         |                            |                    |
 *          +-----ustream_create                                   |                    |
 *          |       (content_ptr, content_size, take_ownership)--->|                    |
 *          |                         |                     +------+                    |
 *          |                         |                     | data_source = content_ptr |
 *          |                         |                     | data_sourceSize = content_size
 *          |                         |                     | ownership = true          |
 *          |                         |                     +----->|                    |
 *          |<-----------------ustream_interface-------------------+                    |
 *          +-ustream_interface------>|                            |                    |
 *
 *  Now that the consumer has the uStream with the content, it will print it using the 
 *   iterator ustream_read.
 *
 *          |                         +------------------malloc(1024)------------------>|
 *          |                         |<-----------------local_buffer-------------------+
 *  .. while ustream_read return ULIB_SUCCESS .....................................................
 *  :       |                         +-ustream_read               |                    |         :
 *  :       |                         |  (ustream_interface,       |                    |         :
 *  :       |                         |   local_buffer,            |                    |         :
 *  :       |                         |   1024)------------------->|                    |         :
 *  :       |                         |                     +------+                    |         :
 *  :       |                         |                     | copy the next 1024 bytes from the   :
 *  :       |                         |                     |  data_source to the local_buffer.   :
 *  :       |                         |                     +----->|                    |         :
 *  :       |                         |<---ULIB_SUCCESS---------+                       |         :
 *  :       |                  +------+                            |                    |         :
 *  :       |                  | use the content in the local_buffer                    |         :
 *  :       |                  +----->|                            |                    |         :
 *  ...............................................................................................
 *          |                         +---------------free(local_buffer)--------------->|
 *          |                         +-ustream_dispose            |                    |
 *          |                         |       (ustream_interface)->|                    |
 *          |                         |                            +-free(data_source)->|
 *          |                         |                            |                    |
 * </code></pre>
 *
 * <i><b> Heterogeneous buffer:
 * <p> Data can be stored in multiple, different medias, like RAM, flash, file, or cloud. Each media
 *      has its own read requirements. A simple way to unify it is copying it all to the RAM. For
 *      example, if an HTTP package contains a header that is in the flash, with some data in the RAM
 *      and the content in a file in the external flash, to concatenate it all in a single datagram
 *      you can allocate a single area in the RAM that fits it all, and bring all the data to this
 *      memory. The problem with this approach is the amount of memory required for that, which can be
 *      multiple times the total RAM that you have for the entire system.
 * <p> A second option to solve this problem is to make each component that needs to access this data
 *      understand each media and implement code to handle it. This approach will not require
 *      storing all data in the RAM, but will increase the size of the program itself, and
 *      is not easily portable, as different hardware will contain different media with different
 *      requirements.
 * <p> The `uStream` resolves this problem by creating a single interface that can handle any media,
 *      exposing it as a standard iterator. Whoever wants to expose a type of media as a uStream shall
 *      implement the functions described on the interface, handling all implementation details for
 *      each API. For example, the  {@link ustream_read} can be a simple copy of the flash to
 *      the RAM for a buffer that handles constants, or be as complex as creating a TCP/IP connection
 *      to bring the data for a buffer that handles data in the cloud.
 * <p> The consumer of the uStream can use all kinds of media in the same way, and may easily
 *      concatenate it by exposing a uStream that handles multiple uStream's.
 *
 * <i><b> Ownership:
 * <p> The uStream is an owner-less buffer: every instance of the buffer has the same rights. They
 *      all can read the buffer content, release the parts that are not necessary anymore, and dispose
 *      it.
 * <p> Each instance of the uStream is owned by who created it, and should never be shared by 
 *      multiple consumers. When a consumer receives a uStream and intends to make operations over 
 *      it, this consumer must first make a clone of the buffer, creating its own instance of it, and 
 *      then make the needed operations.
 * <p> Cloning a buffer creates a new set of controls for the buffer that will share the same content of
 *      the original buffer. The content itself is a smart pointer with a {@code refCount} that
 *      controls the total number of instances.
 * <p> Disposing an instance of the buffer will decrease the {@code refCount} of this buffer. If the
 *      number of references reaches 0, the buffer will destroy itself, releasing all allocated memory.
 *      {@b Not disposing an instance of the buffer will leak memory}.
 * <p> Instances of the buffer can be created in 2 ways:
 *      - @b Factory - when a producer exposes data using this buffer, it must create the buffer
 *          using a factory, so the operation buffer {@code Create} returns the first instance of the
 *          buffer.
 *      - @b Clone - when a consumer needs a copy of the buffer, it can use the {@link ustream_clone}.
 *
 * <i><b> Thread safe:
 * <p> The uStream **IS NOT** thread safe for multiple accesses over the same instance. The ownership
 *      of the instance of a uStream shall **NOT** be shared, especially not by consumers that run on 
 *      different threads. The owner thread shall create a clone of the uStream and pass it to the other 
 *      thread.
 * <p> The uStream **IS** thread safe for accesses between instances. It means that any access to
 *      memory shared by multiple instances shall be thread safe.
 *
 * <i><b> Data retention:
 * <p> As with any buffer, this buffer shall be used to handle data that was created by the producer as a
 *      result of an operation.
 * <p> This interface only exposes read functions, so once created, the content of the buffer cannot
 *      be changed by the producer of any of the consumers. Changing the content of the data source will
 *      result in a data mismatch.
 * <p> Consumers can do a partial release of the buffer by calling {@link ustream_release}.
 *      Calling the release does not imply that part of the memory will be immediately released. Once a
 *      buffer can handle multiple instances, a memory can only be free if all instances released it.
 *      A buffer implementation can or cannot have the ability to do partial releases. For instance, a
 *      buffer that handles constant data stored in the flash will never release any memory on the
 *      {@link ustream_release} API.
 * <p> Released data cannot be accessed, even if it is still available in the memory.
 *
 * <i><b> Appendable:
 * <p> New data can be appended at the end of the uStream by calling  {@link ustream_append}.
 *      This can include uStream's from other different medias. In this way, the uStream can
 *      be used as a Stream of data.
 * <p> To protect the immutability of the uStream, appending a new uStream to an existing one will
 *      only affect the instance that is calling the {@link ustream_append}.
 * <i><b>Example:
 * <p> A producer created 3 uStreams named A, B, and C. At this point, it handles one instance of each
 *      buffer. A consumer received an instance of the buffer A and C, and appends C to A creating a new buffer AC.
 *      After that, the producer will append B to A, creating the new AB buffer.
 * <p> Observe the fact that the consumer appendeding C to A on its own instance didn't affect the buffer
 *      A on the producer, and when the producer appended B to A, it creates AB, not ACB, and it didn't change
 *      the consumer AB buffer creating ABC or ACB on it.
 *
 * <i><b> Lazy:
 * <p> The buffer can contain the full content, bring it into memory when required, or even create the content
 *      when it is necessary. The implementation of the {@link ustream_read} function can be smart 
 *      enough to use the minimal amount of memory. 
 * <p> The only restriction is if a consumer accesses the same position of the buffer multiple times, it shall
 *      return the same data.
 * <i><b>Example:
 * <p> A random generator can expose random numbers using the uStream. To do that it shall generate a 
 *      new number when the consumer calls {@link ustream_read}. But to preserve the immutability,
 *      the implementation of the {@link ustream_read} shall store the number in a recover queue up
 *      to the point that the consumer releases this data. Because, if in some point in time, the consumer 
 *      seeks this old position, the {@link ustream_read} shall return the same value created in
 *      the first call of {@link ustream_read}.
 *
 * <i><b> Data conversion:
 * <p> When the data is copied from the data source to the local buffer, the {@link ustream_read}
 *      may do a data conversion, which means that the content exposed on the local buffer is a function
 *      of the content in the data source. It directly implies that the number of bytes written in the
 *      local buffer may be different than the number of bytes read from the data source.
 * <i><b>Example:
 * <p> A uStream can have the data source in binary format with 36 bytes, but it shall expose the 
 *      content encoded in base64. The base64 creates 4 encoded bytes for each 3 bytes read. So, seeking the 
 *      beginning of the file, the {@link ustream_get_remaining_size} shall return 48 (= 36 / 3 * 4),
 *      instead of 36. If the consumer provides a local buffer of 16 bytes, the {@link ustream_read} 
 *      shall read only 12 bytes from the data source, and encode it in base64 expanding the 12 bytes to 
 *      16 bytes on the local buffer.
 * <pre><code>
 *                  uStream domain                      ::      consumer domain
 *                                                      ::
 *                    Data source                       ::
 *                    +-------+--------------------+    ::
 *    binary data --> |       |                    |    ::
 *                    +-------+--------------------+    ::
 * inner position --> 0       12                   36   ::
 *                     \--+--/                          ::      Local buffer
 *                        | size = 12                   ::      +----------------+
 *                        +---> base64 encoder ---------------> | base64         |
 *                                                      ::      +----------------+
 *                                                      ::        size' = 16
 * </code></pre>
 *
 * <i><b> Data offset:
 * <p> In the data source, each byte is associated with a position, called {@b inner position}. The first 
 *      byte is always placed in the inner position `0`, followed by the other bytes which are incremented
 *      in a sequential manner. The uStream assigns a sequential number to each byte
 *      in the local buffer as well, called {@b logical position}. When a new uStream is created,
 *      the logical position matches the inner position, both starting at position `0`.
 * <p> When the buffer is cloned, an offset shall be provided. This offset is the new first logical 
 *      position. The implementation of the uStream shall handle the difference between the inner
 *      and logical position, making the conversion in all the uStream API. Providing an offset to a 
 *      buffer can be useful in many cases. For example, to concatenate buffers, the second buffer can 
 *      have an offset of the end of the first buffer plus one, or in the TCP connection, to make the 
 *      local position the same value of the octet sequence number.
 * <i><b>Example:
 * <p> A uStream was created from the flash with 100 bytes. The inner position is a sequence from
 *      `0` to `99`, and it matches the logical position. The consumer clones this buffer providing an
 *      offset of `1000`. The new instance contains the same content as the original one, but the 
 *      logical positions are now from `1000` to `1099`.
 * <p> If the owner of the first instance wants to set the position to position 10, it shall call {@link ustream_set_position}
 *      with the logical position 10. For the cloned instance, to set the position to the same position 10, it shall call 
 *      {@link ustream_set_position} with the logical position 1010.
 *
 * <i><b> Sliding window:
 * <p> One of the target use cases of the uStream is to accelerate and simplify the implementation of 
 *      sliding window protocols, like TCP. As described in this document, the uStream associates 
 *      a single byte (octet) to a single position, which means that every byte can be accessed by its
 *      position. For the consumer, this position is the logical position.
 * <p> To better understand the sliding window concept on the uStream, the Data source can be split
 *      in 4 segments.
 * <pre><code>
 *      Data Source:
 *           Released                       Pending                          Future
 *       |----------------|---------------------:--------------------|---------------------|
 *       |\                \                    :       Read         |\                    |
 *       | 0           First valid position     :                    | Current position    |
 *       |                                      :                    |                     |
 *       |                                      :<--- read size ---->|                     |
 *       |                                                                                 |
 *       |<--------------------------- data source size ---------------------------------->|
 * </code></pre>
 *      - @b Released - Sequence of bytes in the data source that is already acknowledged by the consumer, 
 *          and shall not be accessed anymore.
 *      - @b Pending - Sequence of bytes in the data source that is already read by the consumer, but not 
 *          acknowledged yet. The consumer can seek these bytes with {@link ustream_set_position} and read it again. This sequence starts at 
 *          the First Valid Position and ends at the last byte before the Current Position.
 *      - @b Read - Is the last read portion of the data source. On the read operation, the Read starts
 *          in the Current Position up to the read size. At the end of the read, this segment is 
 *          incorporated to Pending by changing the Current Position to the end of the Read.
 *      - @b Future - Sequence of bytes in the data source that is not read by the consumer yet. It starts 
 *          at the Current Position and ends at the end of the data source, which has the position calculated
 *          by data source size - 1. 
 *
 * <p> To read a new portion of the data source, the consumer shall provide memory (the local buffer), where 
 *      the implementation of the buffer will write the bytes that were read and converted from the data source. 
 *      The consumer can use this data in its own context: for example, to transmit as a TCP packet. When
 *      the consumer finishes using the data in the local buffer, this data can be discarded
 *      and the local buffer recycled to get the next portion of the data source.
 * <p> If at some point in the future, the consumer needs this data again, it can set the position to the needed position 
 *      and get the same content using the read.
 * <p> The consumer may confirm that a portion of the data is not necessary anymore. For example, after transmitting
 *      multiple TCP packets, the receiver of these packets answers with an ACK for a sequence number. In this case,
 *      the consumer can release this data in the data source by calling the {@link ustream_release}, moving 
 *      the First Valid Position to the next one after the released position.
 * <p> A common scenario is when the consumer needs to read over the data source starting on the first byte after
 *      the last released one. For example, when a timeout happens for a transmitted packet without ACK, the 
 *      sender shall retransmit the data starting from that point. In this case, the consumer can call the API
 *      {@link ustream_reset}. 
 */

#include "ulib_config.h"
#include "ulib_result.h"
#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
#include <cstdint>
#include <cstddef>
extern "C" {
#else
#include <stdint.h>
#include <stddef.h>
#endif /* __cplusplus */

/**
 * @brief   Define offset_t with the same size as size_t.
 */
typedef size_t offset_t;

/**
 * @brief   Forward declaration of USTREAM_INTERFACE
 */
typedef struct USTREAM_INTERFACE_TAG USTREAM_INTERFACE;

/**
 * @brief   Interface description.
 */
typedef struct USTREAM_TAG
{
    const USTREAM_INTERFACE* api;
    void* handle;
} USTREAM;

/**
 * @brief   vTable with the uStream APIs.
 *
 * <p> Any module that exposes the uStream shall implement the functions on this vTable.
 * <p> Any code that will use an exposed uStream shall call the APIs using the `uStream...`
 *      macros.
 */
struct USTREAM_INTERFACE_TAG
{
    ULIB_RESULT(*set_position)(USTREAM* ustream_interface, offset_t position);
    ULIB_RESULT(*reset)(USTREAM* ustream_interface);
    ULIB_RESULT(*read)(USTREAM* ustream_interface, uint8_t* const buffer, size_t buffer_length, size_t* const size);
    ULIB_RESULT(*get_remaining_size)(USTREAM* ustream_interface, size_t* const size);
    ULIB_RESULT(*get_position)(USTREAM* ustream_interface, offset_t* const position);
    ULIB_RESULT(*release)(USTREAM* ustream_interface, offset_t position);
    USTREAM*(*clone)(USTREAM* ustream_interface, offset_t offset);
    ULIB_RESULT(*dispose)(USTREAM* ustream_interface);
};


/**
 * @brief   Check if a handle is the same type of the API.
 *
 * <p> It will return true if the handle is valid and it is the same type of the API. It will
 *      return false if the handle is NULL or note the correct type.
 */
#define USTREAM_IS_NOT_TYPE_OF(handle, typeApi)   ((handle == NULL) || (handle->api != &typeApi))

/**
 * @brief   Change the current position of the buffer.
 *
 * <p> The current position is the one that will be returned in the local buffer by the next
 *      {@link ustream_read}. Consumers can call this API to go back or forward, but it cannot be exceed
 *      the end of the buffer or precede the fist valid position (last released position + 1).
 *
 * <p> The {@code ustream_set_position} API shall follow these minimum requirements:
 *      - The set_position shall change the current position of the buffer.
 *      - If the provided position is out of the range of the buffer, the set_position shall return
 *          ULIB_NO_SUCH_ELEMENT_ERROR, and will not change the current position.
 *      - If the provided position is already released, the set_position shall return
 *          ULIB_NO_SUCH_ELEMENT_ERROR, and will not change the current position.
 *      - If the provided interface is NULL, the set_position shall return ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented buffer type, the set_position shall return
 *          ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param:  ustream_interface       The {@link USTREAM*} with the interface of the buffer. It
 *                              cannot be {@code NULL}, and it shall be a valid buffer that is the
 *                              implemented buffer type.
 * @param:  position                The {@code offset_t} with the new current position in the buffer.
 * @return: The {@link ULIB_RESULT} with the result of the set_position operation. The results can be:
 *          - @b ULIB_SUCCESS - If the buffer changed the current position with success.
 *          - @b ULIB_BUSY_ERROR - If the resource necessary for the set_position operation is busy.
 *          - @b ULIB_CANCELLED_ERROR - If the set_position operation was cancelled.
 *          - @b ULIB_ILLEGAL_ARGUMENT_ERROR - If one of the provided parameters is invalid.
 *          - @b ULIB_NO_SUCH_ELEMENT_ERROR - If the position is out of the buffer range.
 *          - @b ULIB_OUT_OF_MEMORY_ERROR - If there is not enough memory to execute the
 *              set_position operation.
 *          - @b ULIB_SECURITY_ERROR - if the set_position operation was denied for security
 *              reasons.
 *          - @b ULIB_SYSTEM_ERROR - if the set_position operation failed on the system level.
 */
static inline ULIB_RESULT ustream_set_position(USTREAM* ustream_interface, offset_t position)
{
    return ustream_interface->api->set_position(ustream_interface, position);
}

/**
 * @brief   Changes the current position to the first valid position.
 *
 * <p> The current position is the one that will be returned in the local buffer by the next
 *      {@link ustream_read}. Reset will bring the current position to the first valid one, which
 *      is the first byte after the released position.
 *
 * <p> The {@code ustream_reset} API shall follow the following minimum requirements:
 *      - The reset shall change the current position of the buffer to the first byte after the
 *          released position.
 *      - If all bytes are already released, the buffer reset shall return
 *          ULIB_NO_SUCH_ELEMENT_ERROR, and will not change the current position.
 *      - If the provided interface is NULL, the buffer reset shall return ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented buffer type, the buffer reset shall return
 *          ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param:  ustream_interface       The {@link USTREAM*} with the interface of the buffer. It
 *                              cannot be {@code NULL}, and it shall be a valid buffer that is the
 *                              implemented buffer type.
 * @return: The {@link ULIB_RESULT} with the result of the reset operation. The results can be:
 *          - @b ULIB_SUCCESS - If the buffer changed the current position with success.
 *          - @b ULIB_BUSY_ERROR - If the resource necessary for the reset operation is
 *              busy.
 *          - @b ULIB_CANCELLED_ERROR - If the reset operation was cancelled.
 *          - @b ULIB_ILLEGAL_ARGUMENT_ERROR - If one of the provided parameters is invalid.
 *          - @b ULIB_NO_SUCH_ELEMENT_ERROR - If all previous bytes in the buffer were already
 *              released.
 *          - @b ULIB_OUT_OF_MEMORY_ERROR - If there is not enough memory to execute the
 *              reset operation.
 *          - @b ULIB_SECURITY_ERROR - If the reset operation was denied for security
 *              reasons.
 *          - @b ULIB_SYSTEM_ERROR - If the reset operation failed on the system level.
 */
static inline ULIB_RESULT ustream_reset(USTREAM* ustream_interface)
{
    return ustream_interface->api->reset(ustream_interface);
}

/**
 * @brief   Gets the next portion of the buffer starting at the current position.
 *
 * <p> The {@code ustream_read} API will copy the contents of the Data source to the local buffer
 *      starting at the current position. The local buffer is the one referenced by the parameter
 *      `buffer`, and with the maximum size buffer_length.
 * <p> The buffer is defined as a {@code uint8_t*} and can represent any sequence of data. Pay
 *      special attention, if the data is a string, the buffer will still copy it as a sequence of
 *      {@code uint8_t}, and will **NOT** put any terminator at the end of the string. The size of 
 *      the content copied in the local buffer will be returned in the parameter `size`.
 *
 * <p> The read API shall follow the following minimum requirements:
 *      - The read shall copy the contents of the Data Source to the provided local buffer.
 *      - If the contents of the Data Source is bigger than the `buffer_length`, the read shall
 *          limit the copy size up to the buffer_length.
 *      - The read shall return the number of valid {@code uint8_t} values in the local buffer in 
 *          the provided `size`.
 *      - If there is no more content to return, the read shall return
 *          ULIB_EOF, size shall receive 0, and will not change the contents
 *          of the local buffer.
 *      - If the provided buffer_length is zero, the read shall return ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided buffer_length is lower than the minimum number of bytes that the buffer can copy, the 
 *          read shall return ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is NULL, the read shall return ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented buffer type, the read shall return
 *          ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided local buffer is NULL, the read shall return ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided return size pointer is NULL, the read shall return
 *          ULIB_ILLEGAL_ARGUMENT_ERROR and will not change the local buffer contents or the
 *          current position of the buffer.
 *
 * @param:  ustream_interface       The {@link USTREAM*} with the interface of the buffer. It
 *                              cannot be {@code NULL}, and it shall be a valid buffer that is the
 *                              implemented buffer type.
 * @param:  buffer                  The {@code uint8_t* const} that points to the local buffer. It cannot be {@code NULL}.
 * @param:  buffer_length           The {@code size_t} with the size of the local buffer. It shall be
 *                              bigger than 0.
 * @param:  size                    The {@code size_t* const} that points to the place where the read shall store
 *                              the number of valid {@code uint8_t} values in the local buffer. It cannot be {@code NULL}.
 * @return: The {@link ULIB_RESULT} with the result of the read operation. The results can be:
 *          - @b ULIB_SUCCESS - If the buffer copied the content of the Data Source to the local buffer
 *              with success.
 *          - @b ULIB_BUSY_ERROR - If the resource necessary to copy the buffer content is busy.
 *          - @b ULIB_CANCELLED_ERROR - If the copy of the content was cancelled.
 *          - @b ULIB_ILLEGAL_ARGUMENT_ERROR - If one of the provided parameters is invalid.
 *          - @b ULIB_EOF - If there are no more {@code uint8_t} values in the 
 *              Data Source to copy.
 *          - @b ULIB_OUT_OF_MEMORY_ERROR - If there is not enough memory to execute the copy.
 *          - @b ULIB_SECURITY_ERROR - If the read was denied for security reasons.
 *          - @b ULIB_SYSTEM_ERROR - If the read operation failed on the system level.
 */
static inline ULIB_RESULT ustream_read(USTREAM* ustream_interface, uint8_t* const buffer, size_t buffer_length, size_t* const size)
{
    return ustream_interface->api->read(ustream_interface, buffer, buffer_length, size);
}

/**
 * @brief   Returns the remaining size of the buffer.
 *
 * <p> This API returns the number of bytes between the current position and the end of the buffer.
 *
 * <p> The {@code ustream_get_remaining_size} API shall follow the following minimum requirements:
 *      - The get_remaining_size shall return the number of bytes between the current position and the
 *          end of the buffer.
 *      - If the provided interface is NULL, the get_remaining_size shall return ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented buffer type, the get_remaining_size shall
 *          return ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided size is NULL, the get_remaining_size shall return ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param:  ustream_interface       The {@link USTREAM*} with the interface of the buffer. It
 *                              cannot be {@code NULL}, and it shall be a valid buffer that is the
 *                              implemented buffer type.
 * @param:  size                    The {@code size_t* const} to returns the remaining number of {@code uint8_t} values 
 *                              copied to the buffer. It cannot be {@code NULL}.
 * @return: The {@link ULIB_RESULT} with the result of the get_remaining_size operation. The results can be:
 *          - @b ULIB_SUCCESS - If it succeeded to get the remaining size of the buffer.
 *          - @b ULIB_BUSY_ERROR - If the resource necessary to the get the remain size of
 *              the buffer is busy.
 *          - @b ULIB_CANCELLED_ERROR - If the get remaining size was cancelled.
 *          - @b ULIB_ILLEGAL_ARGUMENT_ERROR - If one of the provided parameters is invalid.
 *          - @b ULIB_OUT_OF_MEMORY_ERROR - If there is not enough memory to execute the get
 *              remaining size operation.
 *          - @b ULIB_SECURITY_ERROR - If the get remaining size was denied for security reasons.
 *          - @b ULIB_SYSTEM_ERROR - If the get remaining size operation failed on the
 *              system level.
 */
static inline ULIB_RESULT ustream_get_remaining_size(USTREAM* ustream_interface, size_t* const size)
{
    return ustream_interface->api->get_remaining_size(ustream_interface, size);
}

/**
 * @brief   Returns the current position in the buffer.
 *
 * <p> This API returns the logical current position.
 *
 * <p> The {@code ustream_get_position} API shall follow the following minimum requirements:
 *      - The get_position shall return the logical current position of the buffer.
 *      - If the provided interface is NULL, the get_position shall return ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented buffer type, the get_position
 *          shall return ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided position is NULL, the get_position shall return ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param:  ustream_interface       The {@link USTREAM*} with the interface of the buffer. It
 *                              cannot be {@code NULL}, and it shall be a valid buffer that is the
 *                              implemented buffer type.
 * @param:  position                The {@code offset_t* const} to returns the logical current position in the
 *                              buffer. It cannot be {@code NULL}.
 * @return: The {@link ULIB_RESULT} with the result of the get_position operation. The results can be:
 *          - @b ULIB_SUCCESS - If it provided the logical current position of the buffer.
 *          - @b ULIB_BUSY_ERROR - If the resource necessary for the getting the logical current
 *              position is busy.
 *          - @b ULIB_CANCELLED_ERROR - If the get logical current position was cancelled.
 *          - @b ULIB_ILLEGAL_ARGUMENT_ERROR - If one of the provided parameters is invalid.
 *          - @b ULIB_OUT_OF_MEMORY_ERROR - If there is not enough memory to execute the get
 *              logical current position operation.
 *          - @b ULIB_SECURITY_ERROR - If the get logical current position was denied for
 *              security reasons.
 *          - @b ULIB_SYSTEM_ERROR - If the get logical current position operation failed on
 *              the system level.
 */
static inline ULIB_RESULT ustream_get_position(USTREAM* ustream_interface, offset_t* const position)
{
    return ustream_interface->api->get_position(ustream_interface, position);
}

/**
 * @brief   Releases all the resources related to the Data Source before and including the released position.
 *
 * <p> Calling this API will notify the buffer that the user will not need its content from the start
 *      to {@code position} (inclusive). It means that the implementation of the buffer can dispose
 *      any resources allocated to control and contain this part of the buffer.
 * <p> It is up to the implementation of the buffer to decide to release any resource. For example, if the
 *      buffer is a string in the Flash, it do not make sense to release it.
 * <p> The provided position shall be the logical position, and it shall be between the logical first
 *      valid position of the buffer and the logical current position minus one. For example, the following
 *      code releases all bytes from the start to the last received position:
 *
 * <pre><code>
 * offset_t pos;
 * if(ustream_get_position(myBuffer, &pos) == ULIB_SUCCESS)
 * {
 *     ustream_release(myBuffer, pos - 1);
 * }
 * </code></pre>
 *
 * <p> The {@code ustream_release} API shall follow the following requirements:
 *      - The release shall dispose all resources necessary to handle the content of buffer before and 
 *          including the release position.
 *      - If the release position is after the current position or the buffer size, the release shall
 *          return ULIB_ILLEGAL_ARGUMENT_ERROR, and do not release any resource.
 *      - If the release position is already released, the release shall return
 *          ULIB_NO_SUCH_ELEMENT_ERROR, and do not release any resource.
 *      - If the provided interface is NULL, the release shall return ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented buffer type, the release shall return
 *          ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param:  ustream_interface       The {@link USTREAM*} with the interface of the buffer. It
 *                              cannot be {@code NULL}, and it shall be a valid buffer that is the
 *                              implemented buffer type.
 * @param:  position                The {@code offset_t} with the position in the buffer to release. The
 *                              buffer will release the {@code uint8_t} on the position and all {@code uint8_t} 
 *                              before the position. It shall be bigger than 0.
 * @return: The {@link ULIB_RESULT} with the result of the release operation. The results can be:
 *          - @b ULIB_SUCCESS - If the buffer releases the position with success.
 *          - @b ULIB_ILLEGAL_ARGUMENT_ERROR - If one of the provided parameters is invalid.
 *          - @b ULIB_NO_SUCH_ELEMENT_ERROR - If the position is already released.
 *          - @b ULIB_SYSTEM_ERROR - If the release operation failed on the system level.
 */
static inline ULIB_RESULT ustream_release(USTREAM* ustream_interface, offset_t position)
{
    return ustream_interface->api->release(ustream_interface, position);
}

/**
 * @brief   Creates a new instance of the buffer and return it.
 *
 * <p> Clone a buffer will create a new instance of the buffer that share the same content of the
 *      original one. The clone shall not copy the content of the buffer, but only add a reference to
 *      it.
 * <p> Both, the start position and the current position of the cloned buffer will be the current
 *      position of the original buffer. The logical position of it will be determined by the provided
 *      offset.
 * <p> The size of the new buffer will be the remaining size of the original buffer, which is the size
 *      menus the current position.
 *
 * <i><b>Example 1:
 * <p> Consider a buffer with 1500 bytes, that was created from the factory, with `Logical` and `Inner`
 *      positions as `0`. After some operations, 1000 bytes was read (from 0 to 999), so, the
 *      current position is `1000`, and 200 bytes was released (from 0 to 199), so the released
 *      position is `199`.
 * <p> For the following examples, the positions is represented by {@code [Logical, Inner]}.
 *
 * <pre><code>
 * Original buffer:
 *
 *  |      Released     |             Pending             |               Future              |
 *  |-------------------|---------------------------------|-----------------------------------|
 *  |<- start [0, 0]    |<- released [199, 199]           |<- current [1000, 1000]            |<- end [1499, 1499]
 *
 *
 * Clone this original buffer with offset 0 will result in the follow buffer:
 *
 *                       |||             Future                |
 *                       |||-----------------------------------|
 *  released [-1, 999] ->|||<- start, current [0, 1000]        |<- end [499, 1499]
 *
 *
 * Clone the same original buffer with offset 100 will result in the follow buffer:
 *
 *                       |||             Future                |
 *                       |||-----------------------------------|
 *  released [99, 999] ->|||<- start, current [100, 1000]      |<- end [599, 1499]
 * </code></pre>
 *
 * <i><b>Example 2:
 * <p> Consider a buffer with 5000 bytes, that was created from the factory, with `Logical` and `Inner`
 *      positions as `0`. After some operations, 250 bytes was read (from 0 to 249), so, the
 *      current position is `250`, and no release was made, so the released position is still `-1`.
 * <p> For the following examples, the positions is represented by {@code [Logical, Inner]}.
 *
 * <pre><code>
 * Original buffer:
 *
 *                     ||     Pending       |                         Future                      |
 *                     ||-------------------+-----------------------------------------------------|
 *  released [-1, 0] ->||<- start [0, 0]    |<- current [250, 250]                                |<- end [4999, 4999]
 *
 *
 * Clone this original buffer with offset 10000 will result in the follow buffer:
 *
 *                         |||                Future                |
 *                         |||--------------------------------------|
 *  released [9999, 249] ->|||<- start, current [10000, 250]        |<- end [14749, 4999]
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
 *  |          Released       |           Pending          |               Future              |
 *  |-------------------------+----------------------------+-----------------------------------|
 *  |<- start [10000, 250]    |<- released [10499, 749]    |<- current [11000, 1250]           |<- end [14749, 4999]
 *
 * Clone this cloned buffer with offset 0 will result in the follow buffer:
 *
                          |||                Future                |
 *                        |||--------------------------------------|
 *  released [-1, 1249] ->|||<- start, current [0, 1250]           |<- end [3749, 4999]
 *
 * </code></pre>
 *
 * <p> **NOTE:**
 *      - From the point of view of an consumer, the `Inner` position never matters, it will
 *          always use the `Logical` position for all operations.
 *      - If the position is not important to the consumer, make the offset equals `0` is a safe option.
 *
 * <p> The {@code ustream_clone} API shall follow the following minimum requirements:
 *      - The clone shall return a buffer with the same content of the original buffer.
 *      - If the provided interface is NULL, the clone shall return NULL.
 *      - If the provided interface is not type of the implemented buffer, the clone shall return NULL.
 *      - If there is not enough memory to control the new buffer, the clone shall return NULL.
 *      - If the provided offset plus the buffer size is bigger than the maximum size_t, the clone
 *          shall return NULL.
 *      - The cloned buffer shall not interfere in the instance of the original buffer and vice versa.
 *
 * @param:  ustream_interface       The {@link USTREAM*} with the interface of the buffer.
 *                              It cannot be {@code NULL}, and it shall be a valid buffer that is
 *                              type of the implemented buffer.
 * @param:  offset                  The {@code offset_t} with the `Logical` position of the first byte in
 *                              the cloned buffer.
 * @return: The {@link USTREAM*} with the result of the clone operation. The results can be:
 *          - @b not NULL - If the buffer was copied with success.
 *          - @b NULL - If one of the provided parameters is invalid or there is not enough memory to
 *              control the new buffer.
 */
static inline USTREAM* ustream_clone(USTREAM* ustream_interface, offset_t offset)
{
    return ustream_interface->api->clone(ustream_interface, offset);
}

/**
 * @brief   Release all the resources allocated to control the instance of the buffer.
 *
 * <p> The dispose will release the instance of the buffer and decrement the reference of the buffer.
 *          If there is no more references to the buffer, the dispose will release all recourses
 *          allocated to control the buffer.
 *
 * <p> The {@code ustream_dispose} API shall follow the following requirements:
 *      - The dispose shall free all allocated resources for the instance of the buffer.
 *      - If there is no more instances of the buffer, the dispose shall release all allocated
 *          resources to control the buffer.
 *      - If the provided interface is NULL, the dispose shall return ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not type of the implemented buffer, the dispose shall return
 *          ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param:  ustream_interface       The {@link USTREAM*} with the interface of the buffer. It
 *                              cannot be {@code NULL}, and it shall be a valid buffer that is type
 *                              of the implemented buffer.
 * @return: The {@link ULIB_RESULT} with the result of the dispose operation. The results can be:
 *          - @b ULIB_SUCCESS - If the instance of the buffer was disposed with success.
 *          - @b ULIB_ILLEGAL_ARGUMENT_ERROR - If one of the provided parameters is invalid.
 */
static inline ULIB_RESULT ustream_dispose(USTREAM* ustream_interface)
{
    return ustream_interface->api->dispose(ustream_interface);
}

/**
  * @brief   Append a uStream to the existing buffer.
  *
  * <p> The append will add a buffer at the end of the current one. To do that, the append will convert 
  *         the current buffer in a {@link USTREAM_MULTI_INSTANCE}, and append a clone of the provided buffer
  *         on it. If the current buffer is already an instance of {@link USTREAM_MULTI_INSTANCE}, this API
  *         will only append the new buffer.
  *
  * <p> The {@code ustream_append} API shall follow the following requirements:
  *      - The Append shall append the provided buffer at the end of the current one.
  *      - If current buffer is not a multi-buffer, the Append shall convert the current buffer in a multi-buffer.
  *      - If the provided interface is NULL, the Append shall return ULIB_ILLEGAL_ARGUMENT_ERROR.
  *      - If the provided buffer to add is NULL, the Append shall return ULIB_ILLEGAL_ARGUMENT_ERROR.
  *      - If there is not enough memory to append the buffer, the Append shall return 
  *         ULIB_OUT_OF_MEMORY_ERROR.
  *
  * @param:  ustream_interface       The {@link USTREAM*} with the interface of 
  *                             the buffer. It cannot be {@code NULL}, and it shall be a valid buffer.
  * @param:  ustream_to_append       The {@link USTREAM*} with the interface of 
  *                             the buffer to be appended to the original buffer. It cannot be {@code NULL}, 
  *                             and it shall be a valid buffer.
  * @return: The {@link ULIB_RESULT} with the result of the Append operation. The results can be:
  *          - @b ULIB_SUCCESS - If the buffer was appended with success.
  *          - @b ULIB_ILLEGAL_ARGUMENT_ERROR - If one of the provided parameters is invalid.
  *          - @b ULIB_OUT_OF_MEMORY_ERROR - If there is no memory to append the buffer.
  */
MOCKABLE_FUNCTION( , 
    ULIB_RESULT, ustream_append,
    USTREAM*, ustream_interface, 
    USTREAM*, ustream_to_append);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_USTREAM_BASE_H_ */
