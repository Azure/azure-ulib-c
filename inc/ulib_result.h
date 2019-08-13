// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/**
 * @file ulib_result.h
 * 
 * @brief ulib return values
 */

#ifndef AZ_ULIB_RESULT_H
#define AZ_ULIB_RESULT_H

#include "azure_macro_utils/macro_utils.h"
#include "umock_c/umock_c_prod.h"

#ifndef __cplusplus
#else
extern "C" {
#endif

/**
 * @brief   Error bit for error values.
 */
#define AZ_ULIB_ERROR_FLAG 0X80

/**
 * @brief   Enumeration of ulib results.
 */
MU_DEFINE_ENUM(
    AZ_ULIB_RESULT,

    //SUCCESS RESULTS
    AZ_ULIB_SUCCESS                    = 0,                           /**<Successful return */
    AZ_ULIB_EOF                        = 1,                           /**<End of file (no more data to read) */
    AZ_ULIB_PENDING                    = 2,                           /**<Intermediate state waiting for the end of the job */

    //ERROR RESULTS
    AZ_ULIB_OUT_OF_MEMORY_ERROR        = (AZ_ULIB_ERROR_FLAG | 0),    /**<Out of memory error */
    AZ_ULIB_NO_SUCH_ELEMENT_ERROR      = (AZ_ULIB_ERROR_FLAG | 1),    /**<No such element error */
    AZ_ULIB_ILLEGAL_ARGUMENT_ERROR     = (AZ_ULIB_ERROR_FLAG | 2),    /**<Illegal argument error */
    AZ_ULIB_SECURITY_ERROR             = (AZ_ULIB_ERROR_FLAG | 3),    /**<Security error */
    AZ_ULIB_SYSTEM_ERROR               = (AZ_ULIB_ERROR_FLAG | 4),    /**<System error */
    AZ_ULIB_CANCELLED_ERROR            = (AZ_ULIB_ERROR_FLAG | 5),    /**<Cancelled error */
    AZ_ULIB_BUSY_ERROR                 = (AZ_ULIB_ERROR_FLAG | 6),    /**<Busy error */
    AZ_ULIB_PRECONDITION_ERROR         = (AZ_ULIB_ERROR_FLAG | 7),    /**<Precondition for executing the command was not met */
    AZ_ULIB_ELEMENT_DUPLICATE_ERROR    = (AZ_ULIB_ERROR_FLAG | 8),    /**<New element already exists */
    AZ_ULIB_DISABLED_ERROR             = (AZ_ULIB_ERROR_FLAG | 9),    /**<Disabled error */
    AZ_ULIB_INCOMPATIBLE_VERSION_ERROR = (AZ_ULIB_ERROR_FLAG | 10),   /**<Required version is not available error */
    AZ_ULIB_NOT_INITIALIZED_ERROR      = (AZ_ULIB_ERROR_FLAG | 11),   /**<Use a component that was not properly initialized */
    AZ_ULIB_ALREADY_INITIALIZED_ERROR  = (AZ_ULIB_ERROR_FLAG | 12)    /**<A singleton component is already initialized */
);

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  //AZ_ULIB_RESULT_H
