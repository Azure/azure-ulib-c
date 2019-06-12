// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_INC_USTREAM_BASE_H_
#define AZURE_ULIB_C_INC_USTREAM_BASE_H_

/**
 * @file ustream_base.h
 * 
 * @brief uStream Interface
 *
 *  This is the definition of a heterogeneous buffer that helps other modules in a system
 *      expose large amounts of data without using a large amount of memory. Modules in the system can expose
 *      their own data using this interface. To do that, the module shall implement the functions in the
 *      interface. This implementation shall follow the definition described in this file, which includes
 *      not only the prototype of the header, but the behavior as well.
 *  uStream defines a provider-consumer interface when:
 *      - <b>Provider</b> - is the module of code that handles data to be exposed. This module implements
 *          the uStream interface to expose the data to the consumer.
 *      - <b>Consumer</b> - is the module of code that will use the data exposed by the provider.
 *
 *  The uStream shall have a clear separation between the internal content (provider domain)
 *      and what it exposes as external content (consumer domain). The uStream shall never expose 
 *      the internal content (ex: providing a pointer to a internal memory position). All 
 *      exposed content shall be copied from the internal data source to some given external memory. To do
 *      that in a clear way, the uStream shall always work with the concept of two buffers, the 
 *      `data source` and the `local buffer`, adhering to the following definition:
 *      - <b>Data source</b> - is the place where the data is stored by the implementation of the buffer
 *          interface. The data source is in the provider domain, and it shall be protected, immutable, 
 *          and non volatile. Consumers can read the data from the data source by the calling
 *          the ustream_read() API, which will copy a snapshot of the data to the
 *          provided external memory, called local buffer.
 *      - <b>Local buffer</b> - is the consumer domain buffer, where the ustream_read() API 
 *          will copy the required bytes from the data source. The local buffer belongs to the consumer 
 *          of this interface, which means that the consumer shall allocate and free this memory, and the
 *          content of the local buffer can be changed and released.
 *
 * <i><b>Example</b></i>
 * 
 *  A provider wants to create a uStream to expose data to the consumer. The provider will 
 *      store the content in the HEAP, and will create a uStream from it, passing the ownership of 
 *      the content to the uStream. Consumer will print the content of the uStream, using a 
 *      local buffer of 1K. The following diagram represents this operation.
 *
 * <pre><code>
 *  +----------------+        +----------------+         +------------------+     +------------+
 *  |    Provider    |        |    Consumer    |         |      uStream     |     |    HEAP    |
 *  +----------------+        +----------------+         +------------------+     +------------+
 *          |                         |                            |                    |
 *          |<-get_provider_content()-+                            |                    |
 *          +----------------------------malloc(content_size)-------------------------->|
 *          |<--------------------------------content_ptr-------------------------------+
 *   +------+                         |                            |                    |
 *   | generate the content and store in the content_ptr           |                    |
 *   +----->|                         |                            |                    |
 *          +-----ustream_create                                   |                    |
 *          |       (content_ptr, content_size, take_ownership)--->|                    |
 *          |                         |                     +------+                    |
 *          |                         |                     | data_source = content_ptr |
 *          |                         |                     | data_source_size = content_size
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
 * <h2>Heterogeneous buffer</h2>
 *  Data can be stored in multiple, different medias, like RAM, flash, file, or cloud. Each media
 *      has its own read requirements. A simple way to unify it is copying it all to the RAM. For
 *      example, if an HTTP package contains a header that is in the flash, with some data in the RAM
 *      and the content in a file in the external flash, to concatenate it all in a single datagram
 *      you can allocate a single area in the RAM that fits it all, and bring all the data to this
 *      memory. The problem with this approach is the amount of memory required for that, which can be
 *      multiple times the total RAM that you have for the entire system.
 * 
 *  A second option to solve this problem is to make each component that needs to access this data
 *      understand each media and implement code to handle it. This approach will not require
 *      storing all data in the RAM, but will increase the size of the program itself, and
 *      is not easily portable, as different hardware will contain different media with different
 *      requirements.
 * 
 *  The uStream resolves this problem by creating a single interface that can handle any media,
 *      exposing it as a standard iterator. Whoever wants to expose a type of media as a uStream shall
 *      implement the functions described on the interface, handling all implementation details for
 *      each API. For example, the ustream_read() can be a simple copy of the flash to
 *      the RAM for a buffer that handles constants, or be as complex as creating a TCP/IP connection
 *      to bring the data for a buffer that handles data in the cloud.
 * 
 *  The consumer of the uStream can use all kinds of media in the same way, and may easily
 *      concatenate it by exposing a uStream that handles multiple uStream's.
 *
 * <h2>Ownership</h2>
 *  The uStream is an owner-less buffer: every instance of the buffer has the same rights. They
 *      all can read the buffer content, release the parts that are not necessary anymore, and dispose
 *      it.
 *  Each instance of the uStream is owned by who created it, and should never be shared by 
 *      multiple consumers. When a consumer receives a uStream and intends to make operations over 
 *      it, this consumer must first make a clone of the buffer, creating its own instance of it, and 
 *      then make the needed operations.
 * <p> Cloning a buffer creates a new set of controls for the buffer that will share the same content of
 *      the original buffer. The content itself is a smart pointer with a <tt>ref_count</tt> that
 *      controls the total number of instances.
 * <p> Disposing an instance of the buffer will decrease the <tt>ref_count</tt> of this buffer. If the
 *      number of references reaches 0, the buffer will destroy itself, releasing all allocated memory.
 *      <b>Not disposing an instance of the buffer will leak memory</b>.
 *  Instances of the buffer can be created in 2 ways:
 *      - @b Factory - when a producer exposes data using this buffer, it must create the buffer
 *          using a factory, so the operation <tt>buffer create</tt> returns the first instance of the
 *          buffer.
 *      - @b Clone - when a consumer needs a copy of the buffer, it can use the ustream_clone().
 *
 * <h2>Thread safe</h2>
 *  The uStream **IS NOT** thread safe for multiple accesses over the same instance. The ownership
 *      of the instance of a uStream shall <b>NOT</b> be shared, especially not by consumers that run on 
 *      different threads. The owner thread shall create a clone of the uStream and pass it to the other 
 *      thread.
 *  The uStream <b>IS</b> thread safe for accesses between instances. It means that any access to
 *      memory shared by multiple instances shall be thread safe.
 *
 * <h2>Data retention</h2>
 *  As with any buffer, this buffer shall be used to handle data that was created by the producer as a
 *      result of an operation.
 * 
 *  This interface only exposes read functions, so once created, the content of the buffer cannot
 *      be changed by the producer of any of the consumers. Changing the content of the data source will
 *      result in a data mismatch.
 * 
 *  Consumers can do a partial release of the buffer by calling ustream_release().
 *      Calling the release does not imply that part of the memory will be immediately released. Once a
 *      buffer can handle multiple instances, a memory can only be free if all instances released it.
 *      A buffer implementation can or cannot have the ability to do partial releases. For instance, a
 *      buffer that handles constant data stored in the flash will never release any memory on the
 *      ustream_release() API.
 * 
 *  Released data cannot be accessed, even if it is still available in the memory.
 *
 * <h2>Appendable</h2>
 *  New data can be appended at the end of the uStream by calling ustream_append().
 *      This can include uStream's from other different medias. In this way, the uStream can
 *      be used as a Stream of data.
 *  To protect the immutability of the uStream, appending a new uStream to an existing one will
 *      only affect the instance that is calling the ustream_append().
 * 
 * <i><b>Example</b></i>
 *  A producer created 3 uStreams named A, B, and C. At this point, it handles one instance of each
 *      buffer. A consumer received an instance of the buffer A and C, and appends C to A creating a new buffer AC.
 *      After that, the producer will append B to A, creating the new AB buffer.
 * 
 *  Observe the fact that the consumer appendeding C to A on its own instance didn't affect the buffer
 *      A on the producer, and when the producer appended B to A, it creates AB, not ACB, and it didn't change
 *      the consumer AB buffer creating ABC or ACB on it.
 *
 * <h2>Lazy</h2>
 *  The buffer can contain the full content, bring it into memory when required, or even create the content
 *      when it is necessary. The implementation of the ustream_read() function can be smart 
 *      enough to use the minimal amount of memory.
 * 
 *  The only restriction is if a consumer accesses the same position of the buffer multiple times, it shall
 *      return the same data.
 * 
 * <i><b>Example</b></i>
 *  A random generator can expose random numbers using the uStream. To do that it shall generate a 
 *      new number when the consumer calls ustream_read(). But to preserve the immutability,
 *      the implementation of the ustream_read() shall store the number in a recover queue up
 *      to the point that the consumer releases this data. Because, if in some point in time, the consumer 
 *      seeks this old position, the ustream_read() shall return the same value created in
 *      the first call of ustream_read().
 *
 * <h2>Data conversion</h2>
 *  When the data is copied from the data source to the local buffer, the ustream_read()
 *      may do a data conversion, which means that the content exposed on the local buffer is a function
 *      of the content in the data source. It directly implies that the number of bytes written in the
 *      local buffer may be different than the number of bytes read from the data source.
 * 
 * <i><b>Example</b></i>
 *  A uStream can have the data source in binary format with 36 bytes, but it shall expose the 
 *      content encoded in base64. The base64 creates 4 encoded bytes for each 3 bytes read. So, seeking the 
 *      beginning of the file, the ustream_get_remaining_size() shall return 48 (= 36 / 3 * 4),
 *      instead of 36. If the consumer provides a local buffer of 16 bytes, the ustream_read() 
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
 * <h2>Data offset</h2>
 *  In the data source, each byte is associated with a position, called <tt>inner position</tt>. The first 
 *      byte is always placed in the inner position <tt>0</tt>, followed by the other bytes which are incremented
 *      in a sequential manner. The uStream assigns a sequential number to each byte
 *      in the local buffer as well, called <tt>logical position</tt>. When a new uStream is created,
 *      the logical position matches the inner position, both starting at position <tt>0</tt>.
 * 
 *  When the buffer is cloned, an offset shall be provided. This offset is the new first logical 
 *      position. The implementation of the uStream shall handle the difference between the inner
 *      and logical position, making the conversion in all the uStream API. Providing an offset to a 
 *      buffer can be useful in many cases. For example, to concatenate buffers, the second buffer can 
 *      have an offset of the end of the first buffer plus one, or in the TCP connection, to make the 
 *      local position the same value of the octet sequence number.
 * 
 * <i><b>Example</b></i>
 *  A uStream was created from the flash with 100 bytes. The inner position is a sequence from
 *      <tt>0</tt> to <tt>99</tt>, and it matches the logical position. The consumer clones this buffer providing an
 *      offset of <tt>1000</tt>. The new instance contains the same content as the original one, but the 
 *      logical positions are now from <tt>1000</tt> to <tt>1099</tt>.
 * 
 *  If the owner of the first instance wants to set the position to position 10, it shall call ustream_set_position()
 *      with the logical position 10. For the cloned instance, to set the position to the same position 10, it shall call 
 *      ustream_set_position() with the logical position 1010.
 *
 * <h2>Sliding window</h2>
 *  One of the target use cases of the uStream is to accelerate and simplify the implementation of 
 *      sliding window protocols, like TCP. As described in this document, the uStream associates 
 *      a single byte (octet) to a single position, which means that every byte can be accessed by its
 *      position. For the consumer, this position is the logical position.
 * 
 *  To better understand the sliding window concept on the uStream, the Data source can be split
 *      in 4 segments.
 * <pre><code>
 *      <tt>Data Source</tt>:
 *           Released                       Pending                          Future
 *       |----------------|---------------------:--------------------|---------------------|
 *       |\                \                    :       Read         |\                    |
 *       | 0           First Valid Position     :                    | Current Position    |
 *       |                                      :                    |                     |
 *       |                                      :<--- Read Size ---->|                     |
 *       |                                                                                 |
 *       |<--------------------------- <tt>Data Source</tt> Size ---------------------------------->|
 * </code></pre>
 *      - @b Released - Sequence of bytes in the data source that is already acknowledged by the consumer, 
 *          and shall not be accessed anymore.
 *      - @b Pending - Sequence of bytes in the data source that is already read by the consumer, but not 
 *          acknowledged yet. The consumer can seek these bytes with ustream_set_position() and read it again. This sequence starts at 
 *          the <tt>First Valid Position</tt> and ends at the last byte before the <tt>Current Position</tt>.
 *      - @b Read - Is the last read portion of the data source. On the read operation, the <tt>Read</tt> starts
 *          in the <tt>Current Position</tt> up to the <tt>Read Size</tt>. At the end of the read, this segment is 
 *          incorporated to <tt>Pending</tt> by changing the <tt>Current Position</tt> to the end of the Read.
 *      - @b Future - Sequence of bytes in the data source that is not read by the consumer yet. It starts 
 *          at the <tt>Current Position</tt> and ends at the end of the data source, which has the position calculated
 *          by <tt><tt>Data Source</tt> Size - 1</tt>. 
 *
 *  To read a new portion of the data source, the consumer shall provide memory (the local buffer), where 
 *      the implementation of the buffer will write the bytes that were read and converted from the data source. 
 *      The consumer can use this data in its own context: for example, to transmit as a TCP packet. When
 *      the consumer finishes using the data in the local buffer, this data can be discarded
 *      and the local buffer recycled to get the next portion of the data source.
 * 
 *  If at some point in the future, the consumer needs this data again, it can set the position to the needed position 
 *      and get the same content using the read.
 * 
 *  The consumer may confirm that a portion of the data is not necessary anymore. For example, after transmitting
 *      multiple TCP packets, the receiver of these packets answers with an ACK for a sequence number. In this case,
 *      the consumer can release this data in the data source by calling the ustream_release(), moving 
 *      the <tt>First Valid Position</tt> to the next one after the released position.
 * 
 *  A common scenario is when the consumer needs to read over the data source starting on the first byte after
 *      the last released one. For example, when a timeout happens for a transmitted packet without ACK, the 
 *      sender shall retransmit the data starting from that point. In this case, the consumer can call the API
 *      ustream_reset(). 
 *
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
    const USTREAM_INTERFACE* api;   /**<api handle for USTREAM instance */
    void* handle;                   /**<handle to data control block */
} USTREAM;

