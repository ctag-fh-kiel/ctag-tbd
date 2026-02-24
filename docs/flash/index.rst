Flash & Updates
===============

.. raw:: html

   <p class="flash-intro">
     The TBD-16 ships with all 50+ DSP plugins ready to go &mdash; you only need
     to flash when a new firmware release is available, you want to run a custom
     build, or something went wrong. Firmware updates <strong>never erase</strong>
     your SD card.
   </p>

   <div class="flash-grid">

     <!-- Flash DSP -->
     <div class="flash-card">
       <div class="flash-card-icon">&#9889;</div>
       <div class="flash-card-body">
         <h3>Flash DSP Firmware</h3>
         <p>Re-flash the <strong>ESP32-P4</strong> audio engine directly from
         your browser &mdash; WebSerial, no tools or drivers needed.</p>
         <ul class="flash-card-meta">
           <li>Chrome, Edge or Opera</li>
           <li>JTAG USB-C cable to front port</li>
         </ul>
         <a class="flash-card-link" href="25_flash_dsp.html">Flash DSP &rarr;</a>
       </div>
     </div>

     <!-- Flash UI -->
     <div class="flash-card">
       <div class="flash-card-icon">&#128187;</div>
       <div class="flash-card-body">
         <h3>Flash UI Firmware</h3>
         <p>Re-flash the <strong>RP2350</strong> front-end from your browser
         via WebUSB &mdash; put the device in BOOTSEL mode first.</p>
         <ul class="flash-card-meta">
           <li>Chrome, Edge or Opera</li>
           <li>Hold BOOTSEL while connecting USB</li>
         </ul>
         <a class="flash-card-link" href="30_flash_ui.html">Flash UI &rarr;</a>
       </div>
     </div>

     <!-- Device Recovery -->
     <div class="flash-card flash-card-recovery">
       <div class="flash-card-icon">&#128295;</div>
       <div class="flash-card-body">
         <h3>Device Recovery</h3>
         <p>Full step-by-step guide to re-initialize your TBD-16 from scratch
         &mdash; reflash both processors and rebuild both SD cards.</p>
         <ul class="flash-card-meta">
           <li>~15&ndash;20 minutes</li>
           <li>2&times; USB-C cables &amp; 2&times; SD cards needed</li>
         </ul>
         <a class="flash-card-link" href="50_device_recovery.html">Start Recovery &rarr;</a>
       </div>
     </div>

   </div>

.. toctree::
   :hidden:
   :glob:

   [0-9]*
