#include "../open_usb_app_i.h"
#include <furi_hal_power.h>
#include <storage/storage.h>

static bool open_usb_file_select(OpenUsbApp* open_usb) {
    furi_assert(open_usb);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, OPEN_USB_APP_SCRIPT_EXTENSION, &I_openusb_10px);
    browser_options.base_path = OPEN_USB_APP_BASE_FOLDER;
    browser_options.skip_assets = true;

    // Input events and views are managed by file_browser
    bool res = dialog_file_browser_show(
        open_usb->dialogs, open_usb->file_path, open_usb->file_path, &browser_options);

    return res;
}

void open_usb_scene_file_select_on_enter(void* context) {
    OpenUsbApp* open_usb = context;

    if(open_usb->open_usb_script) {
        open_usb_script_close(open_usb->open_usb_script);
        open_usb->open_usb_script = NULL;
    }

    if(open_usb_file_select(open_usb)) {
        scene_manager_next_scene(open_usb->scene_manager, OpenUsbSceneWork);
    } else {
        view_dispatcher_stop(open_usb->view_dispatcher);
    }
}

bool open_usb_scene_file_select_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    // OpenUsbApp* open_usb = context;
    return false;
}

void open_usb_scene_file_select_on_exit(void* context) {
    UNUSED(context);
    // OpenUsbApp* open_usb = context;
}