/**
 * @brief   vTable with the uStream APIs.
 *
 *  Any module that exposes the uStream shall implement the functions on this vTable.
 *
 *  Any code that will use an exposed uStream shall call the APIs using the `ustream_...`
 *      inline functions.
 */
struct USTREAM_INTERFACE_TAG
{
    ULIB_RESULT(*set_position)(USTREAM* ustream_interface, offset_t position);          /**<internal <tt>set_position</tt> implementation*/
    ULIB_RESULT(*reset)(USTREAM* ustream_interface);                                    /**<internal <tt>reset</tt> implementation*/
    ULIB_RESULT(*read)(USTREAM* ustream_interface, uint8_t* const buffer, 
                                            size_t buffer_length, size_t* const size);  /**<internal <tt>read</tt> implementation*/
    ULIB_RESULT(*get_remaining_size)(USTREAM* ustream_interface, size_t* const size);   /**<internal <tt>get_remaining_size</tt> implementation*/
    ULIB_RESULT(*get_position)(USTREAM* ustream_interface, offset_t* const position);   /**<internal <tt>get_position</tt> implementation*/
    ULIB_RESULT(*release)(USTREAM* ustream_interface, offset_t position);               /**<internal <tt>release</tt> implementation*/
    USTREAM*(*clone)(USTREAM* ustream_interface, offset_t offset);                      /**<internal <tt>clone</tt> implementation*/
    ULIB_RESULT(*dispose)(USTREAM* ustream_interface);                                  /**<internal <tt>dispose</tt> implementation*/
};


