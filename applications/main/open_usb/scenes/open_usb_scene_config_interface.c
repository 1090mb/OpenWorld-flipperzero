#include "../open_usb_app_i.h"

const char* interface_mode_text[] = {
    [OpenUsbHidInterfaceUsb] = "USB",
    [OpenUsbHidInterfaceBle] = "BLE (OpenBLE)",
};

static void open_usb_scene_config_interface_var_item_list_change_callback(
    VariableItem* item,
    void* context) {
    OpenUsbApp* open_usb = context;
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, interface_mode_text[index]);
    open_usb->interface = index;
}

void open_usb_scene_config_interface_on_enter(void* context) {
    OpenUsbApp* open_usb = context;
    VariableItemList* var_item_list = open_usb->var_item_list;
    VariableItem* item;

    variable_item_list_reset(var_item_list);

    item = variable_item_list_add(
        var_item_list,
        "Interface Mode",
        2,
        open_usb_scene_config_interface_var_item_list_change_callback,
        open_usb);

    variable_item_set_current_value_index(item, open_usb->interface);
    variable_item_set_current_value_text(item, interface_mode_text[open_usb->interface]);

    view_dispatcher_switch_to_view(open_usb->view_dispatcher, OpenUsbAppViewConfig);
}

bool open_usb_scene_config_interface_on_event(void* context, SceneManagerEvent event) {
    OpenUsbApp* open_usb = context;
    UNUSED(event);
    UNUSED(open_usb);
    return false;
}

void open_usb_scene_config_interface_on_exit(void* context) {
    OpenUsbApp* open_usb = context;
    variable_item_list_reset(open_usb->var_item_list);
}
