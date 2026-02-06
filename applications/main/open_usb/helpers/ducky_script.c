#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <lib/toolbox/args.h>
#include <lib/toolbox/strint.h>
#include <storage/storage.h>
#include "ducky_script.h"
#include "ducky_script_i.h"
#include <dolphin/dolphin.h>

#define TAG "OpenUsb"

#define WORKER_TAG TAG "Worker"

#define BADUSB_ASCII_TO_KEY(script, x) \
    (((uint8_t)x < 128) ? (script->layout[(uint8_t)x]) : HID_KEYBOARD_NONE)

typedef enum {
    WorkerEvtStartStop = (1 << 0),
    WorkerEvtPauseResume = (1 << 1),
    WorkerEvtEnd = (1 << 2),
    WorkerEvtConnect = (1 << 3),
    WorkerEvtDisconnect = (1 << 4),
} WorkerEvtFlags;

static const char ducky_cmd_id[] = {"ID"};

static const uint8_t numpad_keys[10] = {
    HID_KEYPAD_0,
    HID_KEYPAD_1,
    HID_KEYPAD_2,
    HID_KEYPAD_3,
    HID_KEYPAD_4,
    HID_KEYPAD_5,
    HID_KEYPAD_6,
    HID_KEYPAD_7,
    HID_KEYPAD_8,
    HID_KEYPAD_9,
};

uint32_t ducky_get_command_len(const char* line) {
    char* first_space = strchr(line, ' ');
    return first_space ? (first_space - line) : 0;
}

bool ducky_is_line_end(const char chr) {
    return (chr == ' ') || (chr == '\0') || (chr == '\r') || (chr == '\n');
}

uint16_t ducky_get_keycode(OpenUsbScript* open_usb, const char* param, bool accept_modifiers) {
    uint16_t keycode = ducky_get_keycode_by_name(param);
    if(keycode != HID_KEYBOARD_NONE) {
        return keycode;
    }

    if(accept_modifiers) {
        uint16_t keycode = ducky_get_modifier_keycode_by_name(param);
        if(keycode != HID_KEYBOARD_NONE) {
            return keycode;
        }
    }

    if(strlen(param) > 0) {
        return BADUSB_ASCII_TO_KEY(open_usb, param[0]) & 0xFF;
    }
    return 0;
}

bool ducky_get_number(const char* param, uint32_t* val) {
    uint32_t value = 0;
    if(strint_to_uint32(param, NULL, &value, 10) == StrintParseNoError) {
        *val = value;
        return true;
    }
    return false;
}

void ducky_numlock_on(OpenUsbScript* open_usb) {
    if((open_usb->hid->get_led_state(open_usb->hid_inst) & HID_KB_LED_NUM) == 0) {
        open_usb->hid->kb_press(open_usb->hid_inst, HID_KEYBOARD_LOCK_NUM_LOCK);
        open_usb->hid->kb_release(open_usb->hid_inst, HID_KEYBOARD_LOCK_NUM_LOCK);
    }
}

bool ducky_numpad_press(OpenUsbScript* open_usb, const char num) {
    if((num < '0') || (num > '9')) return false;

    uint16_t key = numpad_keys[num - '0'];
    open_usb->hid->kb_press(open_usb->hid_inst, key);
    open_usb->hid->kb_release(open_usb->hid_inst, key);

    return true;
}

bool ducky_altchar(OpenUsbScript* open_usb, const char* charcode) {
    uint8_t i = 0;
    bool state = false;

    open_usb->hid->kb_press(open_usb->hid_inst, KEY_MOD_LEFT_ALT);

    while(!ducky_is_line_end(charcode[i])) {
        state = ducky_numpad_press(open_usb, charcode[i]);
        if(state == false) break;
        i++;
    }

    open_usb->hid->kb_release(open_usb->hid_inst, KEY_MOD_LEFT_ALT);
    return state;
}

bool ducky_altstring(OpenUsbScript* open_usb, const char* param) {
    uint32_t i = 0;
    bool state = false;

    while(param[i] != '\0') {
        if((param[i] < ' ') || (param[i] > '~')) {
            i++;
            continue; // Skip non-printable chars
        }

        char temp_str[4];
        snprintf(temp_str, 4, "%u", param[i]);

        state = ducky_altchar(open_usb, temp_str);
        if(state == false) break;
        i++;
    }
    return state;
}