/**
 * @brief   Check if a handle is the same type of the API.
 *
 *  It will return true if the handle is valid and it is the same type of the API. It will
 *      return false if the handle is <tt>NULL</tt> or note the correct type.
 */
#define USTREAM_IS_NOT_TYPE_OF(handle, type_api)   ((handle == NULL) || (handle->api != &type_api))

/**
 * @brief   Change the current position of the buffer.
 *
 *  The current position is the one that will be returned in the local buffer by the next
 *      ustream_read(). Consumers can call this API to go back or forward, but it cannot be exceed
 *      the end of the buffer or precede the fist valid position (last released position + 1).
 *
 *  The <tt>ustream_set_position</tt> API shall follow these minimum requirements:
 *      - The <tt>set_position</tt> shall change the current position of the buffer.
 *      - If the provided position is out of the range of the buffer, the <tt>set_position</tt> shall return
 *          #ULIB_NO_SUCH_ELEMENT_ERROR, and will not change the current position.
 *      - If the provided position is already released, the <tt>set_position</tt> shall return
 *          #ULIB_NO_SUCH_ELEMENT_ERROR, and will not change the current position.
 *      - If the provided interface is <tt>NULL</tt>, the <tt>set_position</tt> shall return #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented buffer type, the <tt>set_position</tt> shall return
 *          #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param[in]   ustream_interface       The {@link USTREAM}* with the interface of the buffer. It
 *                                      cannot be <tt>NULL</tt>, and it shall be a valid buffer that is the
 *                                      implemented buffer type.
 * @param[in]   position                The <tt>offset_t</tt> with the new current position in the buffer.
 * 
 * @return The {@link ULIB_RESULT} with the result of the <tt>set_position</tt> operation.
 *          @retval     ULIB_SUCCESS                If the buffer changed the current position with success.
 *          @retval     ULIB_BUSY_ERROR             If the resource necessary for the <tt>set_position</tt> operation is busy.
 *          @retval     ULIB_CANCELLED_ERROR        If the <tt>set_position</tt> operation was cancelled.
 *          @retval     ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
 *          @retval     ULIB_NO_SUCH_ELEMENT_ERROR  If the position is out of the buffer range.
 *          @retval     ULIB_OUT_OF_MEMORY_ERROR    If there is not enough memory to execute the
 *                                                  <tt>set_position</tt> operation.
 *          @retval     ULIB_SECURITY_ERROR         If the <tt>set_position</tt> operation was denied for security
 *                                                  reasons.
 *          @retval ULIB_SYSTEM_ERROR               If the <tt>set_position</tt> operation failed on the system level.
 */
