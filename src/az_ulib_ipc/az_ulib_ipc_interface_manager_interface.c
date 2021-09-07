// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "_az_ulib_interfaces.h"
#include "az_ulib_capability_api.h"
#include "az_ulib_descriptor_api.h"
#include "az_ulib_interface_manager_1_model.h"
#include "az_ulib_ipc_api.h"
#include "az_ulib_result.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static az_result interface_manager_1_set_default_concrete(
    const interface_manager_1_set_default_model_in* const in,
    az_ulib_model_out* out)
{
  (void)out;
  return az_ulib_ipc_set_default(
      in->package_name, in->package_version, in->interface_name, in->interface_version);
}

static az_result interface_manager_1_set_default_span_wrapper(
    az_span model_in_span,
    az_span* model_out_span)
{
  AZ_ULIB_TRY
  {
    // Unmarshalling JSON in model_in_span to interface_manager_model_in.
    az_json_reader jr;
    interface_manager_1_set_default_model_in interface_manager_model_in
        = { .package_name = AZ_SPAN_EMPTY,
            .package_version = AZ_ULIB_VERSION_DEFAULT,
            .interface_name = AZ_SPAN_EMPTY,
            .interface_version = AZ_ULIB_VERSION_DEFAULT };
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_init(&jr, model_in_span, NULL));
    AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    while (jr.token.kind != AZ_JSON_TOKEN_END_OBJECT)
    {
      if (az_json_token_is_text_equal(
              &jr.token, AZ_SPAN_FROM_STR(INTERFACE_MANAGER_1_SET_DEFAULT_INTERFACE_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        az_span interface_full_name
            = az_span_create(az_span_ptr(jr.token.slice), az_span_size(jr.token.slice));
        az_span device_name = AZ_SPAN_EMPTY;
        az_span capability_name = AZ_SPAN_EMPTY;
        AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_split_method_name(
            interface_full_name,
            &device_name,
            &interface_manager_model_in.package_name,
            &interface_manager_model_in.package_version,
            &interface_manager_model_in.interface_name,
            &interface_manager_model_in.interface_version,
            &capability_name));
      }
      else if (az_json_token_is_text_equal(
                   &jr.token, AZ_SPAN_FROM_STR(INTERFACE_MANAGER_1_SET_DEFAULT_PACKAGE_NAME_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        interface_manager_model_in.package_name
            = az_span_create(az_span_ptr(jr.token.slice), az_span_size(jr.token.slice));
      }
      else if (az_json_token_is_text_equal(
                   &jr.token,
                   AZ_SPAN_FROM_STR(INTERFACE_MANAGER_1_SET_DEFAULT_PACKAGE_VERSION_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        AZ_ULIB_THROW_IF_AZ_ERROR(
            az_json_token_get_uint32(&jr.token, &interface_manager_model_in.package_version));
      }
      else if (az_json_token_is_text_equal(
                   &jr.token,
                   AZ_SPAN_FROM_STR(INTERFACE_MANAGER_1_SET_DEFAULT_INTERFACE_NAME_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        interface_manager_model_in.interface_name
            = az_span_create(az_span_ptr(jr.token.slice), az_span_size(jr.token.slice));
      }
      else if (az_json_token_is_text_equal(
                   &jr.token,
                   AZ_SPAN_FROM_STR(INTERFACE_MANAGER_1_SET_DEFAULT_INTERFACE_VERSION_NAME)))
      {
        AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
        AZ_ULIB_THROW_IF_AZ_ERROR(
            az_json_token_get_uint32(&jr.token, &interface_manager_model_in.interface_version));
      }
      AZ_ULIB_THROW_IF_AZ_ERROR(az_json_reader_next_token(&jr));
    }
    AZ_ULIB_THROW_IF_AZ_ERROR(AZ_ULIB_TRY_RESULT);

    // Check if the required data was provided.
    AZ_ULIB_THROW_IF_ERROR(az_span_size(interface_manager_model_in.package_name) > 0, AZ_ERROR_ARG);
    AZ_ULIB_THROW_IF_ERROR(
        interface_manager_model_in.package_version != AZ_ULIB_VERSION_DEFAULT, AZ_ERROR_ARG);
    AZ_ULIB_THROW_IF_ERROR(
        az_span_size(interface_manager_model_in.interface_name) > 0, AZ_ERROR_ARG);
    AZ_ULIB_THROW_IF_ERROR(
        interface_manager_model_in.interface_version != AZ_ULIB_VERSION_DEFAULT, AZ_ERROR_ARG);

    // Call.
    AZ_ULIB_THROW_IF_AZ_ERROR(
        interface_manager_1_set_default_concrete(&interface_manager_model_in, NULL));

    // Marshalling empty install_model_out to JSON in model_out_span.
    *model_out_span = az_span_create_from_str("{}");
  }
  AZ_ULIB_CATCH(...) {}

  return AZ_ULIB_TRY_RESULT;
}

static const az_ulib_capability_descriptor INTERFACE_MANAGER_1_CAPABILITIES[]
    = { AZ_ULIB_DESCRIPTOR_ADD_CAPABILITY(
        INTERFACE_MANAGER_1_SET_DEFAULT_COMMAND_NAME,
        interface_manager_1_set_default_concrete,
        interface_manager_1_set_default_span_wrapper) };

static const az_ulib_interface_descriptor INTERFACE_MANAGER_1_DESCRIPTOR
    = AZ_ULIB_DESCRIPTOR_CREATE(
        IPC_1_PACKAGE_NAME,
        IPC_1_PACKAGE_VERSION,
        INTERFACE_MANAGER_1_INTERFACE_NAME,
        INTERFACE_MANAGER_1_INTERFACE_VERSION,
        INTERFACE_MANAGER_1_CAPABILITIES);

az_result _az_ulib_ipc_interface_manager_interface_publish(void)
{
  return az_ulib_ipc_publish(&INTERFACE_MANAGER_1_DESCRIPTOR);
}

az_result _az_ulib_ipc_interface_manager_interface_unpublish(void)
{
  return az_ulib_ipc_unpublish(&INTERFACE_MANAGER_1_DESCRIPTOR, AZ_ULIB_NO_WAIT);
}
