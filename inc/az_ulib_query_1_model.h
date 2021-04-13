// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

/********************************************************************
 * This code was auto-generated from query v1 DL and shall not be
 * modified.
 ********************************************************************/

#ifndef AZ_ULIB_QUERY_1_MODEL_H
#define AZ_ULIB_QUERY_1_MODEL_H

#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"
#include "azure/az_core.h"

#ifdef __cplusplus
#include <cstdint>
extern "C"
{
#else
#include <stdint.h>
#endif

/*
 * interface definition
 */
#define QUERY_1_INTERFACE_NAME "query"
#define QUERY_1_INTERFACE_VERSION 1
#define QUERY_1_CAPABILITY_SIZE 2

/*
 * Define query command on query interface.
 */
#define QUERY_1_QUERY_COMMAND (az_ulib_capability_index)0
#define QUERY_1_QUERY_COMMAND_NAME "query"
#define QUERY_1_QUERY_QUERY_NAME "query"
#define QUERY_1_QUERY_RESULT_NAME "result"
#define QUERY_1_QUERY_CONTINUATION_TOKEN_NAME "continuation_token"
  typedef struct
  {
    az_span query;
  } query_1_query_model_in;
  typedef struct
  {
    az_span* result;
    uint32_t continuation_token;
  } query_1_query_model_out;

/*
 * Define next command on query interface.
 */
#define QUERY_1_NEXT_COMMAND (az_ulib_capability_index)1
#define QUERY_1_NEXT_COMMAND_NAME "next"
#define QUERY_1_NEXT_CONTINUATION_TOKEN_NAME "continuation_token"
#define QUERY_1_NEXT_RESULT_NAME "result"
  typedef struct
  {
    uint32_t continuation_token;
  } query_1_next_model_in;
  typedef query_1_query_model_out query_1_next_model_out;

#ifdef __cplusplus
}
#endif

#endif /* AZ_ULIB_QUERY_1_MODEL_H */
