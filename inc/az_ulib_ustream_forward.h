// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/**
 * @file az_ulib_ustream_forward.h
 *
 * @brief The ustream_forward interfaces and accompanying APIs.
 */

#ifndef AZ_ULIB_USTREAM_FORWARD_H
#define AZ_ULIB_USTREAM_FORWARD_H

#include "az_ulib_base.h"
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
 * @brief   Forward declaration of az_ulib_ustream_forward. See #az_ulib_ustream_forward_tag for
 *          struct members.
 */
typedef struct az_ulib_ustream_forward_tag az_ulib_ustream_forward;

/**
 * @brief   vTable with the ustream_forward APIs.
 *
 *  Any module that exposes the ustream_forward shall implement the functions on this vTable.
 *
 *  Any code that will use an exposed ustream_forward shall call the APIs using the
 *      `az_ulib_ustream_forward_...` inline functions.
 */

// buffer passed in by value
typedef struct az_ulib_ustream_forward_interface_tag
{

  /** Concrete `read` implementation. */
  /**
   * Change to:
   * az_result (*read)(
      az_ulib_ustream_forward* ustream_forward,
      az_span user_buffer,
      az_span* output_buffer);
   * */
  az_result (*read)(
      az_ulib_ustream_forward* ustream_forward,
      az_span* buffer,
      size_t* const size);

  /** Concrete `get_size` implementation. */
  size_t (*get_size)(az_ulib_ustream_forward* ustream_forward);

  /** Concrete `dispose` implementation. */ 
  /**
   * Change to:
  az_result (*close)(az_ulib_ustream_forward* ustream_forward);
   * 
  */
  az_result (*dispose)(az_ulib_ustream_forward* ustream_forward);
} az_ulib_ustream_forward_interface;

/**
 * @brief   Pointer to the data from which to read
 *
 * void pointer to memory where the data is located or any needed controls to access the data.
 * The content of the memory to which this points is up to the ustream_forward implementation.
 *
 */
typedef void* az_ulib_ustream_forward_data;

/**
 * @brief   Structure for the ustream_forward control block
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

    /** The #az_ulib_release_callback to call to release `ptr` */
    az_ulib_release_callback data_release;

    /** The #az_ulib_release_callback to call to release the #az_ulib_ustream_forward. */
    az_ulib_release_callback ustream_forward_release;

    /** The #offset_t used to keep track of the current position (next returned position). */
    offset_t inner_current_position;

    /** The number of `uint8_t` bytes of data in the control_block. */
    size_t length;
  } _internal;
};

/**
 * @brief   Check if a handle is the same type of the API.
 *
 *  It will return true if the handle is valid and it is the same type of the API. It will
 *      return false if the handle is `NULL` or not the correct type.
 */
#define AZ_ULIB_USTREAM_FORWARD_IS_TYPE_OF(handle, type_api) \
  (!((handle == NULL) || (handle->_internal.api == NULL) || (handle->_internal.api != &type_api)))

/**
 * @brief   Gets the next portion of the ustream_forward.
 *
 * The `az_ulib_ustream_forward_read` serves 2 scenarios depending on the input parameters:
 * 
 *      Scenario (1): Copy memory to caller-provided local buffer
 *      If the provided local #az_span `buffer` is NOT #AZ_SPAN_EMPTY, the API will  
 *      copy the contents of the ustream_forward to the provided #az_span `buffer` starting at   
 *      the offset set by the previous call to `az_ulib_ustream_forward_read` and ending at   
 *      either the end of the provided #az_span `buffer` or the end of the ustream_forward. The   
 *      starting position will be the beginning of the stream (offset of 0) the first time this 
 *      API is called. The size of the content copied in the local buffer will be returned in the
 *      parameter `size`. 
 *
 *      Scenario (2): Give caller direct access to memory
 *      If the provided local #az_span `buffer` is #AZ_SPAN_EMPTY, the API will point   
 *      this `buffer` directly to the contents of the ustream_forward. The size of the   
 *      ustream_forward will be returned in the parameter `size`. In the event that ustream_forward  
 *      is associated with a data stream, requiring multiple calls to an external API to grab all  
 *      the data, `size` will be set to the number of bytes returned by the most recent call to this  
 *      external API. 
 * 
 * The data associated with the #az_span `buffer` is defined as a `uint8_t*` and can represent any
 *      sequence of data. Pay special attention, if the data is a string, the buffer will still 
 *      refer to it as a sequence of `uint8_t`, and will <b>NOT</b> put any terminator at the end   
 *      of the string. 
 * 
 *  In the event that the ustream_forward is associated with a data stream and the source (provider) 
 *      memory is not immediately available without a call to an external API, the read operation 
 *      will call this external API, grabbing the returned bytes (often referred to
 *      as the "inner buffer") to handle them in accordance with the 2 scenarios outlined above. 
 *      Subsequent calls to the `az_ulib_ustream_forward_read` will only make additional calls to  
 *      this external API once the inner buffer from the previous call has been completely iterated 
 *      over. 
 *
 * The `az_ulib_ustream_forward_read` API shall follow the following minimum requirements:
 *  
 * Scenario (1)
 *      - The read shall copy the contents of the `Data Source` to the provided local buffer.
 *      - If the contents of the `Data Source` is bigger than the #az_span_size(`buffer`), the read
 *        shall limit the copy size up to the size of the #az_span.
 *      - The read shall return the number of valid `uint8_t` values in the local buffer in
 *        the provided `size`.
 *      - If there is no more content to return, the read shall return #AZ_ULIB_EOF, size shall be 
 *        set to 0, and will not change the contents of the local buffer.
 * 
 * Scenario (2)
 *      - The read operation shall create an #az_span from the `Data Source` and shall return it
 *        along with the `size_t` size.
 * 
 * Both Scenarios
 *      - If the totality of `Data Source` is not available in the provided memory location,
 *        the API shall request the next portion of the data using the appropriate external API.
 *      - The API shall satisfy all of the precondition requirements laid forth below.
 *
 * @param[in]       ustream_forward     The #az_ulib_ustream_forward* with the interface of the
 *                                      ustream_forward.
 * @param[out]      buffer              The #az_span* that points to the local buffer.
 * @param[out]      size                The `size_t* const` that points to the place where the
 *                                      read shall store the number of valid `uint8_t` values
 *                                      returned in the local buffer.
 *
 * @pre     \p ustream_forward          shall not be `NULL`.
 * @pre     \p ustream_forward          shall be a valid ustream_forward that is the implemented
 *                                      ustream_forward type.
 * @pre     \p buffer                   shall not be `NULL`.
 * @pre     \p size                     shall not be `NULL`.
 *
 * @return The #az_result with the result of the read operation.
 *      @retval #AZ_OK                        If the ustream_forward executed scenario (1) or (2)
 *                                            successfully.
 *      @retval #AZ_ERROR_ULIB_BUSY           If the resource necessary to read the ustream_forward
 *                                            content is busy.
 *      @retval #AZ_ERROR_CANCELED            If the read of the content was cancelled.
 *      @retval #AZ_ULIB_EOF                  If there are no more `uint8_t` values in the `Data
 *                                            Source` to read.
 *      @retval #AZ_ERROR_NOT_ENOUGH_SPACE    If there is not enough memory to execute the read.
 *      @retval #AZ_ERROR_ULIB_SECURITY       If the read was denied for security reasons.
 *      @retval #AZ_ERROR_ULIB_SYSTEM         If the read operation failed on the system level.
 */
