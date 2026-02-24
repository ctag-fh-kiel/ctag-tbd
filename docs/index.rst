######################
dada tbd/docs
######################

.. raw:: html

   <style>
     /* Hide the RST page title — the hero replaces it */
     article section h1,
     article > h1 { display: none !important; }
   </style>

   <!-- ========== HERO ========== -->
   <div class="dada-hero">
     <img src="_static/assets/dadamachines-tbd-16_mockup_002.jpg"
          alt="dadamachines TBD-16" />

     <p class="dada-hero-headline">
       The open audio platform<br>for musicians and developers.
     </p>
     <p class="dada-hero-sub">
       The <strong>dadamachines TBD-16</strong> is a standalone desktop
       instrument built on
       <a href="https://github.com/ctag-fh-kiel/ctag-tbd">CTAG&nbsp;TBD</a>,
       an open-source audio DSP engine. It combines 50+ synthesizers and
       effects with standard MIDI, a browser-based UI, and Ableton Link.
       Plug in power and start making music &mdash; no computer required.
     </p>
   </div>


   <!-- ========== WHY ========== -->
   <div class="dada-section">
     <div class="dada-section-label">Why</div>
     <h2 class="dada-section-title">
       An instrument you can truly make your own.
     </h2>
     <p class="dada-section-body">
       Most electronic instruments are closed boxes — you get the sounds
       the manufacturer chose, and that's it. <strong>TBD is
       different.</strong> The DSP engine, firmware, and plugin system
       are fully open source (GPL/LGPL). You can play it out of the box,
       or dive into the code and shape it into exactly the instrument
       you need.
     </p>

     <div class="dada-features">
       <div class="dada-feature">
         <div class="dada-feature-icon">
           <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"/><polygon points="10 8 16 12 10 16 10 8"/></svg>
         </div>
         <h4>Ready to Play</h4>
         <p>Ships as a complete instrument. Plug in USB-C power, connect
         speakers, and you're making music in seconds.</p>
       </div>
       <div class="dada-feature">
         <div class="dada-feature-icon">
           <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="16 18 22 12 16 6"/><polyline points="8 6 2 12 8 18"/></svg>
         </div>
         <h4>Open-Source Software</h4>
         <p>The core DSP engine is
         <a href="https://www.gnu.org/licenses/gpl-3.0.txt">GPL&nbsp;3.0</a>.
         Web UI, tools, and docs are
         <a href="https://www.gnu.org/licenses/lgpl-3.0.txt">LGPL&nbsp;3.0</a>.
         Full source access — study, modify, and contribute.</p>
       </div>
       <div class="dada-feature">
         <div class="dada-feature-icon">
           <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="11" width="18" height="11" rx="2" ry="2"/><path d="M7 11V7a5 5 0 019.9-1"/></svg>
         </div>
         <h4>No Lock-In</h4>
         <p>Standard MIDI, standard audio jacks, SD card storage.
         Your instruments and sounds belong to you.</p>
       </div>
       <div class="dada-feature">
         <div class="dada-feature-icon">
           <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><line x1="6" y1="3" x2="6" y2="15"/><circle cx="18" cy="6" r="3"/><circle cx="6" cy="18" r="3"/><path d="M18 9a9 9 0 01-9 9"/></svg>
         </div>
         <h4>Built on CTAG TBD</h4>
         <p>The audio engine and 50+ plugins come from the
         <a href="https://github.com/ctag-fh-kiel/ctag-tbd">CTAG&nbsp;TBD</a>
         project by Robert Manzke.</p>
       </div>
     </div>
   </div>


   <!-- ========== HOW ========== -->
   <div class="dada-section">
     <div class="dada-section-label">How</div>
     <h2 class="dada-section-title">
       Multicore architecture. Three processors, one open platform.
     </h2>
     <p class="dada-section-body">
       The TBD-16 runs three purpose-built processors in parallel: the
       <strong>ESP32-P4</strong> (dual-core RISC-V) handles real-time
       audio DSP, the <strong>RP2350</strong> (dual-core ARM/RISC-V)
       drives the hardware UI, MIDI, and sequencing, and the
       <strong>ESP32-C6</strong> manages WiFi &amp; Ableton Link.
       Each layer is independently programmable &mdash; change one
       without touching the others.
     </p>

     <div class="dada-steps">
       <div class="dada-step">
         <h4>Controller Apps</h4>
         <p>Build MIDI controllers, sequencers, or control surfaces on
         the RP2350 using Arduino &amp; PlatformIO.</p>
       </div>
       <div class="dada-step">
         <h4>DSP Plugins</h4>
         <p>Write audio code in C++ that runs on the ESP32-P4. Test in
         the desktop simulator before flashing to hardware.</p>
       </div>
       <div class="dada-step">
         <h4>Web UI</h4>
         <p>Built-in WiFi serves a web interface for preset management,
         plugin configuration, and firmware updates.</p>
       </div>
       <div class="dada-step">
         <h4>Ableton Link</h4>
         <p>Wireless tempo sync with Ableton Live, iOS apps, and any
         Link-enabled device on the same network.</p>
       </div>
     </div>
   </div>

   <!-- Key specs strip -->
   <div class="dada-specs">
     <div class="dada-spec">
       <div class="dada-spec-value">50+</div>
       <div class="dada-spec-label">DSP Plugins</div>
     </div>
     <div class="dada-spec">
       <div class="dada-spec-value">3</div>
       <div class="dada-spec-label">Processors</div>
     </div>
     <div class="dada-spec">
       <div class="dada-spec-value">32-bit</div>
       <div class="dada-spec-label">Audio Processing</div>
     </div>
     <div class="dada-spec">
       <div class="dada-spec-value">&lt;1ms</div>
       <div class="dada-spec-label">Latency</div>
     </div>
   </div>


   <!-- ========== WHAT ========== -->
   <div class="dada-section">
     <div class="dada-section-label">What you can do</div>
     <h2 class="dada-section-title">
       Jump in wherever you are.
     </h2>
     <p class="dada-section-body">
       Whether you're a musician, a developer, or both — there's a path
       for you.
     </p>

     <div class="dada-cards">

       <div class="dada-card">
         <div class="dada-card-icon">
           <svg width="26" height="26" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M9 18V5l12-2v13"/><circle cx="6" cy="18" r="3"/><circle cx="18" cy="16" r="3"/></svg>
         </div>
         <h3>Make Music</h3>
         <p>Your TBD-16 ships ready to use. Switch apps, tweak
         50+ DSP plugins from your browser, sync with Ableton Link.</p>
         <ul>
           <li><a href="get_started/10_using_your_tbd.html">Using Your TBD-16</a></li>
           <li><a href="apps/index.html">Browse Apps</a></li>
           <li><a href="plugins/index.html">Browse Plugins</a></li>
           <li><a href="get_started/15_wifi_and_link.html">WiFi &amp; Ableton Link</a></li>
         </ul>
       </div>

       <div class="dada-card">
         <div class="dada-card-icon">
           <svg width="26" height="26" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="2" y="3" width="20" height="14" rx="2" ry="2"/><line x1="8" y1="21" x2="16" y2="21"/><line x1="12" y1="17" x2="12" y2="21"/></svg>
         </div>
         <h3>Build Controller Apps</h3>
         <p>Create custom MIDI controllers, sequencers, or control
         surfaces on the RP2350 — Arduino &amp; PlatformIO. No audio
         programming required.</p>
         <ul>
           <li><a href="apps/getting-started.html">Development Setup</a></li>
           <li><a href="apps/rp2350.html">App Architecture</a></li>
           <li><a href="apps/bootloader.html">Bootloader &amp; Multi-App</a></li>
           <li><a href="apps/debugging.html">Debugging</a></li>
         </ul>
       </div>

       <div class="dada-card">
         <div class="dada-card-icon">
           <svg width="26" height="26" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="16 18 22 12 16 6"/><polyline points="8 6 2 12 8 18"/></svg>
         </div>
         <h3>Write DSP Plugins</h3>
         <p>Write audio code in C++ for the ESP32-P4. If you already
         ship VST, AU, or LV2 plugins, the workflow is familiar &mdash;
         start in the desktop simulator, no hardware needed, then flash
         to the device.</p>
         <ul>
           <li><a href="plugins/simulator.html">Desktop Simulator</a></li>
           <li><a href="plugins/architecture.html">Plugin Architecture</a></li>
           <li><a href="plugins/step-by-step.html">Creating a Plugin</a></li>
           <li><a href="plugins/getting-started.html">Development Setup</a></li>
         </ul>
       </div>

       <div class="dada-card">
         <div class="dada-card-icon">
           <svg width="26" height="26" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="4" y="4" width="16" height="16" rx="2" ry="2"/><rect x="9" y="9" width="6" height="6"/><line x1="9" y1="1" x2="9" y2="4"/><line x1="15" y1="1" x2="15" y2="4"/><line x1="9" y1="20" x2="9" y2="23"/><line x1="15" y1="20" x2="15" y2="23"/><line x1="20" y1="9" x2="23" y2="9"/><line x1="20" y1="14" x2="23" y2="14"/><line x1="1" y1="9" x2="4" y2="9"/><line x1="1" y1="14" x2="4" y2="14"/></svg>
         </div>
         <h3>Hardware &amp; Platform</h3>
         <p>The TBD-16 Devkit ships with 50+ plugins and full multi-app
         support. Products built on the platform can run the same broad
         palette <em>or</em> focus on a single dedicated plugin &mdash;
         the architecture supports both.</p>
         <ul>
           <li><a href="hardware/10_tbd16.html">TBD-16 Specs</a></li>
           <li><a href="hardware/20_tbd_core.html">TBD-Core Module</a></li>
           <li><a href="hardware/30_custom_integration.html">Custom Integration</a></li>
           <li><a href="flash/index.html">Flash &amp; Updates</a></li>
         </ul>
       </div>

     </div>
   </div>


   <!-- ========== GET INVOLVED ========== -->
   <div class="dada-section">
     <div class="dada-section-label">Get Involved</div>
     <h2 class="dada-section-title">
       Shape the platform.
     </h2>
     <p class="dada-section-body">
       TBD is built by a growing community of developers, musicians,
       and manufacturers. Here's how you can contribute.
     </p>

     <div class="dada-ctas">
       <div class="dada-cta">
         <h3>Contribute a Plugin</h3>
         <p>Write a synthesizer, effect, or drum machine in C++. Test it
         in the desktop simulator, then submit a pull request.</p>
         <a href="plugins/step-by-step.html" class="dada-cta-link">Create a Plugin &rarr;</a>
       </div>
       <div class="dada-cta">
         <h3>Build an App</h3>
         <p>Create a MIDI controller, sequencer, or performance app
         for the RP2350 — Arduino &amp; PlatformIO, no audio code
         needed.</p>
         <a href="apps/getting-started.html" class="dada-cta-link">App Dev Guide &rarr;</a>
       </div>
     </div>

     <div class="dada-cta-wide">
       <h3>Build Your Own Hardware Product</h3>
       <p>Already shipping VST, AU, or LV2 plugins? Your C++ DSP code
       can run on TBD hardware. Use the
       <a href="hardware/20_tbd_core.html">TBD-Core</a> module or work
       with dadamachines on a
       <a href="hardware/30_custom_integration.html">fully custom PCB
       integration</a>. Your product can focus on a single signature
       sound, offer a curated collection, or ship the full multi-plugin
       TBD experience &mdash; same open engine, your hardware, your
       brand.</p>
       <a href="https://dadamachines.com/contact/" class="dada-cta-link">Talk to dadamachines &rarr;</a>
     </div>
   </div>


.. include:: _includes/newsletter.rst

.. include:: _includes/footer-links.rst


.. toctree::
    :hidden:

    Blog <blog/index>
    Artist Interviews <interviews/index>
    get_started/index
    Apps <apps/index>
    Plugins <plugins/index>
    flash/index
    hardware/index
    about/index
    faq