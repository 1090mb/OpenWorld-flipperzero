#include <furi_hal.h>
#include "ducky_script_i.h"

typedef struct {
    char* name;
    size_t name_len;
    uint16_t keycode;
} DuckyKey;

#define DUCKY_KEY(str, code) {str, sizeof(str) - 1, code}

static const DuckyKey ducky_modifier_keys[] = {
    DUCKY_KEY("CTRL", KEY_MOD_LEFT_CTRL),
    DUCKY_KEY("CONTROL", KEY_MOD_LEFT_CTRL),
    DUCKY_KEY("SHIFT", KEY_MOD_LEFT_SHIFT),
    DUCKY_KEY("ALT", KEY_MOD_LEFT_ALT),
    DUCKY_KEY("GUI", KEY_MOD_LEFT_GUI),
    DUCKY_KEY("WINDOWS", KEY_MOD_LEFT_GUI),
};

static const DuckyKey ducky_keys[] = {
    DUCKY_KEY("DOWNARROW", HID_KEYBOARD_DOWN_ARROW),
    DUCKY_KEY("DOWN", HID_KEYBOARD_DOWN_ARROW),
    DUCKY_KEY("LEFTARROW", HID_KEYBOARD_LEFT_ARROW),
    DUCKY_KEY("LEFT", HID_KEYBOARD_LEFT_ARROW),
    DUCKY_KEY("RIGHTARROW", HID_KEYBOARD_RIGHT_ARROW),
    DUCKY_KEY("RIGHT", HID_KEYBOARD_RIGHT_ARROW),
    DUCKY_KEY("UPARROW", HID_KEYBOARD_UP_ARROW),
    DUCKY_KEY("UP", HID_KEYBOARD_UP_ARROW),

    DUCKY_KEY("ENTER", HID_KEYBOARD_RETURN),
    DUCKY_KEY("BREAK", HID_KEYBOARD_PAUSE),
    DUCKY_KEY("PAUSE", HID_KEYBOARD_PAUSE),
    DUCKY_KEY("CAPSLOCK", HID_KEYBOARD_CAPS_LOCK),
    DUCKY_KEY("DELETE", HID_KEYBOARD_DELETE_FORWARD),
    DUCKY_KEY("BACKSPACE", HID_KEYBOARD_DELETE),
    DUCKY_KEY("END", HID_KEYBOARD_END),
    DUCKY_KEY("ESC", HID_KEYBOARD_ESCAPE),
    DUCKY_KEY("ESCAPE", HID_KEYBOARD_ESCAPE),
    DUCKY_KEY("HOME", HID_KEYBOARD_HOME),
    DUCKY_KEY("INSERT", HID_KEYBOARD_INSERT),
    DUCKY_KEY("NUMLOCK", HID_KEYPAD_NUMLOCK),
    DUCKY_KEY("PAGEUP", HID_KEYBOARD_PAGE_UP),
    DUCKY_KEY("PAGEDOWN", HID_KEYBOARD_PAGE_DOWN),
    DUCKY_KEY("PRINTSCREEN", HID_KEYBOARD_PRINT_SCREEN),
    DUCKY_KEY("SCROLLLOCK", HID_KEYBOARD_SCROLL_LOCK),
    DUCKY_KEY("SPACE", HID_KEYBOARD_SPACEBAR),
    DUCKY_KEY("TAB", HID_KEYBOARD_TAB),
    DUCKY_KEY("MENU", HID_KEYBOARD_APPLICATION),
    DUCKY_KEY("APP", HID_KEYBOARD_APPLICATION),

    DUCKY_KEY("F1", HID_KEYBOARD_F1),
    DUCKY_KEY("F2", HID_KEYBOARD_F2),
    DUCKY_KEY("F3", HID_KEYBOARD_F3),
    DUCKY_KEY("F4", HID_KEYBOARD_F4),
    DUCKY_KEY("F5", HID_KEYBOARD_F5),
    DUCKY_KEY("F6", HID_KEYBOARD_F6),
    DUCKY_KEY("F7", HID_KEYBOARD_F7),
    DUCKY_KEY("F8", HID_KEYBOARD_F8),
    DUCKY_KEY("F9", HID_KEYBOARD_F9),
    DUCKY_KEY("F10", HID_KEYBOARD_F10),
    DUCKY_KEY("F11", HID_KEYBOARD_F11),
    DUCKY_KEY("F12", HID_KEYBOARD_F12),
    DUCKY_KEY("F13", HID_KEYBOARD_F13),
    DUCKY_KEY("F14", HID_KEYBOARD_F14),
    DUCKY_KEY("F15", HID_KEYBOARD_F15),
    DUCKY_KEY("F16", HID_KEYBOARD_F16),
    DUCKY_KEY("F17", HID_KEYBOARD_F17),
    DUCKY_KEY("F18", HID_KEYBOARD_F18),
    DUCKY_KEY("F19", HID_KEYBOARD_F19),
    DUCKY_KEY("F20", HID_KEYBOARD_F20),
    DUCKY_KEY("F21", HID_KEYBOARD_F21),
    DUCKY_KEY("F22", HID_KEYBOARD_F22),
    DUCKY_KEY("F23", HID_KEYBOARD_F23),
    DUCKY_KEY("F24", HID_KEYBOARD_F24),
};

