// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file ulib_result.h
 * 
 * @brief ulib return values
 */

#ifndef AZIOT_ULIB_RESULT_H
#define AZIOT_ULIB_RESULT_H

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Error bit for error values.
 */
#define AZIOT_ULIB_ERROR_FLAG 0X80

/**
 * @brief   Enumeration of ulib results.
 */
MU_DEFINE_ENUM(
    AZIOT_ULIB_RESULT,

    //SUCCESS RESULTS
    AZIOT_ULIB_SUCCESS                    = 0,                              /**<Successful return */
    AZIOT_ULIB_EOF                        = 1,                              /**<End of fle (no more data to read) */

    //ERROR RESULTS
    AZIOT_ULIB_OUT_OF_MEMORY_ERROR        = (AZIOT_ULIB_ERROR_FLAG | 0),    /**<Out of memory error */
    AZIOT_ULIB_NO_SUCH_ELEMENT_ERROR      = (AZIOT_ULIB_ERROR_FLAG | 1),    /**<No such element error */
    AZIOT_ULIB_ILLEGAL_ARGUMENT_ERROR     = (AZIOT_ULIB_ERROR_FLAG | 2),    /**<Illegal argument error */
    AZIOT_ULIB_SECURITY_ERROR             = (AZIOT_ULIB_ERROR_FLAG | 3),    /**<Security error */
    AZIOT_ULIB_SYSTEM_ERROR               = (AZIOT_ULIB_ERROR_FLAG | 4),    /**<System error */
    AZIOT_ULIB_CANCELLED_ERROR            = (AZIOT_ULIB_ERROR_FLAG | 5),    /**<Cancelled error */
    AZIOT_ULIB_BUSY_ERROR                 = (AZIOT_ULIB_ERROR_FLAG | 6)     /**<Busy error */
);


#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  //AZIOT_ULIB_RESULT_H
