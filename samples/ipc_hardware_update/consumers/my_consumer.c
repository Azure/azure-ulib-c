// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "my_consumer.h"
#include "az_ulib_pal_api.h"
#include "az_ulib_result.h"
#include "display_1_model.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define CONTOSO_PACKAGE_NAME "contoso"

static const char hello[] = "Hello world! This is a display test...";
static const size_t hello_size = sizeof(hello) - 1;
static const char manufactory[] = CONTOSO_PACKAGE_NAME;
static const size_t manufactory_size = sizeof(manufactory) - 1;
static const char bunny_1[] = "(\\(\\";
static const size_t bunny_1_size = sizeof(bunny_1) - 1;
static const char bunny_2[] = "( -.-)";
static const size_t bunny_2_size = sizeof(bunny_2) - 1;
static const char bunny_3[] = "o_(\")(\")";
static const size_t bunny_3_size = sizeof(bunny_3) - 1;
static const char bunny_11[] = "/)/)";
static const size_t bunny_11_size = sizeof(bunny_11) - 1;
static const uint32_t animate_display_image_interval = 1000;

static bool end_thread = false;
static az_ulib_pal_thread_handle animate_display_image_thread;

static az_result print_single_line(
    az_ulib_ipc_interface_handle handle,
    int32_t x,
    int32_t y,
    const char* buf,
    size_t size)
{
  display_1_print_model_in in = { .x = x, .y = y, .buffer = buf, .size = size };
  return az_ulib_ipc_call(handle, DISPLAY_1_PRINT_COMMAND, &in, NULL);
}

static az_ulib_pal_thread_ret animate_display_image(az_ulib_pal_thread_args args)
{
  bool* end_token = (bool*)args;
  char state = 0;
  az_ulib_ipc_interface_handle display = { 0 };

  while (!*end_token)
  {
    az_result result = az_ulib_ipc_try_get_interface(
        AZ_SPAN_EMPTY,
        AZ_SPAN_FROM_STR(CONTOSO_PACKAGE_NAME),
        AZ_ULIB_VERSION_DEFAULT,
        AZ_SPAN_FROM_STR(DISPLAY_1_INTERFACE_NAME),
        DISPLAY_1_INTERFACE_VERSION,
        &display);

    if (result == AZ_ULIB_RENEW)
    {
      result = AZ_OK;
      state = 0;
    }

    if (result == AZ_OK)
    {
      switch (state)
      {
        case 0:
        {
          AZ_ULIB_TRY
          {
            int32_t max_x = 0;
            int32_t max_y = 0;
            AZ_ULIB_THROW_IF_AZ_ERROR(
                az_ulib_ipc_call(display, DISPLAY_1_MAX_X_PROPERTY, NULL, &max_x));
            AZ_ULIB_THROW_IF_AZ_ERROR(
                az_ulib_ipc_call(display, DISPLAY_1_MAX_Y_PROPERTY, NULL, &max_y));
            AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_call(display, DISPLAY_1_CLS_COMMAND, NULL, NULL));
            AZ_ULIB_THROW_IF_AZ_ERROR(
                print_single_line(display, 0, 0, manufactory, manufactory_size));
            char buf[50];
            int buf_size
                = snprintf(buf, sizeof(buf), "Dimensions %" PRIi32 "x%" PRIi32, max_x, max_y);
            AZ_ULIB_THROW_IF_AZ_ERROR(print_single_line(display, 0, 1, buf, (size_t)buf_size));
            AZ_ULIB_THROW_IF_AZ_ERROR(print_single_line(display, 0, 3, hello, hello_size));
            AZ_ULIB_THROW_IF_AZ_ERROR(
                az_ulib_ipc_call(display, DISPLAY_1_INVALIDATE_COMMAND, NULL, NULL));
            state = 1;
          }
          AZ_ULIB_CATCH(...)
          {
            (void)printf(
                "Using display.1 interface failed with code %" PRIi32 "\r\n", AZ_ULIB_TRY_RESULT);
          }
          break;
        }
        case 1:
        {
          AZ_ULIB_TRY
          {
            AZ_ULIB_THROW_IF_AZ_ERROR(az_ulib_ipc_call(display, DISPLAY_1_CLS_COMMAND, NULL, NULL));
            AZ_ULIB_THROW_IF_AZ_ERROR(print_single_line(display, 6, 1, bunny_1, bunny_1_size));
            AZ_ULIB_THROW_IF_AZ_ERROR(print_single_line(display, 5, 2, bunny_2, bunny_2_size));
            AZ_ULIB_THROW_IF_AZ_ERROR(print_single_line(display, 5, 3, bunny_3, bunny_3_size));
            AZ_ULIB_THROW_IF_AZ_ERROR(
                az_ulib_ipc_call(display, DISPLAY_1_INVALIDATE_COMMAND, NULL, NULL));
            state = 2;
          }
          AZ_ULIB_CATCH(...)
          {
            (void)printf(
                "Using display.1 interface failed with code %" PRIi32 "\r\n", AZ_ULIB_TRY_RESULT);
          }
          break;
        }
        case 2:
        {
          AZ_ULIB_TRY
          {
            AZ_ULIB_THROW_IF_AZ_ERROR(print_single_line(display, 6, 1, bunny_11, bunny_11_size));
            AZ_ULIB_THROW_IF_AZ_ERROR(
                az_ulib_ipc_call(display, DISPLAY_1_INVALIDATE_COMMAND, NULL, NULL));
            state = 3;
          }
          AZ_ULIB_CATCH(...)
          {
            (void)printf(
                "Using display.1 interface failed with code %" PRIi32 "\r\n", AZ_ULIB_TRY_RESULT);
            state = 0;
          }
          break;
        }
        case 3:
        {
          AZ_ULIB_TRY
          {
            AZ_ULIB_THROW_IF_AZ_ERROR(print_single_line(display, 6, 1, bunny_1, bunny_1_size));
            AZ_ULIB_THROW_IF_AZ_ERROR(
                az_ulib_ipc_call(display, DISPLAY_1_INVALIDATE_COMMAND, NULL, NULL));
            state = 2;
          }
          AZ_ULIB_CATCH(...)
          {
            (void)printf(
                "Using display.1 interface failed with code %" PRIi32 "\r\n", AZ_ULIB_TRY_RESULT);
            state = 0;
          }
          break;
        }
      }

      result = az_ulib_ipc_release_interface(display);
      (void)result;
    }
    else if (result == AZ_ERROR_ITEM_NOT_FOUND)
    {
      (void)printf("display.1 is not available.\r\n");
    }
    else
    {
      (void)printf("Get display.1 interface failed with code %" PRIi32 "\r\n", result);
    }
    az_pal_os_sleep(animate_display_image_interval);
  }
  return 0;
}

void my_consumer_create(void)
{
  az_result result;

  (void)printf("Create my consumer...\r\n");

  end_thread = false;

  if ((result
       = az_pal_os_thread_create(animate_display_image, &end_thread, &animate_display_image_thread))
      != AZ_OK)
  {
    (void)printf("Animation thread failed with error %" PRIi32 "\r\n", result);
  }
  else
  {
    (void)printf("Animation thread started with success\r\n");
  }
}

void my_consumer_destroy(void)
{
  (void)printf("Destroy my consumer\r\n");

  end_thread = true;

  (void)az_pal_os_thread_join(animate_display_image_thread, NULL);
}
