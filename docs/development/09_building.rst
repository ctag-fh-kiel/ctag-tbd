*********************
Building the Firmware
*********************

There are multiple ways to build and flash TBB applications:

- using an IDE with the provided dev container (advised option)
- using an IDE without a dev container (requires installing all dependencies)
- in terminal using provided build container
- in terminal not using the build container (requires installing all dependencies)

The easiest way to build the firmware is using the TBD  `dev container <https://containers.dev/>`_
provided by the TBD repo. This offers the following advantages:

- tools and system libraries ready to go and with tested
- batteries included: desktop/embedded compilers, emulator, documentation toolchain...
- terminal configuration ready to go
- TBD helper tools ready to go

Make sure you have a working `docker <https://www.docker.com/>`_ installation, before
continuing.

.. note:: 

    Although using the dev container is strongly advised, the amount of tools the container
    needs to provide makes a rather large docker container.


Building the Firmware using Docker
==================================

First clone the TBD repo dev branch (make sure to clone recursively). 
In a terminal Navigate to the desired parent directory of your code and run Git clone:

.. code-block:: shell

    cd desired_parent_dir
    git clone -b dev --recursive https://github.com/ctag-fh-kiel/ctag-tbd.git

Navigate to the TBD project root

.. code-block:: shell

    cd ctag-tbd

The first container build may take a while, because the base container and many of the 
core dependencies need to be downloaded. To proceed run

.. code-block:: shell

    docker buildx build -t tbddev .devcontainer

Once the build has completed you can build any variant of the TBD firmware from the 
project root dir as

.. code-block:: shell

    docker run -it -v$PWD:/code tbddev tbb dada configure build

Note that in order to build any other TBD variant just replace ``dada`` with the desired 
variant.

To flash your firmware to a TBD device you need to make the usb connection available to 
the docker container. On Windows and OSX this process 
`can be rather complex <https://docs.espressif.com/projects/esp-idf/en/v5.4-beta1/esp32/api-guides/tools/idf-docker-image.html>`_
and it can be advisable to simply install `esptool.py` or use any other suitable method
from instead of docker.

For linux users just make the usb device available to the container using ``--device``.
Typically the device can be found in ``/dev/ttyUSB0``. To flash run 

.. code-block:: shell

    docker run --device /dev/ttyUSB0 -it -v$PWD:/code tbddev tbb dada flash

You can then view the device log using 

.. code-block:: shell

    docker run --device /dev/ttyUSB0 -it -v$PWD:/code tbddev tbb dada monitor


Building the Firmware without Docker
====================================

Building using tbb Helper
-------------------------

The ``tbb`` tool is a small utility that helps building for different platforms, without
having to enter long ``cmake`` invocations. The tool is located in ``tools/bin``. Add
the tool to your system ``PATH`` or enter full path to tool.

To setup and build use

.. code-block:: shell

    tbb some_platform configure build

tbb Tool Arguments
------------------

Usage: ``tbb [-p DIR] [-b DIR] [--silent] stage1 [stage2 ...]``

You need to specify at least one or multiple build stages as arguments. The order of those
arguments does not have any effect and build stages will be performed in the required order.
Build stages can be

- ``configure``: rum cmake to generate build description
- ``build``: build the binaries and on embedded platforms create binary image

Options
.......

``--silent`` do not show processing output
  The processing steps generate a lot of output. To hide this output set this flag.

``-p DIR``, ``--project-dir DIR`` project directory
  The root directory of the TBD source code. If this option is not present the directory
  will be determined in the following order:

  1. value of the ``TBD_PROJECT_DIR`` environment variable
  2. root of the Git repository containing the current working directory
  3. current working directory

``-b DIR``, ``--build-dir DIR`` shared root directory for all builds
  Default is ``build/`` relative to the current working directory.  


Building Manually using CMake
-----------------------------

This can be done from the dev container or from your host system (if all required dependencies
for building are installed). First set up the build files using CMake from your project
directory

.. code-block:: shell

    cmake -G Ninja -Bbuild/dada  -DTBD_PLATFORM=dada

replacing ``dada`` with the desired TBD variant.

All three options to cmake are important:

``-G Ninja``:
    Use Ninja as the builder and build description format. This is not only strongly
    advised but also required for some of the esp32 tools that automatically get invoked
    by the esp32 CMake library.

``-B``: 
    Specify the build directory. If you do not provide this option all build files will be
    placed in the default ``build`` folder which will mess up builds for other platforms
    
``-DTBD_PLATFORM``:
    Specify the TBD variant you want to build.

Once the build is configured you can run the actual build:

.. code-block:: shell

    cmake --build build/dada -t ctag-tbd.elf

where ``build/dada`` is your previously defined build directory. 

.. note::
    The build target ``ctag-tbd.elf`` is common for all device firmware builds. For desktop 
    builds the target name can differ.

Alternatively you can use the idf.py tool to build, flash and monitor the firmware:

.. code-block:: shell

    idf.py -B build/dada -D TBD_PLATFORM=dada build flash monitor

To install all Python dada dependencies on top of the IDF dependencies, run

.. code-block:: shell

    ./tools/resources/src/tbd_deps.sh

copy the output in the command line to install the dependencies.


