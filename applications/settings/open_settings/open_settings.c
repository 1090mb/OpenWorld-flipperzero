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
    OpenSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, experimental_features_text[index]);
    // Store experimental features flag in RTC
    if(index) {
        furi_hal_rtc_set_flag(FuriHalRtcFlagDebug);
    } else {
        furi_hal_rtc_reset_flag(FuriHalRtcFlagDebug);
    }
}

// Look & Feel Settings Callbacks

static char brightness_text[8];
static void brightness_changed(VariableItem* item) {
    OpenSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    app->settings.display_brightness = (index + 1) * 10; // 10% to 100%
    snprintf(brightness_text, sizeof(brightness_text), "%d%%", app->settings.display_brightness);
    variable_item_set_current_value_text(item, brightness_text);
    open_world_settings_save(&app->settings);
}

static char contrast_text[8];
static void contrast_changed(VariableItem* item) {
    OpenSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    app->settings.display_contrast = (int8_t)(index - 10) * 10; // -100 to +100
    snprintf(contrast_text, sizeof(contrast_text), "%d", app->settings.display_contrast);
    variable_item_set_current_value_text(item, contrast_text);
    open_world_settings_save(&app->settings);
}

const char* const orientation_text[] = {
    "Horizontal",
    "H-Flip",
    "Vertical",
    "V-Flip",
};

static void orientation_changed(VariableItem* item) {
    OpenSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    app->settings.screen_orientation = index;
    variable_item_set_current_value_text(item, orientation_text[index]);
    open_world_settings_save(&app->settings);
}

static char timeout_text[16];
static void timeout_changed(VariableItem* item) {
    OpenSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    const uint32_t timeouts[] = {5000, 10000, 15000, 30000, 60000, 120000, 300000}; // 5s to 5min
    app->settings.display_timeout_ms = timeouts[index];
    uint32_t seconds = app->settings.display_timeout_ms / 1000;
    if(seconds >= 60) {
        snprintf(timeout_text, sizeof(timeout_text), "%lum", seconds / 60);
    } else {
        snprintf(timeout_text, sizeof(timeout_text), "%lus", seconds);
    }
    variable_item_set_current_value_text(item, timeout_text);
    open_world_settings_save(&app->settings);
}

const char* const onoff_text[] = {
    "OFF",
    "ON",
};

static void animations_changed(VariableItem* item) {
    OpenSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    app->settings.animations_enabled = index;
    variable_item_set_current_value_text(item, onoff_text[index]);
    open_world_settings_save(&app->settings);
}

const char* const speed_text[] = {
    "Slow",
    "Normal",
    "Fast",
};

static void animation_speed_changed(VariableItem* item) {
    OpenSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    app->settings.animation_speed = index;
    variable_item_set_current_value_text(item, speed_text[index]);
    open_world_settings_save(&app->settings);
}

const char* const contrast_mode_text[] = {
    "Normal",
    "High",
    "Inverted",
};

static void contrast_mode_changed(VariableItem* item) {
    OpenSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    app->settings.contrast_mode = index;
    variable_item_set_current_value_text(item, contrast_mode_text[index]);
    open_world_settings_save(&app->settings);
}

const char* const font_size_text[] = {
    "Small",
    "Medium",
    "Large",
};

static void font_size_changed(VariableItem* item) {
    OpenSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    app->settings.font_size = index;
    variable_item_set_current_value_text(item, font_size_text[index]);
    open_world_settings_save(&app->settings);
}

static void scrollbar_changed(VariableItem* item) {
    OpenSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    app->settings.show_scrollbar = index;
    variable_item_set_current_value_text(item, onoff_text[index]);
    open_world_settings_save(&app->settings);
}

static void transitions_changed(VariableItem* item) {
    OpenSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    app->settings.smooth_transitions = index;
    variable_item_set_current_value_text(item, onoff_text[index]);
    open_world_settings_save(&app->settings);
}

const char* const icon_style_text[] = {
    "Default",
    "Alternate",
};

static void icon_style_changed(VariableItem* item) {
    OpenSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    app->settings.icon_style = index;
    variable_item_set_current_value_text(item, icon_style_text[index]);
    open_world_settings_save(&app->settings);
}

static void battery_percentage_changed(VariableItem* item) {
    OpenSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    app->settings.show_battery_percentage = index;
    variable_item_set_current_value_text(item, onoff_text[index]);
    open_world_settings_save(&app->settings);
}

static void statusbar_clock_changed(VariableItem* item) {
    OpenSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    app->settings.show_clock_on_statusbar = index;
    variable_item_set_current_value_text(item, onoff_text[index]);
    open_world_settings_save(&app->settings);
}

static uint32_t open_settings_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

