*********************
Building the Firmware
*********************

The TBD firmware is based on `ESPHome <https://esphome.io/>`_ to allow for configurable hardware specific
builds. At the core of esphome builds is a hardware specific YAML config file. ESPHome will generate a
`PlatformIO <https://platformio.org/>`_ project containing only the components required for a specific firmware flavour.

Manually Building the Firmware
==============================

First clone the TBD repository and navigate to the TBD root directory

.. code-block:: bash

    git clone https://github.com/ctag-fh-kiel/ctag-tbd.git
    cd ctag-tbd

If you want to build the latest testing version of TBD checkout the :code:`dev` branch

.. code-block:: bash

    git checkout dev

To manually build a TBD firmware you will need ESPHome alongside some additional python packages. The TBD repository
comes with a :code:`pyproject.toml` file for use with the `Poetry <https://python-poetry.org/>`_ package manager.
To create a Python venv containing all required python packages use poetry install

.. code-block:: bash

    poetry install

The :code:`tbd_core.cli` module provides a wrapper to invoke ESPHome, without having to manually specify required
parameters and environment variables. Build the firmware for a specific configuration with

.. code-block:: bash

    poetry run tbd build <path-to-config-file>

or generate/refresh config specific build project using

.. code-block::

    poetry run tbd generate <path-to-config-file>