int32_t ducky_error(OpenUsbScript* open_usb, const char* text, ...) {
    va_list args;
    va_start(args, text);

    vsnprintf(open_usb->st.error, sizeof(open_usb->st.error), text, args);

    va_end(args);
    return SCRIPT_STATE_ERROR;
}

bool ducky_string(OpenUsbScript* open_usb, const char* param) {
    uint32_t i = 0;

    while(param[i] != '\0') {
        if(param[i] != '\n') {
            uint16_t keycode = BADUSB_ASCII_TO_KEY(open_usb, param[i]);
            if(keycode != HID_KEYBOARD_NONE) {
                open_usb->hid->kb_press(open_usb->hid_inst, keycode);
                open_usb->hid->kb_release(open_usb->hid_inst, keycode);
            }
        } else {
            open_usb->hid->kb_press(open_usb->hid_inst, HID_KEYBOARD_RETURN);
            open_usb->hid->kb_release(open_usb->hid_inst, HID_KEYBOARD_RETURN);
        }
        i++;
    }
    open_usb->stringdelay = 0;
    return true;
}

static bool ducky_string_next(OpenUsbScript* open_usb) {
    if(open_usb->string_print_pos >= furi_string_size(open_usb->string_print)) {
        return true;
    }

    char print_char = furi_string_get_char(open_usb->string_print, open_usb->string_print_pos);

    if(print_char != '\n') {
        uint16_t keycode = BADUSB_ASCII_TO_KEY(open_usb, print_char);
        if(keycode != HID_KEYBOARD_NONE) {
            open_usb->hid->kb_press(open_usb->hid_inst, keycode);
            open_usb->hid->kb_release(open_usb->hid_inst, keycode);
        }
    } else {
        open_usb->hid->kb_press(open_usb->hid_inst, HID_KEYBOARD_RETURN);
        open_usb->hid->kb_release(open_usb->hid_inst, HID_KEYBOARD_RETURN);
    }

    open_usb->string_print_pos++;

    return false;
}

static int32_t ducky_parse_line(OpenUsbScript* open_usb, FuriString* line) {
    uint32_t line_len = furi_string_size(line);
    const char* line_cstr = furi_string_get_cstr(line);

    if(line_len == 0) {
        return SCRIPT_STATE_NEXT_LINE; // Skip empty lines
    }
    FURI_LOG_D(WORKER_TAG, "line:%s", line_cstr);

    // Ducky Lang Functions
    int32_t cmd_result = ducky_execute_cmd(open_usb, line_cstr);
    if(cmd_result != SCRIPT_STATE_CMD_UNKNOWN) {
        return cmd_result;
    }

    // Mouse Keys
    uint16_t key = ducky_get_mouse_keycode_by_name(line_cstr);
    if(key != HID_MOUSE_INVALID) {
        open_usb->hid->mouse_press(open_usb->hid_inst, key);
        open_usb->hid->mouse_release(open_usb->hid_inst, key);
        return 0;
    }

    // Parse chain of modifiers linked by spaces and hyphens
    uint16_t modifiers = 0;
    while(1) {
        key = ducky_get_next_modifier_keycode_by_name(&line_cstr);
        if(key == HID_KEYBOARD_NONE) break;

        modifiers |= key;
        char next_char = *line_cstr;
        if(next_char == ' ' || next_char == '-') line_cstr++;
    }

    // Main key
    char next_char = *line_cstr;
    key = modifiers | ducky_get_keycode(open_usb, line_cstr, false);

    if(key == 0 && next_char) ducky_error(open_usb, "No keycode defined for %s", line_cstr);

    open_usb->hid->kb_press(open_usb->hid_inst, key);
    open_usb->hid->kb_release(open_usb->hid_inst, key);
    return 0;
}

static bool ducky_set_usb_id(OpenUsbScript* open_usb, const char* line) {
    if(sscanf(line, "%lX:%lX", &open_usb->hid_cfg.vid, &open_usb->hid_cfg.pid) == 2) {
        open_usb->hid_cfg.manuf[0] = '\0';
        open_usb->hid_cfg.product[0] = '\0';

        uint8_t id_len = ducky_get_command_len(line);
        if(!ducky_is_line_end(line[id_len + 1])) {
            sscanf(
                &line[id_len + 1],
                "%31[^\r\n:]:%31[^\r\n]",
                open_usb->hid_cfg.manuf,
                open_usb->hid_cfg.product);
        }
        FURI_LOG_D(
            WORKER_TAG,
            "set id: %04lX:%04lX mfr:%s product:%s",
            open_usb->hid_cfg.vid,
            open_usb->hid_cfg.pid,
            open_usb->hid_cfg.manuf,
            open_usb->hid_cfg.product);
        return true;
    }
    return false;
}

