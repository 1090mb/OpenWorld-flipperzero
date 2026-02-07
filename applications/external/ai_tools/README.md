# AI Tools

An interactive offline AI classification tool for Flipper Zero that integrates the offline AI library.

## Features

- **Decision Tree Classification**: Classify inputs based on two configurable features
  - Adjust feature values using UP/DOWN buttons
  - Switch between features with RIGHT button
  - Press OK to classify

- **Template Matching**: Pattern recognition for signal classification
  - Switch between pattern types (Sine Low, Sine High, Square)
  - Visual waveform display
  - Press OK to match pattern

## Usage

### Decision Tree Mode
1. Use UP/DOWN to adjust the current feature value (0-100)
2. Press RIGHT to switch between Feature 0 and Feature 1  
3. Press OK to classify
4. View classification result: Weak, Medium, or Strong

### Template Matching Mode
1. Press RIGHT to cycle through pattern types
2. View the pattern visualization
3. Press OK to classify the pattern
4. View matching result with confidence score

### Controls
- **LEFT**: Switch between Decision Tree and Template Match modes
- **RIGHT**: Edit feature (Decision Tree) or Next pattern (Template Match)
- **UP/DOWN**: Adjust feature values (Decision Tree mode only)
- **OK**: Perform classification
- **BACK**: Exit app

## Technical Details

The app uses:
- Fixed-point (16.16) arithmetic for efficient computation
- Pre-trained decision tree with 7 nodes for 3-class classification
- 3 template patterns for pattern matching
- Real-time inference with visual feedback

Classification is performed locally on the device without any network connection.

## Categories

This app appears in the **Tools** category in the Flipper Zero applications menu.
