Flash & Updates
===============

The TBD-16 ships with all 50+ DSP plugins ready to go --- you only need
to flash when a new firmware release is available, you want a custom build,
or something went wrong. Firmware updates **never erase** your SD card.

.. raw:: html

   <div class="dada-ctas">

     <div class="dada-cta">
       <h3>Flash DSP Firmware</h3>
       <p>Re-flash the <strong>ESP32-P4</strong> audio engine directly from
       your browser &mdash; WebSerial, no tools or drivers needed.
       Requires Chrome, Edge or Opera and a JTAG USB-C cable to the front port.</p>
       <a href="25_flash_dsp.html" class="dada-cta-link">Flash DSP &rarr;</a>
     </div>

     <div class="dada-cta">
       <h3>Flash UI Firmware</h3>
       <p>Re-flash the <strong>RP2350</strong> front-end from your browser
       via WebUSB. Hold <strong>BOOTSEL</strong> while connecting USB to enter
       flash mode. Requires Chrome, Edge or Opera.</p>
       <a href="30_flash_ui.html" class="dada-cta-link">Flash UI &rarr;</a>
     </div>

   </div>

   <div class="dada-cta-wide">
     <h3>SD Card Recovery</h3>
     <p>Broken or corrupted SD card? Restore it directly from your browser &mdash;
     no card reader, no terminal, no opening the device. Downloads the factory
     SD card image and writes it via USB. Requires Chrome or Edge.</p>
     <a href="60_sd_card_recovery.html" class="dada-cta-link">Restore SD Card &rarr;</a>
   </div>

   <div class="dada-cta-wide">
     <h3>Full Device Recovery</h3>
     <p>Something went seriously wrong, or you want a completely clean slate?
     This guide walks you through re-initializing your TBD-16 from scratch &mdash;
     reflashing both the <strong>ESP32-P4</strong> and <strong>RP2350</strong>
     processors and rebuilding both SD cards. You will need 2&times; USB-C cables,
     2&times; SD cards, and about 15&ndash;20 minutes.</p>
     <a href="50_device_recovery.html" class="dada-cta-link">Start Full Recovery &rarr;</a>
   </div>

.. include:: /_includes/newsletter.rst

.. include:: /_includes/footer-links.rst

.. toctree::
   :hidden:
   :glob:

   [0-9]*
