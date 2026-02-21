****
Apps
****

The TBD-16 ships with every App ready to go. No flashing, no downloading,
no waiting. All Apps live on the SD card and you switch between them from a
boot menu -- just like choosing a script on a
`monome norns <https://norns.community/>`_ or loading a patch on a
`Critter & Guitari Organelle <https://www.critterandguitari.com/organelle-patches>`_.

Unlike devices that only run one algorithm at a time (like
`Dubby <https://dubby.studio/>`_ or
`Daisy <https://electro-smith.com/daisy>`_), the TBD-16 keeps
**all Apps on the device simultaneously**. Pick one from the menu and start
playing.

.. raw:: html

   <div class="apps-architecture">
     <strong>How it works:</strong> Each App is a firmware for the
     <strong>RP2350</strong> processor (handling the display, buttons,
     encoders, LEDs, and MIDI). Some Apps also pair with specific behaviour
     on the <strong>ESP32-P4</strong> (DSP plugins, Ableton Link, USB audio).
     Together, they define the full experience.
     <br><br>
     👉 See <a href="bootloader.html">UF2 Bootloader</a> for the technical
     details of how Apps are loaded and switched.
   </div>


.. raw:: html

   <h2 class="apps-section-header">Shipping with TBD-16</h2>

   <div class="apps-grid">

     <!-- Groovebox -->
     <div class="app-card" data-tags="sequencer synth midi link default">
       <a href="groovebox.html">
         <div class="app-card-screen">
           <span class="app-status shipping" title="Shipping"></span>
           <span class="app-placeholder">▶ GROOVEBOX</span>
         </div>
         <div class="app-card-body">
           <h3>Groovebox</h3>
           <p class="app-tagline">
             16-track drum machine and groovebox with step sequencing,
             pattern management, and per-track mute/solo. The default App.
           </p>
           <div class="app-tags">
             <span class="app-tag app-tag-default">Default</span>
             <span class="app-tag app-tag-sequencer">Sequencer</span>
             <span class="app-tag app-tag-synth">Synth</span>
             <span class="app-tag app-tag-midi">MIDI</span>
             <span class="app-tag app-tag-link">Link</span>
           </div>
         </div>
       </a>
     </div>

     <!-- Multi Effect / Synth -->
     <div class="app-card" data-tags="synth fx midi link">
       <a href="multi-effect.html">
         <div class="app-card-screen">
           <span class="app-status shipping" title="Shipping"></span>
           <span class="app-placeholder">◆ MULTI FX</span>
         </div>
         <div class="app-card-body">
           <h3>Multi Effect / Synth</h3>
           <p class="app-tagline">
             Browse and play any of the 50+ DSP plugins from the hardware UI.
             Run two plugins simultaneously as a stereo chain, dual synth, or
             any combination.
           </p>
           <div class="app-tags">
             <span class="app-tag app-tag-synth">Synth</span>
             <span class="app-tag app-tag-fx">FX</span>
             <span class="app-tag app-tag-midi">MIDI</span>
             <span class="app-tag app-tag-link">Link</span>
           </div>
         </div>
       </a>
     </div>

     <!-- MCL -->
     <div class="app-card" data-tags="sequencer midi link">
       <a href="mcl.html">
         <div class="app-card-screen">
           <span class="app-status shipping" title="Shipping"></span>
           <span class="app-placeholder">≡ MCL</span>
         </div>
         <div class="app-card-body">
           <h3>MegaCommand Live</h3>
           <p class="app-tagline">
             High-performance MIDI sequencer for external gear. Originally
             built for the Elektron MachineDrum, now running on TBD-16
             with Ableton Link sync.
           </p>
           <div class="app-tags">
             <span class="app-tag app-tag-sequencer">Sequencer</span>
             <span class="app-tag app-tag-midi">MIDI</span>
             <span class="app-tag app-tag-link">Link</span>
           </div>
         </div>
       </a>
     </div>

   </div>


