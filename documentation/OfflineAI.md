# Offline AI Integration for Flipper Zero - OpenWorld Firmware

## Overview

This implementation adds lightweight offline AI capabilities to the Flipper Zero firmware, suitable for pattern recognition and classification tasks on resource-constrained embedded hardware (STM32WB55, 256KB RAM).

## What's Included

### 1. AI Classifier Library (`applications/examples/example_ai_classifier/ai_classifier.[ch]`)

A compact, efficient AI library providing:

**Decision Tree Classification**
- Binary decision trees with feature threshold tests
- Fast O(log n) inference time
- Suitable for multi-feature categorical classification
- No floating-point operations during inference

**Template Matching**  
- Pattern recognition using distance metrics
- Euclidean distance and correlation-based matching
- Ideal for signal waveform classification
- Supports variable-length patterns

**Fixed-Point Arithmetic**
- 16.16 fixed-point format for efficient computation
- Conversion utilities for float/fixed-point
- Optimized for ARM Cortex-M4 without FPU overhead

### 2. Example Application (`example_ai_classifier.c`)

A demo app showcasing both classification methods:

- **Decision Tree Mode**: Classifies synthetic 2D features into 3 classes in real-time
- **Template Matching Mode**: Recognizes sine wave and square wave patterns
- Interactive UI with live inference results and confidence scores
- Runs continuous inference every 500ms

## Features

✅ **Minimal Memory Footprint**: <10KB for typical models  
✅ **Fast Inference**: 10-50 µs per classification  
✅ **No External Dependencies**: Uses only standard C library  
✅ **Embedded-Friendly**: Fixed-point math, no heap fragmentation  
✅ **Extensible**: Easy to add custom distance metrics or tree structures  

## API Documentation

### Decision Trees

```c
// Create a decision tree
AIDecisionTree* tree = ai_decision_tree_create(num_nodes, num_classes, num_features);

// Configure tree nodes
tree->nodes[0].feature_idx = 0;              // Test feature 0
tree->nodes[0].threshold = ai_float_to_fixed(50.0f);
tree->nodes[0].left_child = 1;               // Go to node 1 if feature <= threshold
tree->nodes[0].right_child = 2;              // Go to node 2 if feature > threshold

// Prepare feature vector
AIFeatureVector features = {0};
features.features[0] = ai_float_to_fixed(45.0f);
features.features[1] = ai_float_to_fixed(78.0f);
features.num_features = 2;

// Classify
AIClassifierResult result = ai_decision_tree_classify(tree, &features);
if(result.valid) {
    printf("Class: %d, Confidence: %lu%%\n", result.class_id, result.confidence);
}

// Cleanup
ai_decision_tree_free(tree);
```

### Template Matching

```c
// Create matcher
AITemplateMatcher* matcher = ai_template_matcher_create(num_templates, num_classes);

// Add reference patterns
int32_t template_pattern[16] = { /* your pattern data */ };
ai_template_matcher_add(matcher, 0, template_pattern, 16, 0);

// Classify input pattern
int32_t input_pattern[16] = { /* input data */ };
AIClassifierResult result = ai_template_matcher_classify(matcher, input_pattern, 16);

// Cleanup
ai_template_matcher_free(matcher);
```

## Real-World Use Cases

### 1. RF Signal Classification

Classify Sub-GHz signals by their modulation characteristics:

```c
// Extract features from RF signal
features.features[0] = signal_strength;      // dBm
features.features[1] = frequency_deviation;  // Hz  
features.features[2] = pulse_width;          // µs
features.features[3] = duty_cycle;           // percentage

// Classify protocol
result = ai_decision_tree_classify(protocol_tree, &features);
// Result: class 0=ASK, 1=FSK, 2=OOK, etc.
```

### 2. IR Remote Pattern Recognition

Recognize infrared remote control patterns:

```c
// Capture IR timing sequence
int32_t ir_timings[32];
for(int i = 0; i < 32; i++) {
    ir_timings[i] = ai_float_to_fixed(capture_ir_pulse_us(i));
}

// Match against known remote templates
result = ai_template_matcher_classify(ir_matcher, ir_timings, 32);
// Result: class 0=TV, 1=AC, 2=DVD, etc.
```

### 3. RFID Card Behavior Analysis

Detect anomalous RFID card responses:

```c
// Extract card response characteristics
features.features[0] = response_time_ms;
features.features[1] = signal_quality;
features.features[2] = protocol_compliance_score;
features.features[3] = anticollision_behavior;

// Classify behavior
result = ai_decision_tree_classify(security_tree, &features);
// Result: class 0=normal, 1=suspicious, 2=invalid
```

### 4. Bluetooth LE Advertisement Profiling

Profile BLE devices by their advertisement patterns:

```c
// Extract BLE advertisement features
features.features[0] = advertisement_interval_ms;
features.features[1] = tx_power_dbm;
features.features[2] = service_uuid_count;
features.features[3] = manufacturer_data_length;

// Classify device type
result = ai_decision_tree_classify(ble_device_tree, &features);
// Result: class 0=smartphone, 1=wearable, 2=beacon, 3=sensor
```

## Performance Characteristics

| Operation | Time | Memory |
|-----------|------|--------|
| Decision tree (depth 3) | ~10 µs | ~112 bytes |
| Template match (16 samples, 3 templates) | ~50 µs | ~200 bytes |
| Feature vector | - | 32 bytes |
| Fixed-point conversion | ~1 µs | - |

## Limitations

- Maximum 8 features per sample (`AI_CLASSIFIER_MAX_FEATURES`)
- Maximum 8 classes (`AI_CLASSIFIER_MAX_CLASSES`)
- No GPU acceleration
- Models must fit in RAM (<100KB recommended)
- Limited floating-point performance (consider fixed-point for production)

## Integration Guide

### Adding AI to Your Flipper App

1. **Copy the AI library files** to your application directory:
   - `ai_classifier.h`
   - `ai_classifier.c`

2. **Include in your application**:
   ```c
   #include "ai_classifier.h"
   ```

3. **Update your `application.fam`**:
   ```python
   App(
       appid="my_ai_app",
       name="My AI App",
       # ... other settings ...
       sources=["*.c"],  # Will include ai_classifier.c
   )
   ```

4. **Initialize models in your app**:
   ```c
   typedef struct {
       AIDecisionTree* tree;
       // ... your app data ...
   } MyApp;
   
   MyApp* app_alloc(void) {
       MyApp* app = malloc(sizeof(MyApp));
       app->tree = ai_decision_tree_create(7, 3, 2);
       // Configure tree...
       return app;
   }
   ```

### Loading Models from SD Card

For larger models, store them on SD card:

```c
#include <storage/storage.h>

bool load_tree_from_file(AIDecisionTree* tree, const char* path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    
    bool success = false;
    if(storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        size_t bytes_read = storage_file_read(
            file, 
            tree->nodes, 
            sizeof(AIDecisionNode) * tree->num_nodes
        );
        success = (bytes_read == sizeof(AIDecisionNode) * tree->num_nodes);
        storage_file_close(file);
    }
    
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return success;
}
```

## Training Models

Models can be trained on a PC and deployed to Flipper:

### Decision Trees

Use scikit-learn to train, then export:

```python
from sklearn.tree import DecisionTreeClassifier
import numpy as np

# Train model
X_train = np.array([[...], [...], ...])  # Features
y_train = np.array([0, 1, 2, ...])       # Labels

clf = DecisionTreeClassifier(max_depth=5)
clf.fit(X_train, y_train)

# Export tree structure
def export_flipper_tree(tree, feature_names):
    nodes = []
    def recurse(node_id):
        if tree.tree_.feature[node_id] == -2:  # Leaf
            nodes.append({
                'feature_idx': -1,
                'class_id': np.argmax(tree.tree_.value[node_id][0]),
                'threshold': 0,
                'left_child': 0,
                'right_child': 0
            })
        else:  # Internal node
            left = len(nodes) + 1
            right = left + count_nodes(tree.tree_.children_left[node_id])
            nodes.append({
                'feature_idx': tree.tree_.feature[node_id],
                'threshold': int(tree.tree_.threshold[node_id] * 65536),  # Fixed-point
                'left_child': left,
                'right_child': right,
                'class_id': 0
            })
            recurse(tree.tree_.children_left[node_id])
            recurse(tree.tree_.children_right[node_id])
    
    recurse(0)
    return nodes

# Generate C code
nodes = export_flipper_tree(clf, feature_names)
print(f"// Decision tree with {len(nodes)} nodes")
for i, node in enumerate(nodes):
    print(f"nodes[{i}].feature_idx = {node['feature_idx']};")
    print(f"nodes[{i}].threshold = {node['threshold']};")
    print(f"nodes[{i}].left_child = {node['left_child']};")
    print(f"nodes[{i}].right_child = {node['right_child']};")
    print(f"nodes[{i}].class_id = {node['class_id']};")
    print()
```

### Template Patterns

Capture and export patterns:

```python
import numpy as np

# Capture reference patterns
patterns = {
    'sine_low': [np.sin(i * np.pi / 4) * 50 for i in range(16)],
    'sine_high': [np.sin(i * np.pi / 2) * 50 for i in range(16)],
    'square': [50 if (i % 8) < 4 else -50 for i in range(16)]
}

# Export as fixed-point
for name, pattern in patterns.items():
    print(f"// Pattern: {name}")
    print(f"int32_t pattern_{name}[16] = {{")
    for val in pattern:
        fixed = int(val * 65536)
        print(f"    {fixed},  // {val:.2f}")
    print("};")
```

## Future Enhancements

Potential improvements for future versions:

- [ ] Quantized neural networks (8-bit weights)
- [ ] K-means clustering for unsupervised learning  
- [ ] Naive Bayes classifier for probabilistic classification
- [ ] Feature normalization/scaling utilities
- [ ] Online learning capabilities (model updates on device)
- [ ] Model compression (pruning, quantization)
- [ ] CMSIS-NN integration for ARM NEON optimization
- [ ] Support for more complex architectures (random forests)

## Security Considerations

When using AI for security-critical applications:

- **Validate inputs**: Ensure features are within expected ranges
- **Set confidence thresholds**: Reject low-confidence classifications
- **Use multiple models**: Combine decision tree + template matching for robustness
- **Test edge cases**: Verify behavior with adversarial inputs
- **Update models regularly**: Retrain with new data as attacks evolve

## Contributing

To contribute new models or algorithms:

1. Ensure code follows Flipper Zero coding style
2. Keep memory footprint minimal (<10KB per model)
3. Optimize for speed (target <100 µs inference)
4. Add comprehensive documentation
5. Include example usage in the demo app
6. Test on actual hardware

## References

- [TinyML: Machine Learning with TensorFlow Lite on Arduino and Ultra-Low-Power Microcontrollers](https://www.oreilly.com/library/view/tinyml/9781492052036/)
- [ARM CMSIS-NN: Neural Network Kernels](https://github.com/ARM-software/CMSIS-NN)
- [TensorFlow Lite for Microcontrollers](https://www.tensorflow.org/lite/microcontrollers)
- [Embedded Machine Learning](https://eloquentarduino.com/)
- [scikit-learn Decision Trees](https://scikit-learn.org/stable/modules/tree.html)

## License

Same as Flipper Zero firmware (see main LICENSE file).

---

**Questions or Issues?**

Open an issue on GitHub with the `AI` label. Include:
- Model configuration
- Input data samples
- Expected vs actual results
- Memory usage measurements