static void open_usb_hid_state_callback(bool state, void* context) {
    furi_assert(context);
    OpenUsbScript* open_usb = context;

    if(state == true) {
        furi_thread_flags_set(furi_thread_get_id(open_usb->thread), WorkerEvtConnect);
    } else {
        furi_thread_flags_set(furi_thread_get_id(open_usb->thread), WorkerEvtDisconnect);
    }
}

static bool ducky_script_preload(OpenUsbScript* open_usb, File* script_file) {
    uint8_t ret = 0;
    uint32_t line_len = 0;

    furi_string_reset(open_usb->line);

    do {
        ret = storage_file_read(script_file, open_usb->file_buf, FILE_BUFFER_LEN);
        for(uint16_t i = 0; i < ret; i++) {
            if(open_usb->file_buf[i] == '\n' && line_len > 0) {
                open_usb->st.line_nb++;
                line_len = 0;
            } else {
                if(open_usb->st.line_nb == 0) { // Save first line
                    furi_string_push_back(open_usb->line, open_usb->file_buf[i]);
                }
                line_len++;
            }
        }
        if(storage_file_eof(script_file)) {
            if(line_len > 0) {
                open_usb->st.line_nb++;
                break;
            }
        }
    } while(ret > 0);

    const char* line_tmp = furi_string_get_cstr(open_usb->line);
    bool id_set = false; // Looking for ID command at first line
    if(strncmp(line_tmp, ducky_cmd_id, strlen(ducky_cmd_id)) == 0) {
        id_set = ducky_set_usb_id(open_usb, &line_tmp[strlen(ducky_cmd_id) + 1]);
    }

    if(id_set) {
        open_usb->hid_inst = open_usb->hid->init(&open_usb->hid_cfg);
    } else {
        open_usb->hid_inst = open_usb->hid->init(NULL);
    }
    open_usb->hid->set_state_callback(open_usb->hid_inst, open_usb_hid_state_callback, open_usb);

    storage_file_seek(script_file, 0, true);
    furi_string_reset(open_usb->line);

    return true;
}

static int32_t ducky_script_execute_next(OpenUsbScript* open_usb, File* script_file) {
    int32_t delay_val = 0;

    if(open_usb->repeat_cnt > 0) {
        open_usb->repeat_cnt--;
        delay_val = ducky_parse_line(open_usb, open_usb->line_prev);
        if(delay_val == SCRIPT_STATE_NEXT_LINE) { // Empty line
            return 0;
        } else if(delay_val == SCRIPT_STATE_STRING_START) { // Print string with delays
            return delay_val;
        } else if(delay_val == SCRIPT_STATE_WAIT_FOR_BTN) { // wait for button
            return delay_val;
        } else if(delay_val < 0) { // Script error
            open_usb->st.error_line = open_usb->st.line_cur - 1;
            FURI_LOG_E(WORKER_TAG, "Unknown command at line %zu", open_usb->st.line_cur - 1U);
            return SCRIPT_STATE_ERROR;
        } else {
            return delay_val + open_usb->defdelay;
        }
    }

    furi_string_set(open_usb->line_prev, open_usb->line);
    furi_string_reset(open_usb->line);

    while(1) {
        if(open_usb->buf_len == 0) {
            open_usb->buf_len = storage_file_read(script_file, open_usb->file_buf, FILE_BUFFER_LEN);
            if(storage_file_eof(script_file)) {
                if((open_usb->buf_len < FILE_BUFFER_LEN) && (open_usb->file_end == false)) {
                    open_usb->file_buf[open_usb->buf_len] = '\n';
                    open_usb->buf_len++;
                    open_usb->file_end = true;
                }
            }

            open_usb->buf_start = 0;
            if(open_usb->buf_len == 0) return SCRIPT_STATE_END;
        }
        for(uint8_t i = open_usb->buf_start; i < (open_usb->buf_start + open_usb->buf_len); i++) {
            if(open_usb->file_buf[i] == '\n' && furi_string_size(open_usb->line) > 0) {
                open_usb->st.line_cur++;
                open_usb->buf_len = open_usb->buf_len + open_usb->buf_start - (i + 1);
                open_usb->buf_start = i + 1;
                furi_string_trim(open_usb->line);
                delay_val = ducky_parse_line(open_usb, open_usb->line);
                if(delay_val == SCRIPT_STATE_NEXT_LINE) { // Empty line
                    return 0;
                } else if(delay_val == SCRIPT_STATE_STRING_START) { // Print string with delays
                    return delay_val;
                } else if(delay_val == SCRIPT_STATE_WAIT_FOR_BTN) { // wait for button
                    return delay_val;
                } else if(delay_val < 0) {
                    open_usb->st.error_line = open_usb->st.line_cur;
                    FURI_LOG_E(WORKER_TAG, "Unknown command at line %zu", open_usb->st.line_cur);
                    return SCRIPT_STATE_ERROR;
                } else {
                    return delay_val + open_usb->defdelay;
                }
            } else {
                furi_string_push_back(open_usb->line, open_usb->file_buf[i]);
            }
        }
        open_usb->buf_len = 0;
        if(open_usb->file_end) return SCRIPT_STATE_END;
    }

    return 0;
}

