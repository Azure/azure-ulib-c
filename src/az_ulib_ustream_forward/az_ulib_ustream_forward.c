// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "az_ulib_port.h"
#include "az_ulib_result.h"
#include "az_ulib_ustream_forward.h"
#include "azure/core/az_span.h"

#include <azure/core/internal/az_precondition_internal.h>

#ifdef __clang__
#define IGNORE_POINTER_TYPE_QUALIFICATION \
  _Pragma("clang diagnostic push")        \
      _Pragma("clang diagnostic ignored \"-Wincompatible-pointer-types-discards-qualifiers\"")
#define IGNORE_MEMCPY_TO_NULL _Pragma("GCC diagnostic push")
#define RESUME_WARNINGS _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#define IGNORE_POINTER_TYPE_QUALIFICATION \
  _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wdiscarded-qualifiers\"")
#define IGNORE_MEMCPY_TO_NULL _Pragma("GCC diagnostic push")
#define RESUME_WARNINGS _Pragma("GCC diagnostic pop")
#else
#define IGNORE_POINTER_TYPE_QUALIFICATION __pragma(warning(push));
#define IGNORE_MEMCPY_TO_NULL \
  __pragma(warning(push));    \
  __pragma(warning(suppress : 6387));
#define RESUME_WARNINGS __pragma(warning(pop));
#endif // __clang__

static az_result concrete_read(
    az_ulib_ustream_forward* ustream_forward,
    az_span* buffer,
    size_t* const size);
static size_t concrete_get_size(az_ulib_ustream_forward* ustream_forward);
static az_result concrete_dispose(az_ulib_ustream_forward* ustream_forward);
static const az_ulib_ustream_forward_interface api
    = { concrete_read, concrete_get_size, concrete_dispose };

static az_result concrete_read(
    az_ulib_ustream_forward* ustream_forward,
    az_span* buffer,
    size_t* const size)
{
  _az_PRECONDITION(AZ_ULIB_USTREAM_FORWARD_IS_TYPE_OF(ustream_forward, api));
  _az_PRECONDITION_NOT_NULL(size);

  az_result result;

  /* if the provided buffer length is greater than 0, then we perform a memcpy to the provided
   * buffer*/
  if (!az_span_is_content_equal(*buffer, AZ_SPAN_EMPTY))
  {
    if (ustream_forward->_internal.inner_current_position >= ustream_forward->_internal.length)
    {
      *size = 0;
      result = AZ_ULIB_EOF;
    }
    else
    {
      size_t remain_size = ustream_forward->_internal.length
          - (size_t)ustream_forward->_internal.inner_current_position;
      *size = (az_span_size(*buffer) < remain_size) ? az_span_size(*buffer) : remain_size;

      /**
       * Since pre-conditions can be disabled by the user, compilers throw a warning for a potential
       * memcpy to `NULL`. We disable this warning knowing that it is the user's responsibility to
       * assure `buffer` is a valid pointer when pre-conditions are disabled for release mode.
       * See \ref Pre-conditions
       * "https://azure.github.io/azure-sdk/clang_design.html#pre-conditions" for more details.
       */
      IGNORE_MEMCPY_TO_NULL
      memcpy(
          buffer,
          (const uint8_t*)ustream_forward->_internal.ptr
              + ustream_forward->_internal.inner_current_position,
          *size);
      RESUME_WARNINGS
      ustream_forward->_internal.inner_current_position += *size;

      result = AZ_OK;
    }
  }

  /* if the provided buffer is AZ_SPAN_EMPTY, then we point the buffer directly to the data
   * including any offset from previous reads and set buffer_length as the size of the data*/
  else
  {
    // get size of data
    *size = concrete_get_size(ustream_forward);

    // point to data
    *buffer = az_span_create(
        (uint8_t* const)ustream_forward->_internal.ptr
            + ustream_forward->_internal.inner_current_position,
        (uint32_t)(*size));

    result = AZ_OK;
  }

  return result;
}

static size_t concrete_get_size(az_ulib_ustream_forward* ustream_forward)
{
  _az_PRECONDITION(AZ_ULIB_USTREAM_FORWARD_IS_TYPE_OF(ustream_forward, api));

  return ustream_forward->_internal.length - ustream_forward->_internal.inner_current_position;
}

static az_result concrete_dispose(az_ulib_ustream_forward* ustream_forward)
{
  _az_PRECONDITION(AZ_ULIB_USTREAM_FORWARD_IS_TYPE_OF(ustream_forward, api));

  if (ustream_forward->_internal.data_release)
  {
    /* If `data_relese` was provided is because `ptr` is not `const`. So, we have an Warning
     * exception here to remove the `const` qualification of the `ptr`. */
    IGNORE_POINTER_TYPE_QUALIFICATION
    ustream_forward->_internal.data_release(ustream_forward->_internal.ptr);
    RESUME_WARNINGS
  }
  if (ustream_forward->_internal.ustream_forward_release)
  {
    ustream_forward->_internal.ustream_forward_release(ustream_forward);
  }

  return AZ_OK;
}

AZ_NODISCARD az_result az_ulib_ustream_forward_init(
    az_ulib_ustream_forward* ustream_forward,
    az_ulib_release_callback ustream_forward_release,
    const uint8_t* const data_buffer,
    size_t data_buffer_length,
    az_ulib_release_callback data_buffer_release)
{
  _az_PRECONDITION_NOT_NULL(ustream_forward);
  _az_PRECONDITION_NOT_NULL(data_buffer);
  _az_PRECONDITION(data_buffer_length > 0);

  ustream_forward->_internal.api = &api;
  ustream_forward->_internal.ptr = (const az_ulib_ustream_forward_data*)data_buffer;
  ustream_forward->_internal.data_release = data_buffer_release;
  ustream_forward->_internal.ustream_forward_release = ustream_forward_release;
  ustream_forward->_internal.inner_current_position = 0;
  ustream_forward->_internal.length = data_buffer_length;

  return AZ_OK;
}
