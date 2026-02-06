#include "open_usb_view.h"
#include "../helpers/ducky_script.h"
#include <toolbox/path.h>
#include <gui/elements.h>
#include <assets_icons.h>

#define MAX_NAME_LEN 64

struct OpenUsb {
    View* view;
    OpenUsbButtonCallback callback;
    void* context;
};

typedef struct {
    char file_name[MAX_NAME_LEN];
    char layout[MAX_NAME_LEN];
    OpenUsbState state;
    bool pause_wait;
    uint8_t anim_frame;
    OpenUsbHidInterface interface;
} OpenUsbModel;

static void open_usb_draw_callback(Canvas* canvas, void* _model) {
    OpenUsbModel* model = _model;

    FuriString* disp_str;
    disp_str = furi_string_alloc_set(model->file_name);
    elements_string_fit_width(canvas, disp_str, 128 - 2);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 8, furi_string_get_cstr(disp_str));

    if(strlen(model->layout) == 0) {
        furi_string_set(disp_str, "(default)");
    } else {
        furi_string_printf(disp_str, "(%s)", model->layout);
    }
    elements_string_fit_width(canvas, disp_str, 128 - 2);
    canvas_draw_str(
        canvas, 2, 8 + canvas_current_font_height(canvas), furi_string_get_cstr(disp_str));

    furi_string_reset(disp_str);

    if(model->interface == OpenUsbHidInterfaceBle) {
        canvas_draw_icon(canvas, 22, 24, &I_Bad_BLE_48x22);
    } else {
        canvas_draw_icon(canvas, 22, 24, &I_UsbTree_48x22);
    }

    OpenUsbWorkerState state = model->state.state;

    if((state == OpenUsbStateIdle) || (state == OpenUsbStateDone) ||
       (state == OpenUsbStateNotConnected)) {
        elements_button_center(canvas, "Run");
        if(model->interface == OpenUsbHidInterfaceBle) {
            elements_button_right(canvas, "USB");
            elements_button_left(canvas, "Config");
        } else {
            elements_button_right(canvas, "BLE");
            elements_button_left(canvas, "Layout");
        }
    } else if((state == OpenUsbStateRunning) || (state == OpenUsbStateDelay)) {
        elements_button_center(canvas, "Stop");
        if(!model->pause_wait) {
            elements_button_right(canvas, "Pause");
        }
    } else if(state == OpenUsbStatePaused) {
        elements_button_center(canvas, "End");
        elements_button_right(canvas, "Resume");
    } else if(state == OpenUsbStateWaitForBtn) {
        elements_button_center(canvas, "Press to continue");
    } else if(state == OpenUsbStateWillRun) {
        elements_button_center(canvas, "Cancel");
    }

    if(state == OpenUsbStateNotConnected) {
        canvas_draw_icon(canvas, 4, 26, &I_Clock_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 31, AlignRight, AlignBottom, "Connect");
        canvas_draw_str_aligned(canvas, 127, 43, AlignRight, AlignBottom, "to device");
    } else if(state == OpenUsbStateWillRun) {
        canvas_draw_icon(canvas, 4, 26, &I_Clock_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 31, AlignRight, AlignBottom, "Will run");
        canvas_draw_str_aligned(canvas, 127, 43, AlignRight, AlignBottom, "on connect");
    } else if(state == OpenUsbStateFileError) {
        canvas_draw_icon(canvas, 4, 26, &I_Error_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 31, AlignRight, AlignBottom, "File");
        canvas_draw_str_aligned(canvas, 127, 43, AlignRight, AlignBottom, "ERROR");
    } else if(state == OpenUsbStateScriptError) {
        canvas_draw_icon(canvas, 4, 26, &I_Error_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 33, AlignRight, AlignBottom, "ERROR:");
        canvas_set_font(canvas, FontSecondary);
        furi_string_printf(disp_str, "line %zu", model->state.error_line);
        canvas_draw_str_aligned(
            canvas, 127, 46, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);

        furi_string_set_str(disp_str, model->state.error);
        elements_string_fit_width(canvas, disp_str, canvas_width(canvas));
        canvas_draw_str_aligned(
            canvas, 127, 56, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
    } else if(state == OpenUsbStateIdle) {
        canvas_draw_icon(canvas, 4, 26, &I_Smile_18x18);
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, 114, 40, AlignRight, AlignBottom, "0");
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
    } else if(state == OpenUsbStateRunning) {
        if(model->anim_frame == 0) {
            canvas_draw_icon(canvas, 4, 23, &I_EviSmile1_18x21);
        } else {
            canvas_draw_icon(canvas, 4, 23, &I_EviSmile2_18x21);
        }
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(
            disp_str, "%zu", ((model->state.line_cur - 1) * 100) / model->state.line_nb);
        canvas_draw_str_aligned(
            canvas, 114, 40, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
    } else if(state == OpenUsbStateDone) {
        canvas_draw_icon(canvas, 4, 23, &I_EviSmile1_18x21);
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, 114, 40, AlignRight, AlignBottom, "100");
        furi_string_reset(disp_str);
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
    } else if(state == OpenUsbStateDelay) {
        if(model->anim_frame == 0) {
            canvas_draw_icon(canvas, 4, 23, &I_EviWaiting1_18x21);
        } else {
            canvas_draw_icon(canvas, 4, 23, &I_EviWaiting2_18x21);
        }
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(
            disp_str, "%zu", ((model->state.line_cur - 1) * 100) / model->state.line_nb);
        canvas_draw_str_aligned(
            canvas, 114, 40, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
        canvas_set_font(canvas, FontSecondary);
        furi_string_printf(disp_str, "delay %lus", model->state.delay_remain);
        canvas_draw_str_aligned(
            canvas, 127, 50, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
    } else if((state == OpenUsbStatePaused) || (state == OpenUsbStateWaitForBtn)) {
        if(model->anim_frame == 0) {
            canvas_draw_icon(canvas, 4, 23, &I_EviWaiting1_18x21);
        } else {
            canvas_draw_icon(canvas, 4, 23, &I_EviWaiting2_18x21);
        }
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(
            disp_str, "%zu", ((model->state.line_cur - 1) * 100) / model->state.line_nb);
        canvas_draw_str_aligned(
            canvas, 114, 40, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 127, 50, AlignRight, AlignBottom, "Paused");
        furi_string_reset(disp_str);
    } else {
        canvas_draw_icon(canvas, 4, 26, &I_Clock_18x18);
    }

    furi_string_free(disp_str);
}

static bool open_usb_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    OpenUsb* open_usb = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyLeft) {
            consumed = true;
            furi_assert(open_usb->callback);
            open_usb->callback(event->key, open_usb->context);
        } else if(event->key == InputKeyOk) {
            with_view_model(
                open_usb->view, OpenUsbModel * model, { model->pause_wait = false; }, true);
            consumed = true;
            furi_assert(open_usb->callback);
            open_usb->callback(event->key, open_usb->context);
        } else if(event->key == InputKeyRight) {
            with_view_model(
                open_usb->view,
                OpenUsbModel * model,
                {
                    if((model->state.state == OpenUsbStateRunning) ||
                       (model->state.state == OpenUsbStateDelay)) {
                        model->pause_wait = true;
                    }
                },
                true);
            consumed = true;
            furi_assert(open_usb->callback);
            open_usb->callback(event->key, open_usb->context);
        }
    }

    return consumed;
}