static uint32_t open_usb_flags_get(uint32_t flags_mask, uint32_t timeout) {
    uint32_t flags = furi_thread_flags_get();
    furi_check((flags & FuriFlagError) == 0);
    if(flags == 0) {
        flags = furi_thread_flags_wait(flags_mask, FuriFlagWaitAny, timeout);
        furi_check(((flags & FuriFlagError) == 0) || (flags == (unsigned)FuriFlagErrorTimeout));
    } else {
        uint32_t state = furi_thread_flags_clear(flags);
        furi_check((state & FuriFlagError) == 0);
    }
    return flags;
}

static int32_t open_usb_worker(void* context) {
    OpenUsbScript* open_usb = context;

    OpenUsbWorkerState worker_state = OpenUsbStateInit;
    OpenUsbWorkerState pause_state = OpenUsbStateRunning;
    int32_t delay_val = 0;

    FURI_LOG_I(WORKER_TAG, "Init");
    File* script_file = storage_file_alloc(furi_record_open(RECORD_STORAGE));
    open_usb->line = furi_string_alloc();
    open_usb->line_prev = furi_string_alloc();
    open_usb->string_print = furi_string_alloc();

    while(1) {
        if(worker_state == OpenUsbStateInit) { // State: initialization
            if(storage_file_open(
                   script_file,
                   furi_string_get_cstr(open_usb->file_path),
                   FSAM_READ,
                   FSOM_OPEN_EXISTING)) {
                if((ducky_script_preload(open_usb, script_file)) && (open_usb->st.line_nb > 0)) {
                    if(open_usb->hid->is_connected(open_usb->hid_inst)) {
                        worker_state = OpenUsbStateIdle; // Ready to run
                    } else {
                        worker_state = OpenUsbStateNotConnected; // USB not connected
                    }
                } else {
                    worker_state = OpenUsbStateScriptError; // Script preload error
                }
            } else {
                FURI_LOG_E(WORKER_TAG, "File open error");
                worker_state = OpenUsbStateFileError; // File open error
            }
            open_usb->st.state = worker_state;

        } else if(worker_state == OpenUsbStateNotConnected) { // State: USB not connected
            uint32_t flags = open_usb_flags_get(
                WorkerEvtEnd | WorkerEvtConnect | WorkerEvtDisconnect | WorkerEvtStartStop,
                FuriWaitForever);

            if(flags & WorkerEvtEnd) {
                break;
            } else if(flags & WorkerEvtConnect) {
                worker_state = OpenUsbStateIdle; // Ready to run
            } else if(flags & WorkerEvtStartStop) {
                worker_state = OpenUsbStateWillRun; // Will run when USB is connected
            }
            open_usb->st.state = worker_state;

        } else if(worker_state == OpenUsbStateIdle) { // State: ready to start
            uint32_t flags = open_usb_flags_get(
                WorkerEvtEnd | WorkerEvtStartStop | WorkerEvtDisconnect, FuriWaitForever);

            if(flags & WorkerEvtEnd) {
                break;
            } else if(flags & WorkerEvtStartStop) { // Start executing script
                dolphin_deed(DolphinDeedOpenUsbPlayScript);
                delay_val = 0;
                open_usb->buf_len = 0;
                open_usb->st.line_cur = 0;
                open_usb->defdelay = 0;
                open_usb->stringdelay = 0;
                open_usb->defstringdelay = 0;
                open_usb->repeat_cnt = 0;
                open_usb->key_hold_nb = 0;
                open_usb->file_end = false;
                storage_file_seek(script_file, 0, true);
                worker_state = OpenUsbStateRunning;
            } else if(flags & WorkerEvtDisconnect) {
                worker_state = OpenUsbStateNotConnected; // USB disconnected
            }
            open_usb->st.state = worker_state;

        } else if(worker_state == OpenUsbStateWillRun) { // State: start on connection
            uint32_t flags = open_usb_flags_get(
                WorkerEvtEnd | WorkerEvtConnect | WorkerEvtStartStop, FuriWaitForever);

            if(flags & WorkerEvtEnd) {
                break;
            } else if(flags & WorkerEvtConnect) { // Start executing script
                dolphin_deed(DolphinDeedOpenUsbPlayScript);
                delay_val = 0;
                open_usb->buf_len = 0;
                open_usb->st.line_cur = 0;
                open_usb->defdelay = 0;
                open_usb->stringdelay = 0;
                open_usb->defstringdelay = 0;
                open_usb->repeat_cnt = 0;
                open_usb->file_end = false;
                storage_file_seek(script_file, 0, true);
                // extra time for PC to recognize Flipper as keyboard
                flags = furi_thread_flags_wait(
                    WorkerEvtEnd | WorkerEvtDisconnect | WorkerEvtStartStop,
                    FuriFlagWaitAny | FuriFlagNoClear,
                    1500);
                if(flags == (unsigned)FuriFlagErrorTimeout) {
                    // If nothing happened - start script execution
                    worker_state = OpenUsbStateRunning;
                } else if(flags & WorkerEvtStartStop) {
                    worker_state = OpenUsbStateIdle;
                    furi_thread_flags_clear(WorkerEvtStartStop);
                }
            } else if(flags & WorkerEvtStartStop) { // Cancel scheduled execution
                worker_state = OpenUsbStateNotConnected;
            }
            open_usb->st.state = worker_state;

        } else if(worker_state == OpenUsbStateRunning) { // State: running
            uint16_t delay_cur = (delay_val > 1000) ? (1000) : (delay_val);
            uint32_t flags = furi_thread_flags_wait(
                WorkerEvtEnd | WorkerEvtStartStop | WorkerEvtPauseResume | WorkerEvtDisconnect,
                FuriFlagWaitAny,
                delay_cur);

            delay_val -= delay_cur;
            if(!(flags & FuriFlagError)) {
                if(flags & WorkerEvtEnd) {
                    break;
                } else if(flags & WorkerEvtStartStop) {
                    worker_state = OpenUsbStateIdle; // Stop executing script
                    open_usb->hid->release_all(open_usb->hid_inst);
                } else if(flags & WorkerEvtDisconnect) {
                    worker_state = OpenUsbStateNotConnected; // USB disconnected
                    open_usb->hid->release_all(open_usb->hid_inst);
                } else if(flags & WorkerEvtPauseResume) {
                    pause_state = OpenUsbStateRunning;
                    worker_state = OpenUsbStatePaused; // Pause
                }
                open_usb->st.state = worker_state;
                continue;
            } else if(
                (flags == (unsigned)FuriFlagErrorTimeout) ||
                (flags == (unsigned)FuriFlagErrorResource)) {
                if(delay_val > 0) {
                    open_usb->st.delay_remain--;
                    continue;
                }
                open_usb->st.state = OpenUsbStateRunning;
                delay_val = ducky_script_execute_next(open_usb, script_file);
                if(delay_val == SCRIPT_STATE_ERROR) { // Script error
                    delay_val = 0;
                    worker_state = OpenUsbStateScriptError;
                    open_usb->st.state = worker_state;
                    open_usb->hid->release_all(open_usb->hid_inst);
                } else if(delay_val == SCRIPT_STATE_END) { // End of script
                    delay_val = 0;
                    worker_state = OpenUsbStateIdle;
                    open_usb->st.state = OpenUsbStateDone;
                    open_usb->hid->release_all(open_usb->hid_inst);
                    continue;
                } else if(delay_val == SCRIPT_STATE_STRING_START) { // Start printing string with delays
                    delay_val = open_usb->defdelay;
                    open_usb->string_print_pos = 0;
                    worker_state = OpenUsbStateStringDelay;
                } else if(delay_val == SCRIPT_STATE_WAIT_FOR_BTN) { // set state to wait for user input
                    worker_state = OpenUsbStateWaitForBtn;
                    open_usb->st.state = OpenUsbStateWaitForBtn; // Show long delays
                } else if(delay_val > 1000) {
                    open_usb->st.state = OpenUsbStateDelay; // Show long delays
                    open_usb->st.delay_remain = delay_val / 1000;
                }
            } else {
                furi_check((flags & FuriFlagError) == 0);
            }
        } else if(worker_state == OpenUsbStateWaitForBtn) { // State: Wait for button Press
            uint32_t flags = open_usb_flags_get(
                WorkerEvtEnd | WorkerEvtStartStop | WorkerEvtPauseResume | WorkerEvtDisconnect,
                FuriWaitForever);
            if(!(flags & FuriFlagError)) {
                if(flags & WorkerEvtEnd) {
                    break;
                } else if(flags & WorkerEvtStartStop) {
                    delay_val = 0;
                    worker_state = OpenUsbStateRunning;
                } else if(flags & WorkerEvtDisconnect) {
                    worker_state = OpenUsbStateNotConnected; // USB disconnected
                    open_usb->hid->release_all(open_usb->hid_inst);
                }
                open_usb->st.state = worker_state;
                continue;
            }
        } else if(worker_state == OpenUsbStatePaused) { // State: Paused
            uint32_t flags = open_usb_flags_get(
                WorkerEvtEnd | WorkerEvtStartStop | WorkerEvtPauseResume | WorkerEvtDisconnect,
                FuriWaitForever);
            if(!(flags & FuriFlagError)) {
                if(flags & WorkerEvtEnd) {
                    break;
                } else if(flags & WorkerEvtStartStop) {
                    worker_state = OpenUsbStateIdle; // Stop executing script
                    open_usb->st.state = worker_state;
                    open_usb->hid->release_all(open_usb->hid_inst);
                } else if(flags & WorkerEvtDisconnect) {
                    worker_state = OpenUsbStateNotConnected; // USB disconnected
                    open_usb->st.state = worker_state;
                    open_usb->hid->release_all(open_usb->hid_inst);
                } else if(flags & WorkerEvtPauseResume) {
                    if(pause_state == OpenUsbStateRunning) {
                        if(delay_val > 0) {
                            open_usb->st.state = OpenUsbStateDelay;
                            open_usb->st.delay_remain = delay_val / 1000;
                        } else {
                            open_usb->st.state = OpenUsbStateRunning;
                            delay_val = 0;
                        }
                        worker_state = OpenUsbStateRunning; // Resume
                    } else if(pause_state == OpenUsbStateStringDelay) {
                        open_usb->st.state = OpenUsbStateRunning;
                        worker_state = OpenUsbStateStringDelay; // Resume
                    }
                }
                continue;
            }
        } else if(worker_state == OpenUsbStateStringDelay) { // State: print string with delays
            uint32_t delay = (open_usb->stringdelay == 0) ? open_usb->defstringdelay :
                                                           open_usb->stringdelay;
            uint32_t flags = open_usb_flags_get(
                WorkerEvtEnd | WorkerEvtStartStop | WorkerEvtPauseResume | WorkerEvtDisconnect,
                delay);

            if(!(flags & FuriFlagError)) {
                if(flags & WorkerEvtEnd) {
                    break;
                } else if(flags & WorkerEvtStartStop) {
                    worker_state = OpenUsbStateIdle; // Stop executing script
                    open_usb->hid->release_all(open_usb->hid_inst);
                } else if(flags & WorkerEvtDisconnect) {
                    worker_state = OpenUsbStateNotConnected; // USB disconnected
                    open_usb->hid->release_all(open_usb->hid_inst);
                } else if(flags & WorkerEvtPauseResume) {
                    pause_state = OpenUsbStateStringDelay;
                    worker_state = OpenUsbStatePaused; // Pause
                }
                open_usb->st.state = worker_state;
                continue;
            } else if(
                (flags == (unsigned)FuriFlagErrorTimeout) ||
                (flags == (unsigned)FuriFlagErrorResource)) {
                bool string_end = ducky_string_next(open_usb);
                if(string_end) {
                    open_usb->stringdelay = 0;
                    worker_state = OpenUsbStateRunning;
                }
            } else {
                furi_check((flags & FuriFlagError) == 0);
            }
        } else if(
            (worker_state == OpenUsbStateFileError) ||
            (worker_state == OpenUsbStateScriptError)) { // State: error
            uint32_t flags =
                open_usb_flags_get(WorkerEvtEnd, FuriWaitForever); // Waiting for exit command

            if(flags & WorkerEvtEnd) {
                break;
            }
        }
    }

    open_usb->hid->set_state_callback(open_usb->hid_inst, NULL, NULL);
    open_usb->hid->deinit(open_usb->hid_inst);

    storage_file_close(script_file);
    storage_file_free(script_file);
    furi_string_free(open_usb->line);
    furi_string_free(open_usb->line_prev);
    furi_string_free(open_usb->string_print);

    FURI_LOG_I(WORKER_TAG, "End");

    return 0;
}

