# How-to setup up windows dev environment including esp idf and mingw-gcc for simulator
Note that these instructions are an alternative to setting up esp-idf as compared to espressif sources [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/)
## Create esp-idf dev env
- Install [msys2.org](msys2.org)
- Start mingw64 console
- Execute, which may require restart of mingw64 console:
```
pacman -Syu
```
- Execute to install required packages:
```
pacman -S mingw-w64-x86_64-python mingw-w64-x86_64-ninja mingw-w64-x86_64-python-pip unzip msys2-runtime-devel gettext-devel ncurses-devel wget flex bison gperf vim winpty git mingw-w64-x86_64-make mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-libtool mingw-w64-x86_64-jq mingw-w64-x86_64-boost
```
- Clone and checkout idf release 4.1:
```
git clone https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout release/v4.1
git submodule update --init --recursive
```
- Install requirements:
```
python -m pip install -r requirements.txt
```
- Install xtensa toolchain:
```
cd /opt
wget https://dl.espressif.com/dl/xtensa-esp32-elf-gcc8_4_0-esp-2020r3-win32.zip
unzip https://dl.espressif.com/dl/xtensa-esp32-elf-gcc8_4_0-esp-2020r3-win32.zip
wget https://github.com/espressif/binutils-esp32ulp/releases/download/v2.28.51-esp-20191205/binutils-esp32ulp-win32-2.28.51-esp-20191205.zip
unzip https://github.com/espressif/binutils-esp32ulp/releases/download/v2.28.51-esp-20191205/binutils-esp32ulp-win32-2.28.51-esp-20191205.zip
```
- Edit .bashrc in ~ directory (home) and add at end of file:
```
export IDF_PATH=$HOME/esp-idf
export python=/mingw64/bin/python
export PATH="/opt/esp32ulp-elf-binutils/bin:/home/rma/esp-idf/tools:/opt/xtensa-esp32-elf/bin:$PATH"
```
- Execute:
```
source ~/.bashrc
```
## Build TBD binaries
```
cd ~
git clone https://github.com/ctag-fh-kiel/ctag-tbd.git
cd ctag-tbd
git submodule update --init --recursive
idf.py build
```
## Flash TBD binaries
- Note: You have to adapt your COM port in flash.sh and press 'y' when asked to copy fresh binaries
```
cd ~/ctag-tbd/bin
./flash.sh
```
## Build TBD simulator
```
cd ~/ctag-tbd/simulator
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```
## Execute TBD simulator
```
./tbd-sim
```
## Limitations
- idf.py menuconfig does not work due to missing POSIX environment

