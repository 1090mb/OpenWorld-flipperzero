let openusb = require("openusb");
let notify = require("notification");
let flipper = require("flipper");
let eventLoop = require("event_loop");
let gui = require("gui");
let dialog = require("gui/dialog");

let views = {
    dialog: dialog.makeWith({
        header: "OpenUSB demo",
        text: "Press OK to start",
        center: "Start",
    }),
};

openusb.setup({
    vid: 0xAAAA,
    pid: 0xBBBB,
    mfrName: "Flipper",
    prodName: "Zero",
    layoutPath: "/ext/openusb/assets/layouts/en-US.kl"
});

eventLoop.subscribe(views.dialog.input, function (_sub, button, eventLoop, gui) {
    if (button !== "center")
        return;

    gui.viewDispatcher.sendTo("back");

    if (openusb.isConnected()) {
        notify.blink("green", "short");
        print("USB is connected");

        openusb.println("Hello, world!");

        openusb.press("CTRL", "a");
        openusb.press("CTRL", "c");
        openusb.press("DOWN");
        delay(1000);
        openusb.press("CTRL", "v");
        delay(1000);
        openusb.press("CTRL", "v");

        openusb.println("1234", 200);

        openusb.println("Flipper Model: " + flipper.getModel());
        openusb.println("Flipper Name: " + flipper.getName());
        openusb.println("Battery level: " + flipper.getBatteryCharge().toString() + "%");

        // Alt+Numpad method works only on Windows!!!
        openusb.altPrintln("This was printed with Alt+Numpad method!");

        // There's also openusb.print() and openusb.altPrint()
        // which don't add the return at the end

        notify.success();
    } else {
        print("USB not connected");
        notify.error();
    }

    // Optional, but allows to unlock usb interface to switch profile
    openusb.quit();

    eventLoop.stop();
}, eventLoop, gui);

eventLoop.subscribe(gui.viewDispatcher.navigation, function (_sub, _item, eventLoop) {
    eventLoop.stop();
}, eventLoop);

gui.viewDispatcher.switchTo(views.dialog);
eventLoop.run();