OpenUsb* open_usb_view_alloc(void) {
    OpenUsb* open_usb = malloc(sizeof(OpenUsb));

    open_usb->view = view_alloc();
    view_allocate_model(open_usb->view, ViewModelTypeLocking, sizeof(OpenUsbModel));
    view_set_context(open_usb->view, open_usb);
    view_set_draw_callback(open_usb->view, open_usb_draw_callback);
    view_set_input_callback(open_usb->view, open_usb_input_callback);

    return open_usb;
}

void open_usb_view_free(OpenUsb* open_usb) {
    furi_assert(open_usb);
    view_free(open_usb->view);
    free(open_usb);
}

View* open_usb_view_get_view(OpenUsb* open_usb) {
    furi_assert(open_usb);
    return open_usb->view;
}

void open_usb_view_set_button_callback(
    OpenUsb* open_usb,
    OpenUsbButtonCallback callback,
    void* context) {
    furi_assert(open_usb);
    furi_assert(callback);
    with_view_model(
        open_usb->view,
        OpenUsbModel * model,
        {
            UNUSED(model);
            open_usb->callback = callback;
            open_usb->context = context;
        },
        true);
}

void open_usb_view_set_file_name(OpenUsb* open_usb, const char* name) {
    furi_assert(name);
    with_view_model(
        open_usb->view,
        OpenUsbModel * model,
        { strlcpy(model->file_name, name, MAX_NAME_LEN); },
        true);
}

void open_usb_view_set_layout(OpenUsb* open_usb, const char* layout) {
    furi_assert(layout);
    with_view_model(
        open_usb->view,
        OpenUsbModel * model,
        { strlcpy(model->layout, layout, MAX_NAME_LEN); },
        true);
}

void open_usb_view_set_state(OpenUsb* open_usb, OpenUsbState* st) {
    furi_assert(st);
    with_view_model(
        open_usb->view,
        OpenUsbModel * model,
        {
            memcpy(&(model->state), st, sizeof(OpenUsbState));
            model->anim_frame ^= 1;
            if(model->state.state == OpenUsbStatePaused) {
                model->pause_wait = false;
            }
        },
        true);
}

void open_usb_view_set_interface(OpenUsb* open_usb, OpenUsbHidInterface interface) {
    with_view_model(open_usb->view, OpenUsbModel * model, { model->interface = interface; }, true);
}

bool open_usb_view_is_idle_state(OpenUsb* open_usb) {
    bool is_idle = false;
    with_view_model(
        open_usb->view,
        OpenUsbModel * model,
        {
            if((model->state.state == OpenUsbStateIdle) ||
               (model->state.state == OpenUsbStateDone) ||
               (model->state.state == OpenUsbStateNotConnected)) {
                is_idle = true;
            }
        },
        false);
    return is_idle;
}
