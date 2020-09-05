# CTAG-TBD Simulator

Useful for plugin development without the need to use hardware module.

## Requirements 

Full duplex sound card running at 44100Hz sampling rate and 32-bit float sampling.
Adaptations can be made in the code, RTAudio will open the first available device with such capabilities.

## Dependencies

* Boost.Asio
* RTAudio
* Simple-Web-Server (by C. Eidheim)

Installation instructions for the dependencies needed to compile the simulator on a selection of platforms can be seen below.

### Debian based distributions

```sh
sudo apt-get install libboost-filesystem-dev libboost-thread-dev
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

try yourself

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
    