static inline ULIB_RESULT ustream_set_position(USTREAM* ustream_interface, offset_t position)
{
    return ustream_interface->api->set_position(ustream_interface, position);
}

/**
 * @brief   Changes the current position to the first valid position.
 *
 *  The current position is the one that will be returned in the local buffer by the next
 *      ustream_read(). Reset will bring the current position to the first valid one, which
 *      is the first byte after the released position.
 *
 *  The <tt>ustream_reset</tt> API shall follow the following minimum requirements:
 *      - The reset shall change the current position of the buffer to the first byte after the
 *          released position.
 *      - If all bytes are already released, the buffer reset shall return
 *          #ULIB_NO_SUCH_ELEMENT_ERROR, and will not change the current position.
 *      - If the provided interface is <tt>NULL</tt>, the buffer reset shall return #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented buffer type, the buffer reset shall return
 *          #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param[in]   ustream_interface       The {@link USTREAM}* with the interface of the buffer. It
 *                                  cannot be <tt>NULL</tt>, and it shall be a valid buffer that is the
 *                                  implemented buffer type.
 * 
 * @return The {@link ULIB_RESULT} with the result of the reset operation.
 *          @retval     ULIB_SUCCESS                If the buffer changed the current position with success.
 *          @retval     ULIB_BUSY_ERROR             If the resource necessary for the reset operation is
 *                                                  busy.
 *          @retval     ULIB_CANCELLED_ERROR        If the reset operation was cancelled.
 *          @retval     ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
 *          @retval     ULIB_NO_SUCH_ELEMENT_ERROR  If all previous bytes in the buffer were already
 *                                                  released.
 *          @retval     ULIB_OUT_OF_MEMORY_ERROR    If there is not enough memory to execute the
 *                                                  reset operation.
 *          @retval     ULIB_SECURITY_ERROR         If the reset operation was denied for security
 *                                                  reasons.
 *          @retval     ULIB_SYSTEM_ERROR           If the reset operation failed on the system level.
 */
