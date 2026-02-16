# dadamachines TBD-16

The first standalone desktop audio DSP platform based on [CTAG TBD](https://github.com/ctag-fh-kiel/ctag-tbd), with standard MIDI connectivity — designed to bring open-source audio processing beyond Eurorack.

**TBD-16** combines 50+ high-quality generators and effects in a modular, extensible architecture. It is built for musicians, educators, and audio researchers who want hands-on DSP without proprietary lock-in.

## Documentation

**[dadamachines.github.io/ctag-tbd](https://dadamachines.github.io/ctag-tbd/)**

## What This Fork Does

This repository is a fork of [ctag-fh-kiel/ctag-tbd](https://github.com/ctag-fh-kiel/ctag-tbd) (branch `p4_main`), adapted for the **dadamachines TBD-16** hardware. Our focus:

- **UI/UX** — Redesigned web interface with musician-friendly interaction patterns
- **Documentation** — Clear guides, example workflows, and UX guidelines for plugin developers
- **Desktop Hardware** — Standalone form factor with standard MIDI, no Eurorack required

The DSP engine, plugin system, and core firmware are developed upstream by [Robert Manzke / CTAG](https://www.creative-technologies.de/).

## Getting Started

See the [documentation](https://dadamachines.github.io/ctag-tbd/) for setup guides, plugin reference, and flashing instructions.

## Project Structure

```
components/         DSP plugins and sound processors
docs/               Sphinx documentation source
main/               Firmware entry point and system management
sdcard_image/       Web UI and configuration data
simulator/          Desktop simulator for plugin development
sample_rom/         Stock audio samples
generators/         Plugin scaffolding templates
```

## Contributing

Contributions are welcome. Please open an issue or pull request. For plugin development, see the [Create Plugins](https://dadamachines.github.io/ctag-tbd/) section in the documentation.

## Acknowledgements

**CTAG TBD** was created by [Robert Manzke](https://github.com/ctag-fh-kiel/ctag-tbd) at the [Creative Technologies Arbeitsgruppe](https://www.creative-technologies.de/), Kiel University of Applied Sciences. 

The TBD-16 adaptation is led by [dadamachines](https://dadamachines.com).

UX and instrument design contributions by [Benjamin Weiss / instrument-design](https://instrument-design.com/work/).

## Funding

This project is partially funded through the [NGI0 Commons Fund](https://nlnet.nl/commonsfund), established by [NLnet](https://nlnet.nl/) with financial support from the European Commission's [Next Generation Internet](https://ngi.eu/) programme, under grant agreement No [101135429](https://cordis.europa.eu/project/id/101135429).

Not all work on TBD / TBD-16 is covered by NLnet funding.

[<img src="https://nlnet.nl/logo/banner-320x120.png" alt="NLnet" width="160">](https://nlnet.nl/project/TBD-DSP-Toolkit/)

## License

The TBD software suite is licensed under the [GNU General Public License (GPL 3.0)](https://www.gnu.org/licenses/gpl-3.0.txt).

The **dadamachines TBD-16** is a commercial product. Its specific hardware design is proprietary. Updated open-source hardware reference designs for the TBD platform are planned for future publication in KiCad.

The original CTAG TBD Eurorack hardware designs (V1/V2) are released under [Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)](https://creativecommons.org/licenses/by-nc-sa/4.0/).

Copyright (c) 2020-2026 Robert Manzke. All rights reserved. (CTAG TBD core)

Copyright (c) 2014-2026 Johannes Elias Lohbihler for dadamachines. (TBD-16 adaptation, UI/UX, and Documentation)

License and copyright details for specific submodules are included in their respective component folders / files if different from this license.