OpenSettings* open_settings_alloc(void) {
    OpenSettings* app = malloc(sizeof(OpenSettings));

    // Load saved settings
    open_world_settings_load(&app->settings);

    app->gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    VariableItem* item;
    uint8_t value_index;
    app->var_item_list = variable_item_list_alloc();

    // ===== ORIGINAL SETTINGS =====
    
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

    // ===== DISPLAY SETTINGS =====
    
    // Display Brightness
    item = variable_item_list_add(
        app->var_item_list,
        "Brightness",
        10, // 10% to 100%
        brightness_changed,
        app);
    value_index = (app->settings.display_brightness / 10) - 1;
    variable_item_set_current_value_index(item, value_index);
    snprintf(brightness_text, sizeof(brightness_text), "%d%%", app->settings.display_brightness);
    variable_item_set_current_value_text(item, brightness_text);

    // Display Contrast
    item = variable_item_list_add(
        app->var_item_list,
        "Contrast",
        21, // -100 to +100 in steps of 10
        contrast_changed,
        app);
    value_index = (app->settings.display_contrast / 10) + 10;
    variable_item_set_current_value_index(item, value_index);
    snprintf(contrast_text, sizeof(contrast_text), "%d", app->settings.display_contrast);
    variable_item_set_current_value_text(item, contrast_text);

    // Screen Orientation
    item = variable_item_list_add(
        app->var_item_list,
        "Orientation",
        COUNT_OF(orientation_text),
        orientation_changed,
        app);
    value_index = app->settings.screen_orientation;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, orientation_text[value_index]);

    // Display Timeout
    item = variable_item_list_add(
        app->var_item_list,
        "Timeout",
        7, // 5s, 10s, 15s, 30s, 1m, 2m, 5m
        timeout_changed,
        app);
    // Find closest timeout value
    const uint32_t timeouts[] = {5000, 10000, 15000, 30000, 60000, 120000, 300000};
    value_index = 3; // Default to 30s
    for(uint8_t i = 0; i < 7; i++) {
        if(app->settings.display_timeout_ms == timeouts[i]) {
            value_index = i;
            break;
        }
    }
    variable_item_set_current_value_index(item, value_index);
    uint32_t seconds = app->settings.display_timeout_ms / 1000;
    if(seconds >= 60) {
        snprintf(timeout_text, sizeof(timeout_text), "%lum", seconds / 60);
    } else {
        snprintf(timeout_text, sizeof(timeout_text), "%lus", seconds);
    }
    variable_item_set_current_value_text(item, timeout_text);

    // ===== ANIMATION SETTINGS =====
    
    // Animations Enabled
    item = variable_item_list_add(
        app->var_item_list,
        "Animations",
        COUNT_OF(onoff_text),
        animations_changed,
        app);
    value_index = app->settings.animations_enabled ? 1 : 0;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, onoff_text[value_index]);

    // Animation Speed
    item = variable_item_list_add(
        app->var_item_list,
        "Anim Speed",
        COUNT_OF(speed_text),
        animation_speed_changed,
        app);
    value_index = app->settings.animation_speed;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, speed_text[value_index]);

    // ===== VISUAL SETTINGS =====
    
    // Contrast Mode
    item = variable_item_list_add(
        app->var_item_list,
        "Contrast Mode",
        COUNT_OF(contrast_mode_text),
        contrast_mode_changed,
        app);
    value_index = app->settings.contrast_mode;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, contrast_mode_text[value_index]);

    // Font Size
    item = variable_item_list_add(
        app->var_item_list,
        "Font Size",
        COUNT_OF(font_size_text),
        font_size_changed,
        app);
    value_index = app->settings.font_size;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, font_size_text[value_index]);

    // ===== UI ELEMENT SETTINGS =====
    
    // Show Scrollbar
    item = variable_item_list_add(
        app->var_item_list,
        "Show Scrollbar",
        COUNT_OF(onoff_text),
        scrollbar_changed,
        app);
    value_index = app->settings.show_scrollbar ? 1 : 0;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, onoff_text[value_index]);

    // Smooth Transitions
    item = variable_item_list_add(
        app->var_item_list,
        "Smooth Trans",
        COUNT_OF(onoff_text),
        transitions_changed,
        app);
    value_index = app->settings.smooth_transitions ? 1 : 0;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, onoff_text[value_index]);

    // Icon Style
    item = variable_item_list_add(
        app->var_item_list,
        "Icon Style",
        COUNT_OF(icon_style_text),
        icon_style_changed,
        app);
    value_index = app->settings.icon_style;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, icon_style_text[value_index]);

    // ===== STATUS BAR SETTINGS =====
    
    // Battery Percentage
    item = variable_item_list_add(
        app->var_item_list,
        "Battery %",
        COUNT_OF(onoff_text),
        battery_percentage_changed,
        app);
    value_index = app->settings.show_battery_percentage ? 1 : 0;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, onoff_text[value_index]);

    // Status Bar Clock
    item = variable_item_list_add(
        app->var_item_list,
        "StatusBar Clock",
        COUNT_OF(onoff_text),
        statusbar_clock_changed,
        app);
    value_index = app->settings.show_clock_on_statusbar ? 1 : 0;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, onoff_text[value_index]);

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
