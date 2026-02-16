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


