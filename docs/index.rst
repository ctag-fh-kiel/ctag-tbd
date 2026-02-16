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
     on dedicated hardware you control from your browser.
   </p>

   <div class="persona-grid">

     <div class="persona-card">
       <h3>🎹 Make Music</h3>
       <p>Your TBD-16 ships ready to use. Plug in, open the web interface, and start making sound.</p>
       <ul>
         <li><a href="get_started/10_using_your_tbd.html">Using Your TBD-16</a></li>
         <li><a href="get_started/15_wifi_and_updates.html">WiFi &amp; Firmware Updates</a></li>
         <li><a href="features/10_ableton_link.html">Ableton Link</a></li>
         <li><a href="features/30_midi.html">MIDI</a></li>
         <li><a href="about/30_faq.html">FAQ &amp; Troubleshooting</a></li>
       </ul>
     </div>

     <div class="persona-card">
       <h3>🔧 Build Plugins &amp; Firmware</h3>
       <p>Create your own audio DSP plugins in C++, or build custom UI, MIDI tools, and sequencers for the RP2350.</p>
       <ul>
         <li><a href="create_plugins/10_prerequisites.html">Plugin Architecture</a></li>
         <li><a href="create_plugins/20_step_by_step.html">Step-by-Step Plugin Guide</a></li>
         <li><a href="development/05_simulator.html">Simulator — No Hardware Needed</a></li>
         <li><a href="development/20_rp2350.html">RP2350 UI / MIDI Firmware</a></li>
         <li><a href="development/01_building.html">Building &amp; Setup</a></li>
       </ul>
     </div>

     <div class="persona-card">
       <h3>🏭 Design Products</h3>
       <p>Use the TBD-16 as a devkit, or integrate the platform directly into your own hardware product.</p>
       <ul>
         <li><a href="hardware/10_tbd16.html">TBD-16 Specs</a></li>
         <li><a href="hardware/20_main_pcb.html">Main PCB with FFC Connector</a></li>
         <li><a href="hardware/30_custom_integration.html">Custom PCB Integration</a></li>
         <li><a href="development/10_api_reference.html">REST API Reference</a></li>
       </ul>
     </div>

   </div>

| `dadamachines.com <https://dadamachines.com>`_ · `GitHub <https://github.com/dadamachines/ctag-tbd>`_ · `Community & Support <about/20_community.html>`_


.. toctree::
    :hidden:

    get_started/index
    features/index
    hardware/index
    sound_library/index
    create_plugins/index
    development/index
    about/index


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
