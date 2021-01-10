# **CTAG TBD >>to be determined<<**

![CTAG-TBD](tbd.png)

## Documentation (in the works):
Documentation on TBD usage can be found [here (Google Docs)](https://docs.google.com/document/d/1c8mjxWjdiJNP0xpkU2CxRUp9av6V4W39wARJf3_SMSo/edit?usp=sharing)
Tutorial videos in [YouTube playlist](https://www.youtube.com/playlist?list=PLB5iCbhcvJ2qdD7s1o9wsvQ9qtsCUWVLR)

## What it is:
- The eierlegende Wollmilchsau (German for swiss army) audio processing module.
- Offering already more than 30 high quality audio generators and effects.
- Open source Eurorack sound module based on the ESP32.
- With an easy to extend audio plugin architecture.
- What you make it.

## Why it is:
- A group of audio enthusiasts enjoying coding and hardware making.
- Build a platform to learn, build and practise skills, and engage students.
- To allow anyone to understand technology by offering open access.
- To squeeze and optimize code so that it can work on a small embedded system.
- Because we can.
- To fight boredom and despair of Corona Virus!

## Features: 
- More than 30 high quality audio generators and effects.
- Easy DSP plugin development.
- **NEW!!!** Sample ROM playback and wavetable oscillator with user wavetables from sample ROM [details here](sample_rom/readme.md)
- **NEW!!!** Simulator for cross platform plugin development without hardware module (speeds up plugin development, better code verification).
    - Check description [here](simulator/readme.md)
- With web UI user interface by wireless browser access.
- Through a REST-API.

## Version log
[See here...](versions.md)

## Potential new features / current limitations / work to be done:
- FAUST support.
- Bug identification and fixing.
- Code refactoring to make things more beautiful.
- More user friendly interaction.
- Documentation / tutorials.
- Your ideas?

## How to engage yourself:
- Join the [enthusiastic developer](https://codewithoutrules.com/2018/11/12/enthusiasts-vs-pragmatists/) team on Github.
- Help build and spread the hardware module (and the word).
- Help documenting.
- More ideas?

## How to build
You need C/C++ skills. You may want to check the [TBD simulator](simulator/readme.md) for easy plugin development without TBD hardware.
Install espressif esp-idf, instructions are [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html).
Use git to checkout esp-idf release/v4.1 (TBD may build also with newer versions, you can try), do this within the esp-idf folder:
```sh
git checkout release/v4.1
git submodule update
```
You may need to upgrade CMake to version 3.17 or higher.
Then follow these steps to create the firmware image:
```sh
git clone https://github.com/ctag-fh-kiel/ctag-tbd.git
cd ctag-tbd
git submodule update --init --recursive
idf.py build
```

## How to flash
Either use the binaries available from the [releases from github](https://github.com/ctag-fh-kiel/ctag-tbd/releases) to flash through the web ui 
or execute these commands from the command line within the ctag-tbd folder, in which case 
the module needs to be connected to the PC with a USB cable:
```sh
idf.py flash monitor
```
Alternatively on Mac / Linux you can also have a look at [this script](bin/flash.sh)