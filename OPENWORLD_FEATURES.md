# OpenWorld Flipper Zero - Unlocked Features

## Overview

This firmware unlocks the full capabilities of your Flipper Zero device:

1. **Region Restrictions Removed** - Full 0-1000 MHz frequency access
2. **Custom Asset Pack Support** - Load custom animations, icons, and themes
3. **OpenBLE Integration** - Switch between USB and BLE (Bluetooth) HID modes

---

## Features

### 1. Region Unlocking

The firmware is configured to use **Region 00** (Unlocked) by default, providing:
- Full frequency range: 0 Hz - 1000 MHz
- No transmission restrictions
- Access to all sub-GHz bands

**Note:** Always check and follow your local radio transmission laws and regulations.

### 2. Custom Asset Packs

You can now load custom asset packs to personalize your Flipper Zero.

#### Directory Structure
```
/ext/asset_packs/
├── my_custom_pack/
│   ├── manifest.txt
│   ├── animations/
│   ├── icons/
│   └── sounds/
└── another_pack/
    ├── manifest.txt
    └── ...
```

#### Creating an Asset Pack

1. Create a folder in `/ext/asset_packs/` with your pack name
2. Create a `manifest.txt` file with pack information:
   ```
   Name: My Custom Pack
   Version: 1.0
   Author: Your Name
   Description: Custom animations and icons
   ```
3. Add your custom assets in appropriate subdirectories
4. Go to **Settings → Asset Packs** to select your pack
5. Restart Flipper Zero to apply changes

#### Supported Asset Types
- **Animations**: Custom dolphin animations (.bm files)
- **Icons**: Custom menu icons (.png files)
- **Sounds**: Custom notification sounds (future)
- **Themes**: Full UI themes (future)

### 3. OpenBLE (BadBLE) Support

The OpenUSB app now supports both USB and Bluetooth Low Energy (BLE) HID modes.

#### Switching Modes

1. Open **Open USB** app
2. Press **OK** to enter **Config** menu
3. Select **Interface Mode**
4. Choose between:
   - **USB** - Traditional USB HID (default)
   - **BLE (OpenBLE)** - Bluetooth HID mode

#### OpenBLE Features
- Wireless HID keyboard/mouse emulation
- Ducky script support via Bluetooth
- Pair with any device supporting BLE HID
- Manage pairing via **Remove Pairing** option

#### Usage Tips
- BLE mode requires pairing with target device first
- Battery drains faster in BLE mode
- Use **Remove Pairing** to connect to a new device
- Scripts run identically in both USB and BLE modes

---

## Technical Details

### Region Configuration
- File: `targets/f7/furi_hal/furi_hal_region.c`
- Default region: `furi_hal_region_zero` (00)
- Frequency range: 0-1000 MHz
- Power limit: 12 dBm
- Duty cycle: 50%

### Asset Pack API
- Library: `lib/asset_packs/asset_pack_manager.h`
- Settings: `/ext/.asset_pack.settings`
- Base path: `/ext/asset_packs/`
- Maximum packs: 32
- Pack name length: 64 characters

### OpenBLE Implementation
- Uses Flipper's native BLE stack
- HID profile: `ble_profile_hid`
- Device name: "OpenUSB"
- Pairing storage: `.bt_hid.keys`
- Compatible with keyboard layouts (25+ supported)

---

## Building the Firmware

```bash
./fbt
```

The compiled firmware will be in `dist/f7-D/`.

---

## Safety and Legal

⚠️ **Important Warnings:**

1. **Radio Regulations**: Even though the firmware removes technical restrictions, you are legally responsible for complying with your local radio regulations. Transmitting on unauthorized frequencies can result in fines or criminal charges.

2. **Responsible Use**: The OpenBLE/BadBLE features are intended for security research, penetration testing, and authorized testing only. Unauthorized access to computer systems is illegal.

3. **Device Safety**: Using this firmware is at your own risk. Transmitting at full power on all frequencies may damage your device or violate warranty.

---

## Changelog

### Version 1.0 (OpenWorld)
- ✅ Removed region restrictions (full 0-1000 MHz access)
- ✅ Added custom asset pack support
- ✅ Integrated OpenBLE (BadBLE) mode switching
- ✅ Added Asset Pack Settings app
- ✅ Added Interface Mode selector in OpenUSB Config

---

## Credits

- **Base Firmware**: Flipper Devices Inc.
- **OpenWorld Modifications**: Region unlocking, asset packs, OpenBLE integration
- **Community**: Thanks to the Flipper Zero community for testing and feedback

---

## License

This firmware is based on the official Flipper Zero firmware and inherits its license. Modifications are provided as-is for educational and research purposes.
