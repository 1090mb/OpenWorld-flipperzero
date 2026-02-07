/**
 * @file example_ai_classifier.c
 * @brief Example application demonstrating offline AI classification
 * 
 * This app demonstrates:
 * - Decision tree classification for pattern recognition
 * - Template matching for signal classification
 * - Real-time inference on Flipper Zero hardware
 */

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <gui/elements.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include <ai_classifier.h>

#define TAG "AIClassifierExample"

typedef enum {
    ClassifierModeDecisionTree,
    ClassifierModeTemplate,
    ClassifierModeMax
} ClassifierMode;

typedef struct {
    ViewPort* view_port;
    Gui* gui;
    FuriMessageQueue* event_queue;
    NotificationApp* notifications;
    
    // AI Models
    AIDecisionTree* decision_tree;
    AITemplateMatcher* template_matcher;
    
    // State
    ClassifierMode mode;
    AIFeatureVector current_features;
    AIClassifierResult last_result;
    bool running;
    uint32_t inference_count;
    
    // Demo data generation
    uint32_t tick_count;
} AIClassifierApp;

// Initialize a simple decision tree for demo
// Tree classifies based on two features: signal strength and frequency
static bool init_decision_tree(AIClassifierApp* app) {
    // Create tree with 7 nodes, 3 classes, 2 features
    app->decision_tree = ai_decision_tree_create(7, 3, 2);
    if(!app->decision_tree) return false;
    
    // Build a simple tree:
    // [0]: feature[0] <= 50?
    //   left: [1]: leaf(0)
    //   right: [2]: feature[1] <= 70?
    //     left: [3]: leaf(1)
    //     right: [4]: feature[0] <= 80?
    //       left: [5]: leaf(1)
    //       right: [6]: leaf(2)
    
    AIDecisionNode* nodes = app->decision_tree->nodes;
    
    // Node 0: Root - test feature 0
    nodes[0].feature_idx = 0;
    nodes[0].threshold = ai_float_to_fixed(50.0f);
    nodes[0].left_child = 1;
    nodes[0].right_child = 2;
    
    // Node 1: Leaf - class 0
    nodes[1].feature_idx = -1;
    nodes[1].class_id = 0;
    
    // Node 2: Test feature 1
    nodes[2].feature_idx = 1;
    nodes[2].threshold = ai_float_to_fixed(70.0f);
    nodes[2].left_child = 3;
    nodes[2].right_child = 4;
    
    // Node 3: Leaf - class 1
    nodes[3].feature_idx = -1;
    nodes[3].class_id = 1;
    
    // Node 4: Test feature 0 again
    nodes[4].feature_idx = 0;
    nodes[4].threshold = ai_float_to_fixed(80.0f);
    nodes[4].left_child = 5;
    nodes[4].right_child = 6;
    
    // Node 5: Leaf - class 1
    nodes[5].feature_idx = -1;
    nodes[5].class_id = 1;
    
    // Node 6: Leaf - class 2
    nodes[6].feature_idx = -1;
    nodes[6].class_id = 2;
    
    return true;
}

// Initialize template matcher with sample patterns
static bool init_template_matcher(AIClassifierApp* app) {
    // Create matcher with 3 templates, 3 classes
    app->template_matcher = ai_template_matcher_create(3, 3);
    if(!app->template_matcher) return false;
    
    // Template 0: Low frequency sine wave (class 0)
    int32_t pattern0[16];
    for(int i = 0; i < 16; i++) {
        pattern0[i] = ai_float_to_fixed(50.0f * sinf(i * 3.14159f / 4.0f));
    }
    ai_template_matcher_add(app->template_matcher, 0, pattern0, 16, 0);
    
    // Template 1: High frequency sine wave (class 1)
    int32_t pattern1[16];
    for(int i = 0; i < 16; i++) {
        pattern1[i] = ai_float_to_fixed(50.0f * sinf(i * 3.14159f / 2.0f));
    }
    ai_template_matcher_add(app->template_matcher, 1, pattern1, 16, 1);
    
    // Template 2: Square wave (class 2)
    int32_t pattern2[16];
    for(int i = 0; i < 16; i++) {
        pattern2[i] = ai_float_to_fixed((i % 8 < 4) ? 50.0f : -50.0f);
    }
    ai_template_matcher_add(app->template_matcher, 2, pattern2, 16, 2);
    
    return true;
}

// Generate synthetic features for demo
static void generate_demo_features(AIClassifierApp* app) {
    // Generate features that vary over time
    uint32_t t = app->tick_count;
    
    // Feature 0: oscillates between 0-100
    float val0 = 50.0f + 40.0f * sinf(t / 50.0f);
    app->current_features.features[0] = ai_float_to_fixed(val0);
    
    // Feature 1: oscillates between 0-100 with different phase
    float val1 = 50.0f + 40.0f * sinf(t / 70.0f + 1.0f);
    app->current_features.features[1] = ai_float_to_fixed(val1);
    
    app->current_features.num_features = 2;
    app->tick_count++;
}

