******************************
Credits & Open-Source Licenses
******************************


Funding
=======

This project is partially funded through the `NGI0 Commons Fund <https://nlnet.nl/commonsfund>`_,
established by `NLnet <https://nlnet.nl/>`_ with financial support from the European Commission's
`Next Generation Internet <https://ngi.eu/>`_ programme, under grant agreement
No `101135429 <https://cordis.europa.eu/project/id/101135429>`_.

Not all work on TBD / TBD-16 is covered by NLnet funding.

| `NLnet project page <https://nlnet.nl/project/TBD-DSP-Toolkit/>`_
| `dadamachines TBD Toolkit <https://dadamachines.com/products/tbd-toolkit>`_


Open-Source Licenses
====================

The hardware and software that make up the TBD platform are released under
open-source licenses, meaning they can be used, studied, and remixed --- so
long as the conditions of each license are followed.

This page gives an overview of the different licenses that apply and explains
what they mean if you want to contribute, build your own instrument, or use
TBD in a commercial product.


Software
--------

This repository contains code under two open-source software licenses:


Core DSP Engine
^^^^^^^^^^^^^^^

The audio engine, sound processors, and platform core --- originally developed
at `CTAG Kiel <https://www.creative-technologies.de>`_ --- are licensed under the
`GNU General Public License (GPL 3.0) <https://www.gnu.org/licenses/gpl-3.0.txt>`_.

This is the upstream license chosen by Robert Manzke for the
`ctag-fh-kiel/ctag-tbd <https://github.com/ctag-fh-kiel/ctag-tbd>`_ repository.
Modifications to this code must be released under the same GPL 3.0 terms.


dadamachines Additions
^^^^^^^^^^^^^^^^^^^^^^

All code, tools, and assets **added by dadamachines** in this repository ---
including the web UI, browser-based flasher, documentation source, build
tools, and utilities --- are licensed under the
`GNU Lesser General Public License (LGPL 3.0) <https://www.gnu.org/licenses/lgpl-3.0.txt>`_.

We chose LGPL because it strikes the right balance:

- **Individual developers** can freely use, modify, and contribute to this
  code without being required to release their own unrelated projects under
  the same license.
- **Modifications to the LGPL'd code itself** must be shared under the same
  terms if distributed --- ensuring improvements flow back to the community.
- **Other companies** may not take these components and use them commercially
  without contributing their modifications back as open source.

This approach is similar to how `Bela <https://bela.io>`_ licenses their core
code under LGPL while keeping the IDE under GPL.


Hardware
--------

Original CTAG TBD Eurorack Designs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The original CTAG TBD Eurorack hardware designs (V1/V2) by Robert Manzke are
released under the
`Creative Commons Attribution-NonCommercial-ShareAlike 4.0
International (CC BY-NC-SA 4.0)
<https://creativecommons.org/licenses/by-nc-sa/4.0/>`_.


dadamachines TBD-16
^^^^^^^^^^^^^^^^^^^

The dadamachines TBD-16 is a commercial product. Its specific hardware design ---
including the industrial design, PCB layout, and custom electronics --- is
proprietary and not released under an open-source license.

However, updated open hardware **reference designs** for the TBD platform are
planned for future publication in KiCad to foster education, prototyping, and
instrument design.


Using TBD in Commercial Products
--------------------------------

You are free to buy TBD-16 units and resell them inside a commercial product,
just like any other off-the-shelf electronic component. We offer volume pricing
and :doc:`custom integrations </hardware/30_custom_integration>` for companies
building products around the TBD platform.

If you **modify the dadamachines-added code** (web UI, tools, documentation)
and distribute it, those modifications must be released under LGPL 3.0.

If you **modify the core DSP engine** and distribute it, those modifications
must be released under GPL 3.0.

If you would like to keep your modifications to either component **proprietary**,
dadamachines can provide a commercial license tailored to your project and scope.
`Contact dadamachines <https://dadamachines.com/contact/>`_ to discuss your needs.


The dadamachines Name and Brand
-------------------------------

The dadamachines name, logo, and TBD-16 product name are trademarks and are
reserved for products produced by or licensed by dadamachines.

If you build a product using the TBD platform, you may reference it as:

- **[YourProduct] for TBD**
- **[YourProduct] (TBD-compatible)**

Using "dadamachines [YourProduct]" or "TBD-16 [YourProduct]" is not permitted
without prior agreement. If you are unsure,
`contact us <https://dadamachines.com/contact/>`_.


Copyright
---------

| Copyright (c) 2020--2026 Robert Manzke. All rights reserved. (Core platform)
| Copyright (c) 2014--2026 Johannes Elias Lohbihler for dadamachines. (TBD-16 adaptation, UI/UX, Documentation)

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