static void open_usb_script_set_default_keyboard_layout(OpenUsbScript* open_usb) {
    furi_assert(open_usb);
    memset(open_usb->layout, HID_KEYBOARD_NONE, sizeof(open_usb->layout));
    memcpy(open_usb->layout, hid_asciimap, MIN(sizeof(hid_asciimap), sizeof(open_usb->layout)));
}

OpenUsbScript* open_usb_script_open(FuriString* file_path, OpenUsbHidInterface interface) {
    furi_assert(file_path);

    OpenUsbScript* open_usb = malloc(sizeof(OpenUsbScript));
    open_usb->file_path = furi_string_alloc();
    furi_string_set(open_usb->file_path, file_path);
    open_usb_script_set_default_keyboard_layout(open_usb);

    open_usb->st.state = OpenUsbStateInit;
    open_usb->st.error[0] = '\0';
    open_usb->hid = open_usb_hid_get_interface(interface);

    open_usb->thread = furi_thread_alloc_ex("OpenUsbWorker", 2048, open_usb_worker, open_usb);
    furi_thread_start(open_usb->thread);
    return open_usb;
} //-V773

void open_usb_script_close(OpenUsbScript* open_usb) {
    furi_assert(open_usb);
    furi_thread_flags_set(furi_thread_get_id(open_usb->thread), WorkerEvtEnd);
    furi_thread_join(open_usb->thread);
    furi_thread_free(open_usb->thread);
    furi_string_free(open_usb->file_path);
    free(open_usb);
}

