#pragma once

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/variable_item_list.h>
#include <furi_hal_region.h>
#include <furi_hal_rtc.h>

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    VariableItemList* var_item_list;
} OpenSettings;

typedef enum {
    OpenSettingsViewVarItemList,
} OpenSettingsView;