// Generate synthetic pattern for template matching
static void generate_demo_pattern(AIClassifierApp* app, int32_t* pattern, uint16_t length) {
    uint32_t t = app->tick_count;
    
    // Rotate between three pattern types
    uint8_t pattern_type = (t / 100) % 3;
    
    for(uint16_t i = 0; i < length; i++) {
        float val = 0.0f;
        switch(pattern_type) {
            case 0: // Low frequency sine
                val = 50.0f * sinf((i + t) * 3.14159f / 4.0f);
                break;
            case 1: // High frequency sine
                val = 50.0f * sinf((i + t) * 3.14159f / 2.0f);
                break;
            case 2: // Square wave
                val = ((i + t) % 8 < 4) ? 50.0f : -50.0f;
                break;
        }
        pattern[i] = ai_float_to_fixed(val);
    }
}

static void draw_callback(Canvas* canvas, void* ctx) {
    AIClassifierApp* app = ctx;
    furi_assert(app);
    
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    
    // Title
    canvas_draw_str(canvas, 2, 10, "AI Classifier Demo");
    
    // Mode
    canvas_set_font(canvas, FontSecondary);
    const char* mode_str = (app->mode == ClassifierModeDecisionTree) ? 
        "Decision Tree" : "Template Match";
    canvas_draw_str(canvas, 2, 22, mode_str);
    
    // Features / Input
    char buffer[64];
    if(app->mode == ClassifierModeDecisionTree) {
        snprintf(buffer, sizeof(buffer), "F0: %.1f  F1: %.1f", 
                 (double)ai_fixed_to_float(app->current_features.features[0]),
                 (double)ai_fixed_to_float(app->current_features.features[1]));
        canvas_draw_str(canvas, 2, 34, buffer);
    } else {
        canvas_draw_str(canvas, 2, 34, "Pattern: Sine/Square");
    }
    
    // Result
    canvas_set_font(canvas, FontPrimary);
    if(app->last_result.valid) {
        const char* class_names[] = {"Class A", "Class B", "Class C"};
        snprintf(buffer, sizeof(buffer), "Result: %s", 
                 class_names[app->last_result.class_id % 3]);
        canvas_draw_str(canvas, 2, 46, buffer);
        
        canvas_set_font(canvas, FontSecondary);
        snprintf(buffer, sizeof(buffer), "Confidence: %lu%%", app->last_result.confidence);
        canvas_draw_str(canvas, 2, 56, buffer);
    } else {
        canvas_draw_str(canvas, 2, 46, "Result: N/A");
    }
    
    // Stats
    canvas_set_font(canvas, FontSecondary);
    snprintf(buffer, sizeof(buffer), "Inferences: %lu", app->inference_count);
    canvas_draw_str(canvas, 2, 64, buffer);
    
    // Instructions
    elements_button_left(canvas, "Mode");
    elements_button_right(canvas, "Run");
}

static void input_callback(InputEvent* input_event, void* ctx) {
    AIClassifierApp* app = ctx;
    furi_message_queue_put(app->event_queue, input_event, FuriWaitForever);
}

static AIClassifierApp* app_alloc(void) {
    AIClassifierApp* app = malloc(sizeof(AIClassifierApp));
    
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
    
    app->mode = ClassifierModeDecisionTree;
    app->running = true;
    app->inference_count = 0;
    app->tick_count = 0;
    app->current_features.num_features = 0;
    app->last_result.valid = false;
    
    FURI_LOG_I(TAG, "AI Classifier initialized");
    
    return app;
}

static void app_free(AIClassifierApp* app) {
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

int32_t example_ai_classifier_app(void* p) {
    UNUSED(p);
    
    AIClassifierApp* app = app_alloc();
    
    InputEvent event;
    uint32_t last_inference_tick = furi_get_tick();
    
    while(app->running) {
        // Run inference every 500ms
        uint32_t current_tick = furi_get_tick();
        if(current_tick - last_inference_tick >= 500) {
            if(app->mode == ClassifierModeDecisionTree) {
                // Generate demo features
                generate_demo_features(app);
                
                // Run inference
                app->last_result = ai_decision_tree_classify(
                    app->decision_tree, 
                    &app->current_features
                );
            } else {
                // Generate demo pattern
                int32_t pattern[16];
                generate_demo_pattern(app, pattern, 16);
                
                // Run inference
                app->last_result = ai_template_matcher_classify(
                    app->template_matcher,
                    pattern,
                    16
                );
            }
            
            if(app->last_result.valid) {
                app->inference_count++;
                FURI_LOG_D(TAG, "Classified as class %d with confidence %lu%%",
                           app->last_result.class_id, app->last_result.confidence);
            }
            
            last_inference_tick = current_tick;
            view_port_update(app->view_port);
        }
        
        // Handle input
        if(furi_message_queue_get(app->event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == InputTypePress) {
                if(event.key == InputKeyBack) {
                    app->running = false;
                } else if(event.key == InputKeyLeft) {
                    // Switch mode
                    app->mode = (app->mode + 1) % ClassifierModeMax;
                    app->inference_count = 0;
                    app->tick_count = 0;
                    view_port_update(app->view_port);
                } else if(event.key == InputKeyRight) {
                    // Manual trigger
                    notification_message(app->notifications, &sequence_blink_blue_10);
                    view_port_update(app->view_port);
                }
            }
        }
    }
    
    app_free(app);
    return 0;
}