void open_usb_script_set_keyboard_layout(OpenUsbScript* open_usb, FuriString* layout_path) {
    furi_assert(open_usb);

    if((open_usb->st.state == OpenUsbStateRunning) || (open_usb->st.state == OpenUsbStateDelay)) {
        // do not update keyboard layout while a script is running
        return;
    }

    File* layout_file = storage_file_alloc(furi_record_open(RECORD_STORAGE));
    if(!furi_string_empty(layout_path)) { //-V1051
        if(storage_file_open(
               layout_file, furi_string_get_cstr(layout_path), FSAM_READ, FSOM_OPEN_EXISTING)) {
            uint16_t layout[128];
            if(storage_file_read(layout_file, layout, sizeof(layout)) == sizeof(layout)) {
                memcpy(open_usb->layout, layout, sizeof(layout));
            }
        }
        storage_file_close(layout_file);
    } else {
        open_usb_script_set_default_keyboard_layout(open_usb);
    }
    storage_file_free(layout_file);
}

void open_usb_script_start_stop(OpenUsbScript* open_usb) {
    furi_assert(open_usb);
    furi_thread_flags_set(furi_thread_get_id(open_usb->thread), WorkerEvtStartStop);
}

void open_usb_script_pause_resume(OpenUsbScript* open_usb) {
    furi_assert(open_usb);
    furi_thread_flags_set(furi_thread_get_id(open_usb->thread), WorkerEvtPauseResume);
}

OpenUsbState* open_usb_script_get_state(OpenUsbScript* open_usb) {
    furi_assert(open_usb);
    return &(open_usb->st);
}
