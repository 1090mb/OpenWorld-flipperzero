#include "../helpers/ducky_script.h"
#include "../open_usb_app_i.h"
#include "../views/open_usb_view.h"
#include <furi_hal.h>
#include "toolbox/path.h"

void open_usb_scene_work_button_callback(InputKey key, void* context) {
    furi_assert(context);
    OpenUsbApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, key);
}

bool open_usb_scene_work_on_event(void* context, SceneManagerEvent event) {
    OpenUsbApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InputKeyLeft) {
            if(open_usb_view_is_idle_state(app->open_usb_view)) {
                open_usb_script_close(app->open_usb_script);
                app->open_usb_script = NULL;

                if(app->interface == OpenUsbHidInterfaceBle) {
                    scene_manager_next_scene(app->scene_manager, OpenUsbSceneConfig);
                } else {
                    scene_manager_next_scene(app->scene_manager, OpenUsbSceneConfigLayout);
                }
            }
            consumed = true;
        } else if(event.event == InputKeyOk) {
            open_usb_script_start_stop(app->open_usb_script);
            consumed = true;
        } else if(event.event == InputKeyRight) {
            if(open_usb_view_is_idle_state(app->open_usb_view)) {
                open_usb_set_interface(
                    app,
                    app->interface == OpenUsbHidInterfaceBle ? OpenUsbHidInterfaceUsb :
                                                              OpenUsbHidInterfaceBle);
                open_usb_script_close(app->open_usb_script);
                app->open_usb_script = open_usb_script_open(app->file_path, app->interface);
            } else {
                open_usb_script_pause_resume(app->open_usb_script);
            }
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        open_usb_view_set_state(app->open_usb_view, open_usb_script_get_state(app->open_usb_script));
    }
    return consumed;
}

void open_usb_scene_work_on_enter(void* context) {
    OpenUsbApp* app = context;

    open_usb_view_set_interface(app->open_usb_view, app->interface);

    app->open_usb_script = open_usb_script_open(app->file_path, app->interface);
    open_usb_script_set_keyboard_layout(app->open_usb_script, app->keyboard_layout);

    FuriString* file_name;
    file_name = furi_string_alloc();
    path_extract_filename(app->file_path, file_name, true);
    open_usb_view_set_file_name(app->open_usb_view, furi_string_get_cstr(file_name));
    furi_string_free(file_name);

    FuriString* layout;
    layout = furi_string_alloc();
    path_extract_filename(app->keyboard_layout, layout, true);
    open_usb_view_set_layout(app->open_usb_view, furi_string_get_cstr(layout));
    furi_string_free(layout);

    open_usb_view_set_state(app->open_usb_view, open_usb_script_get_state(app->open_usb_script));

    open_usb_view_set_button_callback(app->open_usb_view, open_usb_scene_work_button_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, OpenUsbAppViewWork);
}

void open_usb_scene_work_on_exit(void* context) {
    UNUSED(context);
}
