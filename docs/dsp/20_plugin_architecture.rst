********************
Plugin Architecture
********************

This section explains the TBD plugin system and everything you need to create your own
audio processors.

Overview
========

Every plugin in the TBD system is a C++ class that inherits from
``CTAG::SP::ctagSoundProcessor``. The system calls two methods on your plugin:

-  ``Init()`` --- Called once after the plugin is created. Set up your DSP state here.
-  ``Process()`` --- Called repeatedly with a buffer of audio samples to fill.

Plugins run on the **ESP32-P4** at 48 kHz, processing audio in blocks of 32 samples.

Key Concepts
============

Dual-Channel Architecture
-------------------------

The TBD has **two independent processing channels** (Channel 0 and Channel 1).
Each channel can run a different plugin. A plugin can be either:

-  **Mono** --- Processes one channel. Both channels can run different mono plugins.
-  **Stereo** --- Processes both channels as a stereo pair. Only one stereo plugin
   runs at a time, occupying both channels.

Parameter System
----------------

Each plugin declares its parameters in a **MUI (Menu UI) JSON file** (``mui-PluginName.jsn``).
Parameters can be:

-  ``int`` --- Integer values with min/max range. Can be modulated via a **CV** input.
-  ``bool`` --- On/off toggles. Can be modulated via a **trigger** input.

The Web UI is auto-generated from the MUI file. You never need to write HTML.

.. note::

   The "CV" and "trigger" terms come from the original CTAG TBD Eurorack module.
   On the TBD-16, these modulation inputs are driven by the **RP2350** front-end
   (sequencers, arpeggiators, MIDI mapping) rather than physical CV jacks.

At runtime, parameters are stored as ``atomic<int32_t>`` member variables. The code
generator creates these automatically from your MUI file.

Parameter Macros
~~~~~~~~~~~~~~~~

In your ``Process()`` method, use these macros to read parameters with optional
modulation:

.. code-block:: cpp

   // Boolean parameter with trigger override
   MK_BOOL_PAR(fEnabled, enable);

   // Float parameter, absolute CV (0..1 maps to 0..scale)
   MK_FLT_PAR_ABS(fCutoff, cutoff, 4095.f, 1.f);

   // Float parameter with bipolar CV (-1..1)
   MK_FLT_PAR(fAmount, amount, 4095.f, 1.f);

   // Integer parameter with absolute CV
   MK_INT_PAR_ABS(iMode, mode, 3);

Memory Allocation
-----------------

Plugins use a custom arena allocator to avoid heap fragmentation when switching
between plugins. The ``Init()`` method receives ``blockSize`` and ``blockPtr`` ---
a pre-allocated memory block you can use for DSP buffers.

For larger allocations (reverb buffers, delay lines), use:

.. code-block:: cpp

   heap_caps_malloc(size, MALLOC_CAP_SPIRAM);

Free explicitly in your destructor.


File Structure
==============

Each plugin consists of these files:

.. code-block:: text

   sdcard_image/data/sp/
       mui-MyPlugin.jsn       # UI definition (parameters, groups, hints)
       mp-MyPlugin.jsn        # Preset storage (auto-generated)

   components/ctagSoundProcessor/
       ctagSoundProcessorMyPlugin.hpp    # Header file
       ctagSoundProcessorMyPlugin.cpp    # Implementation


The MUI File
============

The MUI file defines your plugin's identity and parameters. Example:

.. code-block:: json

   {
     "id": "MyPlugin",
     "isStereo": false,
     "name": "My Plugin",
     "hint": "A simple example plugin",
     "params": [
       {
         "id": "gain",
         "name": "Gain",
         "hint": "Output gain level",
         "type": "int",
         "min": 0,
         "max": 4095
       },
       {
         "id": "enable",
         "name": "Enable",
         "hint": "Enable processing",
         "type": "bool"
       },
       {
         "id": "filter",
         "name": "Filter",
         "type": "group",
         "params": [
           {
             "id": "cutoff",
             "name": "Cutoff",
             "type": "int",
             "min": 0,
             "max": 4095
           },
           {
             "id": "resonance",
             "name": "Resonance",
             "type": "int",
             "min": 0,
             "max": 4095
           }
         ]
       }
     ]
   }

Rules:

-  ``id`` must be short (max 8 characters due to filesystem constraints).
-  ``isStereo`` determines whether the plugin occupies one or both channels.
-  Parameters of type ``group`` create collapsible sections in the Web UI.
-  ``type: "int"`` parameters get a CV mapping option; ``type: "bool"`` get a trigger mapping.
