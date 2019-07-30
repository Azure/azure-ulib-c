// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file ulib_result.h
 */

#ifndef AZULIB_RESULT_H
#define AZULIB_RESULT_H

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Error bit for error values.
 */
#define AZULIB_ERROR_FLAG 0X80

/**
 * @brief   Enumeration of uLib results.
 */
MU_DEFINE_ENUM(
    AZULIB_RESULT,

    //SUCCESS RESULTS
    AZULIB_SUCCESS                      = 0,                          /**<Successful return */
    AZULIB_EOF                          = 1,                          /**<End of fle (no more data to read) */

    //ERROR RESULTS
    AZULIB_OUT_OF_MEMORY_ERROR          = (AZULIB_ERROR_FLAG | 0),    /**<Out of memory error */
    AZULIB_NO_SUCH_ELEMENT_ERROR        = (AZULIB_ERROR_FLAG | 1),    /**<No such element error */
    AZULIB_ILLEGAL_ARGUMENT_ERROR       = (AZULIB_ERROR_FLAG | 2),    /**<Illegal argument error */
    AZULIB_SECURITY_ERROR               = (AZULIB_ERROR_FLAG | 3),    /**<Security error */
    AZULIB_SYSTEM_ERROR                 = (AZULIB_ERROR_FLAG | 4),    /**<System error */
    AZULIB_CANCELLED_ERROR              = (AZULIB_ERROR_FLAG | 5),    /**<Cancelled error */
    AZULIB_BUSY_ERROR                   = (AZULIB_ERROR_FLAG | 6)     /**<Busy error */
);


#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  //AZULIB_RESULT_H
