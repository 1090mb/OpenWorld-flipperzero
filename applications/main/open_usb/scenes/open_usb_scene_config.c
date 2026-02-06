#include "../open_usb_app_i.h"

enum SubmenuIndex {
    ConfigIndexKeyboardLayout,
    ConfigIndexBleUnpair,
};

void open_usb_scene_config_select_callback(void* context, uint32_t index) {
    OpenUsbApp* open_usb = context;

    view_dispatcher_send_custom_event(open_usb->view_dispatcher, index);
}

static void draw_menu(OpenUsbApp* open_usb) {
    VariableItemList* var_item_list = open_usb->var_item_list;

    variable_item_list_reset(var_item_list);

    variable_item_list_add(var_item_list, "Keyboard Layout (global)", 0, NULL, NULL);

    variable_item_list_add(var_item_list, "Remove Pairing", 0, NULL, NULL);
}

void open_usb_scene_config_on_enter(void* context) {
    OpenUsbApp* open_usb = context;
    VariableItemList* var_item_list = open_usb->var_item_list;

    variable_item_list_set_enter_callback(
        var_item_list, open_usb_scene_config_select_callback, open_usb);
    draw_menu(open_usb);
    variable_item_list_set_selected_item(var_item_list, 0);

    view_dispatcher_switch_to_view(open_usb->view_dispatcher, OpenUsbAppViewConfig);
}

bool open_usb_scene_config_on_event(void* context, SceneManagerEvent event) {
    OpenUsbApp* open_usb = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == ConfigIndexKeyboardLayout) {
            scene_manager_next_scene(open_usb->scene_manager, OpenUsbSceneConfigLayout);
        } else if(event.event == ConfigIndexBleUnpair) {
            scene_manager_next_scene(open_usb->scene_manager, OpenUsbSceneConfirmUnpair);
        } else {
            furi_crash("Unknown key type");
        }
    }

    return consumed;
}

void open_usb_scene_config_on_exit(void* context) {
    OpenUsbApp* open_usb = context;
    VariableItemList* var_item_list = open_usb->var_item_list;

    variable_item_list_reset(var_item_list);
}
