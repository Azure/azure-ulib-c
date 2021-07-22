// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#ifndef AZ_ULIB_ustream_forward_BASE_H
#define AZ_ULIB_ustream_forward_BASE_H

#include "az_ulib_base.h"
#include "az_ulib_config.h"
#include "az_ulib_pal_os.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#else
#include <stddef.h>
#include <stdint.h>
#endif /* __cplusplus */

#include "azure/core/_az_cfg_prefix.h"

/**
 * @brief   Define offset_t with the same size as size_t.
 */
typedef size_t offset_t;

/**
 * @brief   Forward declaration of az_ulib_ustream_forward. See #az_ulib_ustream_forward_tag for
 *          struct members.
 */
typedef struct az_ulib_ustream_forward_tag az_ulib_ustream_forward;

/**
 * @brief Signature of the function to be invoked by the `ustream_forward->_internal.flush` operation when  
 *        the `const az_span* const` buffer has been created.
 * 
 * @param[in]   buffer                  The `const uint8_t* const` buffer to be handled by the 
 *                                      implementation of this callback.
 * @param[in]   size                    The `size_t` size of the `const uint8_t* const` buffer.
 * @param[in]   flush_callback_context  The #flush_callback_context contract held between the owner  
 *                                      of this callback and the `ustream_forward->_internal.flush` 
 *                                      operation.
 */
typedef void (*az_ulib_flush_callback)(
                const uint8_t* const buffer, 
                size_t size, 
                az_ulib_callback_context flush_callback_context);

/**
 * @brief   vTable with the ustream_forward APIs.
 *
 *  Any module that exposes the ustream_forward shall implement the functions on this vTable.
 *
 *  Any code that will use an exposed ustream_forward shall call the APIs using the 
 *      `az_ulib_ustream_forward_...` inline functions.
 */
typedef struct az_ulib_ustream_forward_interface_tag
{
  /** Concrete `flush` implementation. */
  az_result (*flush)(
      az_ulib_ustream_forward* ustream_forward, 
      az_ulib_flush_callback flush_callback, 
      az_ulib_callback_context flush_callback_context);

  /** Concrete `read` implementation. */
  az_result (*read)(
      az_ulib_ustream_forward* ustream_forward,
      uint8_t* const buffer,
      size_t buffer_length,
      size_t* const size);

  /** Concrete `get_size` implementation. */
  az_result (*get_size)(
        az_ulib_ustream_forward* ustream_forward, 
        size_t* const size);

  /** Concrete `dispose` implementation. */
  az_result (*dispose)(
        az_ulib_ustream_forward* ustream_forward);
} az_ulib_ustream_forward_interface;

/**
 * @brief   Signature of the function to release memory passed to the ustream_forward
 *
 * @param[in]   release_pointer       void pointer to memory that needs to be free'd
 *
 */
typedef void (*az_ulib_release_callback)(void* release_pointer);

/**
 * @brief   Pointer to the data from which to read
 *
 * void pointer to memory where the data is located or any needed controls to access the data.
 * The content of the memory to which this points is up to the ustream_forward implementation.
 *
 */
typedef void* az_ulib_ustream_forward_data;

/**
 * @brief   Structure for data control block
 *
 * For any given ustream_forward that is created, one control block is created and initialized.
 *
 * @note    This structure should be viewed and used as internal to the implementation of the
 *          ustream_forward. Users should therefore not act on it directly and only allocate the 
 *          memory necessary for it to be passed to the ustream_forward.
 *
 */
struct az_ulib_ustream_forward_tag
{
  struct 
  {
    /** The #az_ulib_ustream_forward_interface* for this ustream_forward instance type. */
    const az_ulib_ustream_forward_interface* api;

    /** The #az_ulib_ustream_forward_data* pointing to the data to read. It can be anything that a 
     * give ustream_forward implementation needs to access the data, whether it be a memory address 
     * to a buffer, another struct with more controls, etc */
    const az_ulib_ustream_forward_data* ptr;

    /** The #az_ulib_release_callback to call to release `ptr` once the `ref_count` goes to 
     * zero. */
    az_ulib_release_callback data_release;

    /** The #az_ulib_release_callback to call to release the #az_ulib_ustream_forward_data_cb once 
     * the `ref_count` goes to zero */
    az_ulib_release_callback ustream_forward_release;

    /** The #offset_t used to keep track of the current position (next returned position). */
    offset_t inner_current_position;

    /** The `size_t` with the length of the data in the control_block. */
    size_t length;
  } _internal;
};

/**
 * @brief   Check if a handle is the same type of the API.
 *
 *  It will return true if the handle is valid and it is the same type of the API. It will
 *      return false if the handle is `NULL` or not the correct type.
 */
#define AZ_ULIB_USTREAM_FORWARD_IS_TYPE_OF(handle, type_api)                                     \
  (!((handle == NULL) || (handle->_internal.api == NULL) \
     || (handle->_internal.api != &type_api)))

