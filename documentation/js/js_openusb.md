# OpenUSB module {#js_openusb}

```js
let openusb = require("openusb");
```
# Methods
## setup()
Start USB HID with optional parameters. Should be called before all other methods.

**Parameters**

Configuration object *(optional)*:
- vid, pid (number): VID and PID values, both are mandatory
- mfrName (string): Manufacturer name (32  ASCII characters max), optional
- prodName (string): Product name (32  ASCII characters max), optional
- layoutPath (string): Path to keyboard layout file, optional

**Examples**
```js
// Start USB HID with default parameters
openusb.setup();
// Start USB HID with custom vid:pid = AAAA:BBBB, manufacturer and product strings not defined
openusb.setup({ vid: 0xAAAA, pid: 0xBBBB }); 
// Start USB HID with custom vid:pid = AAAA:BBBB, manufacturer string = "Flipper Devices", product string = "Flipper Zero"
openusb.setup({ vid: 0xAAAA, pid: 0xBBBB, mfrName: "Flipper Devices", prodName: "Flipper Zero" });
```

<br>

## isConnected()
Returns USB connection state.

**Example**
```js
if (openusb.isConnected()) {
    // Do something
} else {
    // Show an error
}
```

<br>

## press()
Press and release a key.

**Parameters**

Key or modifier name, key code.

See a [list of key names below](#js_openusb_keynames).

**Examples**
```js
openusb.press("a"); // Press "a" key
openusb.press("A"); // SHIFT + "a"
openusb.press("CTRL", "a"); // CTRL + "a"
openusb.press("CTRL", "SHIFT", "ESC"); // CTRL + SHIFT + ESC combo
openusb.press(98); // Press key with HID code (dec) 98 (Numpad 0 / Insert)
openusb.press(0x47); // Press key with HID code (hex) 0x47 (Scroll lock)
```

<br>

## hold()
Hold a key. Up to 5 keys (excluding modifiers) can be held simultaneously.

**Parameters**

Same as `press`.

**Examples**
```js
openusb.hold("a"); // Press and hold "a" key
openusb.hold("CTRL", "v"); // Press and hold CTRL + "v" combo
```

<br>

## release()
Release a previously held key.

**Parameters**

Same as `press`.

Release all keys if called without parameters.

**Examples**
```js
openusb.release(); // Release all keys
openusb.release("a"); // Release "a" key
```

<br>

## print()
Print a string.

**Parameters**

- A string to print
- *(optional)* Delay between key presses

**Examples**
```js
openusb.print("Hello, world!"); // print "Hello, world!"
openusb.print("Hello, world!", 100); // Add 100ms delay between key presses
```
<br>

## println()
Same as `print` but ended with "ENTER" press.

**Parameters**

- A string to print
- *(optional)* Delay between key presses

**Examples**
```js
openusb.println("Hello, world!");  // print "Hello, world!" and press "ENTER"
```
<br>

## altPrint()
Prints a string by Alt+Numpad method - works only on Windows!

**Parameters**

- A string to print
- *(optional)* delay between key presses

**Examples**
```js
openusb.altPrint("Hello, world!"); // print "Hello, world!"
openusb.altPrint("Hello, world!", 100); // Add 100ms delay between key presses
```
<br>

## altPrintln()
Same as `altPrint` but ended with "ENTER" press.

**Parameters**

- A string to print
- *(optional)* delay between key presses

**Examples**
```js
openusb.altPrintln("Hello, world!");  // print "Hello, world!" and press "ENTER"
```
<br>

## quit()
Releases usb, optional, but allows to interchange with usbdisk.

**Examples**
```js
openusb.quit();
usbdisk.start(...)
```
<br>

# Key names list {#js_openusb_keynames}

## Modifier keys

| Name          |
| ------------- |
| CTRL          |
| SHIFT         |
| ALT           |
| GUI           |

## Special keys

| Name               | Notes            |
| ------------------ | ---------------- |
| DOWN               | Down arrow       |
| LEFT               | Left arrow       |
| RIGHT              | Right arrow      |
| UP                 | Up arrow         |
| ENTER              |                  |
| DELETE             |                  |
| BACKSPACE          |                  |
| END                |                  |
| HOME               |                  |
| ESC                |                  |
| INSERT             |                  |
| PAGEUP             |                  |
| PAGEDOWN           |                  |
| CAPSLOCK           |                  |
| NUMLOCK            |                  |
| SCROLLLOCK         |                  |
| PRINTSCREEN        |                  |
| PAUSE              | Pause/Break key  |
| SPACE              |                  |
| TAB                |                  |
| MENU               | Context menu key |
| Fx                 | F1-F24 keys      |
| NUMx               | NUM0-NUM9 keys   |
