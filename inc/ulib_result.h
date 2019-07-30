// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file ulib_result.h
 */

#ifndef AZULIB_ULIB_RESULT_H
#define AZULIB_ULIB_RESULT_H

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Error bit for error values.
 */
#define AZULIB_ULIB_ERROR_FLAG 0X80

/**
 * @brief   Enumeration of uLib results.
 */
MU_DEFINE_ENUM(
    AZULIB_ULIB_RESULT,

    //SUCCESS RESULTS
    AZULIB_ULIB_SUCCESS                    = 0,                              /**<Successful return */
    AZULIB_ULIB_EOF                        = 1,                              /**<End of fle (no more data to read) */

    //ERROR RESULTS
    AZULIB_ULIB_OUT_OF_MEMORY_ERROR        = (AZULIB_ULIB_ERROR_FLAG | 0),    /**<Out of memory error */
    AZULIB_ULIB_NO_SUCH_ELEMENT_ERROR      = (AZULIB_ULIB_ERROR_FLAG | 1),    /**<No such element error */
    AZULIB_ULIB_ILLEGAL_ARGUMENT_ERROR     = (AZULIB_ULIB_ERROR_FLAG | 2),    /**<Illegal argument error */
    AZULIB_ULIB_SECURITY_ERROR             = (AZULIB_ULIB_ERROR_FLAG | 3),    /**<Security error */
    AZULIB_ULIB_SYSTEM_ERROR               = (AZULIB_ULIB_ERROR_FLAG | 4),    /**<System error */
    AZULIB_ULIB_CANCELLED_ERROR            = (AZULIB_ULIB_ERROR_FLAG | 5),    /**<Cancelled error */
    AZULIB_ULIB_BUSY_ERROR                 = (AZULIB_ULIB_ERROR_FLAG | 6)     /**<Busy error */
);


#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  //AZULIB_ULIB_RESULT_H
