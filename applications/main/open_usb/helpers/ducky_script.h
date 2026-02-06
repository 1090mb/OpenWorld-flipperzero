#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <furi.h>
#include <furi_hal.h>
#include "open_usb_hid.h"

typedef enum {
    OpenUsbStateInit,
    OpenUsbStateNotConnected,
    OpenUsbStateIdle,
    OpenUsbStateWillRun,
    OpenUsbStateRunning,
    OpenUsbStateDelay,
    OpenUsbStateStringDelay,
    OpenUsbStateWaitForBtn,
    OpenUsbStatePaused,
    OpenUsbStateDone,
    OpenUsbStateScriptError,
    OpenUsbStateFileError,
} OpenUsbWorkerState;

typedef struct {
    OpenUsbWorkerState state;
    size_t line_cur;
    size_t line_nb;
    uint32_t delay_remain;
    size_t error_line;
    char error[64];
} OpenUsbState;

typedef struct OpenUsbScript OpenUsbScript;

OpenUsbScript* open_usb_script_open(FuriString* file_path, OpenUsbHidInterface interface);

void open_usb_script_close(OpenUsbScript* open_usb);

void open_usb_script_set_keyboard_layout(OpenUsbScript* open_usb, FuriString* layout_path);

void open_usb_script_start(OpenUsbScript* open_usb);

void open_usb_script_stop(OpenUsbScript* open_usb);

void open_usb_script_start_stop(OpenUsbScript* open_usb);

void open_usb_script_pause_resume(OpenUsbScript* open_usb);

OpenUsbState* open_usb_script_get_state(OpenUsbScript* open_usb);

#ifdef __cplusplus
}
#endif
