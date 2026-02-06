#include "../open_usb_app_i.h"
#include <storage/storage.h>

static bool open_usb_layout_select(OpenUsbApp* open_usb) {
    furi_assert(open_usb);

    FuriString* predefined_path;
    predefined_path = furi_string_alloc();
    if(!furi_string_empty(open_usb->keyboard_layout)) {
        furi_string_set(predefined_path, open_usb->keyboard_layout);
    } else {
        furi_string_set(predefined_path, OPEN_USB_APP_PATH_LAYOUT_FOLDER);
    }

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, OPEN_USB_APP_LAYOUT_EXTENSION, &I_keyboard_10px);
    browser_options.base_path = OPEN_USB_APP_PATH_LAYOUT_FOLDER;
    browser_options.skip_assets = false;

    // Input events and views are managed by file_browser
    bool res = dialog_file_browser_show(
        open_usb->dialogs, open_usb->keyboard_layout, predefined_path, &browser_options);

    furi_string_free(predefined_path);
    return res;
}

void open_usb_scene_config_layout_on_enter(void* context) {
    OpenUsbApp* open_usb = context;

    if(open_usb_layout_select(open_usb)) {
        scene_manager_search_and_switch_to_previous_scene(open_usb->scene_manager, OpenUsbSceneWork);
    } else {
        scene_manager_previous_scene(open_usb->scene_manager);
    }
}

bool open_usb_scene_config_layout_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    // OpenUsbApp* open_usb = context;
    return false;
}

void open_usb_scene_config_layout_on_exit(void* context) {
    UNUSED(context);
    // OpenUsbApp* open_usb = context;
}