AZ_NODISCARD AZ_INLINE az_result az_ulib_ustream_forward_read(
    az_ulib_ustream_forward* ustream_forward,
    az_span* buffer,
    size_t* const size)
{
  return ustream_forward->_internal.api->read(ustream_forward, buffer, size);
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
 *
 * @pre     \p ustream_forward      shall not be `NULL`.
 * @pre     \p ustream_forward      shall be a valid ustream_forward that is the implemented
 *                                  ustream_forward type.
 *
 * @return The `size_t` of the ustream_forward
 */
AZ_NODISCARD AZ_INLINE size_t
az_ulib_ustream_forward_get_size(az_ulib_ustream_forward* ustream_forward)
{
  return ustream_forward->_internal.api->get_size(ustream_forward);
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
 *
 * @note: While the simplest implementation of this API returns only #AZ_OK, other implementations
 *        may return errors.
 */
AZ_NODISCARD AZ_INLINE az_result
az_ulib_ustream_forward_dispose(az_ulib_ustream_forward* ustream_forward)
{
  return ustream_forward->_internal.api->dispose(ustream_forward);
}

/**
 * @brief   Factory to initialize a new ustream_forward.
 *
 *  This factory initializes a ustream_forward that handles the content of the provided buffer. As a
 *      result, it will return an #az_ulib_ustream_forward* with this content. The initialized
 *      ustream_forward takes ownership of the passed memory and will release the memory with the
 *      passed #az_ulib_release_callback function when the caller goes out of scope.
 *
 * @param[out]      ustream_forward             The pointer to the allocated
 *                                              #az_ulib_ustream_forward struct.
 * @param[in]       ustream_forward_release     The #az_ulib_release_callback function that will be
 *                                              called to release the ustream_forward block (the
 *                                              passed `ustream_forward` parameter). If `NULL` is
 *                                              passed, the `az_ulib_ustream_forward_dispose` will
 *                                              not release the memory associated with
 *                                              `ustream_forward`, so it will be the caller's
 *                                              responsibility to do so. For example, developers
 *                                              may use the stdlib `free` to release malloc'd
 *                                              memory.
 * @param[in]       data_buffer                 The `const uint8_t* const` that points to a memory
 *                                              position where the buffer starts.
 * @param[in]       data_buffer_length          The `size_t` with the number of `uint8_t` in the
 *                                              provided buffer.
 * @param[in]       data_buffer_release         The #az_ulib_release_callback function that will be
 *                                              called to release the data. If `NULL` is passed,
 *                                              the `az_ulib_ustream_forward_dispose` will not
 *                                              release the memory associated with
 *                                              `ustream_forward`, so it will be the caller's
 *                                              responsibility to do so. For example, developers
 *                                              may use the stdlib `free` to release malloc'd
 *                                              memory.
 *
 * @pre \p          ustream_forward             shall not be `NULL`.
 * @pre \p          data_buffer                 shall not be `NULL`.
 * @pre \p          data_buffer_length          shall be greater than `0`.
 *
 * @return The #az_result with result of the initialization.
 *      @retval #AZ_OK                          If the #az_ulib_ustream_forward* is successfully
 *                                              initialized.
 *
 * @note: While the simplest implementation of this API returns only #AZ_OK, other implementations
 *       may return errors.
 */
AZ_NODISCARD az_result az_ulib_ustream_forward_init(
    az_ulib_ustream_forward* ustream_forward,
    az_ulib_release_callback ustream_forward_release,
    const uint8_t* const data_buffer,
    size_t data_buffer_length,
    az_ulib_release_callback data_buffer_release);

#include "azure/core/_az_cfg_suffix.h"

#endif /* AZ_ULIB_USTREAM_FORWARD_H */
