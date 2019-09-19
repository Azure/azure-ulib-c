// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZIOT_USTREAM_BASE_H
#define AZIOT_USTREAM_BASE_H

/**
 * @file ustream_base.h
 *
 * @brief **uStream Interface Definition**
 *
 *  This is the definition of a heterogeneous buffer that helps other modules in a system
 *      expose large amounts of data without using a large amount of memory. Modules in the system can expose
 *      their own data using this interface. To do that, the module shall implement the functions in the
 *      interface. This implementation shall follow the definition described in this file, which includes
 *      not only the prototype of the header, but the behavior as well.
 *  uStream defines a provider-consumer interface when:
 *      - <b>Provider</b> - is the module of code that handles data to be exposed. This module implements
 *          the ustream interface to expose the data to the consumer.
 *      - <b>Consumer</b> - is the module of code that will use the data exposed by the provider.
 *
 *  The ustream shall have a clear separation between the internal content (provider domain)
 *      and what it exposes as external content (consumer domain). The ustream shall never expose 
 *      the internal content (ex: providing a pointer to a internal memory position). All 
 *      exposed content shall be copied from the internal data source to some given external memory. To do
 *      that in a clear way, the ustream shall always work with the concept of two buffers, the 
 *      <tt>data source</tt> and the <tt>local buffer</tt>, adhering to the following definition:
 *      - <b>Data source</b> - is the place where the data is stored by the implementation of the ustream
 *          interface. The data source is in the provider domain, and it shall be protected, immutable, 
 *          and non volatile. Consumers can read the data from the data source by the calling
 *          the aziot_ustream_read() API, which will copy a snapshot of the data to the
 *          provided external memory, called local buffer.
 *      - <b>Local buffer</b> - is the consumer domain buffer, where the aziot_ustream_read() API 
 *          will copy the required bytes from the data source. The local buffer belongs to the consumer 
 *          of this interface, which means that the consumer shall allocate and free (if necessary) this memory,
 *          and the content of the local buffer can be changed and released.
 * 
 * <i><b>Example</b></i>
 *
 *  A provider wants to create a ustream to expose data to the consumer. The provider will 
 *      store the content in the HEAP, and will create a ustream from it, passing the ownership of 
 *      the content to the ustream. Consumer will print the content of the ustream, using a 
 *      local buffer of 1K. The following diagram represents this operation.
 *
 * <pre><code>
 *  +----------------+        +----------------+              +------------------+     +------------+
 *  |    Provider    |        |    Consumer    |              |      ustream     |     |    HEAP    |
 *  +----------------+        +----------------+              +------------------+     +------------+
 *          |                         |                                  |                    |
 *          |<-get_provider_content()-+                                  |                    |
 *          +----------------------------malloc(content_size)-------------------------------->|
 *          |<--------------------------------content_ptr-------------------------------------+
 *          +--------------------malloc(sizeof(AZIOT_USTREAM_INNER_BUFFER))------------------>|
 *          <---------------------------------inner_buffer_ptr--------------------------------+
 *   +------+                         |                                  |                    |
 *   | generate the content and store in the content_ptr                 |                    |
 *   +----->|                         |                                  |                    |
 *          +-----aziot_ustream_init                                     |                    |
 *          |       (ustream_instance,                                   |                    |
 *          |        inner_buffer_ptr, free,                             |                    |
 *          |        content_ptr, content_size, free)------------------->|                    |
 *          |                         |                           +------+                    |
 *          |                         |                           | data_source = content_ptr |
 *          |                         |                           | data_source_size = content_size
 *          |                         |                           +----->|                    |
 *          |<-----------------ustream_interface-------------------------+                    |
 *          +---ustream_interface---->|                                  |                    |
 *
 * </code></pre>
 *
 *
 *  Now that the consumer has the ustream with the content, it will print it using the 
 *   iterator aziot_ustream_read().
 *
 * <pre><code>
 *
 *          |                         +------------------malloc(1024)------------------------>|
 *          |                         |<-----------------local_buffer-------------------------+
 *  .. while aziot_ustream_read return AZIOT_ULIB_SUCCESS ...............................................
 *  :       |                         +--aziot_ustream_read              |                    |         :
 *  :       |                         |  (ustream_interface,             |                    |         :
 *  :       |                         |   local_buffer,                  |                    |         :
 *  :       |                         |   1024,                          |                    |         :
 *  :       |                         |   &size)------------------------>|                    |         :
 *  :       |                         |                 +----------------+                    |         :
 *  :       |                         |                 | copy the next 1024 bytes from the   |         :
 *  :       |                         |                 |  data_source to the local_buffer.   |         :
 *  :       |                         |                 +--------------->|                    |         :
 *  :       |                         |<---AZIOT_ULIB_SUCCESS------------+                    |         :
 *  :       |                     +---+                                  |                    |         :
 *  :       |                     | use the content in the local_buffer  |                    |         :
 *  :       |                     +-->|                                  |                    |         :
 *  .....................................................................................................
 *          |                         +---------------free(local_buffer)--------------------->|
 *          |                         +-aziot_ustream_dispose            |                    |
 *          |                         |       (ustream_interface)------->|                    |
 *          |                         |                             +----+                    |
 *          |                         |                             | free(inner_buffer_ptr)->|
 *          |                         |                             | free(data_source)------>|
 *          |                         |                             +----+
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
 *  The ustream solves this problem by creating a single interface that can handle any media,
 *      exposing it as a standard iterator. Whoever wants to expose a type of media as a ustream shall
 *      implement the functions described on the interface, handling all implementation details for
 *      each API. For example, the aziot_ustream_read() can be a simple copy of the flash to
 *      the RAM for a buffer that handles constants, or be as complex as creating a TCP/IP connection
 *      to bring the data for a buffer that handles data in the cloud.
 *
 *  The consumer of the ustream can use all kinds of media in the same way, and may easily
 *      concatenate it by exposing a ustream that handles multiple ustream's.
 * 
 * <h2>Static Memory</h2>
 *  The ustream does not use any calls to <tt>malloc</tt>. Memory that it uses is required to be passed by the developer
 *      with an associated release function unless otherwise specified.
 *
 * <h2>Ownership</h2>
 *  The ustream is an owner-less buffer: every instance of the ustream has the same rights. They
 *      all can read the ustream content, release the parts that are not necessary anymore, and dispose
 *      it.
 *  Each instance of the ustream is owned by who created it, and should never be shared by 
 *      multiple consumers. When a consumer receives a ustream and intends to make operations over 
 *      it, this consumer must first make a clone of the ustream, creating its own instance of it, and 
 *      then make the needed operations.
 * <p> Cloning a ustream creates a new set of controls for the ustream that will share the same content of
 *      the original ustream. The content itself is a smart pointer with a <tt>ref_count</tt> that
 *      controls the total number of instances.
 * <p> Disposing an instance of the ustream will decrease the <tt>ref_count</tt> of this ustream. If the
 *      number of references reaches 0, the ustream will destroy itself by calling the provided release functions.
 *
 *  @warning Not disposing an instance of the ustream will leak memory.
 *
 *  Instances of the ustream can be created in 2 ways:
 *      - @b Factory - when a producer exposes data using a ustream, it must create the ustream
 *          using a factory, so the operation <tt>ustream create</tt> returns the first instance of the
 *          ustream.
 *      - @b Clone - when a consumer needs a copy of the ustream, it can use the aziot_ustream_clone().
 *
 * <h2>Thread safe</h2>
 *  The ustream <b>IS NOT</b> thread safe for multiple accesses over the same instance. The ownership
 *      of the instance of a ustream shall <b>NOT</b> be shared, especially not by consumers that run on 
 *      different threads. The owner thread shall create a clone of the ustream and pass it to the other 
 *      thread.
 *  The ustream <b>IS</b> thread safe for accesses between instances. It means that any access to
 *      memory shared by multiple instances shall be thread safe.
 *
 * <h2>Data retention</h2>
 *  As with any buffer, this ustream shall be used to handle data that was created by the producer as a
 *      result of an operation.
 *
 *  This interface only exposes read functions, so once created, the content of the ustream cannot
 *      be changed by the producer of any of the consumers. Changing the content of the data source will
 *      result in a data mismatch.
 *
 *  Consumers can do a partial release of the ustream by calling aziot_ustream_release().
 *      Calling the release does not imply that part of the memory will be immediately released. Once a
 *      ustream can handle multiple instances, a memory can only be free'd if all instances release it.
 *      A ustream implementation can or cannot have the ability to do partial releases. For instance, a
 *      ustream that handles constant data stored in the flash will never release any memory on the
 *      aziot_ustream_release() API.
 *
 *  Released data cannot be accessed, even if it is still available in the memory.
 *
 * <h2>Concatenate</h2>
 *  New data can be concatenated at the end of the ustream by calling aziot_ustream_concat().
 *      This can include ustream's from other different medias. In this way, the ustream can
 *      be used as a Stream of data.
 *  To protect the immutability of the ustream, concatenating a new ustream to an existing one will
 *      only affect the instance that is calling the aziot_ustream_concat().
 *
 * <i><b>Example</b></i>
 *  A producer created 3 ustreams named A, B, and C. At this point, it handles one instance of each
 *      ustream. A consumer received an instance of the ustream A and C, and concats C to A creating a new ustream AC.
 *      After that, the producer will concat B to A, creating the new AB ustream.
 *
 *  Observe the fact that the consumer concatenating C to A on its own instance didn't affect the ustream
 *      A on the producer, and when the producer concatenated B to A, it creates AB, not ACB, and it didn't change
 *      the consumer AB ustream creating ABC or ACB on it.
 *
 * <h2>Lazy</h2>
 *  The ustream can contain the full content, bring it into memory when required, or even create the content
 *      when it is necessary. The implementation of the aziot_ustream_read() function can be smart 
 *      enough to use the minimal amount of memory.
 *
 *  The only restriction is if a consumer accesses the same position of the ustream multiple times, it shall
 *      return the same data.
 *
 * <i><b>Example</b></i>
 *  A random number generator can expose random numbers using the ustream. To do that it shall generate a 
 *      new number when the consumer calls aziot_ustream_read(). But to preserve the immutability,
 *      the implementation of the aziot_ustream_read() shall store the number in a recover queue, up
 *      to the point that the consumer releases this data. Because, if at some point in time, the consumer 
 *      seeks this old position, the aziot_ustream_read() shall return the same value created in
 *      the first call of aziot_ustream_read().
 *
 * <h2>Data conversion</h2>
 *  When the data is copied from the data source to the local buffer, the aziot_ustream_read()
 *      may do a data conversion, which means that the content exposed on the local buffer is a function
 *      of the content in the data source. It directly implies that the number of bytes written in the
 *      local buffer may be different than the number of bytes read from the data source.
 *
 * <i><b>Example</b></i>
 *  A ustream can have the data source in binary format with 36 bytes, but it shall expose the 
 *      content encoded in base64. The base64 creates 4 encoded bytes for each 3 bytes read. So, seeking the 
 *      beginning of the file, the aziot_ustream_get_remaining_size() shall return 48 (= 36 / 3 * 4),
 *      instead of 36. If the consumer provides a local buffer of 16 bytes, the aziot_ustream_read() 
 *      shall read only 12 bytes from the data source, and encode it in base64 expanding the 12 bytes to 
 *      16 bytes on the local buffer.
 * <pre><code>
 *                  ustream domain                      ::      consumer domain
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
 *      byte is always placed at the inner position <tt>0</tt>, followed by the other bytes which are incremented
 *      in a sequential manner. The ustream assigns a sequential number to each byte
 *      in the local buffer as well, called <tt>logical position</tt>. When a new ustream is created,
 *      the logical position matches the inner position, both starting at position <tt>0</tt>.
 *
 *  When the ustream is cloned, an offset shall be provided. This offset is the new first logical 
 *      position. The implementation of the ustream shall handle the difference between the inner
 *      and logical position, making the conversion in all the ustream API. Providing an offset to a 
 *      ustream can be useful in many cases. For example, to concatenate buffers, the second ustream can 
 *      have an offset of the end of the first ustream plus one, or in a TCP connection, make the 
 *      logical position the same value of the octet sequence number.
 *
 * <i><b>Example</b></i>
 *  A ustream was created from the flash with 100 bytes. The inner position is a sequence from
 *      <tt>0</tt> to <tt>99</tt>, and it matches the logical position. The consumer clones this ustream providing an
 *      offset of <tt>1000</tt>. The new instance contains the same content as the original one, but the 
 *      logical positions are now from <tt>1000</tt> to <tt>1099</tt>.
 *
 *  If the owner of the first instance wants to set the position to position 10, it shall call aziot_ustream_set_position()
 *      with the logical position 10. For the cloned instance, to set the position to the same position 10, it shall call 
 *      aziot_ustream_set_position() with the logical position 1010.
 *
 * <h2>Sliding window</h2>
 *  One of the target use cases of the ustream is to accelerate and simplify the implementation of 
 *      sliding window protocols, like TCP. As described in this document, the ustream associates 
 *      a single byte (octet) to a single position, which means that every byte can be accessed by its
 *      position. For the consumer, this position is the logical position.
 *
 *  To better understand the sliding window concept of the ustream, the Data source can be split
 *      in 4 segments.
 *
 * <pre><code>
 *      Data Source:
 *           Released                       Pending                          Future
 *       |----------------|---------------------:--------------------|---------------------|
 *       |\                \                    :       Read         |\                    |
 *       | 0           First Valid Position     :                    | Current Position    |
 *       |                                      :                    |                     |
 *       |                                      :<--- Read Size ---->|                     |
 *       |                                                                                 |
 *       |<------------------------------ Data Source Size ------------------------------->|
 * </code></pre>
 *      - @b Released - Sequence of bytes in the data source that is already acknowledged by the consumer, 
 *          and shall not be accessed anymore.
 *      - @b Pending - Sequence of bytes in the data source that is already read by the consumer, but not 
 *          acknowledged yet. The consumer can seek these bytes with aziot_ustream_set_position() and read it again. 
 *          This sequence starts at the <tt>First Valid Position</tt> and ends at the last byte before the <tt>Current Position</tt>.
 *      - @b Read - The last read portion of the data source. On the read operation, the <tt>Read</tt> starts
 *          at the <tt>Current Position</tt> up to the <tt>Read Size</tt>. At the end of the read, this segment is 
 *          incorporated to <tt>Pending</tt> by changing the <tt>Current Position</tt> to the end of the Read.
 *      - @b Future - Sequence of bytes in the data source that is not read by the consumer yet. It starts 
 *          at the <tt>Current Position</tt> and ends at the end of the data source, which has the position calculated
 *          by <tt>Data Source Size - 1</tt>. 
 *
 *  To read a new portion of the data source, the consumer shall provide memory (the local buffer), where 
 *      the implementation of the ustream will write the bytes that were read and converted from the data source. 
 *      The consumer can use this data in its own context: for example, to transmit as a TCP packet. When
 *      the consumer finishes using the data in the local buffer, this data can be discarded
 *      and the local buffer recycled to get the next portion of the data source.
 *
 *  If at some point in the future, the consumer needs this data again, it can set the position to the needed position 
 *      and get the same content using the read.
 *
 *  The consumer may confirm that a portion of the data is not necessary anymore. For example, after transmitting
 *      multiple TCP packets, the receiver of these packets answers with an ACK for a sequence number. In this case,
 *      the consumer can release this data in the data source by calling the aziot_ustream_release(), moving 
 *      the <tt>First Valid Position</tt> to the next one after the released position.
 *
 *  A common scenario is when the consumer needs to read over the data source starting on the first byte after
 *      the last released one. For example, when a timeout happens for a transmitted packet without ACK, the 
 *      sender shall retransmit the data starting from that point. In this case, the consumer can call the API
 *      aziot_ustream_reset().
 * 
 * 
 *
 */