.. raw:: html

   <h2 class="apps-section-header">Planned</h2>

   <div class="apps-grid">

     <!-- MIDI Controller + Audio Interface -->
     <div class="app-card" data-tags="controller midi audio planned">
       <a href="midi-controller.html">
         <div class="app-card-screen">
           <span class="app-status planned" title="Planned"></span>
           <span class="app-placeholder">⎈ MIDI + AUDIO</span>
         </div>
         <div class="app-card-body">
           <h3>MIDI Controller + Audio Interface</h3>
           <p class="app-tagline">
             Turn the TBD-16 into an iPad/iPhone companion. 30 buttons and
             4 encoders as MIDI CC, plus stereo USB audio -- all over one
             USB-C cable.
           </p>
           <div class="app-tags">
             <span class="app-tag app-tag-planned">Planned</span>
             <span class="app-tag app-tag-controller">Controller</span>
             <span class="app-tag app-tag-midi">MIDI</span>
             <span class="app-tag app-tag-audio">Audio</span>
           </div>
         </div>
       </a>
     </div>

     <!-- Your App -->
     <div class="app-card app-card-cta" data-tags="community">
       <a href="bootloader.html#adding-your-own-app">
         <div class="app-card-screen">
           <span class="app-placeholder">+</span>
         </div>
         <div class="app-card-body">
           <h3>Build Your Own</h3>
           <p class="app-tagline">
             The App system is open. Write RP2350 firmware, compile to
             .uf2, drop it on the SD card. Your App shows up in the boot
             menu alongside everything else.
           </p>
           <div class="app-tags">
             <span class="app-tag app-tag-community">Community</span>
           </div>
         </div>
       </a>
     </div>

   </div>


.. raw:: html

   <h2 class="apps-section-header">Utilities</h2>

   <div class="apps-grid">

     <!-- Debug Probe -->
     <div class="app-card" data-tags="utility">
       <a href="utilities.html#debug-probe">
         <div class="app-card-screen">
           <span class="app-status shipping" title="Shipping"></span>
           <span class="app-placeholder">⚙ DEBUG</span>
         </div>
         <div class="app-card-body">
           <h3>Debug Probe</h3>
           <p class="app-tagline">
             CMSIS-DAP debug probe for flashing the STM32 on the UI board.
           </p>
           <div class="app-tags">
             <span class="app-tag app-tag-utility">Utility</span>
           </div>
         </div>
       </a>
     </div>

     <!-- USB Mass Storage -->
     <div class="app-card" data-tags="utility">
       <a href="utilities.html#usb-mass-storage">
         <div class="app-card-screen">
           <span class="app-status shipping" title="Shipping"></span>
           <span class="app-placeholder">💾 USB</span>
         </div>
         <div class="app-card-body">
           <h3>USB Mass Storage</h3>
           <p class="app-tagline">
             Mount the SD card as a USB drive. Copy files without removing the card.
           </p>
           <div class="app-tags">
             <span class="app-tag app-tag-utility">Utility</span>
           </div>
         </div>
       </a>
     </div>

     <!-- UI Test -->
     <div class="app-card" data-tags="utility">
       <a href="utilities.html#ui-test">
         <div class="app-card-screen">
           <span class="app-status shipping" title="Shipping"></span>
           <span class="app-placeholder">◻ UI TEST</span>
         </div>
         <div class="app-card-body">
           <h3>UI Test</h3>
           <p class="app-tagline">
             Hardware test for display, LEDs, pots, buttons, and SD card.
           </p>
           <div class="app-tags">
             <span class="app-tag app-tag-utility">Utility</span>
           </div>
         </div>
       </a>
     </div>

     <!-- Game -->
     <div class="app-card" data-tags="utility">
       <a href="utilities.html#game">
         <div class="app-card-screen">
           <span class="app-status shipping" title="Shipping"></span>
           <span class="app-placeholder">🎮 GAME</span>
         </div>
         <div class="app-card-body">
           <h3>Game</h3>
           <p class="app-tagline">
             A small game showing the RP2350 can run anything, not just audio firmware.
           </p>
           <div class="app-tags">
             <span class="app-tag app-tag-utility">Utility</span>
           </div>
         </div>
       </a>
     </div>

   </div>


.. tip::
   When real screenshots of each App's display become available, they will
   replace the placeholder screens above -- just like the monochrome screen
   captures on `norns.community/explore <https://norns.community/explore>`_.


.. toctree::
   :hidden:

   groovebox
   multi-effect
   mcl
   midi-controller
   utilities
   bootloader
