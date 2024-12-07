********
Overview
********

This section is intended for contributors and people who wish to port the TBD or extend
the core functionality. For information on how to implement your own sounds and effects
consult the Plugins section of the documentation.


Platforms and Toolchains
========================

A specific TBD software variant is called a *platform*. This is not to be confused with
a hardware platform as in ``x86`` and ``xtensa``. These later are refered to as
*toolchains*. There exist a multitude of esp32 TBD boards with different pinouts and
peripherals. Each of these if a *platform*. Each variant of the desktop software
is also a different TBD *platform*. For embedded TBD variants each platform corresponds
to a single board and toolchain. The desktop variants may be built by different toolchains
(OSX/Windows/Linux).

How to Develop
==============

It is strongly advised to use the TBD development Docker container. You can either use
the container as a dev container in conjunction with an API or with docker run to build,
emulate, flash, etc. For more information see the :doc:`building <09_building>`

The development process on core functionality typically has multiple phases:

1. working on TBD library code and debugging testing locally using the *desktop* and 
   *simulator* targets
2. verifying that builds will work on embedded devices and easily debugging 
   architecture/port specific problems using the *emu* target for hardware emulation
3. flashing to embedded devices and making sure the changes also work with real hardware
   peripherals and ensuring the sound output and performance are not negatively impacted

TBD uses CMake/Ninja for all of its builds. Every IDE that can handle either of these, 
should work for TBD development. In addition there are several tools and scripts available 
to simplify working on TBD:

- ``tbd``: main tool for many TBD development related tasks
- ``tbb``: (to be built): shorthand for ``tbd firmware`` with the same functionality
- ``tbe``: (to be emulated): tool for running the *emu* target, for testing and debugging 
    in an esp32 environment


Important Folders
=================

The TBD source is organised in a monorepo at `<https://github.com/ctag-fh-kiel/ctag-tbd>`_.
All TBD variants are build using CMake, but may require additional libraries and tools
to build. The most important folders in the repo are:

``apps/``
    Main implementations for different TBD variants. Several platforms can share an
    implementation (all esp32 platforms do). These implementations should be simple and
    only glue different tbd libraries together. Implementing different hardware or
    architecture support should be done in ``ports``.

``config/``
    Contains device description files as well as additional configuration files required
    by specific platform (esp32 sdkconfig files). Each supported TBD platform is defined
    through a ``config/platforms`` file. New platforms are specified and added by placing
    a description file here.

``docs/``
    Main documentation. Contains all reference documentation as a sphinx documentation
    tree.

``ports/``
    Implementations of library functionality that is platform dependent. Subfolder names
    match those found in ``tbd``.

``tbd/``
    TBD libraries and headers required to put together a TBD application. These libraries
    can contain unimplemented headers, that need to be implemented in platform ports. All
    TBD libraries are prefixed ``tbd``. There should be no platform dependent code in
    here.

``tools/``
    Helpers and tools both for building, developing as well as for using TBD.

``vendor/``
    3rd party libraries. ALthough the preferred method to include libraries is currently
    to use Git submodules, ome of these libraries are had to be altered in order to work
    with TBD.

``.devcontainer``
    Contains the devcontainer configuration ``devcontainer.json`` as well as the official
    TBD development Dockerfile.
