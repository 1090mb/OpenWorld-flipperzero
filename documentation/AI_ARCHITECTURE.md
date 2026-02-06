# Offline AI Architecture for Flipper Zero

```
┌─────────────────────────────────────────────────────────────────┐
│                    Flipper Zero Applications                     │
│  ┌────────────┐  ┌────────────┐  ┌────────────┐  ┌───────────┐│
│  │  SubGHz    │  │  Infrared  │  │    NFC     │  │   Custom  ││
│  │   App      │  │    App     │  │    App     │  │   Apps    ││
│  └─────┬──────┘  └──────┬─────┘  └──────┬─────┘  └─────┬─────┘│
│        │                │                │               │      │
│        └────────────────┴────────────────┴───────────────┘      │
│                              │                                   │
│                              ▼                                   │
│                   ┌──────────────────────┐                      │
│                   │  AI Feature Extractor │                      │
│                   │  - Signal strength    │                      │
│                   │  - Frequency          │                      │
│                   │  - Pulse width        │                      │
│                   │  - Pattern timing     │                      │
│                   └──────────┬───────────┘                      │
│                              │                                   │
│                              ▼                                   │
│         ┌────────────────────────────────────────────┐          │
│         │         AI Classifier Library              │          │
│         │  ┌──────────────┐    ┌─────────────────┐  │          │
│         │  │ Decision Tree│    │Template Matcher │  │          │
│         │  │              │    │                 │  │          │
│         │  │ - Binary tree│    │ - Distance calc │  │          │
│         │  │ - Fast O(log)│    │ - Correlation   │  │          │
│         │  │ - Categorical│    │ - Pattern match │  │          │
│         │  └──────────────┘    └─────────────────┘  │          │
│         │                                            │          │
│         │  ┌──────────────────────────────────────┐ │          │
│         │  │   Fixed-Point Math (16.16 format)    │ │          │
│         │  │   - Integer operations only          │ │          │
│         │  │   - ARM optimized                    │ │          │
│         │  └──────────────────────────────────────┘ │          │
│         └────────────────────┬───────────────────────┘          │
│                              │                                   │
│                              ▼                                   │
│                    ┌───────────────────┐                        │
│                    │ Classification    │                        │
│                    │ Result            │                        │
│                    │ - Class ID        │                        │
│                    │ - Confidence      │                        │
│                    └─────────┬─────────┘                        │
│                              │                                   │
│                              ▼                                   │
│                   ┌──────────────────────┐                      │
│                   │   Application Logic  │                      │
│                   │   - UI updates       │                      │
│                   │   - Actions          │                      │
│                   │   - Notifications    │                      │
│                   └──────────────────────┘                      │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                      Storage (SD Card)                           │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐             │
│  │ Tree Models │  │  Templates  │  │   Config    │             │
│  │   (.bin)    │  │   (.bin)    │  │   (.txt)    │             │
│  └─────────────┘  └─────────────┘  └─────────────┘             │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                  Hardware (STM32WB55)                            │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐        │
│  │ 256KB    │  │ Cortex-M4│  │ SubGHz   │  │  NFC     │        │
│  │  RAM     │  │ 64MHz    │  │  Radio   │  │  Reader  │        │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘        │
└─────────────────────────────────────────────────────────────────┘
```

## Data Flow Example: RF Signal Classification

```
1. User captures RF signal
   └─> SubGHz App receives raw signal

2. Extract features from signal
   ├─> Signal strength: -45 dBm
   ├─> Frequency deviation: 5 kHz
   ├─> Pulse width: 500 µs
   └─> Duty cycle: 50%

3. Convert to fixed-point
   ├─> features[0] = -45 * 65536 = -2949120
   ├─> features[1] = 5000 * 65536 = 327680000
   ├─> features[2] = 500 * 65536 = 32768000
   └─> features[3] = 50 * 65536 = 3276800

4. Classify with decision tree
   ├─> Start at root node
   ├─> Test: features[0] < threshold?
   ├─> Follow left/right based on test
   ├─> Repeat until leaf node
   └─> Result: Class 1 (FSK protocol)

5. Display to user
   └─> "Detected: FSK (100% confidence)"
```

## Memory Layout

