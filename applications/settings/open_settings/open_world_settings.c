#include "open_world_settings.h"
#include "open_world_settings_filename.h"

#include <saved_struct.h>
#include <storage/storage.h>
#include <furi_hal.h>

#define TAG "OpenWorldSettings"

#define OPEN_WORLD_SETTINGS_VER (1)
#define OPEN_WORLD_SETTINGS_PATH INT_PATH(OPEN_WORLD_SETTINGS_FILE_NAME)
#define OPEN_WORLD_SETTINGS_MAGIC (0x0E)

static void open_world_settings_init_defaults(OpenWorldSettings* settings) {
    memset(settings, 0, sizeof(OpenWorldSettings));
    
    // Default display settings
    settings->display_brightness = 100; // Full brightness
    settings->display_contrast = 0; // Default contrast
    settings->screen_orientation = OpenWorldOrientationHorizontal;
    settings->display_timeout_ms = 30000; // 30 seconds
    
    // Default animation settings
    settings->animations_enabled = true;
    settings->animation_speed = OpenWorldAnimationSpeedNormal;
    
    // Default visual settings
    settings->contrast_mode = OpenWorldContrastModeNormal;
    settings->font_size = OpenWorldFontSizeMedium;
    
    // Default UI element settings
    settings->show_scrollbar = true;
    settings->smooth_transitions = true;
    settings->icon_style = 0; // Default icon style
    
    // Default status bar settings
    settings->show_battery_percentage = true;
    settings->show_clock_on_statusbar = true;
}

void open_world_settings_load(OpenWorldSettings* settings) {
    furi_assert(settings);
    
    bool success = false;
    
    do {
        uint8_t version;
        if(!saved_struct_get_metadata(OPEN_WORLD_SETTINGS_PATH, NULL, &version, NULL)) break;
        
        if(version == OPEN_WORLD_SETTINGS_VER) {
            success = saved_struct_load(
                OPEN_WORLD_SETTINGS_PATH,
                settings,
                sizeof(OpenWorldSettings),
                OPEN_WORLD_SETTINGS_MAGIC,
                OPEN_WORLD_SETTINGS_VER);
        }
    } while(false);
    
    if(!success) {
        FURI_LOG_W(TAG, "Failed to load file, using defaults");
        open_world_settings_init_defaults(settings);
        open_world_settings_save(settings);
    }
}

void open_world_settings_save(const OpenWorldSettings* settings) {
    furi_assert(settings);
    
    const bool success = saved_struct_save(
        OPEN_WORLD_SETTINGS_PATH,
        settings,
        sizeof(OpenWorldSettings),
        OPEN_WORLD_SETTINGS_MAGIC,
        OPEN_WORLD_SETTINGS_VER);
    
    if(!success) {
        FURI_LOG_E(TAG, "Failed to save file");
    }
}

void open_world_settings_apply(const OpenWorldSettings* settings) {
    furi_assert(settings);
    
    // Apply display brightness
    // Note: This is a placeholder - actual implementation would interface with display HAL
    FURI_LOG_I(TAG, "Applying settings:");
    FURI_LOG_I(TAG, "  Brightness: %d%%", settings->display_brightness);
    FURI_LOG_I(TAG, "  Contrast: %d", settings->display_contrast);
    FURI_LOG_I(TAG, "  Animations: %s", settings->animations_enabled ? "ON" : "OFF");
    FURI_LOG_I(TAG, "  Font Size: %d", settings->font_size);
    
    // Future: Apply settings to actual system components
    // This would require integration with:
    // - Display HAL for brightness/contrast
    // - GUI service for font size and UI elements
    // - Animation manager for animation settings
}
