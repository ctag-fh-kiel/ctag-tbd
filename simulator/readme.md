# CTAG-TBD Simulator

Useful for plugin development without the need to use hardware module.

## Requirements 

Full duplex sound card running at 44100Hz sampling rate and 32-bit float sampling.
Adaptations can be made in the code, RTAudio will open the first available device with such capabilities.

## Dependencies

* Boost Asio, FileSystem, Thread, ProgramOptions
* RTAudio
* Simple-Web-Server (by C. Eidheim)
* CMake Version 3.16 or higher

Installation instructions for the dependencies needed to compile the simulator on a selection of platforms can be seen below.

### Debian based distributions

```sh
sudo apt-get install libboost-filesystem-dev libboost-thread-dev libboost-program-options-dev libasound2-dev
```

### Arch Linux based distributions

```sh
sudo pacman -S boost
```

### MacOS

```sh
brew install boost
```

### Windows

#### Approach 1 (64bit)
Install [msys2.org](https://www.msys2.org) and launch the MinGW 64-bit shell from the Start menu, not the default MSYS shell. Update the package manager itself:
```sh
pacman -Syu
```

Then restart the shell and install packages:
```sh
pacman -Su git mingw-w64-x86_64-make mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-libtool mingw-w64-x86_64-jq mingw-w64-x86_64-boost
```

When running cmake use:
```sh
cmake -G "MinGW Makefiles" ..
```
instead of just using "cmake .." as written below.

To make run:
```sh
mingw32-make
```

#### Approach 2 (32bit)
- Getting the simulator running on Windows has been tested using the MingW GCC environment (32-bit)
- Tests with the MS Visual Studio Compiler resulted in many errors
- Here are the steps which lead to a success
    - Required tools / steps:
        - JetBrains CLion IDE (could work also with others)
        - MingW follow these steps (but use 32-bit, i686)
            - https://www.jetbrains.com/help/clion/quick-tutorial-on-configuring-clion-on-windows.html
        - Install Boost, follow these steps (update the version number accordingly and use the 32-bit MingW locations)
            - https://gist.github.com/zrsmithson/0b72e0cb58d0cb946fc48b5c88511da8
        - Set Boost_DIR environment variable (e.g. Boost_DIR=c:\boost)
            - https://www.jetbrains.com/help/clion/add-environment-variables-and-program-arguments.html

## Clone with complete CTAG-TBD source repository

```sh
git clone https://github.com/ctag-fh-kiel/ctag-tbd.git
cd ctag-tbd
git submodule update --init --recursive
```

## Compile and run

Compile with a C++17 compliant compiler:
```sh
cd simulator (from ctag-tbd folder)
mkdir build
cd build
cmake ..
make
./tbd-sim
```

## Usage

After ./tbd-sim command is executed, a local web server is running on port 8080. Simply open a browser and 
enter [http://localhost:8080]. You will land on exactly the same web UI used by the hardware module.
CV, Pot and Trig simulation is possible by adjusting settings on the URL [http://localhost:8080/ctrl].
You can use a .wav file (stereo float32) using the -w option if you want to simulate the audio input instead of using the
real-time data of the sound card.
The simulator uses 99% the same code as the hardware unit. Only difference is the plugin manager and the web server part.
You can start developing your plugins just like you do for the hardware module, same directory and file structures.
You can run your plugin prior to flashing it to the hardware on your local host. Once the plugin is stable, you build
and flash your project for the real hardware. 
The simulator greatly speeds up plugin development.

## Options
```sh
  -h [ --help ]            this help message
  -l [ --list ]            list sound cards
  -d [ --device ] arg (=0) sound card device id, default 0
  -o [ --output ]          use output only (if no duplex device available)
  -s [ --srom ] arg        file to emulate sample rom
  -w [ --wav ] arg         read audio in from wav file (arg), must be 2 channel
                           stereo float32 data, will be cycled through 
                           indefinitely
  -o [ --output ]          use output only (if no duplex device available)
```

## Limitations

* General module configuration not supported

## Credits
* Demo wav files by 
    * https://freesound.org/people/vacuumfan7072/sounds/322128/
    