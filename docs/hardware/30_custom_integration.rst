******************
Custom Integration
******************

.. raw:: html

   <div style="text-align:center; margin: 0 0 2em 0;">
     <img src="../_static/assets/dadamachines-tbd-16_mockup_003.jpg"
          alt="dadamachines Custom Integration"
          style="width:100%; max-width:720px; border-radius:8px;" />
   </div>

For manufacturers and instrument designers who need full control over every aspect
of their product, dadamachines offers custom integration of the TBD platform
directly onto your PCB.

Instead of using the pre-built TBD-Core module, you integrate the ESP32-P4,
RP2350, audio codec, and supporting components directly into your product's
circuit board --- giving you complete freedom over form factor, connectors, cost
optimization, and production scaling.

.. raw:: html

   <div style="display:flex; gap:0.75em; flex-wrap:wrap; margin:1.5em 0 2em;">
     <a href="#what-we-offer" style="padding:0.5em 1.2em; border:1px solid var(--color-background-border, #ccc); border-radius:6px; text-decoration:none; font-weight:500;">What We Offer</a>
     <a href="#core-components" style="padding:0.5em 1.2em; border:1px solid var(--color-background-border, #ccc); border-radius:6px; text-decoration:none; font-weight:500;">Components</a>
     <a href="#audio-codec-options" style="padding:0.5em 1.2em; border:1px solid var(--color-background-border, #ccc); border-radius:6px; text-decoration:none; font-weight:500;">Codec Options</a>
     <a href="#how-it-works" style="padding:0.5em 1.2em; border:1px solid var(--color-background-border, #ccc); border-radius:6px; text-decoration:none; font-weight:500;">Process</a>
     <a href="https://dadamachines.com/contact/" style="padding:0.5em 1.2em; border:1px solid var(--color-background-border, #ccc); border-radius:6px; text-decoration:none; font-weight:500;">Contact</a>
   </div>


What We Offer
=============

dadamachines is a team of experts in:

- **Hardware design** --- Schematic capture, PCB layout, signal integrity
- **Electronics engineering** --- Power management, audio circuit design, EMC compliance
- **Industrial design** --- Enclosure design, manufacturing-ready mechanical engineering
- **UX/UI design** --- Interaction design, display interfaces, control surface ergonomics

We work as collaborators, not just vendors. Whether you're a synth maker, a studio
equipment company, or an artist building a one-of-a-kind instrument, we can help you
bring your product from concept to production.


Core Components
===============

A custom integration typically includes:

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Component
     - Purpose
   * - **ESP32-P4**
     - Audio DSP engine, WiFi server, plugin management
   * - **RP2350**
     - User interface, MIDI, hardware I/O, sequencer
   * - **ESP32-C6**
     - WiFi co-processor (optional, for wireless control)
   * - **Audio codec**
     - ADC/DAC for professional-grade audio I/O (see options below)
   * - **Power management**
     - Voltage regulation, USB power delivery

All firmware and the full plugin library work out of the box on custom hardware.
You can also develop custom plugins specific to your product.


Audio Codec Options
===================

The TBD-16 and TBD-Core use the
`TLV320AIC3254 <https://www.ti.com/product/en-us/TLV320AIC3254/part-details/TLV320AIC3254IRHBR>`_
(stereo, 44.1 kHz). For custom integrations, we can work with alternative codecs
to match your product's requirements:

.. list-table::
   :header-rows: 1
   :widths: 30 20 50

   * - Codec
     - Channels
     - Use Case
   * - `TLV320AIC3254 <https://www.ti.com/product/en-us/TLV320AIC3254/part-details/TLV320AIC3254IRHBR>`_
     - 2 in / 2 out
     - Default --- same as TBD-16 and TBD-Core
   * - `AK4619VN <https://www.akm.com/global/en/products/audio/audio-codec/ak4619vn/>`_
     - 4 in / 4 out
     - Multi-channel instruments, mixer applications, multi-FX

Other codecs can be evaluated on request depending on your sample rate,
channel count, and form factor requirements.


How It Works
============

1. **Consultation** --- We discuss your product concept, requirements, and timeline.
2. **Architecture** --- We help you select the right components and define the
   system architecture based on the TBD platform.
3. **Design** --- We design the PCB, enclosure, and interaction model in collaboration
   with your team.
4. **Prototyping** --- We build and validate prototypes, including firmware customization.
5. **Production support** --- We help you prepare for manufacturing with DFM-optimized
   designs and test procedures.


Why Custom Over a Module?
=========================

- **Lower BOM cost at scale** --- No module markup, optimized component selection
- **Smaller form factor** --- Place components exactly where you need them
- **Custom I/O** --- Choose your own connectors, jack types, channel count, codec
- **Brand identity** --- No visible third-party modules; your product is truly yours
- **Production control** --- Full ownership of your supply chain


Proven Lineage
==============

The TBD platform has a proven track record:

- **TBD-16** --- The current-generation desktop instrument by dadamachines
- **CTAG TBD Eurorack** --- The original open-source module, commercially sold by
  `Instruments of Things <https://instrumentsofthings.com/products/tbd>`_
- **AE Modular TBD** --- Compact adaptation by
  `tangible waves <https://www.tangiblewaves.com/store/p149/TBD.html>`_
  (`wiki <https://wiki.aemodular.com/#/modules/tbd>`_)
- **Open-source core** --- Upstream repository:
  `ctag-fh-kiel/ctag-tbd <https://github.com/ctag-fh-kiel/ctag-tbd>`_


Get in Touch
============

`Contact dadamachines <https://dadamachines.com/contact/>`_ to discuss your project.

We work with teams at any stage --- from napkin sketches to nearly-finished products
that need audio DSP integration.

----

.. raw:: html

   <div style="display:flex; gap:0.75em; flex-wrap:wrap; margin:1.5em 0;">
     <a href="https://dadamachines.com/contact/" style="padding:0.6em 1.5em; background:var(--color-brand-primary, #6a5acd); color:#fff; border-radius:6px; text-decoration:none; font-weight:600;">Contact dadamachines</a>
     <a href="https://dadamachines.com" style="padding:0.6em 1.5em; border:1px solid var(--color-background-border, #ccc); border-radius:6px; text-decoration:none; font-weight:500;">dadamachines.com</a>
   </div>