#include "ulib_config.h"
#include "ulib_result.h"
#include "az_pal_os.h"
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
 * @struct AZIOT_USTREAM_INTERFACE_TAG
 */
typedef struct AZIOT_USTREAM_INTERFACE_TAG AZIOT_USTREAM_INTERFACE;

/**
 * @brief   Interface description.
 */
typedef struct AZIOT_USTREAM_TAG AZIOT_USTREAM;

/**
 * @brief   vTable with the ustream APIs.
 *
 *  Any module that exposes the ustream shall implement the functions on this vTable.
 *
 *  Any code that will use an exposed ustream shall call the APIs using the <tt>aziot_ustream_...</tt>
 *      inline functions.
 */
struct AZIOT_USTREAM_INTERFACE_TAG
{
    AZIOT_ULIB_RESULT(*set_position)(AZIOT_USTREAM* ustream_interface, offset_t position);          /**<concrete <tt>set_position</tt> implementation*/
    AZIOT_ULIB_RESULT(*reset)(AZIOT_USTREAM* ustream_interface);                                    /**<concrete <tt>reset</tt> implementation*/
    AZIOT_ULIB_RESULT(*read)(AZIOT_USTREAM* ustream_interface, uint8_t* const buffer, 
                                            size_t buffer_length, size_t* const size);              /**<concrete <tt>read</tt> implementation*/
    AZIOT_ULIB_RESULT(*get_remaining_size)(AZIOT_USTREAM* ustream_interface, size_t* const size);   /**<concrete <tt>get_remaining_size</tt> implementation*/
    AZIOT_ULIB_RESULT(*get_position)(AZIOT_USTREAM* ustream_interface, offset_t* const position);   /**<concrete <tt>get_position</tt> implementation*/
    AZIOT_ULIB_RESULT(*release)(AZIOT_USTREAM* ustream_interface, offset_t position);               /**<concrete <tt>release</tt> implementation*/
    AZIOT_ULIB_RESULT(*clone)(AZIOT_USTREAM* ustream_interface_clone, 
                                            AZIOT_USTREAM* ustream_interface, offset_t offset);     /**<concrete <tt>clone</tt> implementation*/
    AZIOT_ULIB_RESULT(*dispose)(AZIOT_USTREAM* ustream_interface);                                  /**<concrete <tt>dispose</tt> implementation*/
};

