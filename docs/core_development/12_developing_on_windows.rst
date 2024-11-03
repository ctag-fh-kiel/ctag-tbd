**************************
TBD Development on Windows
**************************

Instructions on how-to setup up windows dev environment including esp idf and mingw-gcc for simulator.

.. note::
    these instructions are an alternative to setting up esp-idf as compared to espressif sources `here <https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/>`_

Required knowledge about MSYS2
==============================

Msys2 offers several subsystems for development:

- MSYS2 shell, (mostly) posix cygwin based environment
- MINGGW native windows environment
- There are dedicated toolchains for each environment.
- ESP-IDF uses separate xtensa cross compiler


Setting up ESP IDF development environment
=============================================

- Install [msys2.org](msys2.org)
- Start mingw64 console (not MSYS2)
- Execute, which may require restart of mingw64 console:

.. code-block:: shell

    pacman -Syu

Execute to install required packages:

.. code-block:: shell
    
    pacman -S mingw-w64-x86_64-python mingw-w64-x86_64-ninja mingw-w64-x86_64-python-pip unzip msys2-runtime-devel gettext-devel ncurses-devel wget flex bison gperf vim winpty git mingw-w64-x86_64-make mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-libtool mingw-w64-x86_64-jq mingw-w64-x86_64-boost

Clone and checkout idf release 4.1:

.. code-block:: shell
    
    git clone https://github.com/espressif/esp-idf.git
    cd esp-idf
    git checkout release/v4.1
    git submodule update --init --recursive

Install xtensa toolchain:

.. code-block:: shell

    cd /opt
    wget https://dl.espressif.com/dl/xtensa-esp32-elf-gcc8_4_0-esp-2020r3-win32.zip
    unzip https://dl.espressif.com/dl/xtensa-esp32-elf-gcc8_4_0-esp-2020r3-win32.zip
    wget https://github.com/espressif/binutils-esp32ulp/releases/download/v2.28.51-esp-20191205/binutils-esp32ulp-win32-2.28.51-esp-20191205.zip
    unzip https://github.com/espressif/binutils-esp32ulp/releases/download/v2.28.51-esp-20191205/binutils-esp32ulp-win32-2.28.51-esp-20191205.zip

- Edit .bashrc in ~ directory (home) and add at end of file:

.. code-block:: shell

    export IDF_PATH=$HOME/esp-idf
    export python=/mingw64/bin/python
    export PATH="$IDF_PATH/tools:/opt/esp32ulp-elf-binutils/bin:/opt/xtensa-esp32-elf/bin:$PATH"

Execute:

.. code-block:: shell

    source ~/.bashrc

Install pip requirements:

.. code-block:: shell

    cd ~/esp-idf
    python -m pip install -r requirements.txt


Build TBD binaries
==================

.. code-block:: shell

    cd ~
    git clone https://github.com/ctag-fh-kiel/ctag-tbd.git
    cd ctag-tbd
    git submodule update --init --recursive
    idf.py build

Manually build spiffs image, it is somehow not correctly built with idf.py!
---------------------------------------------------------------------------

.. code-block:: shell

    $IDF_PATH/components/spiffs/spiffsgen.py 0x300000 build/spiffs_image build/storage.bin

Flash TBD binaries
------------------

.. note::
    You have to adapt your COM port in flash.sh and press 'y' when asked to copy fresh binaries

.. code-block:: shell

    cd ~/ctag-tbd/bin
    ./flash.sh

Build TBD simulator
-------------------

.. code-block:: shell

    cd ~/ctag-tbd/simulator
    mkdir build
    cd build
    cmake .. -G "MinGW Makefiles"
    mingw32-make

Run TBD simulator
-----------------

.. code-block:: shell

    ./tbd-sim

Limitations
-----------

- idf.py menuconfig does not work due to missing POSIX environment
- This may be fixed when using msys2 environment instead of mingw
- **TODO**: msys2 tests for idf.py menuconfig

pacman -S openssl-devel libffi-devel libcrypt-devel gettext-devel gcc git make ncurses-devel flex bison gperf vim mingw-w64-i686-python-pip mingw-w64-i686-python-cryptography unzip winpty mingw-w64-i686-gcc