static inline ULIB_RESULT ustream_reset(USTREAM* ustream_interface)
{
    return ustream_interface->api->reset(ustream_interface);
}

/**
 * @brief   Gets the next portion of the buffer starting at the current position.
 * 
 * The <tt>ustream_read</tt> API will copy the contents of the Data source to the local buffer
 *      starting at the current position. The local buffer is the one referenced by the parameter
 *      <tt>buffer</tt>, and with the maximum size buffer_length.
 *
 *  The buffer is defined as a <tt>uint8_t*</tt> and can represent any sequence of data. Pay
 *      special attention, if the data is a string, the buffer will still copy it as a sequence of
 *      <tt>uint8_t</tt>, and will <b>NOT</b> put any terminator at the end of the string. The size of 
 *      the content copied in the local buffer will be returned in the parameter <tt>size</tt>.
 * 
 *  The <tt>ustream_read</tt> API shall follow the following minimum requirements:
 *      - The read shall copy the contents of the <tt>Data Source</tt> to the provided local buffer.
 *      - If the contents of the <tt>Data Source</tt> is bigger than the <tt>buffer_length</tt>, the read shall
 *          limit the copy size up to the buffer_length.
 *      - The read shall return the number of valid <tt>uint8_t</tt> values in the local buffer in 
 *          the provided <tt>size</tt>.
 *      - If there is no more content to return, the read shall return
 *          #ULIB_EOF, size shall receive 0, and will not change the contents
 *          of the local buffer.
 *      - If the provided buffer_length is zero, the read shall return #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided buffer_length is lower than the minimum number of bytes that the buffer can copy, the 
 *          read shall return #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is <tt>NULL</tt>, the read shall return #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented buffer type, the read shall return
 *          #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided local buffer is <tt>NULL</tt>, the read shall return #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided return size pointer is <tt>NULL</tt>, the read shall return
 *          #ULIB_ILLEGAL_ARGUMENT_ERROR and will not change the local buffer contents or the
 *          current position of the buffer.
 * 
 * @param[in]       ustream_interface       The {@link USTREAM}* with the interface of the buffer. It
 *                                          cannot be <tt>NULL</tt>, and it shall be a valid buffer that is the
 *                                          implemented buffer type.
 * @param[in,out]   buffer                  The <tt>uint8_t* const</tt> that points to the local buffer. It cannot be <tt>NULL</tt>.
 * @param[in]       buffer_length           The <tt>size_t</tt> with the size of the local buffer. It shall be
 *                                          bigger than 0.
 * @param[out]      size                    The <tt>size_t* const</tt> that points to the place where the read shall store
 *                                          the number of valid <tt>uint8_t</tt> values in the local buffer. It cannot be <tt>NULL</tt>.
 * 
 * @return The {@link ULIB_RESULT} with the result of the read operation.
 *          @retval     ULIB_SUCCESS                If the buffer copied the content of the <tt>Data Source</tt> to the local buffer
 *                                                  with success.
 *          @retval     ULIB_BUSY_ERROR             If the resource necessary to copy the buffer content is busy.
 *          @retval     ULIB_CANCELLED_ERROR        If the copy of the content was cancelled.
 *          @retval     ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
 *          @retval     ULIB_EOF                    If there are no more <tt>uint8_t</tt> values in the <tt>Data Source</tt> to copy.
 *          @retval     ULIB_OUT_OF_MEMORY_ERROR    If there is not enough memory to execute the copy.
 *          @retval     ULIB_SECURITY_ERROR         If the read was denied for security reasons.
 *          @retval     ULIB_SYSTEM_ERROR           If the read operation failed on the system level.
 */