/**
 * @brief   Signature of the function to release memory passed to the ustream
 * 
 * @param[in]   void*       void pointer to memory that needs to be free'd
 * 
 * @return  void
 */
typedef void (*AZIOT_RELEASE_CALLBACK)(void*);

/**
 * @brief   Pointer to the data from which to read
 * 
 * @param[in]   void*       void pointer to memory where the data is located or any needed controls to access the data.
 *                          The content of the memory to which this points is up to the ustream implementation.
 * 
 * @return  void
 */
typedef void* AZIOT_USTREAM_DATA;

/**
 * @brief   Structure for inner buffer control block
 * 
 * For any given ustream that is created, one inner control block is created and initialized. 
 * 
 * @note This structure should be viewed and used as internal to the implementation of the ustream. Users should therefore not act on
 *       it directly and only allocate the memory necessary for it to be passed to the ustream.
 * 
 */
typedef struct AZIOT_USTREAM_INNER_BUFFER_TAG
{
    const AZIOT_USTREAM_INTERFACE* api;             /**<The @link AZIOT_USTREAM_INTERFACE_TAG AZIOT_USTREAM_INTERFACE*
                                                            @endlink for this ustream instance type */
    AZIOT_USTREAM_DATA* ptr;                        /**<The #AZIOT_USTREAM_DATA* pointing to the data to read. It can 
                                                            be anything that a given ustream implementation needs to
                                                            access the data, whether it be a memory address to a buffer,
                                                            another struct with more controls, etc */
    volatile uint32_t ref_count;                    /**<The <tt>volatile uint32_t</tt> with the number of references
                                                            taken for this memory */
    AZIOT_RELEASE_CALLBACK data_release;            /**<The #AZIOT_RELEASE_CALLBACK to call to release <tt>ptr</tt> 
                                                            once the <tt>ref_count</tt> goes to zero */
    AZIOT_RELEASE_CALLBACK inner_buffer_release;    /**<The #AZIOT_RELEASE_CALLBACK to call to release the #AZIOT_USTREAM_INNER_BUFFER
                                                            once the <tt>ref_count</tt> goes to zero */
} AZIOT_USTREAM_INNER_BUFFER;

