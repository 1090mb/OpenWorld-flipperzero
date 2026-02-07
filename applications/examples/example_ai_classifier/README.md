# AI Classifier Example Application

This example demonstrates offline AI classification on Flipper Zero hardware.

## Features

This application showcases two AI classification methods:

### 1. Decision Tree Classifier
- Classifies 2-dimensional feature vectors into 3 classes
- Uses a 7-node binary decision tree
- Features vary sinusoidally in real-time for demo purposes
- Shows classification result and 100% confidence (deterministic)

### 2. Template Matcher
- Recognizes waveform patterns (sine waves and square waves)
- Matches input patterns against 3 reference templates
- Uses Euclidean distance for pattern matching
- Shows classification result and distance-based confidence score

## How to Use

### Running the App

1. Navigate to **Applications > Examples > AI Classifier**
2. The app starts in Decision Tree mode
3. Watch as it classifies features in real-time every 500ms

### Controls

- **Left Button**: Switch between Decision Tree and Template Matching modes
- **Right Button**: Trigger LED blink (visual feedback)
- **Back Button**: Exit the application

### Display Information

The screen shows:
- **Title**: "AI Classifier Demo"
- **Mode**: Current classification method (Decision Tree or Template Match)
- **Features/Input**: Current input values or pattern type
- **Result**: Predicted class (Class A, B, or C)
- **Confidence**: Classification confidence (0-100%)
- **Inferences**: Total number of classifications performed

## Code Structure

### Files
- `ai_classifier.h` - AI library interface
- `ai_classifier.c` - AI library implementation  
- `example_ai_classifier.c` - Demo application
- `application.fam` - App manifest

### Key Functions

**Decision Tree Initialization**
```c
init_decision_tree(AIClassifierApp* app)
```
Creates a 7-node tree that classifies based on two features.

**Template Matcher Initialization**
```c
init_template_matcher(AIClassifierApp* app)
```
Creates 3 templates: low-freq sine, high-freq sine, and square wave.

**Feature Generation**
```c
generate_demo_features(AIClassifierApp* app)
```
Generates synthetic 2D features that oscillate over time.

**Pattern Generation**
```c
generate_demo_pattern(AIClassifierApp* app, int32_t* pattern, uint16_t length)
```
Generates synthetic waveform patterns that rotate between types.

## Understanding the Demo

### Decision Tree Logic

The decision tree uses this structure:

```
Root: Is Feature0 <= 50?
├─ Yes: Class A
└─ No: Is Feature1 <= 70?
    ├─ Yes: Class B
    └─ No: Is Feature0 <= 80?
        ├─ Yes: Class B
        └─ No: Class C
```

### Template Matching Logic

Three templates are defined:
1. **Template 0 (Class A)**: Low-frequency sine wave (π/4 period)
2. **Template 1 (Class B)**: High-frequency sine wave (π/2 period)
3. **Template 2 (Class C)**: Square wave (50% duty cycle)

The classifier computes the Euclidean distance to each template and selects the closest match.

## Performance

- **Inference Time**: 
  - Decision Tree: ~10 µs
  - Template Matching: ~50 µs
- **Memory Usage**: <1KB total
- **Update Rate**: 500ms (configurable)

## Extending the Example

### Adding More Features

To add more features to the decision tree:

1. Increase `num_features` in `ai_decision_tree_create()`
2. Update `generate_demo_features()` to populate additional features
3. Modify tree structure to test new features

### Adding More Templates

To add more templates:

1. Increase `num_templates` in `ai_template_matcher_create()`
2. Call `ai_template_matcher_add()` for each new template
3. Update `generate_demo_pattern()` if needed

### Using Real Sensor Data

Replace synthetic data generation with real sensor inputs:

```c
// Example: Using GPIO input as features
features.features[0] = ai_float_to_fixed(read_analog_sensor(0));
features.features[1] = ai_float_to_fixed(read_analog_sensor(1));
features.num_features = 2;

result = ai_decision_tree_classify(app->decision_tree, &features);
```

## Building

This app is built as part of the firmware:

```bash
./fbt fap_example_ai_classifier
```

Or build all examples:

```bash
./fbt fap_examples
```

The compiled `.fap` file will be in `build/f7-firmware-D/.extapps/`

## Troubleshooting

**Problem**: App doesn't appear in menu  
**Solution**: Make sure it's built and installed in the correct location

**Problem**: Classification results don't change  
**Solution**: This is normal for demo mode - features cycle through the same values

**Problem**: Confidence is always 100%  
**Solution**: Decision trees are deterministic and always return 100% confidence. Try Template Matching mode for variable confidence.

## Learning More

See the comprehensive documentation at:
- `/documentation/OfflineAI.md` - Full AI integration guide
- `ai_classifier.h` - API documentation

## Real-World Applications

This demo can be adapted for:
- RF signal protocol detection
- IR remote identification  
- RFID card behavior analysis
- Bluetooth LE device profiling
- Sensor anomaly detection
- Environmental pattern recognition

See `/documentation/OfflineAI.md` for implementation examples.
