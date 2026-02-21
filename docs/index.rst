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
       grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
       gap: 1.5em;
       margin: 1.5em 0 2.5em;
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
       <p>Your TBD-16 ships ready to use. Switch between Apps — Groovebox, Multi Effect, MCL, and more — all pre-installed on the SD card.</p>
       <ul>
         <li><a href="apps/index.html">Apps</a></li>
         <li><a href="get_started/10_using_your_tbd.html">Using Your TBD-16</a></li>
         <li><a href="get_started/15_wifi_and_link.html">WiFi &amp; Ableton Link</a></li>
         <li><a href="plugins/index.html">50+ DSP Plugins</a></li>
         <li><a href="faq.html">FAQ &amp; Troubleshooting</a></li>
       </ul>
     </div>

     <div class="persona-card">
       <h3>🔧 Build Your Own Firmware</h3>
       <p>Start with the RP2350 frontend — build custom MIDI controllers, sequencers, or control surfaces using Arduino and PlatformIO.</p>
       <ul>
         <li><a href="frontend/10_getting_started.html">Frontend — Getting Started</a></li>
         <li><a href="frontend/20_rp2350.html">Architecture &amp; SPI API</a></li>
         <li><a href="dsp/10_getting_started.html">DSP — Getting Started</a></li>
         <li><a href="dsp/20_plugin_architecture.html">Plugin Architecture</a></li>
         <li><a href="dsp/50_simulator.html">DSP Simulator</a></li>
       </ul>
     </div>

     <div class="persona-card">
       <h3>🏭 Design Products</h3>
       <p>Build custom instruments on the TBD platform, or integrate the DSP engine into your own hardware.</p>
       <ul>
         <li><a href="hardware/10_tbd16.html">TBD-16 Hardware Specs</a></li>
         <li><a href="hardware/index.html#platform-options">TBD-Core &amp; Custom Integration</a></li>
         <li><a href="dsp/60_web_api.html">Web UI API</a></li>
         <li><a href="https://dadamachines.com/contact/">Contact dadamachines</a></li>
       </ul>
     </div>

   </div>

| `dadamachines.com <https://dadamachines.com>`_ · `GitHub <https://github.com/dadamachines/ctag-tbd>`_ · `Community & Support <about/20_community.html>`_


.. toctree::
    :hidden:

    get_started/index
    flash/index
    hardware/index
    Plugins <plugins/index>
    Apps <apps/index>
    frontend/index
    dsp/index
    Blog <blog/index>
    faq
    about/index