/**
 * @brief   Structure for instance control block
 * 
 * For any given ustream that is created, there may be mutliple <tt>AZIOT_USTREAM</tt>'s
 *      pointing to the same <tt>AZIOT_USTREAM_INNER_BUFFER</tt>. Each instance control block serves to
 *      manage a given developer's usage of the memory pointed to inside the <tt>AZIOT_USTREAM_INNER_BUFFER</tt>.
 *      Each time an <tt>AZIOT_USTREAM</tt> is cloned using aziot_ustream_clone(), the
 *      <tt>ref_count</tt> inside the <tt>AZIOT_USTREAM_INNER_BUFFER</tt> is incremented to signal a reference
 *      to the memory has been acquired. Once the instance is done being used, aziot_ustream_release() 
 *      must be called to decrememnt <tt>ref_count</tt>.
 * 
 * @note This structure should be viewed and used as internal to the implementation of the ustream. Users should therefore not act on 
 *       it directly and only allocate the memory necessary for it to be passed to the ustream.
 *  
 */
struct AZIOT_USTREAM_TAG
{
    /* Inner buffer. */
    AZIOT_USTREAM_INNER_BUFFER* inner_buffer;   /**<The #AZIOT_USTREAM_INNER_BUFFER* on which this instance operates on. */

    /* Instance controls. */
    offset_t offset_diff;                       /**<The #offset_t used as the logical position for this instance. */
    offset_t inner_current_position;            /**<The #offset_t used to keep track of the current position (next returned position). */
    offset_t inner_first_valid_position;        /**<The #offset_t used to keep track of the earliest position to reset. */
    size_t length;                              /**<The <tt>size_t</tt> with the length of the inner_buffer */
};

/**
 * @brief   Structure to keep track of concatenated ustreams.
 * 
 * When concatenating a ustream to another ustream, the instances are placed into a <tt>AZIOT_USTREAM_MULTI_DATA</tt>. The base ustream onto which you wish
 *      to concatenate will be copied into the <tt>ustream_one</tt> structure and the ustream to concatenate will be cloned into the <tt>ustream_two</tt>
 *      structure. The difference being that the first #AZIOT_USTREAM*, when returned, will point to the newly populated multi instance and the ownership
 *      of the passed instance will be assumed by the multi instance. The second ustream which was passed will not be changed, only cloned into the
 *      <tt>AZIOT_USTREAM_MULTI_DATA</tt> structure.
 * 
 * @note This structure should be viewed and used as internal to the implementation of the ustream. Users should therefore not act on 
 *       it directly and only allocate the memory necessary for it to be passed to the ustream.
 * 
 */
typedef struct AZIOT_USTREAM_MULTI_DATA_TAG
{
    AZIOT_USTREAM ustream_one;                  /**<The #AZIOT_USTREAM with the first ustream instance*/
    AZIOT_USTREAM ustream_two;                  /**<The #AZIOT_USTREAM with the second ustream instance*/
    volatile uint32_t ustream_one_ref_count;    /**<The <tt>uint32_t</tt> with the number of references to the first ustream */
    volatile uint32_t ustream_two_ref_count;    /**<The <tt>uint32_t</tt> with the number of references to the second ustream */
    AZ_PAL_OS_LOCK lock;                        /**<The #AZ_PAL_OS_LOCK with controls the critical section of the read from the multi ustream */
} AZIOT_USTREAM_MULTI_DATA;


typedef struct AZIOT_USTREAM_MESSAGE_TAG
{
    const char* host_name;
    const char* message_verb;
    uint64_t time;

    char content_type[20];
    char ms_version[11];
} AZIOT_USTREAM_MESSAGE;


/**
 * @brief   Check if a handle is the same type of the API.
 *
 *  It will return true if the handle is valid and it is the same type of the API. It will
 *      return false if the handle is <tt>NULL</tt> or not the correct type.
 */