static inline ULIB_RESULT ustream_read(USTREAM* ustream_interface, uint8_t* const buffer, size_t buffer_length, size_t* const size)
{
    return ustream_interface->api->read(ustream_interface, buffer, buffer_length, size);
}

/**
 * @brief   Returns the remaining size of the buffer.
 *
 *  This API returns the number of bytes between the current position and the end of the buffer.
 *
 *  The <tt>ustream_get_remaining_size</tt> API shall follow the following minimum requirements:
 *      - The get_remaining_size shall return the number of bytes between the current position and the
 *          end of the buffer.
 *      - If the provided interface is <tt>NULL</tt>, the get_remaining_size shall return #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented buffer type, the get_remaining_size shall
 *          return #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided size is <tt>NULL</tt>, the get_remaining_size shall return #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param[in]   ustream_interface       The {@link USTREAM}* with the interface of the buffer. It
 *                                      cannot be <tt>NULL</tt>, and it shall be a valid buffer that is the
 *                                      implemented buffer type.
 * @param[out]   size                   The <tt>size_t* const</tt> to returns the remaining number of <tt>uint8_t</tt> values 
 *                                      copied to the buffer. It cannot be <tt>NULL</tt>.
 * 
 * @return The {@link ULIB_RESULT} with the result of the get_remaining_size operation.
 *          @retval     ULIB_SUCCESS                If it succeeded to get the remaining size of the buffer.
 *          @retval     ULIB_BUSY_ERROR             If the resource necessary to the get the remain size of
 *                                                  the buffer is busy.
 *          @retval     ULIB_CANCELLED_ERROR        If the get remaining size was cancelled.
 *          @retval     ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
 *          @retval     ULIB_OUT_OF_MEMORY_ERROR    If there is not enough memory to execute the get
 *                                                  remaining size operation.
 *          @retval     ULIB_SECURITY_ERROR         If the get remaining size was denied for security reasons.
 *          @retval     ULIB_SYSTEM_ERROR           If the get remaining size operation failed on the
 *                                                  system level.
 */
static inline ULIB_RESULT ustream_get_remaining_size(USTREAM* ustream_interface, size_t* const size)
{
    return ustream_interface->api->get_remaining_size(ustream_interface, size);
}

/**
 * @brief   Returns the current position in the buffer.
 *
 *  This API returns the logical current position.
 *
 *  The <tt>ustream_get_position</tt> API shall follow the following minimum requirements:
 *      - The get_position shall return the logical current position of the buffer.
 *      - If the provided interface is <tt>NULL</tt>, the get_position shall return #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented buffer type, the get_position
 *          shall return #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided position is <tt>NULL</tt>, the get_position shall return #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param[in]   ustream_interface   The {@link USTREAM}* with the interface of the buffer. It
 *                                  cannot be <tt>NULL</tt>, and it shall be a valid buffer that is the
 *                                  implemented buffer type.
 * @param[out]   position           The <tt>offset_t* const</tt> to returns the logical current position in the
 *                                  buffer. It cannot be <tt>NULL</tt>.
 * 
 * @return The {@link ULIB_RESULT} with the result of the get_position operation.
 *          @retval     ULIB_SUCCESS                If it provided the logical current position of the buffer.
 *          @retval     ULIB_BUSY_ERROR             If the resource necessary for the getting the logical current
 *                                                  position is busy.
 *          @retval     ULIB_CANCELLED_ERROR        If the get logical current position was cancelled.
 *          @retval     ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
 *          @retval     ULIB_OUT_OF_MEMORY_ERROR    If there is not enough memory to execute the get
 *                                                  logical current position operation.
 *          @retval     ULIB_SECURITY_ERROR         If the get logical current position was denied for
 *                                                  security reasons.
 *          @retval     ULIB_SYSTEM_ERROR           If the get logical current position operation failed on
 *                                                  the system level.
 */
static inline ULIB_RESULT ustream_get_position(USTREAM* ustream_interface, offset_t* const position)
{
    return ustream_interface->api->get_position(ustream_interface, position);
}

