######################
dada tbd/docs
######################

.. raw:: html

   <style>
     /* Hide the page title to keep hero image at top */
     /* We target all H1s within the main article content on this page */
     article section h1,
     article > h1 {
       display: none !important;
     }

     .hero-image {
       width: 100%;
       max-width: 100%;
       border-radius: 8px;
       margin: 0 0 2em 0;
     }
     .persona-grid {
       display: grid;
       grid-template-columns: repeat(2, 1fr);
       gap: 1.5em;
       margin: 1.5em 0 2.5em;
     }
     @media (max-width: 640px) {
       .persona-grid {
         grid-template-columns: 1fr;
       }
     }
     .persona-card {
       border: 1px solid var(--color-background-border, #e5e7eb);
       border-radius: 8px;
       padding: 1.6em;
       background: var(--color-background-secondary, #fafafa);
       transition: border-color 0.15s;
     }
     .persona-card:hover {
       border-color: var(--color-brand-primary, #6a5acd);
     }
     .persona-card h3 {
       margin-top: 0;
       font-size: 1.15em;
     }
     .persona-card ul {
       padding-left: 1.2em;
       margin-bottom: 0;
     }
     .persona-card li {
       margin-bottom: 0.4em;
     }
     .persona-card a {
       text-decoration: none;
       font-weight: 500;
     }
     .tagline {
       font-size: 1.15em;
       color: var(--color-foreground-secondary, #555);
       margin-bottom: 2em;
     }
   </style>

   <img src="https://docs.dadamachines.com/images/dadamachines-video-poster.jpg"
        alt="dadamachines TBD-16"
        class="hero-image" />

   <p class="tagline">
     An open platform for audio DSP — synthesizers, effects, and drum machines
     on dedicated hardware. Just plug in power and start making music.
   </p>

   <div class="persona-grid">

     <div class="persona-card">
       <h3>🎹 Make Music</h3>
       <p>Your TBD-16 ships ready to use. Switch between Apps on the SD card, tweak 50+ DSP plugins from your browser, and sync with Ableton Link.</p>
       <ul>
         <li><a href="get_started/10_using_your_tbd.html">Using Your TBD-16</a></li>
         <li><a href="apps/index.html">Browse Apps</a></li>
         <li><a href="plugins/index.html">Browse Plugins</a></li>
         <li><a href="get_started/15_wifi_and_link.html">WiFi &amp; Ableton Link</a></li>
       </ul>
     </div>

     <div class="persona-card">
       <h3>🔧 Build Apps</h3>
       <p>Create custom MIDI controllers, sequencers, or control surfaces on the RP2350 — using Arduino and PlatformIO. No audio programming required.</p>
       <ul>
         <li><a href="apps/getting-started.html">Development Setup</a></li>
         <li><a href="apps/rp2350.html">App Architecture</a></li>
         <li><a href="apps/bootloader.html">Bootloader &amp; Multi-App</a></li>
         <li><a href="apps/debugging.html">Debugging</a></li>
       </ul>
     </div>

     <div class="persona-card">
       <h3>🎛️ Build Plugins</h3>
       <p>Write DSP code in C++ that runs on the ESP32-P4. Start with the desktop simulator — no hardware needed — then flash to the device.</p>
       <ul>
         <li><a href="plugins/simulator.html">Desktop Simulator</a></li>
         <li><a href="plugins/architecture.html">Plugin Architecture</a></li>
         <li><a href="plugins/step-by-step.html">Creating a Plugin</a></li>
         <li><a href="plugins/getting-started.html">Development Setup</a></li>
       </ul>
     </div>

     <div class="persona-card">
       <h3>🏭 Hardware &amp; Platform</h3>
       <p>Specs, schematics, and platform options — from the TBD-16 desktop instrument to TBD-Core modules and custom integration.</p>
       <ul>
         <li><a href="hardware/10_tbd16.html">TBD-16 Specs</a></li>
         <li><a href="hardware/index.html#platform-options">TBD-Core &amp; Custom Integration</a></li>
         <li><a href="flash/index.html">Flash &amp; Updates</a></li>
         <li><a href="https://dadamachines.com/contact/">Contact dadamachines</a></li>
       </ul>
     </div>

   </div>

| `dadamachines.com <https://dadamachines.com>`_ · `GitHub <https://github.com/dadamachines/ctag-tbd>`_ · `Community & Support <about/20_community.html>`_


.. toctree::
    :hidden:

    get_started/index
    Apps <apps/index>
    Plugins <plugins/index>
    flash/index
    hardware/index
    Blog <blog/index>
    faq
    about/index