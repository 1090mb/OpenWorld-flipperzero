#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    OpenWorldContrastModeNormal,
    OpenWorldContrastModeHigh,
    OpenWorldContrastModeInverted,
} OpenWorldContrastMode;

typedef enum {
    OpenWorldFontSizeSmall,
    OpenWorldFontSizeMedium,
    OpenWorldFontSizeLarge,
} OpenWorldFontSize;

typedef enum {
    OpenWorldAnimationSpeedSlow,
    OpenWorldAnimationSpeedNormal,
    OpenWorldAnimationSpeedFast,
} OpenWorldAnimationSpeed;

typedef enum {
    OpenWorldOrientationHorizontal,
    OpenWorldOrientationHorizontalFlip,
    OpenWorldOrientationVertical,
    OpenWorldOrientationVerticalFlip,
} OpenWorldOrientation;

typedef struct {
    // Display Settings
    uint8_t display_brightness; // 0-100
    int8_t display_contrast; // -128 to 127
    OpenWorldOrientation screen_orientation;
    uint32_t display_timeout_ms; // Screen timeout in ms
    
    // Animation Settings
    bool animations_enabled;
    OpenWorldAnimationSpeed animation_speed;
    
    // Visual Settings
    OpenWorldContrastMode contrast_mode;
    OpenWorldFontSize font_size;
    
    // UI Element Settings
    bool show_scrollbar;
    bool smooth_transitions;
    uint8_t icon_style; // 0 = default, 1 = alternate
    
    // Status Bar Settings
    bool show_battery_percentage;
    bool show_clock_on_statusbar;
} OpenWorldSettings;

void open_world_settings_load(OpenWorldSettings* settings);
void open_world_settings_save(const OpenWorldSettings* settings);
void open_world_settings_apply(const OpenWorldSettings* settings);
