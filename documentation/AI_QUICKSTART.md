# Quick Start: Adding AI to Your Flipper App

This guide gets you started with AI classification in under 5 minutes.

## Step 1: Copy the Library (30 seconds)

```bash
# From your app directory
cp applications/examples/example_ai_classifier/ai_classifier.h .
cp applications/examples/example_ai_classifier/ai_classifier.c .
```

## Step 2: Include in Your App (10 seconds)

```c
#include "ai_classifier.h"
```

## Step 3: Create a Model (2 minutes)

### Option A: Simple Decision Tree

```c
typedef struct {
    AIDecisionTree* tree;
    // ... your other app data ...
} MyApp;

MyApp* app_alloc(void) {
    MyApp* app = malloc(sizeof(MyApp));
    
    // Create tree: 3 nodes, 2 classes, 2 features
    app->tree = ai_decision_tree_create(3, 2, 2);
    
    // Node 0: Root - test feature 0
    app->tree->nodes[0].feature_idx = 0;
    app->tree->nodes[0].threshold = ai_float_to_fixed(50.0f);
    app->tree->nodes[0].left_child = 1;
    app->tree->nodes[0].right_child = 2;
    
    // Node 1: Leaf - class 0
    app->tree->nodes[1].feature_idx = -1;
    app->tree->nodes[1].class_id = 0;
    
    // Node 2: Leaf - class 1
    app->tree->nodes[2].feature_idx = -1;
    app->tree->nodes[2].class_id = 1;
    
    return app;
}
```

### Option B: Template Matcher

```c
typedef struct {
    AITemplateMatcher* matcher;
    // ... your other app data ...
} MyApp;

MyApp* app_alloc(void) {
    MyApp* app = malloc(sizeof(MyApp));
    
    // Create matcher: 2 templates, 2 classes
    app->matcher = ai_template_matcher_create(2, 2);
    
    // Add template 0 (sine wave)
    int32_t sine_pattern[8];
    for(int i = 0; i < 8; i++) {
        sine_pattern[i] = ai_float_to_fixed(50.0f * sinf(i * 3.14f / 4.0f));
    }
    ai_template_matcher_add(app->matcher, 0, sine_pattern, 8, 0);
    
    // Add template 1 (square wave)
    int32_t square_pattern[8];
    for(int i = 0; i < 8; i++) {
        square_pattern[i] = ai_float_to_fixed((i % 4 < 2) ? 50.0f : -50.0f);
    }
    ai_template_matcher_add(app->matcher, 1, square_pattern, 8, 1);
    
    return app;
}
```

## Step 4: Use It (2 minutes)

### Classify with Decision Tree

```c
void classify_signal(MyApp* app, float signal_strength, float frequency) {
    // Prepare features
    AIFeatureVector features = {0};
    features.features[0] = ai_float_to_fixed(signal_strength);
    features.features[1] = ai_float_to_fixed(frequency);
    features.num_features = 2;
    
    // Classify
    AIClassifierResult result = ai_decision_tree_classify(app->tree, &features);
    
    // Use result
    if(result.valid) {
        const char* class_names[] = {"Type A", "Type B"};
        FURI_LOG_I("AI", "Classified as: %s (confidence: %lu%%)",
                   class_names[result.class_id], result.confidence);
    }
}
```

### Classify with Template Matcher

```c
void classify_pattern(MyApp* app, float* samples, size_t count) {
    // Convert to fixed-point
    int32_t pattern[8];
    for(size_t i = 0; i < count && i < 8; i++) {
        pattern[i] = ai_float_to_fixed(samples[i]);
    }
    
    // Classify
    AIClassifierResult result = 
        ai_template_matcher_classify(app->matcher, pattern, count);
    
    // Use result
    if(result.valid) {
        if(result.class_id == 0) {
            FURI_LOG_I("AI", "Detected: Sine wave (%lu%% confidence)", 
                      result.confidence);
        } else {
            FURI_LOG_I("AI", "Detected: Square wave (%lu%% confidence)",
                      result.confidence);
        }
    }
}
```

## Step 5: Clean Up (30 seconds)

```c
void app_free(MyApp* app) {
    // Free AI models
    if(app->tree) {
        ai_decision_tree_free(app->tree);
    }
    if(app->matcher) {
        ai_template_matcher_free(app->matcher);
    }
    
    // Free app
    free(app);
}
```

## Complete Example: RF Signal Classifier

