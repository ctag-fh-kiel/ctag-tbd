******
System
******


WIFI 
====

.. note::
      The following does not apply to the AEM TBD version

Initially, the TBD creates its own WIFI with the SSID ctag-tbd (no
password). You can join that network, open a browser and enter
“\ http://ctag-tbd.local\ ”. This should open the main control page of
TBD. Here you can set your plugins and their parameters. If you enter
the “Edit configuration” page, you can make adjustments to your WIFI
settings:

-  WIFI mode:

   -  Access point (AP) → TBD creates its own network which you can
         join, no additional router is required, you can secure the
         network with a password

   -  Station (STA) → TBD will join an existing network

-  SSID: The network name TBD will create or join

-  Password: The password for the network

-  mDNS Name: The domain name which will used to access the TBD

-  Example use-case for live scenario with multiple TBDs:

   -  Set one TBD as AP

   -  Set other TBDs as STA and join network of first TBD in AP mode

   -  You can now access all TBDs through the primary TBDs network in AP
         mode → no external wireless network needed, no router needed


Other settings
==============

+-----------------------+-----------------------+-----------------------+
| Setting               | Description           | Remarks               |
+=======================+=======================+=======================+
| CV0|1 Config          | Allows for uni-       | Only available on MK1 |
|                       | (default, 0..5V) and  | V1+2, and AEM HW      |
|                       | bi-polar (-5..5V) CV  | versions              |
|                       | input                 |                       |
+-----------------------+-----------------------+-----------------------+
| POT0|1 Config         | Allows for            | Only available on MK1 |
|                       | uni/bi-polar Pot      | V1+2, and AEM HW      |
|                       | config, some plugins  | versions              |
|                       | may allow for some    |                       |
|                       | parameters the range  |                       |
|                       | from 0..1 some others |                       |
|                       | from -1..1            |                       |
+-----------------------+-----------------------+-----------------------+
| Input Noise Gate      | Applies a noise gate  |                       |
|                       | to audio inputs       |                       |
+-----------------------+-----------------------+-----------------------+
| CH0 To CH1 Daisy      | Internally routes     | Can be used to run    |
| Chain                 | output CH0 to input   | two mono plugins in a |
|                       | CH1                   | chain (serial)        |
+-----------------------+-----------------------+-----------------------+
| CH0|1 To Stereo       | Allows to output      | Can be used to run    |
|                       | single channel on     | two mono plugins with |
|                       | both channels or      | parallel output       |
|                       | route CH0 out to CH1  |                       |
|                       | (mix) and vice versa  |                       |
+-----------------------+-----------------------+-----------------------+
| CH0|1 Soft Clip       | Applies a soft        | Avoids digital        |
|                       | clipping algorithm to | distortion when a     |
|                       | output                | plugin level is too   |
|                       |                       | high                  |
+-----------------------+-----------------------+-----------------------+
| Codec Output Levels   | Allows to adjust      | Use this to adjust    |
|                       | output level of audio | output levels e.g. to |
|                       | codec                 | modular (58) or line  |
+-----------------------+-----------------------+-----------------------+
| Backup                | Allows to download    | All preset and        |
|                       | and upload backup     | configuration data    |
|                       | data                  | are downloaded,       |
|                       |                       | all.json is a JSON    |
|                       |                       | file which includes   |
|                       |                       | all data and can be   |
|                       |                       | used to restore       |
|                       |                       | module data           |
+-----------------------+-----------------------+-----------------------+


Firmware update
===============

.. warning::
      before you start make sure 

      1) you have a stable, uninterrupted power supply 
      2) you have a stable wifi connection with the module 
      3) download the correct firmware files for your hardware version. 
      

.. warning:: 

      Note that after a firmware update, all your patches,
      calibration data and other settings such as WIFI credentials are being
      overwritten. You may be able to recover them by creating a backup prior
      to firmware update and restoring it, this can be done from the “Edit
      configuration” tab. If you use a custom firmware with custom partition
      sizes, you cannot use the method mentioned below. The same holds true
      for the AEM hardware version as this is connected through a serial USB
      bridge, emulating the web ui. For AEM you have to follow specific
      instructions on the AEM TBD Github fork.`


You can upload firmware releases and your own firmware through the web
ui. This is a fast and easy process. 1) Download a release e.g. from
here https://github.com/ctag-fh-kiel/ctag-tbd/releases 2) set the
ctag-tbd.bin and storage.bin files (one is the firmware with all the dsp
plugin code, one is the storage data partition, including web and patch
data) 3) Hit the “upgrade firmware” button 4) wait until the update is
complete, TBD will then reboot and create a wireless network called
ctag-tbd, where you can make all you settings.

In case you brick your TBD in this process, you can always manually
flash it through a USB connection (see
https://github.com/ctag-fh-kiel/ctag-tbd#how-to-flash for details).


Calibration
===========

| If you own a TBD which has 2 control pots (+2 audio in level pots) and
  2 CV inputs, it likely uses the ESP32’s (the microcontroller which
  powers TBD) analog to digital converters (ADCs). These ADCs are rather
  imprecise and noisy unfortunately. You may notice issues with pitch
  tracking over 5 octaves and some faint but possibly noticeable jitter
  on whatever you control with the CVs. The accuracy of the ADC can be
  improved by applying a simple calibration procedure outlined in the
  following. This almost certainly leads to better pitch tracking. Note
  however, that the calibration likely will not improve above mentioned
  jitter noise.
| Note that TBD allows for uni-polar CV 0..5V range (default) and
  bi-polar CV -5..5V, configurable in the configuration page.
  Calibration can help to improve both modes. In order to calibrate
  bi-polar operation, you need a stable -5V voltage source.

Calibration Procedure:

-  Preparation:

   -  Make sure you have a well calibrated 1V/Octave CV keyboard
         available

      -  0V corresponds to C0

      -  2.5V corresponds approx. to F#2

      -  5V corresponds to C5

      -  -5V may not be achieved, in this case your TBD cannot be
            calibrated for bi-polar CV use

   -  Or have a CV generator available which can be adjusted to precise
         voltages

      -  Elektron Analog 4 or `ornament and crime <https://ornament-and-cri.me/user-manual-v1_3/#anchor-references>`__
            work well for this purpose

   -  An adjustable lab voltage source together with a precise
         multimeter does the trick as well

   -  Allow for TBD two CV ins to be connected at the same time to the
         calibration voltage source

-  Procedure:

   -  In the main menu, hit the “Edit configuration” button to enter the
         configuration tab

   -  Hit the “Reboot, calibrate CVs” button → TBD will reboot in
         calibration mode, the LED blinks red

1) Turn CV pots all left, apply 0V to CV in jacks → single press trig0

2) Turn CV pots mid position, apply 2.5V to CV jacks → single press
      trig0

3) Turn CV pots all right, apply 5V to CV jacks → single press trig0

4) Turn CV pots all left, apply -5V to CV in jacks → single press trig0

5) Turn CV pots mid position, apply 0V to CV in jacks → single press
      trig0

6) Turn CV pots all right, apply 5V to CV in jacks → single press trig0

-  TBD is now calibrated and will reboot to operating mode

-  Backup / Restore

   -  You can backup and restore your calibration data e.g. prior to a
         fresh calibration or system update by hitting the “Calibration
         Backup” in the Configuration page. A new page will pop up, here
         you can down/upload calibration data (JSON format).