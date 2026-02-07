/**
 * @file ai_tools.c
 * @brief AI Tools - Interactive offline AI classification app
 * 
 * This app provides a user-friendly interface to interact with AI models:
 * - Manual input for decision tree classification
 * - Real-time pattern recognition
 * - Interactive model selection
 */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <gui/elements.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include "ai_classifier.h"

#define TAG "AITools"

typedef enum {
    AIToolsModeInput,      // Manual feature input
    AIToolsModeClassify,   // Show classification result
    AIToolsModePattern,    // Pattern matching mode
} AIToolsMode;

typedef struct {
    ViewPort* view_port;
    Gui* gui;
    FuriMessageQueue* event_queue;
    NotificationApp* notifications;
    
    // AI Models
    AIDecisionTree* decision_tree;
    AITemplateMatcher* template_matcher;
    
    // State
    AIToolsMode mode;
    uint8_t selected_model;  // 0 = decision tree, 1 = template match
    
    // Input features for decision tree
    float feature_values[2];
    uint8_t current_feature;  // Which feature is being edited
    
    // Classification result
    AIClassifierResult last_result;
    bool has_result;
    
    // Pattern generation for template matching
    uint32_t pattern_tick;
    uint8_t pattern_type;  // 0 = sine low, 1 = sine high, 2 = square
    
    bool running;
} AIToolsApp;

// Initialize a simple decision tree for classification
static bool init_decision_tree(AIToolsApp* app) {
    // Create tree with 7 nodes, 3 classes, 2 features
    app->decision_tree = ai_decision_tree_create(7, 3, 2);
    if(!app->decision_tree) return false;
    
    AIDecisionNode* nodes = app->decision_tree->nodes;
    
    // Build decision tree for classifying based on two features
    // Feature 0: Signal strength (0-100)
    // Feature 1: Frequency (0-100)
    
    // Node 0: Root - test feature 0
    nodes[0].feature_idx = 0;
    nodes[0].threshold = ai_float_to_fixed(50.0f);
    nodes[0].left_child = 1;
    nodes[0].right_child = 2;
    
    // Node 1: Low signal - class 0 (Weak)
    nodes[1].feature_idx = -1;
    nodes[1].class_id = 0;
    
    // Node 2: High signal - test feature 1
    nodes[2].feature_idx = 1;
    nodes[2].threshold = ai_float_to_fixed(70.0f);
    nodes[2].left_child = 3;
    nodes[2].right_child = 4;
    
    // Node 3: Medium frequency - class 1 (Medium)
    nodes[3].feature_idx = -1;
    nodes[3].class_id = 1;
    
    // Node 4: Test feature 0 again
    nodes[4].feature_idx = 0;
    nodes[4].threshold = ai_float_to_fixed(80.0f);
    nodes[4].left_child = 5;
    nodes[4].right_child = 6;
    
    // Node 5: Medium-high signal - class 1 (Medium)
    nodes[5].feature_idx = -1;
    nodes[5].class_id = 1;
    
    // Node 6: Very high signal - class 2 (Strong)
    nodes[6].feature_idx = -1;
    nodes[6].class_id = 2;
    
    return true;
}

// Initialize template matcher with signal patterns
static bool init_template_matcher(AIToolsApp* app) {
    app->template_matcher = ai_template_matcher_create(3, 3);
    if(!app->template_matcher) return false;
    
    // Template 0: Low frequency sine wave
    int32_t pattern0[16];
    for(int i = 0; i < 16; i++) {
        pattern0[i] = ai_float_to_fixed(50.0f * sinf(i * 3.14159f / 4.0f));
    }
    ai_template_matcher_add(app->template_matcher, 0, pattern0, 16, 0);
    
    // Template 1: High frequency sine wave
    int32_t pattern1[16];
    for(int i = 0; i < 16; i++) {
        pattern1[i] = ai_float_to_fixed(50.0f * sinf(i * 3.14159f / 2.0f));
    }
    ai_template_matcher_add(app->template_matcher, 1, pattern1, 16, 1);
    
    // Template 2: Square wave
    int32_t pattern2[16];
    for(int i = 0; i < 16; i++) {
        pattern2[i] = ai_float_to_fixed((i % 8 < 4) ? 50.0f : -50.0f);
    }
    ai_template_matcher_add(app->template_matcher, 2, pattern2, 16, 2);
    
    return true;
}

