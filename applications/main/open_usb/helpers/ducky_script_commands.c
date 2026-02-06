#include <furi_hal.h>
#include <lib/toolbox/strint.h>
#include "ducky_script.h"
#include "ducky_script_i.h"

typedef int32_t (*DuckyCmdCallback)(OpenUsbScript* open_usb, const char* line, int32_t param);

typedef struct {
    char* name;
    size_t name_len;
    DuckyCmdCallback callback;
    int32_t param;
} DuckyCmd;

#define DUCKY_CMD(str, cb, p) {str, sizeof(str) - 1, cb, p}

static int32_t ducky_fnc_delay(OpenUsbScript* open_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint32_t delay_val = 0;
    bool state = ducky_get_number(line, &delay_val);
    if((state) && (delay_val > 0)) {
        return (int32_t)delay_val;
    }

    return ducky_error(open_usb, "Invalid number %s", line);
}

static int32_t ducky_fnc_defdelay(OpenUsbScript* open_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_get_number(line, &open_usb->defdelay);
    if(!state) {
        return ducky_error(open_usb, "Invalid number %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_strdelay(OpenUsbScript* open_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_get_number(line, &open_usb->stringdelay);
    if(!state) {
        return ducky_error(open_usb, "Invalid number %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_defstrdelay(OpenUsbScript* open_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_get_number(line, &open_usb->defstringdelay);
    if(!state) {
        return ducky_error(open_usb, "Invalid number %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_string(OpenUsbScript* open_usb, const char* line, int32_t param) {
    line = &line[ducky_get_command_len(line) + 1];
    furi_string_set_str(open_usb->string_print, line);
    if(param == 1) {
        furi_string_cat(open_usb->string_print, "\n");
    }

    if(open_usb->stringdelay == 0 &&
       open_usb->defstringdelay == 0) { // stringdelay not set - run command immediately
        bool state = ducky_string(open_usb, furi_string_get_cstr(open_usb->string_print));
        if(!state) {
            return ducky_error(open_usb, "Invalid string %s", line);
        }
    } else { // stringdelay is set - run command in thread to keep handling external events
        return SCRIPT_STATE_STRING_START;
    }

    return 0;
}

static int32_t ducky_fnc_repeat(OpenUsbScript* open_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    bool state = ducky_get_number(line, &open_usb->repeat_cnt);
    if((!state) || (open_usb->repeat_cnt == 0)) {
        return ducky_error(open_usb, "Invalid number %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_sysrq(OpenUsbScript* open_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint16_t key = ducky_get_keycode(open_usb, line, false);
    open_usb->hid->kb_press(open_usb->hid_inst, KEY_MOD_LEFT_ALT | HID_KEYBOARD_PRINT_SCREEN);
    open_usb->hid->kb_press(open_usb->hid_inst, key);
    open_usb->hid->release_all(open_usb->hid_inst);
    return 0;
}

static int32_t ducky_fnc_altchar(OpenUsbScript* open_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    ducky_numlock_on(open_usb);
    bool state = ducky_altchar(open_usb, line);
    if(!state) {
        return ducky_error(open_usb, "Invalid altchar %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_altstring(OpenUsbScript* open_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    ducky_numlock_on(open_usb);
    bool state = ducky_altstring(open_usb, line);
    if(!state) {
        return ducky_error(open_usb, "Invalid altstring %s", line);
    }
    return 0;
}

static int32_t ducky_fnc_hold(OpenUsbScript* open_usb, const char* line, int32_t param) {
    UNUSED(param);
    line = &line[ducky_get_command_len(line) + 1];

    if(open_usb->key_hold_nb > (HID_KB_MAX_KEYS - 1)) {
        return ducky_error(open_usb, "Too many keys are held");
    }

    // Handle Mouse Keys here
    uint16_t key = ducky_get_mouse_keycode_by_name(line);
    if(key != HID_MOUSE_NONE) {
        open_usb->key_hold_nb++;
        open_usb->hid->mouse_press(open_usb->hid_inst, key);
        return 0;
    }

    // Handle Keyboard keys here
    key = ducky_get_keycode(open_usb, line, true);
    if(key != HID_KEYBOARD_NONE) {
        open_usb->key_hold_nb++;
        open_usb->hid->kb_press(open_usb->hid_inst, key);
        return 0;
    }

    // keyboard and mouse were none
    return ducky_error(open_usb, "Unknown keycode for %s", line);
}

static int32_t ducky_fnc_release(OpenUsbScript* open_usb, const char* line, int32_t param) {
    UNUSED(param);
    line = &line[ducky_get_command_len(line) + 1];

    if(open_usb->key_hold_nb == 0) {
        return ducky_error(open_usb, "No keys are held");
    }

    // Handle Mouse Keys here
    uint16_t key = ducky_get_mouse_keycode_by_name(line);
    if(key != HID_MOUSE_NONE) {
        open_usb->key_hold_nb--;
        open_usb->hid->mouse_release(open_usb->hid_inst, key);
        return 0;
    }

    //Handle Keyboard Keys here
    key = ducky_get_keycode(open_usb, line, true);
    if(key != HID_KEYBOARD_NONE) {
        open_usb->key_hold_nb--;
        open_usb->hid->kb_release(open_usb->hid_inst, key);
        return 0;
    }

    // keyboard and mouse were none
    return ducky_error(open_usb, "No keycode defined for %s", line);
}

static int32_t ducky_fnc_media(OpenUsbScript* open_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint16_t key = ducky_get_media_keycode_by_name(line);
    if(key == HID_CONSUMER_UNASSIGNED) {
        return ducky_error(open_usb, "No keycode defined for %s", line);
    }
    open_usb->hid->consumer_press(open_usb->hid_inst, key);
    open_usb->hid->consumer_release(open_usb->hid_inst, key);
    return 0;
}

static int32_t ducky_fnc_globe(OpenUsbScript* open_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[ducky_get_command_len(line) + 1];
    uint16_t key = ducky_get_keycode(open_usb, line, false);
    if(key == HID_KEYBOARD_NONE) {
        return ducky_error(open_usb, "No keycode defined for %s", line);
    }

    open_usb->hid->consumer_press(open_usb->hid_inst, HID_CONSUMER_FN_GLOBE);
    open_usb->hid->kb_press(open_usb->hid_inst, key);
    open_usb->hid->kb_release(open_usb->hid_inst, key);
    open_usb->hid->consumer_release(open_usb->hid_inst, HID_CONSUMER_FN_GLOBE);
    return 0;
}

static int32_t ducky_fnc_waitforbutton(OpenUsbScript* open_usb, const char* line, int32_t param) {
    UNUSED(param);
    UNUSED(open_usb);
    UNUSED(line);

    return SCRIPT_STATE_WAIT_FOR_BTN;
}

static int32_t ducky_fnc_mouse_scroll(OpenUsbScript* open_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[strcspn(line, " ") + 1];
    int32_t mouse_scroll_dist = 0;

    if(strint_to_int32(line, NULL, &mouse_scroll_dist, 10) != StrintParseNoError) {
        return ducky_error(open_usb, "Invalid Number %s", line);
    }

    open_usb->hid->mouse_scroll(open_usb->hid_inst, mouse_scroll_dist);

    return 0;
}

static int32_t ducky_fnc_mouse_move(OpenUsbScript* open_usb, const char* line, int32_t param) {
    UNUSED(param);

    line = &line[strcspn(line, " ") + 1];
    int32_t mouse_move_x = 0;
    int32_t mouse_move_y = 0;

    if(strint_to_int32(line, NULL, &mouse_move_x, 10) != StrintParseNoError) {
        return ducky_error(open_usb, "Invalid Number %s", line);
    }

    line = &line[strcspn(line, " ") + 1];

    if(strint_to_int32(line, NULL, &mouse_move_y, 10) != StrintParseNoError) {
        return ducky_error(open_usb, "Invalid Number %s", line);
    }

    open_usb->hid->mouse_move(open_usb->hid_inst, mouse_move_x, mouse_move_y);

    return 0;
}

static const DuckyCmd ducky_commands[] = {
    DUCKY_CMD("REM", NULL, -1),
    DUCKY_CMD("ID", NULL, -1),
    DUCKY_CMD("DELAY", ducky_fnc_delay, -1),
    DUCKY_CMD("STRING", ducky_fnc_string, 0),
    DUCKY_CMD("STRINGLN", ducky_fnc_string, 1),
    DUCKY_CMD("DEFAULT_DELAY", ducky_fnc_defdelay, -1),
    DUCKY_CMD("DEFAULTDELAY", ducky_fnc_defdelay, -1),
    DUCKY_CMD("STRINGDELAY", ducky_fnc_strdelay, -1),
    DUCKY_CMD("STRING_DELAY", ducky_fnc_strdelay, -1),
    DUCKY_CMD("DEFAULT_STRING_DELAY", ducky_fnc_defstrdelay, -1),
    DUCKY_CMD("DEFAULTSTRINGDELAY", ducky_fnc_defstrdelay, -1),
    DUCKY_CMD("REPEAT", ducky_fnc_repeat, -1),
    DUCKY_CMD("SYSRQ", ducky_fnc_sysrq, -1),
    DUCKY_CMD("ALTCHAR", ducky_fnc_altchar, -1),
    DUCKY_CMD("ALTSTRING", ducky_fnc_altstring, -1),
    DUCKY_CMD("ALTCODE", ducky_fnc_altstring, -1),
    DUCKY_CMD("HOLD", ducky_fnc_hold, -1),
    DUCKY_CMD("RELEASE", ducky_fnc_release, -1),
    DUCKY_CMD("WAIT_FOR_BUTTON_PRESS", ducky_fnc_waitforbutton, -1),
    DUCKY_CMD("MEDIA", ducky_fnc_media, -1),
    DUCKY_CMD("GLOBE", ducky_fnc_globe, -1),
    DUCKY_CMD("MOUSEMOVE", ducky_fnc_mouse_move, -1),
    DUCKY_CMD("MOUSE_MOVE", ducky_fnc_mouse_move, -1),
    DUCKY_CMD("MOUSESCROLL", ducky_fnc_mouse_scroll, -1),
    DUCKY_CMD("MOUSE_SCROLL", ducky_fnc_mouse_scroll, -1),
};

#define TAG "OpenUsb"

#define WORKER_TAG TAG "Worker"

int32_t ducky_execute_cmd(OpenUsbScript* open_usb, const char* line) {
    size_t cmd_word_len = strcspn(line, " ");
    for(size_t i = 0; i < COUNT_OF(ducky_commands); i++) {
        size_t cmd_compare_len = ducky_commands[i].name_len;

        if(cmd_compare_len != cmd_word_len) {
            continue;
        }

        if(strncmp(line, ducky_commands[i].name, cmd_compare_len) == 0) {
            if(ducky_commands[i].callback == NULL) {
                return 0;
            } else {
                return (ducky_commands[i].callback)(open_usb, line, ducky_commands[i].param);
            }
        }
    }

    return SCRIPT_STATE_CMD_UNKNOWN;
}