/**
 * @brief Copy data directly from source (provider) to destination (consumer)
 * 
 *  The `az_ulib_ustream_forward_flush` API will copy ALL of the contents of the data from the
 *    source (provider) directly to the destination buffer provided by the consumer in the 
 *    `#az_ulib_flush_callback* flush_callback`.
 *  
 *  In the event that the source (provider) memory is not immediately available without a call
 *    to an external API (e.g. an HTTP connection to a blob storage provider), the flush operation
 *    will call this external API (e.g. http_response_body_get) in a loop, invoking the
 *    `flush_callback` each pass of the loop.
 *  
 *  The `az_ulib_ustream_forward_flush` API will provide the consumer a pointer to the memory to be
 *    copied in the form of a `const az_span* const` buffer, to be consumed in the `flush_callback`
 *    function implemented by the consumer. 
 * 
 * @note: It is the consumer's responsibility to increment any write offset referenced in the
 *        context inside the flush_callback implementation.
 * 
 *  The `az_ulib_ustream_forward_flush` API shall meet the following minimum requirements:
 *      - The flush operation create a `const az_span* const` from the `Data Source` and hand it
 *        off to the `flush_callback`
 *      - If the totality of `Data Source` is not available in the provided memory location,
 *        the API shall request the next portion of the data using the appropriate external API. 
 *        This process shall be repeated in a loop until the totality of the `Data Source` has been
 *        copied.
 *      - The API shall satisfy all of the precondition requirements laid forth below.
 * 
 * @param[in]       ustream_forward             The #az_ulib_ustream_forward* with the interface of
 *                                              the ustream_forward.
 * @param[out]      flush_callback              The #az_ulib_flush_callback* for the consumer to 
 *                                              handle the incoming data.
 * @param[out]      flush_callback_context      The #flush_callback_context contract between the 
 *                                              caller and flush_callback.
 * 
 * @pre     \p ustream_forward                  shall not be `NULL`.
 * @pre     \p ustream_forward                  shall be a valid ustream_forward that is the  
 *                                              implemented ustream_forward type.
 * @pre     \p flush_callback                   shall not be `NULL`.
 * 
 * @return The #az_result with the result of the flush operation
 *      @retval #AZ_OK                        If the flush operation succeeded in pushing the 
 *                                            entire stream content to `flush_callback`.
 *      @retval #AZ_ERROR_ULIB_BUSY           If the resource necessary to get the next portion of
 *                                            data is busy.
 *      @retval #AZ_ERROR_CANCELED            If any one of the flush operation's dependent
 *                                            external calls is canceled.
 *      @retval #AZ_ERROR_NOT_ENOUGH_SPACE    If there is not enough memory to finish copying the
 *                                            data from source to destination.
 *      @retval #AZ_ERROR_ULIB_SECURITY       If any one of the flush operation's dependent
 *                                            external calls returns an error for security reasons.
 *      @retval #AZ_ERROR_ULIB_SYSTEM         If any one of the flush operation's dependent
 *                                            fails at the system level.
 */ 
AZ_INLINE az_result az_ulib_ustream_forward_flush(
    az_ulib_ustream_forward* ustream_forward,
    az_ulib_flush_callback flush_callback, 
    az_ulib_callback_context flush_callback_context)
{
  return ustream_forward->_internal.api->flush(
            ustream_forward, 
            flush_callback, 
            flush_callback_context);
}

