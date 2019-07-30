// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file ulib_result.h
 */

#ifndef AZURE_ULIB_C_INC_ULIB_RESULT_H_
#define AZURE_ULIB_C_INC_ULIB_RESULT_H_

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Error bit for error values.
 */
#define ULIB_ERROR_FLAG 0X80

/**
 * @brief   Enumeration of uLib results.
 */
MU_DEFINE_ENUM(
    ULIB_RESULT,

    //SUCCESS RESULTS
    ULIB_SUCCESS                        = 0,                        /**<Successful return */
    ULIB_EOF                            = 1,                        /**<End of file (no more data to read) */

    //ERROR RESULTS
    ULIB_OUT_OF_MEMORY_ERROR            = (ULIB_ERROR_FLAG | 0),    /**<Out of memory error */
    ULIB_NO_SUCH_ELEMENT_ERROR          = (ULIB_ERROR_FLAG | 1),    /**<No such element error */
    ULIB_ILLEGAL_ARGUMENT_ERROR         = (ULIB_ERROR_FLAG | 2),    /**<Illegal argument error */
    ULIB_SECURITY_ERROR                 = (ULIB_ERROR_FLAG | 3),    /**<Security error */
    ULIB_SYSTEM_ERROR                   = (ULIB_ERROR_FLAG | 4),    /**<System error */
    ULIB_CANCELLED_ERROR                = (ULIB_ERROR_FLAG | 5),    /**<Canceled error */
    ULIB_BUSY_ERROR                     = (ULIB_ERROR_FLAG | 6),    /**<Busy error */
    ULIB_NOT_SUPPORTED_ERROR            = (ULIB_ERROR_FLAG | 7),    /**<Feature not supported error */
    ULIB_NOT_INITIALIZED_ERROR          = (ULIB_ERROR_FLAG | 8)     /**<System not initialized yet */
)


#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  //AZURE_ULIB_C_INC_ULIB_RESULT_H_