static void draw_callback(Canvas* canvas, void* ctx) {
    AIToolsApp* app = ctx;
    furi_assert(app);
    
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    
    // Title
    canvas_draw_str(canvas, 2, 10, "AI Tools");
    
    // Model selection
    canvas_set_font(canvas, FontSecondary);
    const char* model_str = (app->selected_model == 0) ? "Decision Tree" : "Template Match";
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Model: %s", model_str);
    canvas_draw_str(canvas, 2, 22, buffer);
    
    if(app->selected_model == 0) {
        // Decision Tree Mode
        canvas_draw_str(canvas, 2, 32, "Input Features:");
        
        // Feature 0
        snprintf(buffer, sizeof(buffer), "%c F0: %.1f", 
                 (app->current_feature == 0) ? '>' : ' ',
                 (double)app->feature_values[0]);
        canvas_draw_str(canvas, 2, 42, buffer);
        
        // Feature 1
        snprintf(buffer, sizeof(buffer), "%c F1: %.1f", 
                 (app->current_feature == 1) ? '>' : ' ',
                 (double)app->feature_values[1]);
        canvas_draw_str(canvas, 2, 50, buffer);
        
        // Result
        if(app->has_result && app->last_result.valid) {
            const char* class_names[] = {"Weak", "Medium", "Strong"};
            canvas_set_font(canvas, FontPrimary);
            snprintf(buffer, sizeof(buffer), "Class: %s", 
                     class_names[app->last_result.class_id % 3]);
            canvas_draw_str(canvas, 2, 62, buffer);
        }
    } else {
        // Template Matching Mode
        const char* pattern_names[] = {"Sine Low", "Sine High", "Square"};
        snprintf(buffer, sizeof(buffer), "Pattern: %s", pattern_names[app->pattern_type]);
        canvas_draw_str(canvas, 2, 32, buffer);
        
        // Show waveform visualization
        for(int x = 0; x < 124; x++) {
            float t = (float)x / 124.0f * 16.0f;
            int y_val = 0;
            
            switch(app->pattern_type) {
                case 0:  // Low freq sine
                    y_val = (int)(10.0f * sinf(t * 3.14159f / 4.0f));
                    break;
                case 1:  // High freq sine
                    y_val = (int)(10.0f * sinf(t * 3.14159f / 2.0f));
                    break;
                case 2:  // Square
                    y_val = (((int)t % 8) < 4) ? 10 : -10;
                    break;
            }
            
            canvas_draw_dot(canvas, x + 2, 48 - y_val);
        }
        
        // Result
        if(app->has_result && app->last_result.valid) {
            const char* class_names[] = {"Sine Low", "Sine High", "Square"};
            canvas_set_font(canvas, FontPrimary);
            snprintf(buffer, sizeof(buffer), "Match: %s (%lu%%)", 
                     class_names[app->last_result.class_id % 3],
                     app->last_result.confidence);
            canvas_draw_str(canvas, 2, 62, buffer);
        }
    }
    
    // Instructions
    elements_button_left(canvas, "Model");
    elements_button_center(canvas, "OK");
    elements_button_right(canvas, app->selected_model == 0 ? "Edit" : "Next");
}

static void input_callback(InputEvent* input_event, void* ctx) {
    AIToolsApp* app = ctx;
    furi_message_queue_put(app->event_queue, input_event, FuriWaitForever);
}