/**
 * @brief   Gets the next portion of the ustream_forward.
 *
 * The `az_ulib_ustream_forward_read` API will copy the contents of the `Data Source` to the local 
 *      buffer starting at the offset set by the previous call to `az_ulib_ustream_forward_read`. 
 *      This position will be the beginning of the stream (offset of 0) the first time this API is 
 *      called. The local buffer is the one referenced by the parameter `buffer`, and with the 
 *      maximum size `buffer_length`.
 *
 *  The buffer is defined as a `uint8_t*` and can represent any sequence of data. Pay
 *      special attention, if the data is a string, the buffer will still copy it as a sequence of
 *      `uint8_t`, and will <b>NOT</b> put any terminator at the end of the string. The size of
 *      the content copied in the local buffer will be returned in the parameter `size`.
 *
 *  The `az_ulib_ustream_forward_read` API shall follow the following minimum requirements:
 *      - The read shall copy the contents of the `Data Source` to the provided local buffer.
 *      - If the contents of the `Data Source` is bigger than the `buffer_length`, the read shall
 *          limit the copy size up to the buffer_length.
 *      - The read shall return the number of valid `uint8_t` values in the local buffer in
 *          the provided `size`.
 *      - If there is no more content to return, the read shall return
 *          #AZ_ULIB_EOF, size shall be set to 0, and will not change the contents
 *          of the local buffer.
 *      - The API shall satisfy all of the precondition requirements laid forth below.
 *
 * @param[in]       ustream_forward     The #az_ulib_ustream_forward* with the interface of the 
 *                                      ustream_forward.
 * @param[out]      buffer              The `uint8_t* const` that points to the local buffer.
 * @param[in]       buffer_length       The `size_t` with the size of the local buffer.
 * @param[out]      size                The `size_t* const` that points to the place where the
 *                                      read shall store the number of valid `uint8_t` values
 *                                      returned in the local buffer.
 *
 * @pre     \p ustream_forward          shall not be `NULL`.
 * @pre     \p ustream_forward          shall be a valid ustream_forward that is the implemented 
 *                                      ustream_forward type.
 * @pre     \p buffer                   shall not be `NULL`.
 * @pre     \p buffer_length            shall not be bigger than 0.
 * @pre     \p size                     shall not be `NULL`.
 *
 * @return The #az_result with the result of the read operation.
 *      @retval #AZ_OK                        If the ustream_forward copied the content of the 
 *                                            `Data Source` to the local buffer with success.
 *      @retval #AZ_ERROR_ULIB_BUSY           If the resource necessary to read the ustream_forward
 *                                            content is busy.
 *      @retval #AZ_ERROR_CANCELED            If the read of the content was cancelled.
 *      @retval #AZ_ULIB_EOF                  If there are no more `uint8_t` values in the `Data
 *                                            Source` to read.
 *      @retval #AZ_ERROR_NOT_ENOUGH_SPACE    If there is not enough memory to execute the read.
 *      @retval #AZ_ERROR_ULIB_SECURITY       If the read was denied for security reasons.
 *      @retval #AZ_ERROR_ULIB_SYSTEM         If the read operation failed on the system level.
 */
AZ_INLINE az_result az_ulib_ustream_forward_read(
    az_ulib_ustream_forward* ustream_forward,
    uint8_t* const buffer,
    size_t buffer_length,
    size_t* const size)
{
  return ustream_forward->_internal.api->read(ustream_forward, buffer, buffer_length, size);
}

/**
 * @brief   Returns the size of the ustream_forward.
 *
 *  This API returns the number of bytes in the ustream_forward.
 *
 *  The `az_ulib_ustream_forward_get_size` API shall follow the following minimum requirements:
 *      - The API shall return the number of bytes in the ustream_forward.
 *      - The API shall satisfy all of the precondition requirements laid forth below.
 *
 * @param[in]   ustream_forward     The #az_ulib_ustream_forward* with the interface of the 
 *                                  ustream_forward.
 * @param[out]  size                The `size_t* const` to return the number of `uint8_t` values.
 *
 * @pre     \p ustream_forward      shall not be `NULL`.
 * @pre     \p ustream_forward      shall be a valid ustream_forward that is the implemented 
 *                                  ustream_forward type.
 * @pre     \p size                 shall not be `NULL`.
 *
 * @return The #az_result with the result of the `get_size` operation.
 *      @retval #AZ_OK                        If it succeeded to get the remaining size of the
 *                                            ustream_forward.
 *      @retval #AZ_ERROR_ULIB_BUSY           If the resource necessary to get the remaining
 *                                            size of the ustream_forward is busy.
 *      @retval #AZ_ERROR_CANCELED            If the `get_size` was cancelled.
 *      @retval #AZ_ERROR_NOT_ENOUGH_SPACE    If there is not enough memory to execute the
 *                                            `get_size` operation.
 *      @retval #AZ_ERROR_ULIB_SECURITY       If the `get_size` was denied for
 *                                            security reasons.
 *      @retval #AZ_ERROR_ULIB_SYSTEM         If the `get_size` operation failed on
 *                                            the system level.
 */
AZ_INLINE az_result
az_ulib_ustream_forward_get_size(az_ulib_ustream_forward* ustream_forward, size_t* const size)
{
  return ustream_forward->_internal.api->get_size(ustream_forward, size);
}

/**
 * @brief   Release all the resources allocated to the ustream_forward.
 *
 *  The `az_ulib_ustream_forward_dispose` API shall follow the following minimum requirements:
 *      - The `dispose` shall release all resources allocated to the ustream_forward.
 *      - The API shall satisfy all of the precondition requirements laid forth below.
 *
 * @param[in]   ustream_forward     The #az_ulib_ustream_forward* with the interface of the 
 *                                  ustream_forward.
 *
 * @pre     \p ustream_forward      shall not be `NULL`.
 * @pre     \p ustream_forward      shall be a valid ustream_forward that is the implemented 
 *                                  ustream_forward type.
 *
 * @return The #az_result with the result of the `dispose` operation.
 *      @retval #AZ_OK                        If the instance of the ustream_forward was disposed
 *                                            with success.
 */
AZ_INLINE az_result az_ulib_ustream_forward_dispose(az_ulib_ustream_forward* ustream_forward)
{
  return ustream_forward->_internal.api->dispose(ustream_forward);
}

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_ustream_forward_BASE_H */
