######################
dadamachines TBD-16
######################

The first standalone desktop audio DSP platform based on `CTAG TBD <https://github.com/ctag-fh-kiel/ctag-tbd>`_,
with standard MIDI connectivity — designed to bring open-source audio processing beyond Eurorack.

**TBD-16** combines 50+ high-quality generators and effects in a modular, extensible architecture.
It is built for musicians, educators, and audio researchers who want hands-on DSP without proprietary lock-in.


What This Fork Does
===================

This repository is a fork of `ctag-fh-kiel/ctag-tbd <https://github.com/ctag-fh-kiel/ctag-tbd>`_ (branch ``p4_main``),
adapted for the **dadamachines TBD-16** hardware. Our focus:

- **UI/UX** — Redesigned web interface with musician-friendly interaction patterns
- **Documentation** — Clear guides, example workflows, and UX guidelines for plugin developers
- **Desktop Hardware** — Standalone form factor with standard MIDI, no Eurorack required

The DSP engine, plugin system, and core firmware are developed upstream by
`Robert Manzke / CTAG <https://www.creative-technologies.de/>`_.


Getting Started
===============

See the :doc:`Get Started <get_started/10_initial_setup>` section for setup guides, plugin reference,
and flashing instructions.


Contributing
============

Contributions are welcome. Please open an issue or pull request on
`GitHub <https://github.com/dadamachines/ctag-tbd>`_.
For plugin development, see the :doc:`Create Plugins <create_plugins/10_prerequisites>` section.


.. toctree::
    :hidden:
    :caption: Get Started
    :maxdepth: 4
    :glob:

    get_started/*


.. toctree::
    :hidden:
    :caption: Plugins
    :maxdepth: 2
    :glob:

    sound_library/*


.. toctree::
    :hidden:
    :caption: Create Plugins
    :maxdepth: 2
    :glob:

    create_plugins/*


.. toctree::
    :hidden:
    :caption: Development
    :maxdepth: 2
    :glob:

    development/*


.. toctree::
    :hidden:
    :caption: About
    :maxdepth: 2
    :glob:

    about/*


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