#define AZIOT_USTREAM_IS_NOT_TYPE_OF(handle, type_api)   ((handle == NULL) || (handle->inner_buffer == NULL) || (handle->inner_buffer->api == NULL) || (handle->inner_buffer->api != &type_api))

/**
 * @brief   Change the current position of the ustream.
 *
 *  The current position is the one that will be returned in the local buffer by the next
 *      aziot_ustream_read(). Consumers can call this API to go back or forward, but it cannot exceed
 *      the end of the ustream or precede the fist valid position (last released position + 1).
 *
 *  The <tt>aziot_ustream_set_position</tt> API shall follow these minimum requirements:
 *      - The <tt>set_position</tt> shall change the current position of the ustream.
 *      - If the provided position is out of the range of the ustream, the <tt>set_position</tt> shall return
 *          #AZIOT_ULIB_NO_SUCH_ELEMENT_ERROR, and will not change the current position.
 *      - If the provided position is already released, the <tt>set_position</tt> shall return
 *          #AZIOT_ULIB_NO_SUCH_ELEMENT_ERROR, and will not change the current position.
 *      - If the provided interface is <tt>NULL</tt>, the <tt>set_position</tt> shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented ustream type, the <tt>set_position</tt> shall return
 *          #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param[in]   ustream_interface       The {@link AZIOT_USTREAM}* with the interface of the ustream. It
 *                                      cannot be <tt>NULL</tt>, and it shall be a valid ustream that is the
 *                                      implemented ustream type.
 * @param[in]   position                The <tt>offset_t</tt> with the new current position in the ustream.
 *
 * @return The {@link AZIOT_ULIB_RESULT} with the result of the <tt>set_position</tt> operation.
 *          @retval     AZIOT_ULIB_SUCCESS                If the ustream changed the current position with success.
 *          @retval     AZIOT_ULIB_BUSY_ERROR             If the resource necessary for the <tt>set_position</tt> operation is busy.
 *          @retval     AZIOT_ULIB_CANCELLED_ERROR        If the <tt>set_position</tt> operation was cancelled.
 *          @retval     AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
 *          @retval     AZIOT_ULIB_NO_SUCH_ELEMENT_ERROR  If the position is out of the ustream range.
 *          @retval     AZIOT_ULIB_OUT_OF_MEMORY_ERROR    If there is not enough memory to execute the
 *                                                        <tt>set_position</tt> operation.
 *          @retval     AZIOT_ULIB_SECURITY_ERROR         If the <tt>set_position</tt> operation was denied for security
 *                                                        reasons.
 *          @retval AZIOT_ULIB_SYSTEM_ERROR               If the <tt>set_position</tt> operation failed on the system level.
 */
static inline AZIOT_ULIB_RESULT aziot_ustream_set_position(AZIOT_USTREAM* ustream_interface, offset_t position)
{
    return ustream_interface->inner_buffer->api->set_position(ustream_interface, position);
}

/**
 * @brief   Changes the current position to the first valid position.
 *
 *  The current position is the one that will be returned in the local buffer by the next
 *      aziot_ustream_read(). Reset will bring the current position to the first valid one, which
 *      is the first byte after the released position.
 *
 *  The <tt>aziot_ustream_reset</tt> API shall follow the following minimum requirements:
 *      - The <tt>reset</tt> shall change the current position of the ustream to the first byte after the
 *          released position.
 *      - If all bytes are already released, the ustream <tt>reset</tt> shall return
 *          #AZIOT_ULIB_NO_SUCH_ELEMENT_ERROR, and will not change the current position.
 *      - If the provided interface is <tt>NULL</tt>, the ustream <tt>reset</tt> shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented ustream type, the ustream <tt>reset</tt> shall return
 *          #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param[in]   ustream_interface       The {@link AZIOT_USTREAM}* with the interface of the ustream. It
 *                                      cannot be <tt>NULL</tt>, and it shall be a valid ustream that is the
 *                                      implemented ustream type.
 *
 * @return The {@link AZIOT_ULIB_RESULT} with the result of the <tt>reset</tt> operation.
 *          @retval     AZIOT_ULIB_SUCCESS                If the ustream changed the current position with success.
 *          @retval     AZIOT_ULIB_BUSY_ERROR             If the resource necessary for the <tt>reset</tt> operation is
 *                                                        busy.
 *          @retval     AZIOT_ULIB_CANCELLED_ERROR        If the <tt>reset</tt> operation was cancelled.
 *          @retval     AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
 *          @retval     AZIOT_ULIB_NO_SUCH_ELEMENT_ERROR  If all previous bytes in the ustream were already
 *                                                        released.
 *          @retval     AZIOT_ULIB_OUT_OF_MEMORY_ERROR    If there is not enough memory to execute the
 *                                                        <tt>reset</tt> operation.
 *          @retval     AZIOT_ULIB_SECURITY_ERROR         If the <tt>reset</tt> operation was denied for security
 *                                                        reasons.
 *          @retval     AZIOT_ULIB_SYSTEM_ERROR           If the <tt>reset</tt> operation failed on the system level.
 */
static inline AZIOT_ULIB_RESULT aziot_ustream_reset(AZIOT_USTREAM* ustream_interface)
{
    return ustream_interface->inner_buffer->api->reset(ustream_interface);
}

