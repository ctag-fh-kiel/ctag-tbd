*************
Sound Plugins
*************

TBD sound plugins (also just referred to as plugins) are sound processors and generators that can be loaded and switched
at runtime. They are essentially functions with parameters (functors). Sound plugins come in two flavours: mono and
stereo. All plugins receive audio input from both audio input channels, but mono plugins are restricted to write to
the single output channel they are assigned to. Sounds can be bundled into sound sets (each being their own TBD module)
and multiple of these sound sets can be added to a TBD firmware by including them in the firmware config YAML.

The true power of TBD plugins comes from the ability to utilize additional `parameters` that can be set from the UI,
front panel or API. Parameters can be dynamically configured to be live parameters that are mapped to MIDI or voltage
inputs and updated with every sample run, or have statically set values.

Creating a Sound Plugin
=======================

To create your own sound plugin or plugin set, you will need create a sound set first.

1. Create a directory for your custom TBD components somewhere on disk and add a subfolder with the name of your
   sound set (we will use ``my_tbd_components/my_sounds`` in the example).
2. Add the following three files

   - ``my_sounds/__init__.py``
   - ``my_sounds/include/tbd/my_sounds/my_sound.hpp``
   - ``my_sounds/src/my_sounds/my_sound.cpp``

3. Add the following sound set builder plate code to your ``__init__.py``

.. code-block:: python

    from esphome.components.tbd_sound_registry import new_plugin_registry, SOUND_COLLECTION_SCHEMA

    AUTO_LOAD = ['tbd_sound_registry']
    CONFIG_SCHEMA = SOUND_COLLECTION_SCHEMA

    async def to_code(config):
        new_plugin_registry(__file__, config)

4. Create a basic mono plugin, by adding the following to your ``my_sound.hpp``:

.. code-block:: c++

    #pragma once

    #include <tbd/sound_processor.hpp>

    namespace tbd::my_sounds {

    struct MySound : sound_processor::MonoSoundProcessor {
        ~MySound() override = default;

        void init() override;
        void process_mono(const sound_processor::StereoSampleView& input,
                          const sound_processor::MonoSampleView& output) override;
    };

    }

5. Add sound processing to ``my_sound.cpp``:

.. code-block:: c++

    #include <tbd/sounds/test_plugin.hpp>

    namespace tbd::my_sounds {

    void MySound::init() {

    }

    void MySound::process_mono(const sound_processor::StereoSampleView& input,
                            const sound_processor::MonoSampleView& output)
    {
        // add some sound processing here
    }

    }

This plugin will just output the audio input on the output output.

Now that we have a plugin set and a simple plugin set up, all we need to do is add the plugin set to a firmware. Modify
an existing firmware YAML as follows:

.. code-block:: yaml

    external_components:
      - source: <path-to-tbd>
      # ... other external modules
      # add your custom components to component search path
      - source <full-path>/my_tbd_components

    tbd_module:

    # ... other components

    # actually include your component in the firmware
    my_sounds:

Now your plugin will be built and added to the firmware and can be enabled through the UI, front panel or API.


Sound Plugin Basics
===================

Looking at ``my_sound.hpp``, we can identify the four important building blocks of a sound plugin:

1. Sound plugins are classes that need to inherit from ``tbd::sound_processor::MonoSoundProcessor`` or
   ``tbd::sound_processor::MonoSoundProcessor``.
2. Chunks of samples are fed to the ``process_mono`` or ``process_stereo`` methods, where you can do your actual sound
   generation/processing.
3. If your plugin needs to do some initialization like loading sample files, allocating memory, etc you can add an
   ``init`` method that will be called for setup before any samples are fed to the processor.
4. A virtual destructor (``~MySound`` in this case) needs to be provided.

To actually manipulate or generate sound, you can use the ``input`` and ``output`` arguments of the process method.
These can be used just as arrays or ``std::vector``. Each element in these collections contains a single ``float``
sample value.

TODO: examples of sample manipulation


Parameters
==========

To really make sound generation interesting, you can utilize parameters. Say you have an effect like echoing and you
want to set both the duration of the effect and the intensity. Adding these parameters is as simple as adding a public
parameter type field to your class:

.. code-block:: c++

    #pragma once

    #include <tbd/sound_processor.hpp>

    namespace tbd::my_sounds {

    struct MyEcho : sound_processor::MonoSoundProcessor {
        ~MyEcho() override = default;

        void init() override;
        void process_mono(const sound_processor::StereoSampleView& input,
                          const sound_processor::MonoSampleView& output) override;

        ufloat_par duration_ms;
        mufloat_par intensity;
    };

    }

Once you build and flash the firmware, the parameter will be visible in the UI and can be set through the API. Simple
use the current value in your processing function. Parameters are updated once for every processing run.

.. note::

    Do not attempt to set parameters from withing the processing function. Plugins should only read parameters not set
    them.

There are two kinds of parameter types: basic parameters that can not be mapped to live inputs and a corresponding set
of parameters that allow live input, which are prefixed with an m
The following parameter types are supported:
    - ``int_par``/``mint_par``: ``int32_t`` for integer values
    - ``uint_par``/``muint_par``: ``uint32_t`` for non negative integer values
    - ``float_par``/``mfloat_par``: ``float`` for float values
    - ``ufloat_par``/``mufloat_par``: ``float`` for non negative float values
    - ``trigger_par``/``mtrigger_par``: ``bool`` for simple trigger or on/off values
