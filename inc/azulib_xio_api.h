// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef AZURE_ULIB_C_INC_AZULIB_XIO_API_H_
#define AZURE_ULIB_C_INC_AZULIB_XIO_API_H_

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#include "azulib_option_api.h"

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif /* __cplusplus */

typedef struct XIO_INSTANCE_TAG* XIO_HANDLE;
typedef void* CONCRETE_IO_HANDLE;

MU_DEFINE_ENUM(IO_SEND_RESULT, \
        IO_SEND_OK, \
        IO_SEND_ERROR, \
        IO_SEND_CANCELLED);

MU_DEFINE_ENUM(IO_OPEN_RESULT, \
        IO_OPEN_OK, \
        IO_OPEN_ERROR, \
        IO_OPEN_CANCELLED);

typedef void(*ON_RELEASE_BUFFER)(void* buffer);
typedef void(*ON_BYTES_RECEIVED)(void* context, const unsigned char* buffer, size_t size, ON_RELEASE_BUFFER release_buffer);
typedef void(*ON_SEND_COMPLETE)(void* context, IO_SEND_RESULT send_result);
typedef void(*ON_IO_OPEN_COMPLETE)(void* context, IO_OPEN_RESULT open_result);
typedef void(*ON_IO_CLOSE_COMPLETE)(void* context);
typedef void(*ON_IO_ERROR)(void* context);

typedef int(*IO_CREATE)(CONCRETE_IO_HANDLE concrete_io, AZULIB_OPTION* options);
typedef void(*IO_DESTROY)(CONCRETE_IO_HANDLE concrete_io);
typedef int(*IO_OPEN_ASYNC)(CONCRETE_IO_HANDLE concrete_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context);
typedef int(*IO_CLOSE_ASYNC)(CONCRETE_IO_HANDLE concrete_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
typedef int(*IO_SEND_ASYNC)(CONCRETE_IO_HANDLE concrete_io, const void* buffer, size_t size, ON_RELEASE_BUFFER release_buffer, ON_SEND_COMPLETE on_send_complete, void* callback_context);


typedef struct IO_INTERFACE_DESCRIPTION_TAG
{
    IO_CREATE concrete_io_create;
    IO_DESTROY concrete_io_destroy;
    IO_OPEN_ASYNC concrete_io_open_async;
    IO_CLOSE_ASYNC concrete_io_close_async;
    IO_SEND_ASYNC concrete_io_send_async;
} IO_INTERFACE_DESCRIPTION;

MOCKABLE_FUNCTION(, int, xio_create, XIO_HANDLE, xio, const IO_INTERFACE_DESCRIPTION*, io_interface_description, const void*, io_create_parameters);
MOCKABLE_FUNCTION(, void, xio_destroy, XIO_HANDLE, xio);
MOCKABLE_FUNCTION(, int, xio_open_async, XIO_HANDLE, xio, ON_IO_OPEN_COMPLETE, on_io_open_complete, void*, on_io_open_complete_context, ON_BYTES_RECEIVED, on_bytes_received, void*, on_bytes_received_context, ON_IO_ERROR, on_io_error, void*, on_io_error_context);
MOCKABLE_FUNCTION(, int, xio_close_async, XIO_HANDLE, xio, ON_IO_CLOSE_COMPLETE, on_io_close_complete, void*, callback_context);
MOCKABLE_FUNCTION(, int, xio_send_async, XIO_HANDLE, xio, const void*, buffer, size_t, size, ON_RELEASE_BUFFER release_buffer, ON_SEND_COMPLETE, on_send_complete, void*, callback_context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AZURE_ULIB_C_INC_AZULIB_XIO_API_H_ */
