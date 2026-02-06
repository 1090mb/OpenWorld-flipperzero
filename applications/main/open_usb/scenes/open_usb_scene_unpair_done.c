#include "../open_usb_app_i.h"

static void open_usb_scene_unpair_done_popup_callback(void* context) {
    OpenUsbApp* open_usb = context;
    scene_manager_search_and_switch_to_previous_scene(open_usb->scene_manager, OpenUsbSceneConfig);
}

void open_usb_scene_unpair_done_on_enter(void* context) {
    OpenUsbApp* open_usb = context;
    Popup* popup = open_usb->popup;

    open_usb_hid_ble_remove_pairing();

    popup_set_icon(popup, 48, 4, &I_DolphinDone_80x58);
    popup_set_header(popup, "Done", 20, 19, AlignLeft, AlignBottom);
    popup_set_callback(popup, open_usb_scene_unpair_done_popup_callback);
    popup_set_context(popup, open_usb);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(open_usb->view_dispatcher, OpenUsbAppViewPopup);
}

bool open_usb_scene_unpair_done_on_event(void* context, SceneManagerEvent event) {
    OpenUsbApp* open_usb = context;
    UNUSED(open_usb);
    UNUSED(event);
    bool consumed = false;

    return consumed;
}

void open_usb_scene_unpair_done_on_exit(void* context) {
    OpenUsbApp* open_usb = context;
    Popup* popup = open_usb->popup;
    UNUSED(popup);

    popup_reset(popup);
}