/**
 * @brief   Releases all the resources related to the <tt><tt>Data Source</tt></tt> before and including the released position.
 *
 *  Calling this API will notify the buffer that the user will not need its content from the start
 *      to <tt>position</tt> (inclusive). It means that the implementation of the buffer can dispose
 *      any resources allocated to control and contain this part of the buffer.
 *  It is up to the implementation of the buffer to decide to release any resource. For example, if the
 *      buffer is a string in the Flash, it do not make sense to release it.
 *  The provided position shall be the logical position, and it shall be between the logical first
 *      valid position of the buffer and the logical current position minus one. For example, the following
 *      code releases all bytes from the start to the last received position:
 *
 * <pre><code>
 * offset_t pos;
 * if(ustream_get_position(my_buffer, &pos) == ULIB_SUCCESS)
 * {
 *     ustream_release(my_buffer, pos - 1);
 * }
 * </code></pre>
 *
 *  The <tt>ustream_release</tt> API shall follow the following requirements:
 *      - The release shall dispose all resources necessary to handle the content of buffer before and 
 *          including the release position.
 *      - If the release position is after the current position or the buffer size, the release shall
 *          return #ULIB_ILLEGAL_ARGUMENT_ERROR, and do not release any resource.
 *      - If the release position is already released, the release shall return
 *          #ULIB_NO_SUCH_ELEMENT_ERROR, and do not release any resource.
 *      - If the provided interface is <tt>NULL</tt>, the release shall return #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented buffer type, the release shall return
 *          #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param[in]  ustream_interface    The {@link USTREAM}* with the interface of the buffer. It
 *                                  cannot be <tt>NULL</tt>, and it shall be a valid buffer that is the
 *                                  implemented buffer type.
 * @param[in]  position             The <tt>offset_t</tt> with the position in the buffer to release. The
 *                                  buffer will release the <tt>uint8_t</tt> on the position and all <tt>uint8_t</tt>
 *                                  before the position. It shall be bigger than 0.
 * 
 * @return The {@link ULIB_RESULT} with the result of the release operation.
 *          @retval     ULIB_SUCCESS                If the buffer releases the position with success.
 *          @retval     ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
 *          @retval     ULIB_NO_SUCH_ELEMENT_ERROR  If the position is already released.
 *          @retval     ULIB_SYSTEM_ERROR           If the release operation failed on the system level.
 */
static inline ULIB_RESULT ustream_release(USTREAM* ustream_interface, offset_t position)
{
    return ustream_interface->api->release(ustream_interface, position);
}

/**
 * @brief   Creates a new instance of the buffer and return it.
 *
 *  Clone a buffer will create a new instance of the buffer that share the same content of the
 *      original one. The clone shall not copy the content of the buffer, but only add a reference to
 *      it.
 *  Both, the start position and the current position of the cloned buffer will be the current
 *      position of the original buffer. The logical position of it will be determined by the provided
 *      offset.
 *  The size of the new buffer will be the remaining size of the original buffer, which is the size
 *      menus the current position.
 *
 * <i><b>Example 1</b></i>
 * 
 *  Consider a buffer with 1500 bytes, that was created from the factory, with <tt>Logical</tt> and <tt>Inner</tt>
 *      positions as <tt>0</tt>. After some operations, 1000 bytes was read (from 0 to 999), so, the
 *      current position is <tt>1000</tt>, and 200 bytes was released (from 0 to 199), so the released
 *      position is <tt>199</tt>.
 *  For the following examples, the positions is represented by <tt>[Logical, Inner]</tt>.
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
 * <i><b>Example 2</b></i>
 * 
 *  Consider a buffer with 5000 bytes, that was created from the factory, with <tt>Logical</tt> and <tt>Inner</tt>
 *      positions as <tt>0</tt>. After some operations, 250 bytes was read (from 0 to 249), so, the
 *      current position is <tt>250</tt>, and no release was made, so the released position is still `-1`.
 *  For the following examples, the positions is represented by <tt>[Logical, Inner]</tt>.
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
 * <i><b>Example 3</b></i>
 * 
 *  From the previews cloned buffer, after some operations, the Logical current position is moved
 *      to <tt>11000</tt>, and the Logical released position is <tt>10499</tt>.

 *  For the following examples, the positions is represented by <tt>[Logical, Inner]</tt>.
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
 *  @note
 *  From the point of view of an consumer, the <tt>Inner</tt> position never matters, it will
 *      always use the <tt>Logical</tt> position for all operations.
 *  @note
 *  If the position is not important to the consumer, making the offset equal to <tt>0</tt> is a safe option.
 *
 *  The <tt>ustream_clone</tt> API shall follow the following minimum requirements:
 *      - The clone shall return a buffer with the same content of the original buffer.
 *      - If the provided interface is <tt>NULL</tt>, the clone shall return <tt>NULL</tt>.
 *      - If the provided interface is not type of the implemented buffer, the clone shall return <tt>NULL</tt>.
 *      - If there is not enough memory to control the new buffer, the clone shall return <tt>NULL</tt>.
 *      - If the provided offset plus the buffer size is bigger than the maximum size_t, the clone
 *          shall return <tt>NULL</tt>.
 *      - The cloned buffer shall not interfere in the instance of the original buffer and vice versa.
 *
 * @param[in]   ustream_interface       The {@link USTREAM}* with the interface of the buffer.
 *                                      It cannot be <tt>NULL</tt>, and it shall be a valid buffer that is
 *                                      type of the implemented buffer.
 * @param[out]   offset                 The <tt>offset_t</tt> with the <tt>Logical</tt> position of the first byte in
 *                                      the cloned buffer.
 * 
 * @return The {@link USTREAM}* with the result of the clone operation.
 *          @retval    not NULL         If the buffer was copied with success.
 *          @retval    NULL             If one of the provided parameters is invalid or there is not enough memory to
 *                                      control the new buffer.
 */
