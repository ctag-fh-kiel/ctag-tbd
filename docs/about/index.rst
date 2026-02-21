About
=====

**dada tbd** is an open-source audio DSP platform for musicians, educators, and
audio developers. It runs 50+ synthesizers and effects on dedicated hardware with
a full hardware interface (buttons, encoders, OLED display, RGB LEDs). Just
plug in power and start making music, no computer required. A WiFi-based web
interface is available for configuration and preset management.

The platform is built on `CTAG TBD <https://github.com/ctag-fh-kiel/ctag-tbd>`_,
an open-source embedded synthesizer created by
`Robert Manzke <https://www.creative-technologies.de/>`_ at Kiel University of
Applied Sciences.

The **dadamachines TBD-16** is the first standalone desktop version, a complete
instrument with standard MIDI, high-quality audio I/O, and Ableton Link, designed
to bring open-source audio processing beyond Eurorack.


What dadamachines Contributes
-----------------------------

This repository is a fork of
`dadamachines/ctag-tbd <https://github.com/dadamachines/ctag-tbd>`_
(branch ``p4_main``), adapted for the TBD-16 hardware. Our focus:

- **Desktop Hardware** -- Standalone form factor with hardware UI and standard MIDI, no Eurorack or computer required
- **Hardware Products** -- TBD-16 instrument, TBD-Core with FFC connector, custom integrations
- **UI/UX** -- Redesigned web interface with musician-friendly interaction patterns
- **Documentation** -- Clear guides, example workflows, and UX guidelines for
  plugin developers

The DSP engine, plugin system, and core firmware are developed upstream by
Robert Manzke / CTAG.


The Team
--------

**dadamachines** is a team of experts in hardware design, electronics, industrial
design, and UX/UI design, based in Berlin.

- `Johannes Lohbihler / dadamachines <https://dadamachines.com>`_ -- TBD-16 hardware,
  adaptation, UI/UX, documentation
- `Benjamin Weiss / instrument-design <https://instrument-design.com/work/>`_ --
  UX and instrument design

**dadamachines** friends & contributors:

- `Per-Olov Jernberg aka Possan <https://possan.codes/>`_ -- creator of
  PicoSeqRack, the default app/firmware shipping on the TBD-16
- Servando Barreiro
- Justin Mammarella aka jmamma

**CTAG TBD** core contributors:

- Robert Manzke (rma) -- Creator, DSP engine, plugin system
- Mathias Brüssel (mb)
- Lukas Hermann (lh)
- Lars Schubert (ls)


Community & Support
-------------------

- `dadamachines Forum <https://forum.dadamachines.com>`_ -- Ask questions, share
  patches, report bugs, and connect with other TBD users and developers.
- **Discord** -- We have a Discord server that is currently invite-only and will
  be opened to the public soon.
- `GitHub Issues <https://github.com/dadamachines/ctag-tbd/issues>`_ -- Bug
  reports and feature requests for the firmware and documentation.


Funding
-------

This project is partially funded through the `NGI0 Commons Fund <https://nlnet.nl/commonsfund>`_,
established by `NLnet <https://nlnet.nl/>`_ with financial support from the European Commission's
`Next Generation Internet <https://ngi.eu/>`_ programme, under grant agreement
No `101135429 <https://cordis.europa.eu/project/id/101135429>`_.

Not all work on TBD / TBD-16 is covered by NLnet funding.

| `NLnet project page <https://nlnet.nl/project/TBD-DSP-Toolkit/>`_
| `dadamachines TBD Toolkit <https://dadamachines.com/products/tbd-toolkit>`_

.. image:: images/nlnet-banner.png
   :alt: NLnet
   :width: 160px
   :target: https://nlnet.nl/project/TBD-DSP-Toolkit/

.. image:: images/ngi0-commons.svg
   :alt: NGI0 Commons Fund
   :width: 160px
   :target: https://nlnet.nl/commonsfund


.. toctree::
   :hidden:
   :glob:

   [1-9]*
