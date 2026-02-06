#pragma once

#include <gui/view.h>
#include "../helpers/ducky_script.h"

typedef struct OpenUsb OpenUsb;
typedef void (*OpenUsbButtonCallback)(InputKey key, void* context);

OpenUsb* open_usb_view_alloc(void);

void open_usb_view_free(OpenUsb* open_usb);

View* open_usb_view_get_view(OpenUsb* open_usb);

void open_usb_view_set_button_callback(
    OpenUsb* open_usb,
    OpenUsbButtonCallback callback,
    void* context);

void open_usb_view_set_file_name(OpenUsb* open_usb, const char* name);

void open_usb_view_set_layout(OpenUsb* open_usb, const char* layout);

void open_usb_view_set_state(OpenUsb* open_usb, OpenUsbState* st);

void open_usb_view_set_interface(OpenUsb* open_usb, OpenUsbHidInterface interface);

bool open_usb_view_is_idle_state(OpenUsb* open_usb);
