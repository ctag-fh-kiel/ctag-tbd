Flash & Updates
===============

The TBD-16 ships with all 50+ DSP plugins ready to go --- you only need
to flash when a new firmware release is available, you want a custom build,
or something went wrong. Firmware updates **never erase** your SD card.

.. raw:: html

   <div class="dada-features flash-features">

     <!-- Flash DSP Firmware -->
     <div class="dada-feature">
       <div class="dada-feature-icon">
         <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
           <rect x="4" y="4" width="16" height="16"/>
           <rect x="9" y="9" width="6" height="6"/>
           <line x1="9" y1="1" x2="9" y2="4"/><line x1="15" y1="1" x2="15" y2="4"/>
           <line x1="9" y1="20" x2="9" y2="23"/><line x1="15" y1="20" x2="15" y2="23"/>
           <line x1="20" y1="9" x2="23" y2="9"/><line x1="20" y1="14" x2="23" y2="14"/>
           <line x1="1" y1="9" x2="4" y2="9"/><line x1="1" y1="14" x2="4" y2="14"/>
         </svg>
       </div>
       <h4>Flash DSP Firmware</h4>
       <p>Re-flash the <strong>ESP32-P4</strong> audio engine directly from
       your browser &mdash; WebSerial, no tools or drivers needed.</p>
       <ul class="flash-feature-meta">
         <li>Chrome, Edge or Opera</li>
         <li>JTAG USB-C cable (front port)</li>
       </ul>
       <a class="flash-cta" href="25_flash_dsp.html">Flash DSP &rarr;</a>
     </div>

     <!-- Flash UI Firmware -->
     <div class="dada-feature">
       <div class="dada-feature-icon">
         <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
           <rect x="2" y="3" width="20" height="14"/>
           <line x1="8" y1="21" x2="16" y2="21"/>
           <line x1="12" y1="17" x2="12" y2="21"/>
         </svg>
       </div>
       <h4>Flash UI Firmware</h4>
       <p>Re-flash the <strong>RP2350</strong> front-end from your browser
       via WebUSB &mdash; put the device in BOOTSEL mode first.</p>
       <ul class="flash-feature-meta">
         <li>Chrome, Edge or Opera</li>
         <li>Hold BOOTSEL while connecting USB</li>
       </ul>
       <a class="flash-cta" href="30_flash_ui.html">Flash UI &rarr;</a>
     </div>

     <!-- Device Recovery -->
     <div class="dada-feature">
       <div class="dada-feature-icon">
         <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
           <polyline points="1 4 1 10 7 10"/>
           <path d="M3.51 15a9 9 0 1 0 .49-4.95"/>
         </svg>
       </div>
       <h4>Device Recovery</h4>
       <p>Full step-by-step guide to re-initialize your TBD-16 from scratch
       &mdash; reflash both processors and rebuild both SD cards.</p>
       <ul class="flash-feature-meta">
         <li>~15&ndash;20 minutes</li>
         <li>2&times; USB-C cables &amp; 2&times; SD cards</li>
       </ul>
       <a class="flash-cta" href="50_device_recovery.html">Start Recovery &rarr;</a>
     </div>

   </div>

.. toctree::
   :hidden:
   :glob:

   [0-9]*
