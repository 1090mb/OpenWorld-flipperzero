#include "open_settings.h"

const char* const region_unlock_text[] = {
    "OFF",
    "ON",
};

static void region_unlock_changed(VariableItem* item) {
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, region_unlock_text[index]);
    // This setting is informational - the unlock is always active in OpenWorld firmware
    // The region is hardcoded to "00" (unlocked) in furi_hal_region.c
}

const char* const experimental_features_text[] = {
    "OFF",
    "ON",
};

static void experimental_features_changed(VariableItem* item) {
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, experimental_features_text[index]);
    // Store experimental features flag in RTC
    if(index) {
        furi_hal_rtc_set_flag(FuriHalRtcFlagDebug);
    } else {
        furi_hal_rtc_reset_flag(FuriHalRtcFlagDebug);
    }
}

static uint32_t open_settings_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

OpenSettings* open_settings_alloc(void) {
    OpenSettings* app = malloc(sizeof(OpenSettings));

    app->gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    VariableItem* item;
    uint8_t value_index;
    app->var_item_list = variable_item_list_alloc();

    // Region Unlock (always enabled in OpenWorld firmware)
    item = variable_item_list_add(
        app->var_item_list,
        "Region Unlock",
        COUNT_OF(region_unlock_text),
        region_unlock_changed,
        app);
    value_index = 1; // Always ON
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, region_unlock_text[value_index]);

    // Show current region
    const char* region_name = furi_hal_region_get_name();
    item = variable_item_list_add(app->var_item_list, "Current Region", 1, NULL, app);
    variable_item_set_current_value_text(item, region_name);

    // Experimental Features toggle
    item = variable_item_list_add(
        app->var_item_list,
        "Experimental",
        COUNT_OF(experimental_features_text),
        experimental_features_changed,
        app);
    value_index = furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug) ? 1 : 0;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, experimental_features_text[value_index]);

    view_set_previous_callback(
        variable_item_list_get_view(app->var_item_list), open_settings_exit);
    view_dispatcher_add_view(
        app->view_dispatcher,
        OpenSettingsViewVarItemList,
        variable_item_list_get_view(app->var_item_list));

    view_dispatcher_switch_to_view(app->view_dispatcher, OpenSettingsViewVarItemList);

    return app;
}

void open_settings_free(OpenSettings* app) {
    furi_assert(app);
    // Variable item list
    view_dispatcher_remove_view(app->view_dispatcher, OpenSettingsViewVarItemList);
    variable_item_list_free(app->var_item_list);
    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    // Records
    furi_record_close(RECORD_GUI);
    free(app);
}

int32_t open_settings_app(void* p) {
    UNUSED(p);
    OpenSettings* app = open_settings_alloc();
    view_dispatcher_run(app->view_dispatcher);
    open_settings_free(app);
    return 0;
}
