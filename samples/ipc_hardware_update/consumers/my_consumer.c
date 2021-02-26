// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license.
// See LICENSE file in the project root for full license information.

#include "az_ulib_result.h"
#include "wrappers/display_1_wrapper.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

static const char* hello = "Hello world! This is a test to display a message.";
static const char* bunny_1 = "(\\(\\";
static const char* bunny_2 = "( -.-)";
static const char* bunny_3 = "o_(\")(\")";
static const char* bunny_11 = "/)/)";

static az_ulib_ipc_interface_handle _display_1;

static void get_handle_if_need(void)
{
  az_result result;
  if (_display_1 == NULL)
  {
    if ((result = display_1_create(&_display_1)) == AZ_OK)
    {
      (void)printf("My consumer got display.1 interface with success.\r\n");
    }
    else if (result == AZ_ERROR_ITEM_NOT_FOUND)
    {
      (void)printf("display.1 is not available.\r\n");
    }
    else
    {
      (void)printf("Get display.1 interface failed with code %d\r\n", result);
    }
  }
}

void my_consumer_create(void)
{
  (void)printf("Create my consumer...\r\n");

  _display_1 = NULL;
}

void my_consumer_do_display(void)
{
  static char state = 0;
  (void)printf("My consumer try use display.1 interface... \r\n");

  get_handle_if_need();

  if (_display_1 != NULL)
  {
    switch (state)
    {
      case 0:
      {
        AZ_ULIB_TRY
        {
          AZ_ULIB_THROW_IF_AZ_ERROR(display_1_cls(_display_1));
          AZ_ULIB_THROW_IF_AZ_ERROR(
              display_1_print(_display_1, 0, 0, hello, (int32_t)strlen(hello)));
          AZ_ULIB_THROW_IF_AZ_ERROR(
              display_1_print(_display_1, 6, 1, bunny_1, (int32_t)strlen(bunny_1)));
          AZ_ULIB_THROW_IF_AZ_ERROR(
              display_1_print(_display_1, 5, 2, bunny_2, (int32_t)strlen(bunny_2)));
          AZ_ULIB_THROW_IF_AZ_ERROR(
              display_1_print(_display_1, 5, 3, bunny_3, (int32_t)strlen(bunny_3)));
          AZ_ULIB_THROW_IF_AZ_ERROR(display_1_invalidate(_display_1));
          state = 1;
        }
        AZ_ULIB_CATCH(...)
        {
          if (AZ_ULIB_TRY_RESULT == AZ_ERROR_ITEM_NOT_FOUND)
          {
            (void)printf("display.1 was uninstalled.\r\n");
          }
          else
          {
            (void)printf(
                "my consumer uses display.1.cls failed with error %d.\r\n", AZ_ULIB_TRY_RESULT);
          }
          (void)printf("Release the handle.\r\n");
          display_1_destroy(_display_1);
          _display_1 = NULL;
        }
        break;
      }
      case 1:
      {
        AZ_ULIB_TRY
        {
          AZ_ULIB_THROW_IF_AZ_ERROR(
              display_1_print(_display_1, 6, 1, bunny_11, (int32_t)strlen(bunny_11)));
          AZ_ULIB_THROW_IF_AZ_ERROR(display_1_invalidate(_display_1));
          state = 2;
        }
        AZ_ULIB_CATCH(...)
        {
          if (AZ_ULIB_TRY_RESULT == AZ_ERROR_ITEM_NOT_FOUND)
          {
            (void)printf("display.1 was uninstalled.\r\n");
          }
          else
          {
            (void)printf(
                "my consumer uses display.1.cls failed with error %d.\r\n", AZ_ULIB_TRY_RESULT);
          }
          (void)printf("Release the handle.\r\n");
          display_1_destroy(_display_1);
          _display_1 = NULL;
          state = 0;
        }
        break;
      }
      case 2:
      {
        AZ_ULIB_TRY
        {
          AZ_ULIB_THROW_IF_AZ_ERROR(
              display_1_print(_display_1, 6, 1, bunny_1, (int32_t)strlen(bunny_1)));
          AZ_ULIB_THROW_IF_AZ_ERROR(display_1_invalidate(_display_1));
          state = 1;
        }
        AZ_ULIB_CATCH(...)
        {
          if (AZ_ULIB_TRY_RESULT == AZ_ERROR_ITEM_NOT_FOUND)
            (void)printf("display.1 was uninstalled.\r\n");
          else
            (void)printf(
                "my consumer uses display.1.cls failed with error %d.\r\n", AZ_ULIB_TRY_RESULT);
          (void)printf("Release the handle.\r\n");
          display_1_destroy(_display_1);
          _display_1 = NULL;
          state = 0;
        }
        break;
      }
    }
  }
}

void my_consumer_destroy(void)
{
  (void)printf("Destroy my consumer\r\n");
  if (_display_1 != NULL)
  {
    display_1_destroy(_display_1);
  }
}
