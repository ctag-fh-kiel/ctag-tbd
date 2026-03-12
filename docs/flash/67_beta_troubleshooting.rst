:orphan:

Beta Channel Troubleshooting
============================

Common issues when using the :doc:`Beta Channel <65_beta_channel>` or
:doc:`Beta Channel Archive <66_beta_channel_archive>` flashing tools.

**"Your browser does not support WebSerial"**

  Use Chrome, Edge, or Opera on desktop. Safari and Firefox do not support WebSerial.

**"Your browser does not support the File System Access API"**

  Use Chrome or Edge on desktop. Firefox and Safari do not support this API.
  The File System Access API is required to write files to the mounted SD card.

**Serial port request fails**

  - Make sure the **front JTAG port** cable is connected to your computer
  - Unplug the USB cable and replug it
  - Try a different USB cable
  - Close any terminals or serial monitors that may hold the port open
  - Restart your browser

**SD card doesn't mount after Step 1**

  The most common cause is that the device didn't actually reboot after flashing.
  The software reset via WebSerial is unreliable on ESP32-P4 — you may need a
  manual reset.

  1. **Press the RESET button** on the back of the device (between USB-C Port #1 and MIDI OUT 2)
  2. Wait 15 seconds for the SD card drive to appear
  3. If still nothing: **unplug both USB cables**, wait 3 seconds, replug them

  Also check:

  - Make sure a USB-C cable is connected from **back Port #1** to your computer
  - Port #1 is the back USB-C port closest to the center of the device
  - Check your file manager for a new drive (typically named ``NO NAME``)
  - If still no drive: go back to Step 1 and try again

**"Permission denied" when writing files**

  When Chrome asks for permission to write to the selected directory, click **Allow**.
  If you accidentally denied, close the prompt and click "Select SD Card Drive" again.

**Step 3 connection fails (device in MSC mode)**

  When the device is in USB Mass Storage mode, the serial port may not be available.
  Make sure you **eject the drive first** (macOS: right-click → Eject; Windows: "Safely Remove Hardware"; Linux: unmount),
  then try connecting via the **front JTAG port**. If the serial port
  still doesn't appear, briefly unplug and replug the front USB cable.

**Linux: serial port not accessible**

  On Linux, serial ports are typically owned by the ``dialout`` (or ``uucp``) group.
  If the browser cannot open the serial port, add your user to the group and log out/in::

    sudo usermod -aG dialout $USER

**Linux: Step 5 (RP2350) WebUSB device not found**

  Chrome on Linux needs udev rules to access USB devices. Create
  ``/etc/udev/rules.d/99-pico.rules`` with::

    SUBSYSTEM=="usb", ATTR{idVendor}=="2e8a", MODE="0666"

  Then reload rules::

    sudo udevadm control --reload-rules && sudo udevadm trigger

**Windows: Step 5 (RP2350) device not recognised**

  If Chrome does not show the RP2350 in the WebUSB device picker, you may need to
  install the **WinUSB** driver using `Zadig <https://zadig.akeo.ie/>`_. Select the
  RP2350 Boot device and install WinUSB.

.. include:: /_includes/footer-links.rst