static void perform_classification(AIToolsApp* app) {
    if(app->selected_model == 0) {
        // Decision tree classification
        AIFeatureVector features = {0};
        features.features[0] = ai_float_to_fixed(app->feature_values[0]);
        features.features[1] = ai_float_to_fixed(app->feature_values[1]);
        features.num_features = 2;
        
        app->last_result = ai_decision_tree_classify(app->decision_tree, &features);
        app->has_result = true;
        
        FURI_LOG_I(TAG, "Decision tree: class=%d, conf=%lu%%", 
                   app->last_result.class_id, app->last_result.confidence);
    } else {
        // Template matching classification
        int32_t pattern[16];
        for(int i = 0; i < 16; i++) {
            float val = 0.0f;
            switch(app->pattern_type) {
                case 0:
                    val = 50.0f * sinf(i * 3.14159f / 4.0f);
                    break;
                case 1:
                    val = 50.0f * sinf(i * 3.14159f / 2.0f);
                    break;
                case 2:
                    val = ((i % 8) < 4) ? 50.0f : -50.0f;
                    break;
            }
            pattern[i] = ai_float_to_fixed(val);
        }
        
        app->last_result = ai_template_matcher_classify(app->template_matcher, pattern, 16);
        app->has_result = true;
        
        FURI_LOG_I(TAG, "Template match: class=%d, conf=%lu%%", 
                   app->last_result.class_id, app->last_result.confidence);
    }
    
    notification_message(app->notifications, &sequence_blink_green_100);
}

static AIToolsApp* app_alloc(void) {
    AIToolsApp* app = malloc(sizeof(AIToolsApp));
    
    app->view_port = view_port_alloc();
    app->gui = furi_record_open(RECORD_GUI);
    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    
    view_port_draw_callback_set(app->view_port, draw_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);
    
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    
    // Initialize AI models
    if(!init_decision_tree(app)) {
        FURI_LOG_E(TAG, "Failed to init decision tree");
    }
    if(!init_template_matcher(app)) {
        FURI_LOG_E(TAG, "Failed to init template matcher");
    }
    
    // Initialize state
    app->mode = AIToolsModeInput;
    app->selected_model = 0;
    app->current_feature = 0;
    app->feature_values[0] = 50.0f;
    app->feature_values[1] = 50.0f;
    app->has_result = false;
    app->pattern_type = 0;
    app->pattern_tick = 0;
    app->running = true;
    
    FURI_LOG_I(TAG, "AI Tools initialized");
    
    return app;
}

static void app_free(AIToolsApp* app) {
    furi_assert(app);
    
    // Free AI models
    if(app->decision_tree) {
        ai_decision_tree_free(app->decision_tree);
    }
    if(app->template_matcher) {
        ai_template_matcher_free(app->template_matcher);
    }
    
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_message_queue_free(app->event_queue);
    
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    
    free(app);
}

int32_t ai_tools_app(void* p) {
    UNUSED(p);
    
    AIToolsApp* app = app_alloc();
    
    InputEvent event;
    
    while(app->running) {
        if(furi_message_queue_get(app->event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == InputTypePress) {
                if(event.key == InputKeyBack) {
                    app->running = false;
                } else if(event.key == InputKeyLeft) {
                    // Switch model
                    app->selected_model = (app->selected_model + 1) % 2;
                    app->has_result = false;
                    view_port_update(app->view_port);
                } else if(event.key == InputKeyOk) {
                    // Perform classification
                    perform_classification(app);
                    view_port_update(app->view_port);
                } else if(event.key == InputKeyRight) {
                    if(app->selected_model == 0) {
                        // Switch current feature
                        app->current_feature = (app->current_feature + 1) % 2;
                    } else {
                        // Switch pattern type
                        app->pattern_type = (app->pattern_type + 1) % 3;
                        app->has_result = false;
                    }
                    view_port_update(app->view_port);
                } else if(event.key == InputKeyUp) {
                    if(app->selected_model == 0) {
                        // Increase current feature value
                        app->feature_values[app->current_feature] += 5.0f;
                        if(app->feature_values[app->current_feature] > 100.0f) {
                            app->feature_values[app->current_feature] = 100.0f;
                        }
                        app->has_result = false;
                        view_port_update(app->view_port);
                    }
                } else if(event.key == InputKeyDown) {
                    if(app->selected_model == 0) {
                        // Decrease current feature value
                        app->feature_values[app->current_feature] -= 5.0f;
                        if(app->feature_values[app->current_feature] < 0.0f) {
                            app->feature_values[app->current_feature] = 0.0f;
                        }
                        app->has_result = false;
                        view_port_update(app->view_port);
                    }
                }
            }
        }
    }
    
    app_free(app);
    return 0;
}