```c
#include <furi.h>
#include "ai_classifier.h"

typedef struct {
    AIDecisionTree* tree;
} RFClassifierApp;

RFClassifierApp* app_alloc(void) {
    RFClassifierApp* app = malloc(sizeof(RFClassifierApp));
    
    // Create tree: 7 nodes, 3 classes (ASK/FSK/OOK), 3 features
    app->tree = ai_decision_tree_create(7, 3, 3);
    
    // Build tree (simplified for example)
    app->tree->nodes[0].feature_idx = 0;  // Test signal strength
    app->tree->nodes[0].threshold = ai_float_to_fixed(-50.0f);
    app->tree->nodes[0].left_child = 1;
    app->tree->nodes[0].right_child = 2;
    
    app->tree->nodes[1].feature_idx = -1;
    app->tree->nodes[1].class_id = 0;  // ASK
    
    app->tree->nodes[2].feature_idx = 1;  // Test frequency deviation
    app->tree->nodes[2].threshold = ai_float_to_fixed(3000.0f);
    app->tree->nodes[2].left_child = 3;
    app->tree->nodes[2].right_child = 4;
    
    app->tree->nodes[3].feature_idx = -1;
    app->tree->nodes[3].class_id = 1;  // FSK
    
    app->tree->nodes[4].feature_idx = -1;
    app->tree->nodes[4].class_id = 2;  // OOK
    
    return app;
}

void classify_rf_signal(RFClassifierApp* app, 
                       float strength_dbm,
                       float freq_deviation_hz,
                       float pulse_width_us) {
    AIFeatureVector features = {0};
    features.features[0] = ai_float_to_fixed(strength_dbm);
    features.features[1] = ai_float_to_fixed(freq_deviation_hz);
    features.features[2] = ai_float_to_fixed(pulse_width_us);
    features.num_features = 3;
    
    AIClassifierResult result = ai_decision_tree_classify(app->tree, &features);
    
    if(result.valid) {
        const char* protocols[] = {"ASK", "FSK", "OOK"};
        FURI_LOG_I("RF", "Detected %s modulation", protocols[result.class_id]);
    }
}

void app_free(RFClassifierApp* app) {
    ai_decision_tree_free(app->tree);
    free(app);
}
```

## Common Patterns

### Pattern 1: Threshold-Based Confidence

```c
AIClassifierResult result = ai_decision_tree_classify(tree, &features);

if(result.valid) {
    if(result.confidence >= 90) {
        // High confidence - automatic action
        trigger_action(result.class_id);
    } else if(result.confidence >= 70) {
        // Medium confidence - show warning
        show_warning(result.class_id);
    } else {
        // Low confidence - ask user
        prompt_user(result.class_id);
    }
}
```

### Pattern 2: Majority Voting

```c
// Classify multiple samples
AIClassifierResult results[10];
for(int i = 0; i < 10; i++) {
    extract_features(&samples[i], &features);
    results[i] = ai_decision_tree_classify(tree, &features);
}

// Count votes for each class
uint32_t votes[3] = {0};
for(int i = 0; i < 10; i++) {
    if(results[i].valid) {
        votes[results[i].class_id]++;
    }
}

// Pick winner
uint8_t winner = 0;
for(int i = 1; i < 3; i++) {
    if(votes[i] > votes[winner]) {
        winner = i;
    }
}

FURI_LOG_I("AI", "Consensus: Class %d (%lu/10 votes)", winner, votes[winner]);
```

### Pattern 3: Feature Scaling

```c
// Scale features to 0-100 range
float raw_strength = -70.0f;  // dBm
float scaled_strength = (raw_strength + 100.0f);  // Now 0-100

float raw_freq = 5000.0f;  // Hz
float scaled_freq = (raw_freq / 100.0f);  // Now 0-100

features.features[0] = ai_float_to_fixed(scaled_strength);
features.features[1] = ai_float_to_fixed(scaled_freq);
```

## Debugging Tips

### Print Feature Values

```c
FURI_LOG_D("AI", "Feature 0: %.2f", ai_fixed_to_float(features.features[0]));
FURI_LOG_D("AI", "Feature 1: %.2f", ai_fixed_to_float(features.features[1]));
```

### Validate Results

```c
if(!result.valid) {
    FURI_LOG_E("AI", "Classification failed!");
    return;
}

if(result.class_id >= num_classes) {
    FURI_LOG_E("AI", "Invalid class ID: %d", result.class_id);
    return;
}
```

### Check Performance

```c
uint32_t start = furi_get_tick();
AIClassifierResult result = ai_decision_tree_classify(tree, &features);
uint32_t elapsed = furi_get_tick() - start;

FURI_LOG_I("AI", "Classification took %lu ticks", elapsed);
```

## Next Steps

1. **Run the demo**: `Applications > Examples > AI Classifier`
2. **Read full docs**: `/documentation/OfflineAI.md`
3. **See architecture**: `/documentation/AI_ARCHITECTURE.md`
4. **Train custom models**: Use Python scripts in docs
5. **Join community**: Share your AI-powered Flipper apps!

## Need Help?

- Check `/documentation/OfflineAI.md` for complete API reference
- See `example_ai_classifier.c` for full working example
- Open GitHub issue with `AI` label for questions

## Performance Goals

- ✅ Keep inference under 100 µs
- ✅ Keep models under 10 KB
- ✅ Use fixed-point math
- ✅ Minimize dynamic allocation
- ✅ Test on real hardware

Happy coding! 🐬🤖
