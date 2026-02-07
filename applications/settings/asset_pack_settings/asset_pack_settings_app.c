/**
 * @file asset_pack_settings_app.c
 * Asset Pack Settings Application
 */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/widget.h>
#include <lib/asset_packs/asset_pack_manager.h>
#include <storage/storage.h>

#define TAG "AssetPackSettings"

#define SETTINGS_FILE EXT_PATH(".asset_pack.settings")

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    Widget* widget;
    AssetPack* asset_pack;
    uint32_t selected_pack_index;
} AssetPackSettingsApp;

typedef enum {
    AssetPackSettingsViewSubmenu,
    AssetPackSettingsViewWidget,
} AssetPackSettingsView;

static void asset_pack_settings_save_selection(uint32_t pack_index) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    
    if(storage_file_open(file, SETTINGS_FILE, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        storage_file_write(file, &pack_index, sizeof(pack_index));
        storage_file_close(file);
    }
    
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

static uint32_t asset_pack_settings_load_selection(void) {
    uint32_t pack_index = UINT32_MAX;
    
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    
    if(storage_file_open(file, SETTINGS_FILE, FSAM_READ, FSOM_OPEN_EXISTING)) {
        storage_file_read(file, &pack_index, sizeof(pack_index));
        storage_file_close(file);
    }
    
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    
    return pack_index;
}

static void asset_pack_settings_submenu_callback(void* context, uint32_t index) {
    AssetPackSettingsApp* app = context;
    
    if(index == 0) {
        // Default pack
        app->selected_pack_index = UINT32_MAX;
        asset_pack_unload(app->asset_pack);
    } else {
        // Custom pack
        app->selected_pack_index = index - 1;
        asset_pack_load(app->asset_pack, app->selected_pack_index);
    }
    
    asset_pack_settings_save_selection(app->selected_pack_index);
    
    // Show confirmation
    widget_reset(app->widget);
    widget_add_text_scroll_element(
        app->widget, 0, 0, 128, 64,
        index == 0 ? "Default assets selected\n\nRestart to apply changes" :
                     "Custom pack selected\n\nRestart to apply changes");
    
    view_dispatcher_switch_to_view(app->view_dispatcher, AssetPackSettingsViewWidget);
}

static uint32_t asset_pack_settings_widget_back_callback(void* context) {
    AssetPackSettingsApp* app = context;
    view_dispatcher_switch_to_view(app->view_dispatcher, AssetPackSettingsViewSubmenu);
    return VIEW_IGNORE;
}

static uint32_t asset_pack_settings_exit_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

static AssetPackSettingsApp* asset_pack_settings_app_alloc(void) {
    AssetPackSettingsApp* app = malloc(sizeof(AssetPackSettingsApp));
    
    app->asset_pack = asset_pack_alloc();
    app->selected_pack_index = asset_pack_settings_load_selection();
    
    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    
    // Submenu
    app->submenu = submenu_alloc();
    submenu_add_item(app->submenu, "Default (Built-in)", 0, asset_pack_settings_submenu_callback, app);
    
    // List available asset packs
    uint32_t pack_count = asset_pack_list_available(app->asset_pack);
    for(uint32_t i = 0; i < pack_count; i++) {
        const char* pack_name = asset_pack_get_name(app->asset_pack, i);
        if(pack_name) {
            submenu_add_item(app->submenu, pack_name, i + 1, asset_pack_settings_submenu_callback, app);
        }
    }
    
    // Set current selection
    if(app->selected_pack_index == UINT32_MAX) {
        submenu_set_selected_item(app->submenu, 0);
    } else {
        submenu_set_selected_item(app->submenu, app->selected_pack_index + 1);
    }
    
    view_set_previous_callback(submenu_get_view(app->submenu), asset_pack_settings_exit_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, AssetPackSettingsViewSubmenu, submenu_get_view(app->submenu));
    
    // Widget
    app->widget = widget_alloc();
    view_set_previous_callback(widget_get_view(app->widget), asset_pack_settings_widget_back_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, AssetPackSettingsViewWidget, widget_get_view(app->widget));
    
    view_dispatcher_switch_to_view(app->view_dispatcher, AssetPackSettingsViewSubmenu);
    
    return app;
}

static void asset_pack_settings_app_free(AssetPackSettingsApp* app) {
    furi_assert(app);
    
    view_dispatcher_remove_view(app->view_dispatcher, AssetPackSettingsViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, AssetPackSettingsViewWidget);
    
    submenu_free(app->submenu);
    widget_free(app->widget);
    view_dispatcher_free(app->view_dispatcher);
    
    asset_pack_free(app->asset_pack);
    
    furi_record_close(RECORD_GUI);
    
    free(app);
}

int32_t asset_pack_settings_app(void* p) {
    UNUSED(p);
    
    AssetPackSettingsApp* app = asset_pack_settings_app_alloc();
    view_dispatcher_run(app->view_dispatcher);
    asset_pack_settings_app_free(app);
    
    return 0;
}
