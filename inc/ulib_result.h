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
    ULIB_SUCCESS                        = 0,
    ULIB_EOF                            = 1,

    //ERROR RESULTS
    ULIB_OUT_OF_MEMORY_ERROR            = (ULIB_ERROR_FLAG | 0),
    ULIB_NO_SUCH_ELEMENT_ERROR          = (ULIB_ERROR_FLAG | 1),
    ULIB_ILLEGAL_ARGUMENT_ERROR         = (ULIB_ERROR_FLAG | 2),
    ULIB_SECURITY_ERROR                 = (ULIB_ERROR_FLAG | 3),
    ULIB_SYSTEM_ERROR                   = (ULIB_ERROR_FLAG | 4),
    ULIB_CANCELLED_ERROR                = (ULIB_ERROR_FLAG | 5),
    ULIB_BUSY_ERROR                     = (ULIB_ERROR_FLAG | 6)
)


#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  //AZURE_ULIB_C_INC_ULIB_RESULT_H_