/**
 * @brief   Gets the next portion of the ustream starting at the current position.
 *
 * The <tt>aziot_ustream_read</tt> API will copy the contents of the Data source to the local buffer
 *      starting at the current position. The local buffer is the one referenced by the parameter
 *      <tt>buffer</tt>, and with the maximum size <tt>buffer_length</tt>.
 *
 *  The buffer is defined as a <tt>uint8_t*</tt> and can represent any sequence of data. Pay
 *      special attention, if the data is a string, the buffer will still copy it as a sequence of
 *      <tt>uint8_t</tt>, and will <b>NOT</b> put any terminator at the end of the string. The size of 
 *      the content copied in the local buffer will be returned in the parameter <tt>size</tt>.
 *
 *  The <tt>aziot_ustream_read</tt> API shall follow the following minimum requirements:
 *      - The read shall copy the contents of the <tt>Data Source</tt> to the provided local buffer.
 *      - If the contents of the <tt>Data Source</tt> is bigger than the <tt>buffer_length</tt>, the read shall
 *          limit the copy size up to the buffer_length.
 *      - The read shall return the number of valid <tt>uint8_t</tt> values in the local buffer in 
 *          the provided <tt>size</tt>.
 *      - If there is no more content to return, the read shall return
 *          #AZIOT_ULIB_EOF, size shall be set to 0, and will not change the contents
 *          of the local buffer.
 *      - If the provided buffer_length is zero, the read shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided buffer_length is lower than the minimum number of bytes that the ustream can copy, the 
 *          read shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is <tt>NULL</tt>, the read shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented ustream type, the read shall return
 *          #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided local buffer is <tt>NULL</tt>, the read shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided return size pointer is <tt>NULL</tt>, the read shall return
 *          #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR and will not change the local buffer contents or the
 *          current position of the buffer.
 *
 * @param[in]       ustream_interface       The {@link AZIOT_USTREAM}* with the interface of the ustream. It
 *                                          cannot be <tt>NULL</tt>, and it shall be a valid ustream that is the
 *                                          implemented ustream type.
 * @param[in,out]   buffer                  The <tt>uint8_t* const</tt> that points to the local buffer. It cannot be <tt>NULL</tt>.
 * @param[in]       buffer_length           The <tt>size_t</tt> with the size of the local buffer. It shall be
 *                                          bigger than 0.
 * @param[out]      size                    The <tt>size_t* const</tt> that points to the place where the read shall store
 *                                          the number of valid <tt>uint8_t</tt> values returned in the local buffer. It cannot be <tt>NULL</tt>.
 *
 * @return The {@link AZIOT_ULIB_RESULT} with the result of the read operation.
 *          @retval     AZIOT_ULIB_SUCCESS                If the ustream copied the content of the <tt>Data Source</tt> to the local buffer
 *                                                        with success.
 *          @retval     AZIOT_ULIB_BUSY_ERROR             If the resource necessary to read the ustream content is busy.
 *          @retval     AZIOT_ULIB_CANCELLED_ERROR        If the read of the content was cancelled.
 *          @retval     AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
 *          @retval     AZIOT_ULIB_EOF                    If there are no more <tt>uint8_t</tt> values in the <tt>Data Source</tt> to read.
 *          @retval     AZIOT_ULIB_OUT_OF_MEMORY_ERROR    If there is not enough memory to execute the read.
 *          @retval     AZIOT_ULIB_SECURITY_ERROR         If the read was denied for security reasons.
 *          @retval     AZIOT_ULIB_SYSTEM_ERROR           If the read operation failed on the system level.
 */
static inline AZIOT_ULIB_RESULT aziot_ustream_read(AZIOT_USTREAM* ustream_interface, uint8_t* const buffer, size_t buffer_length, size_t* const size)
{
    return ustream_interface->inner_buffer->api->read(ustream_interface, buffer, buffer_length, size);
}

/**
 * @brief   Returns the remaining size of the ustream.
 *
 *  This API returns the number of bytes between the current position and the end of the ustream.
 *
 *  The <tt>aziot_ustream_get_remaining_size</tt> API shall follow the following minimum requirements:
 *      - The <tt>get_remaining_size</tt> shall return the number of bytes between the current position and the
 *          end of the ustream.
 *      - If the provided interface is <tt>NULL</tt>, the <tt>get_remaining_size</tt> shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented ustream type, the <tt>get_remaining_size</tt> shall
 *          return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided size is <tt>NULL</tt>, the <tt>get_remaining_size</tt> shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param[in]   ustream_interface       The {@link AZIOT_USTREAM}* with the interface of the ustream. It
 *                                      cannot be <tt>NULL</tt>, and it shall be a valid ustream that is the
 *                                      implemented ustream type.
 * @param[out]  size                    The <tt>size_t* const</tt> to return the remaining number of <tt>uint8_t</tt> values 
 *                                      It cannot be <tt>NULL</tt>.
 *
 * @return The {@link AZIOT_ULIB_RESULT} with the result of the <tt>get_remaining_size</tt> operation.
 *          @retval     AZIOT_ULIB_SUCCESS                If it succeeded to get the remaining size of the ustream.
 *          @retval     AZIOT_ULIB_BUSY_ERROR             If the resource necessary to get the remaining size of
 *                                                        the ustream is busy.
 *          @retval     AZIOT_ULIB_CANCELLED_ERROR        If the <tt>get_remaining_size</tt> was cancelled.
 *          @retval     AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
 *          @retval     AZIOT_ULIB_OUT_OF_MEMORY_ERROR    If there is not enough memory to execute the
 *                                                        <tt>get_remaining_size</tt> operation.
 *          @retval     AZIOT_ULIB_SECURITY_ERROR         If the <tt>get_remaining_size</tt> was denied for security reasons.
 *          @retval     AZIOT_ULIB_SYSTEM_ERROR           If the <tt>get_remaining_size</tt> operation failed on the
 *                                                        system level.
 */
static inline AZIOT_ULIB_RESULT aziot_ustream_get_remaining_size(AZIOT_USTREAM* ustream_interface, size_t* const size)
{
    return ustream_interface->inner_buffer->api->get_remaining_size(ustream_interface, size);
}

