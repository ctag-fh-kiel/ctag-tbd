**************************
Open-Source Licenses
**************************

The TBD platform is open source. You can study how it works, learn from it,
build on it, and contribute back. At the same time, the licenses are chosen
so that no one can simply repackage our work into a competing commercial
product without giving back to the community.

dadamachines is a small, independent team --- we don't run on venture capital.
Selling TBD-16 hardware is how we fund continued development of the platform,
the documentation, the web UI, and the tools that make TBD useful for
musicians and developers. The open-source licenses below protect that work
while keeping development fully open for individuals and contributors.


How It Works at a Glance
========================

.. list-table::
   :header-rows: 1
   :widths: 35 20 45

   * - Component
     - License
     - What It Means
   * - Core DSP engine (upstream CTAG TBD)
     - `GPL 3.0 <https://www.gnu.org/licenses/gpl-3.0.txt>`_
     - Modifications must be shared under the same terms
   * - dadamachines additions (web UI, flasher, docs, tools)
     - `LGPL 3.0 <https://www.gnu.org/licenses/lgpl-3.0.txt>`_
     - More permissive for developers; companies must share modifications back
   * - dadamachines TBD-16 plugins (PicoSeqRack, future plugins)
     - `LGPL 3.0 <https://www.gnu.org/licenses/lgpl-3.0.txt>`_
     - Same as above --- open for learning, protected from commercial copying
   * - Original CTAG hardware (V1/V2, Eurorack)
     - `CC BY-NC-SA 4.0 <https://creativecommons.org/licenses/by-nc-sa/4.0/>`_
     - Non-commercial use, share-alike, attribution required
   * - TBD-16 hardware design
     - Proprietary
     - Not open source (open reference designs planned)


Software Licenses in Detail
===========================


Core DSP Engine (GPL 3.0)
-------------------------

The audio engine, sound processors, and platform core were originally developed
at `CTAG Kiel <https://www.creative-technologies.de>`_ by Robert Manzke.
The upstream repository is
`ctag-fh-kiel/ctag-tbd <https://github.com/ctag-fh-kiel/ctag-tbd>`_.

This code is licensed under the
`GNU General Public License (GPL 3.0) <https://www.gnu.org/licenses/gpl-3.0.txt>`_.
If you modify this code and distribute it, your modifications must also be
released under GPL 3.0.


dadamachines Additions (LGPL 3.0)
---------------------------------

Everything **added by dadamachines** in this repository is licensed under the
`GNU Lesser General Public License (LGPL 3.0) <https://www.gnu.org/licenses/lgpl-3.0.txt>`_.
This includes:

- The **web UI** (redesigned interface for TBD-16)
- The **browser-based flasher** for DSP and UI firmware
- The **documentation source** you are reading right now
- **Build tools** and utilities

We chose LGPL because it strikes the right balance:

- **For individual developers and contributors:** You can freely use, study,
  modify, and contribute to this code. You do not need to release your own
  unrelated projects under the same license.
- **For the community:** Any improvements to the LGPL'd code itself must be
  shared back if distributed, so the ecosystem keeps growing.
- **For commercial protection:** Other companies cannot take these components
  and ship them in a competing product without contributing their changes back
  as open source.

This approach is similar to how `Bela <https://bela.io>`_ licenses their core
code under LGPL --- keeping things open for makers and researchers while
protecting the work that sustains the project.


dadamachines TBD-16 Plugins (LGPL 3.0)
--------------------------------------

Plugins developed specifically for the TBD-16 by dadamachines and friends are
also released under LGPL 3.0. This currently includes:

- **PicoSeqRack** --- A MIDI-driven sequencer/rack plugin developed by
  `Per-Olov Jernberg (Possan) <https://possan.codes/>`_
  (`GitHub <https://github.com/possan>`_), custom-built for the TBD-16.

Future plugins developed by Johannes Lohbihler and Benjamin Weiss for
dadamachines will follow the same LGPL 3.0 license.

The existing 50+ plugins in the core library are part of the upstream CTAG TBD
project and remain under GPL 3.0.


Hardware Licenses
=================


Original CTAG TBD Eurorack Designs
----------------------------------

The original CTAG TBD hardware designs (V1/V2) by Robert Manzke are released
under
`Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
(CC BY-NC-SA 4.0) <https://creativecommons.org/licenses/by-nc-sa/4.0/>`_.


dadamachines TBD-Core & TBD-16
------------------------------

The **TBD-16** (complete desktop instrument) and **TBD-Core** (core DSP board
with FFC connector for custom UIs) are commercial products. Their hardware
designs --- including industrial design, PCB layout, and custom electronics ---
are proprietary.

An **open-hardware core design** based on the same ESP32-P4 + RP2350 platform
is planned for future publication in KiCad as an open-source reference. This
will give educators, researchers, and instrument builders a starting point to
learn from and build on --- similar in spirit to how the original CTAG TBD
Eurorack designs were published.

There is currently no Eurorack module based on the new ESP32-P4 + RP2350
platform planned by dadamachines, but we would love to partner with anyone
interested in building one. See
:doc:`Custom Integration </hardware/30_custom_integration>` or
`contact us <https://dadamachines.com/contact/>`_.


Using TBD in Your Own Products
==============================

**As a musician or maker:** Use TBD however you like. Build instruments, perform
live, teach with it, hack on it. That is what it is made for.

**As an individual developer:** Contribute plugins, fix bugs, improve the docs.
Your own projects do not need to be open source unless they include modified
TBD code that you distribute.

**As a company:** If you want to build a product around the TBD platform, two
options are designed for you:

- The :doc:`TBD-Core </hardware/20_tbd_core>` --- our core DSP board with all
  audio, MIDI, and USB I/O assembled. Connect your own UI board via the 30-pin
  FFC and design your own enclosure.
- :doc:`Custom Integration </hardware/30_custom_integration>` --- we integrate
  the ESP32-P4, RP2350, and codec directly onto your PCB for full control over
  form factor, connectors, and BOM.

We also offer volume pricing for TBD-16 units used as finished components inside
your products.

If you **modify and distribute** dadamachines-added code (web UI, tools, docs),
those changes must be released under LGPL 3.0.

If you **modify and distribute** the core DSP engine, those changes must be
released under GPL 3.0.

Want to keep your modifications **proprietary**? dadamachines can provide a
commercial license tailored to your project.
`Contact dadamachines <https://dadamachines.com/contact/>`_ to discuss.


The dadamachines Name and Brand
===============================

The dadamachines name, logo, and TBD-16 product name are trademarks reserved
for products made by or licensed by dadamachines.

If you build something with the TBD platform, you may reference it as:

- **[YourProduct] for TBD**
- **[YourProduct] (TBD-compatible)**

Please do not use "dadamachines [YourProduct]" or "TBD-16 [YourProduct]"
without prior agreement.
`Contact us <https://dadamachines.com/contact/>`_ if you are unsure.


Copyright
=========

| Copyright (c) 2020--2026 Robert Manzke. All rights reserved. (Core platform)
| Copyright (c) 2014--2026 Johannes Elias Lohbihler for dadamachines. (TBD-16 adaptation, UI/UX, Documentation)

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