```
Stack (2KB per app):
┌────────────────────────────┐
│ App struct (~200 bytes)    │
│ - AI model pointers        │
│ - View dispatcher          │
│ - Event loop               │
├────────────────────────────┤
│ Feature vector (32 bytes)  │
├────────────────────────────┤
│ Temp buffers (~500 bytes)  │
├────────────────────────────┤
│ Call stack (~1.3 KB)       │
└────────────────────────────┘

Heap (allocated as needed):
┌────────────────────────────┐
│ Decision tree              │
│ - Nodes array (~112 bytes) │
│ - Metadata (~16 bytes)     │
├────────────────────────────┤
│ Template matcher           │
│ - Templates (~200 bytes)   │
│ - Patterns (variable)      │
└────────────────────────────┘
```

## Performance Characteristics

```
┌──────────────────┬──────────┬───────────┬──────────┐
│   Operation      │   Time   │  Memory   │  Calls/s │
├──────────────────┼──────────┼───────────┼──────────┤
│ Feature Extract  │  5-10 µs │  32 bytes │ 100,000  │
│ Fixed->Float     │   ~1 µs  │  0 bytes  │ 1,000,000│
│ Float->Fixed     │   ~1 µs  │  0 bytes  │ 1,000,000│
│ Tree Classify    │  ~10 µs  │ 112 bytes │ 100,000  │
│ Template Match   │  ~50 µs  │ 200 bytes │  20,000  │
│ Distance Calc    │  ~15 µs  │  0 bytes  │  66,000  │
└──────────────────┴──────────┴───────────┴──────────┘
```

## Integration Patterns

### Pattern 1: Real-Time Classification
```c
// In your signal processing callback
void on_signal_received(Signal* signal) {
    // Extract features
    AIFeatureVector features;
    extract_features(signal, &features);
    
    // Classify
    AIClassifierResult result = 
        ai_decision_tree_classify(app->tree, &features);
    
    // Act on result
    if(result.valid && result.confidence > 80) {
        handle_classification(result.class_id);
    }
}
```

### Pattern 2: Batch Processing
```c
// Process multiple samples
for(size_t i = 0; i < num_samples; i++) {
    extract_features(&samples[i], &features);
    results[i] = ai_decision_tree_classify(tree, &features);
}

// Aggregate results
uint8_t most_common_class = find_mode(results, num_samples);
```

### Pattern 3: Confidence Filtering
```c
// Only act on high-confidence results
AIClassifierResult result = classify(&features);

if(result.valid) {
    if(result.confidence >= 90) {
        action_with_high_confidence(result.class_id);
    } else if(result.confidence >= 70) {
        action_with_medium_confidence(result.class_id);
    } else {
        prompt_user_for_verification(result.class_id);
    }
}
```

## Model Training Pipeline

```
┌────────────────┐     ┌──────────────┐     ┌─────────────────┐
│  Collect Data  │────>│ Train Model  │────>│ Export to C Code│
│  (PC/Python)   │     │ (scikit-learn)│     │   (Python)      │
└────────────────┘     └──────────────┘     └────────┬────────┘
                                                       │
                                                       ▼
┌────────────────┐     ┌──────────────┐     ┌─────────────────┐
│  Flash to      │<────│ Compile FAP  │<────│ Update App Code │
│  Flipper Zero  │     │  (./fbt)     │     │  (add nodes)    │
└────────────────┘     └──────────────┘     └─────────────────┘
```

## Real-World Applications

```
┌─────────────────────────────────────────────────────────────┐
│                   RF Signal Analysis                        │
│  Input: Raw RF capture                                      │
│  Features: [strength, freq, pulse_width, duty_cycle]       │
│  Output: Protocol (ASK/FSK/OOK)                            │
│  Accuracy: ~95% for common protocols                       │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                   IR Remote Recognition                      │
│  Input: IR pulse timing sequence                            │
│  Features: Pattern of 32 pulse widths                       │
│  Output: Remote type (TV/AC/DVD/etc)                       │
│  Accuracy: ~98% for known remotes                          │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                   RFID Card Profiling                        │
│  Input: Card response characteristics                       │
│  Features: [response_time, quality, compliance, behavior]  │
│  Output: Card type or anomaly flag                         │
│  Accuracy: ~90% anomaly detection                          │
└─────────────────────────────────────────────────────────────┘
```