/**
 * @brief   Returns the current position in the ustream.
 *
 *  This API returns the logical current position.
 *
 *  The <tt>aziot_ustream_get_position</tt> API shall follow the following minimum requirements:
 *      - The <tt>get_position</tt> shall return the logical current position of the ustream.
 *      - If the provided interface is <tt>NULL</tt>, the <tt>get_position</tt> shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented ustream type, the <tt>get_position</tt>
 *          shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided position is <tt>NULL</tt>, the <tt>get_position</tt> shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param[in]   ustream_interface   The {@link AZIOT_USTREAM}* with the interface of the ustream. It
 *                                  cannot be <tt>NULL</tt>, and it shall be a valid ustream that is the
 *                                  implemented ustream type.
 * @param[out]  position            The <tt>offset_t* const</tt> to return the logical current position in the
 *                                  ustream. It cannot be <tt>NULL</tt>.
 *
 * @return The {@link AZIOT_ULIB_RESULT} with the result of the <tt>get_position</tt> operation.
 *          @retval     AZIOT_ULIB_SUCCESS                If it provided the position of the ustream.
 *          @retval     AZIOT_ULIB_BUSY_ERROR             If the resource necessary for getting the
 *                                                        position is busy.
 *          @retval     AZIOT_ULIB_CANCELLED_ERROR        If the <tt>get_position</tt> was cancelled.
 *          @retval     AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
 *          @retval     AZIOT_ULIB_OUT_OF_MEMORY_ERROR    If there is not enough memory to execute the
 *                                                        <tt>get_position</tt> operation.
 *          @retval     AZIOT_ULIB_SECURITY_ERROR         If the <tt>get_position</tt> was denied for
 *                                                        security reasons.
 *          @retval     AZIOT_ULIB_SYSTEM_ERROR           If the <tt>get_position</tt> operation failed on
 *                                                        the system level.
 */
static inline AZIOT_ULIB_RESULT aziot_ustream_get_position(AZIOT_USTREAM* ustream_interface, offset_t* const position)
{
    return ustream_interface->inner_buffer->api->get_position(ustream_interface, position);
}

/**
 * @brief   Releases all the resources related to the <tt><tt>Data Source</tt></tt> before and including the released position.
 *
 *  Calling this API will notify the ustream that the developer will not need its content from the start
 *      to <tt>position</tt> (inclusive). It means that the implementation of the ustream can dispose
 *      any resources allocated to control and maintain this part of the ustream.
 *  It is up to the implementation of the ustream to decide to release any resource. For example, if the
 *      ustream is a string in the Flash, it does not make sense to release it.
 *  The provided position shall be the logical position, and it shall be between the logical first
 *      valid position of the ustream and the logical current position minus one. For example, the following
 *      code releases all bytes from the start to the last received position:
 *
 * <pre><code>
 * offset_t pos;
 * if(aziot_ustream_get_position(my_buffer, &pos) == AZIOT_ULIB_SUCCESS)
 * {
 *     aziot_ustream_release(my_buffer, pos - 1);
 * }
 * </code></pre>
 *
 *  The <tt>aziot_ustream_release</tt> API shall follow the following minimum requirements:
 *      - The <tt>release</tt> shall dispose all resources necessary to handle the content of ustream before and 
 *          including the release position.
 *      - If the release position is after the current position or the ustream size, the <tt>release</tt> shall
 *          return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR, and do not release any resource.
 *      - If the release position is already released, the <tt>release</tt> shall return
 *          #AZIOT_ULIB_NO_SUCH_ELEMENT_ERROR, and do not release any resource.
 *      - If the provided interface is <tt>NULL</tt>, the <tt>release</tt> shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the implemented ustream type, the <tt>release</tt> shall return
 *          #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param[in]  ustream_interface    The {@link AZIOT_USTREAM}* with the interface of the ustream. It
 *                                  cannot be <tt>NULL</tt>, and it shall be a valid ustream that is the
 *                                  implemented ustream type.
 * @param[in]  position             The <tt>offset_t</tt> with the position in the ustream to release. The
 *                                  ustream will release the <tt>uint8_t</tt> on the position and all <tt>uint8_t</tt>
 *                                  before the position. It shall be bigger than 0.
 *
 * @return The {@link AZIOT_ULIB_RESULT} with the result of the <tt>release</tt> operation.
 *          @retval     AZIOT_ULIB_SUCCESS                If the ustream releases the position with success.
 *          @retval     AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR If one of the provided parameters is invalid.
 *          @retval     AZIOT_ULIB_NO_SUCH_ELEMENT_ERROR  If the position is already released.
 *          @retval     AZIOT_ULIB_SYSTEM_ERROR           If the <tt>release</tt> operation failed on the system level.
 */
static inline AZIOT_ULIB_RESULT aziot_ustream_release(AZIOT_USTREAM* ustream_interface, offset_t position)
{
    return ustream_interface->inner_buffer->api->release(ustream_interface, position);
}