static inline USTREAM* ustream_clone(USTREAM* ustream_interface, offset_t offset)
{
    return ustream_interface->api->clone(ustream_interface, offset);
}

/**
 * @brief   Release all the resources allocated to control the instance of the buffer.
 *
 *  The dispose will release the instance of the buffer and decrement the reference of the buffer.
 *          If there is no more references to the buffer, the dispose will release all recourses
 *          allocated to control the buffer.
 *
 *  The <tt>ustream_dispose</tt> API shall follow the following requirements:
 *      - The dispose shall free all allocated resources for the instance of the buffer.
 *      - If there is no more instances of the buffer, the dispose shall release all allocated
 *          resources to control the buffer.
 *      - If the provided interface is <tt>NULL</tt>, the dispose shall return #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not type of the implemented buffer, the dispose shall return
 *          #ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param[in]   ustream_interface       The {@link USTREAM}* with the interface of the buffer. It
 *                                      cannot be <tt>NULL</tt>, and it shall be a valid buffer that is type
 *                                      of the implemented buffer.
 * 
 * @return The {@link ULIB_RESULT} with the result of the dispose operation.
 *          @retval ULIB_SUCCESS                    If the instance of the buffer was disposed with success.
 *          @retval ULIB_ILLEGAL_ARGUMENT_ERROR     If one of the provided parameters is invalid.
 */
static inline ULIB_RESULT ustream_dispose(USTREAM* ustream_interface)
{
    return ustream_interface->api->dispose(ustream_interface);
}

/**
  * @brief   Append a uStream to the existing buffer.
  *
  *  The append will add a buffer at the end of the current one. To do that, the append will convert 
  *         the current buffer in a {@link USTREAM_MULTI_INSTANCE}, and append a clone of the provided buffer
  *         on it. If the current buffer is already an instance of {@link USTREAM_MULTI_INSTANCE}, this API
  *         will only append the new buffer.
  *
  *  The <tt>ustream_append</tt> API shall follow the following requirements:
  *      - The append shall append the provided buffer at the end of the current one.
  *      - If current buffer is not a multi-buffer, the append shall convert the current buffer in a multi-buffer.
  *      - If the provided interface is <tt>NULL</tt>, the append shall return #ULIB_ILLEGAL_ARGUMENT_ERROR.
  *      - If the provided buffer to add is <tt>NULL</tt>, the append shall return #ULIB_ILLEGAL_ARGUMENT_ERROR.
  *      - If there is not enough memory to append the buffer, the append shall return 
  *         #ULIB_OUT_OF_MEMORY_ERROR.
  *
  * @param[in, out]     ustream_interface   The {@link USTREAM}* with the interface of 
  *                                         the buffer. It cannot be <tt>NULL</tt>, and it shall be a valid buffer.
  * @param[in]          ustream_to_append   The {@link USTREAM}* with the interface of 
  *                                         the buffer to be appended to the original buffer. It cannot be <tt>NULL</tt>, 
  *                                         and it shall be a valid buffer.
  * @return The {@link ULIB_RESULT} with the result of the append operation.
  *          @retval    ULIB_SUCCESS                If the buffer was appended with success.
  *          @retval    ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
  *          @retval    ULIB_OUT_OF_MEMORY_ERROR    If there is no memory to append the buffer.
  */
MOCKABLE_FUNCTION( , 
    ULIB_RESULT, ustream_append,
    USTREAM*, ustream_interface, 
    USTREAM*, ustream_to_append);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_USTREAM_BASE_H_ */