static const DuckyKey ducky_media_keys[] = {
    DUCKY_KEY("POWER", HID_CONSUMER_POWER),
    DUCKY_KEY("REBOOT", HID_CONSUMER_RESET),
    DUCKY_KEY("SLEEP", HID_CONSUMER_SLEEP),
    DUCKY_KEY("LOGOFF", HID_CONSUMER_AL_LOGOFF),

    DUCKY_KEY("EXIT", HID_CONSUMER_AC_EXIT),
    DUCKY_KEY("HOME", HID_CONSUMER_AC_HOME),
    DUCKY_KEY("BACK", HID_CONSUMER_AC_BACK),
    DUCKY_KEY("FORWARD", HID_CONSUMER_AC_FORWARD),
    DUCKY_KEY("REFRESH", HID_CONSUMER_AC_REFRESH),

    DUCKY_KEY("SNAPSHOT", HID_CONSUMER_SNAPSHOT),

    DUCKY_KEY("PLAY", HID_CONSUMER_PLAY),
    DUCKY_KEY("PAUSE", HID_CONSUMER_PAUSE),
    DUCKY_KEY("PLAY_PAUSE", HID_CONSUMER_PLAY_PAUSE),
    DUCKY_KEY("NEXT_TRACK", HID_CONSUMER_SCAN_NEXT_TRACK),
    DUCKY_KEY("PREV_TRACK", HID_CONSUMER_SCAN_PREVIOUS_TRACK),
    DUCKY_KEY("STOP", HID_CONSUMER_STOP),
    DUCKY_KEY("EJECT", HID_CONSUMER_EJECT),

    DUCKY_KEY("MUTE", HID_CONSUMER_MUTE),
    DUCKY_KEY("VOLUME_UP", HID_CONSUMER_VOLUME_INCREMENT),
    DUCKY_KEY("VOLUME_DOWN", HID_CONSUMER_VOLUME_DECREMENT),

    DUCKY_KEY("FN", HID_CONSUMER_FN_GLOBE),
    DUCKY_KEY("BRIGHT_UP", HID_CONSUMER_BRIGHTNESS_INCREMENT),
    DUCKY_KEY("BRIGHT_DOWN", HID_CONSUMER_BRIGHTNESS_DECREMENT),
};

static const DuckyKey ducky_mouse_keys[] = {
    DUCKY_KEY("LEFTCLICK", HID_MOUSE_BTN_LEFT),
    DUCKY_KEY("LEFT_CLICK", HID_MOUSE_BTN_LEFT),
    DUCKY_KEY("RIGHTCLICK", HID_MOUSE_BTN_RIGHT),
    DUCKY_KEY("RIGHT_CLICK", HID_MOUSE_BTN_RIGHT),
    DUCKY_KEY("MIDDLECLICK", HID_MOUSE_BTN_WHEEL),
    DUCKY_KEY("MIDDLE_CLICK", HID_MOUSE_BTN_WHEEL),
    DUCKY_KEY("WHEELCLICK", HID_MOUSE_BTN_WHEEL),
    DUCKY_KEY("WHEEL_CLICK", HID_MOUSE_BTN_WHEEL),
};

uint16_t ducky_get_next_modifier_keycode_by_name(const char** param) {
    const char* input_str = *param;

    for(size_t i = 0; i < COUNT_OF(ducky_modifier_keys); i++) {
        size_t key_cmd_len = ducky_modifier_keys[i].name_len;
        if((strncmp(input_str, ducky_modifier_keys[i].name, key_cmd_len) == 0)) {
            char next_char_after_key = input_str[key_cmd_len];
            if(ducky_is_line_end(next_char_after_key) || (next_char_after_key == '-')) {
                *param = &input_str[key_cmd_len];
                return ducky_modifier_keys[i].keycode;
            }
        }
    }

    return HID_KEYBOARD_NONE;
}

uint16_t ducky_get_modifier_keycode_by_name(const char* param) {
    for(size_t i = 0; i < COUNT_OF(ducky_modifier_keys); i++) {
        size_t key_cmd_len = ducky_modifier_keys[i].name_len;
        if((strncmp(param, ducky_modifier_keys[i].name, key_cmd_len) == 0) &&
           (ducky_is_line_end(param[key_cmd_len]))) {
            return ducky_modifier_keys[i].keycode;
        }
    }

    return HID_KEYBOARD_NONE;
}

uint16_t ducky_get_keycode_by_name(const char* param) {
    for(size_t i = 0; i < COUNT_OF(ducky_keys); i++) {
        size_t key_cmd_len = ducky_keys[i].name_len;
        if((strncmp(param, ducky_keys[i].name, key_cmd_len) == 0) &&
           (ducky_is_line_end(param[key_cmd_len]))) {
            return ducky_keys[i].keycode;
        }
    }

    return HID_KEYBOARD_NONE;
}

uint16_t ducky_get_media_keycode_by_name(const char* param) {
    for(size_t i = 0; i < COUNT_OF(ducky_media_keys); i++) {
        size_t key_cmd_len = ducky_media_keys[i].name_len;
        if((strncmp(param, ducky_media_keys[i].name, key_cmd_len) == 0) &&
           (ducky_is_line_end(param[key_cmd_len]))) {
            return ducky_media_keys[i].keycode;
        }
    }

    return HID_CONSUMER_UNASSIGNED;
}

uint8_t ducky_get_mouse_keycode_by_name(const char* param) {
    for(size_t i = 0; i < COUNT_OF(ducky_mouse_keys); i++) {
        size_t key_cmd_len = ducky_mouse_keys[i].name_len;
        if((strncmp(param, ducky_mouse_keys[i].name, key_cmd_len) == 0) &&
           (ducky_is_line_end(param[key_cmd_len]))) {
            return ducky_mouse_keys[i].keycode;
        }
    }

    return HID_MOUSE_INVALID;
}