/**
 * @brief   Creates a new instance of the ustream and returns it.
 *
 *  Cloning a ustream will create a new instance of the ustream that shares the same content of the
 *      original one. The clone shall not copy the content of the ustream, but only add a reference to
 *      it.
 *
 *  Both the start position and the current position of the cloned ustream will be the current
 *      position of the original ustream. The logical position of it will be determined by the provided
 *      offset.
 *
 *  The size of the new ustream will be the remaining size of the original ustream, which is the size
 *      minus the current position.
 *
 * <i><b>Example 1</b></i>
 *
 *  Consider a ustream with 1500 bytes, that was created from the factory, with <tt>Logical</tt> and <tt>Inner</tt>
 *      positions equal to <tt>0</tt>. After some operations, 1000 bytes were read (from 0 to 999). The
 *      current position is <tt>1000</tt>, and 200 bytes were released (from 0 to 199), so the released
 *      position is <tt>199</tt>.
 *  For the following examples, the positions are represented by <tt>[Logical, Inner]</tt>.
 *
 * Original ustream:
 *
 * <pre><code>
 *
 *  |      Released     |             Pending             |               Future              |
 *  |-------------------|---------------------------------|-----------------------------------|
 *  |<- start [0, 0]    |<- released [199, 199]           |<- current [1000, 1000]            |<- end [1499, 1499]
 *
 * </code></pre>
 *
 * Cloning the original ustream with offset 0 will result in the following ustream:
 *
 * <pre><code>
 *
 *                       |||             Future                |
 *                       |||-----------------------------------|
 *  released [-1, 999] ->|||<- start, current [0, 1000]        |<- end [499, 1499]
 *
 * </code></pre>
 *
 * Cloning the same original ustream with offset 100 will result in the following ustream:
 *
 * <pre><code>
 *
 *                       |||             Future                |
 *                       |||-----------------------------------|
 *  released [99, 999] ->|||<- start, current [100, 1000]      |<- end [599, 1499]
 *
 * </code></pre>
 *
 * <i><b>Example 2</b></i>
 *
 *  Consider a ustream with 5000 bytes, that was created from the factory, with <tt>Logical</tt> and <tt>Inner</tt>
 *      positions equal to <tt>0</tt>. After some operations, 250 bytes were read (from 0 to 249), so the
 *      current position is <tt>250</tt>, and no release was made, so the released position is still <tt>-1</tt>.
 *
 *  For the following examples, the positions are represented by <tt>[Logical, Inner]</tt>.
 *
 *
 * Original ustream:
 *
 * <pre><code>
 *
 *                     ||     Pending       |                         Future                      |
 *                     ||-------------------+-----------------------------------------------------|
 *  released [-1, 0] ->||<- start [0, 0]    |<- current [250, 250]                                |<- end [4999, 4999]
 *
 * </code></pre>
 *
 * Cloning this original ustream with offset 10000 will result in the following ustream:
 *
 * <pre><code>
 *
 *                         |||                Future                |
 *                         |||--------------------------------------|
 *  released [9999, 249] ->|||<- start, current [10000, 250]        |<- end [14749, 4999]
 *
 * </code></pre>
 *
 * <i><b>Example 3</b></i>
 *
 *  From the previous cloned ustream, after some operations, the <tt>Logical</tt> current position is moved
 *      to <tt>11000</tt>, and the <tt>Logical</tt> released position is <tt>10499</tt>.

 *  For the following examples, the positions are represented by <tt>[Logical, Inner]</tt>.
 *
 * Previous cloned ustream:
 *
 * <pre><code>
 *
 *  |          Released       |           Pending          |               Future              |
 *  |-------------------------+----------------------------+-----------------------------------|
 *  |<- start [10000, 250]    |<- released [10499, 749]    |<- current [11000, 1250]           |<- end [14749, 4999]
 *
 * </code></pre>
 *
 * Cloning this cloned ustream with offset 0 will result in the following ustream:
 *
 * <pre><code>
 *
                          |||                Future                |
 *                        |||--------------------------------------|
 *  released [-1, 1249] ->|||<- start, current [0, 1250]           |<- end [3749, 4999]
 *
 * </code></pre>
 *
 *  @note
 *  From the point of view of a consumer, the <tt>Inner</tt> position never matters, it will
 *      always use the <tt>Logical</tt> position for all operations.
 *  @note
 *  If the position is not important to the consumer, making the offset equal to <tt>0</tt> is a safe option.
 *
 *  The <tt>aziot_ustream_clone</tt> API shall follow the following minimum requirements:
 *      - The <tt>clone</tt> shall return a ustream with the same content of the original ustream.
 *      - If the provided interface is <tt>NULL</tt>, the <tt>clone</tt> shall return <tt>NULL</tt>.
 *      - If the provided interface is not a type of the implemented ustream, the <tt>clone</tt> shall return <tt>NULL</tt>.
 *      - If there is not enough memory to control the new ustream, the <tt>clone</tt> shall return <tt>NULL</tt>.
 *      - If the provided offset plus the ustream size is bigger than the maximum size_t, the <tt>clone</tt>
 *          shall return <tt>NULL</tt>.
 *      - The cloned ustream shall not interfere with the instance of the original ustream and vice versa.
 *
 * @param[in]   ustream_interface_clone           The {@link AZIOT_USTREAM}* with the interface of the ustream.
 *                                          It cannot be <tt>NULL</tt>, and it shall be a valid ustream instance type
 *                                          that is casted to an #AZIOT_USTREAM.
 * @param[in]   ustream_interface           The {@link AZIOT_USTREAM}* to be cloned.
 *                                          It cannot be <tt>NULL</tt>, and it shall be a valid ustream instance type
 *                                          that is casted to an #AZIOT_USTREAM.
 * @param[out]  offset                      The <tt>offset_t</tt> with the <tt>Logical</tt> position of the first byte in
 *                                          the cloned ustream.
 *
 * @return The {@link AZIOT_USTREAM}* with the result of the clone operation.
 *          @retval    not-NULL         If the ustream was cloned with success.
 *          @retval    NULL             If one of the provided parameters is invalid or there is not enough memory to
 *                                      control the new ustream.
 */
static inline AZIOT_ULIB_RESULT aziot_ustream_clone(AZIOT_USTREAM* ustream_interface_clone, AZIOT_USTREAM* ustream_interface, offset_t offset)
{
    return ustream_interface->inner_buffer->api->clone(ustream_interface_clone, ustream_interface, offset);
}

/**
 * @brief   Release all the resources allocated to control the instance of the ustream.
 *
 *  The dispose will release the instance of the ustream and decrement the reference of the ustream.
 *      If there are no more references to the ustream, the dispose will release all resources
 *      allocated to control the ustream.
 *
 *  The <tt>aziot_ustream_dispose</tt> API shall follow the following minimum requirements:
 *      - The <tt>dispose</tt> shall free all allocated resources for the instance of the ustream.
 *      - If there are no more instances of the ustream, the <tt>dispose</tt> shall release all allocated
 *          resources to control the ustream.
 *      - If the provided interface is <tt>NULL</tt>, the <tt>dispose</tt> shall return #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *      - If the provided interface is not the type of the implemented ustream, the <tt>dispose</tt> shall return
 *          #AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR.
 *
 * @param[in]   ustream_interface       The {@link AZIOT_USTREAM}* with the interface of the ustream. It
 *                                      cannot be <tt>NULL</tt>, and it shall be a valid ustream that is a type
 *                                      of the implemented ustream.
 *
 * @return The {@link AZIOT_ULIB_RESULT} with the result of the <tt>dispose</tt> operation.
 *          @retval AZIOT_ULIB_SUCCESS                    If the instance of the ustream was disposed with success.
 *          @retval AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR     If one of the provided parameters is invalid.
 */
static inline AZIOT_ULIB_RESULT aziot_ustream_dispose(AZIOT_USTREAM* ustream_interface)
{
    return ustream_interface->inner_buffer->api->dispose(ustream_interface);
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZIOT_USTREAM_BASE_H */
