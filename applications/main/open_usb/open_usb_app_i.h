#pragma once

#include "open_usb_app.h"
#include "scenes/open_usb_scene.h"
#include "helpers/ducky_script.h"
#include "helpers/open_usb_hid.h"

#include <gui/gui.h>
#include <assets_icons.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>
#include <gui/modules/popup.h>
#include "views/open_usb_view.h"
#include <furi_hal_usb.h>

#define OPEN_USB_APP_BASE_FOLDER        EXT_PATH("openusb")
#define OPEN_USB_APP_PATH_LAYOUT_FOLDER OPEN_USB_APP_BASE_FOLDER "/assets/layouts"
#define OPEN_USB_APP_SCRIPT_EXTENSION   ".txt"
#define OPEN_USB_APP_LAYOUT_EXTENSION   ".kl"

typedef enum {
    OpenUsbAppErrorNoFiles,
    OpenUsbAppErrorCloseRpc,
} OpenUsbAppError;

struct OpenUsbApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    NotificationApp* notifications;
    DialogsApp* dialogs;
    Widget* widget;
    Popup* popup;
    VariableItemList* var_item_list;

    OpenUsbAppError error;
    FuriString* file_path;
    FuriString* keyboard_layout;
    OpenUsb* open_usb_view;
    OpenUsbScript* open_usb_script;

    OpenUsbHidInterface interface;
    FuriHalUsbInterface* usb_if_prev;
};

typedef enum {
    OpenUsbAppViewWidget,
    OpenUsbAppViewPopup,
    OpenUsbAppViewWork,
    OpenUsbAppViewConfig,
} OpenUsbAppView;

void open_usb_set_interface(OpenUsbApp* app, OpenUsbHidInterface interface);
