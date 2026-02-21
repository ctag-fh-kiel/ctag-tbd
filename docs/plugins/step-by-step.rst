*************************
Creating a Plugin
*************************

This guide walks you through creating a new TBD plugin from scratch.

Prerequisites
=============

-  Node.js installed (for the code generator).
-  The TBD repository cloned with submodules (see :doc:`Plugin Development Setup </plugins/getting-started>`).
-  ESP-IDF environment configured.

Step 1: Define the UI
=====================

Create your MUI file by copying the template:

.. code-block:: bash

   cp generators/mui-template.jsn sdcard_image/data/sp/mui-MyPlug.jsn

Edit ``mui-MyPlug.jsn`` to define your plugin's parameters:

.. code-block:: json

   {
     "id": "MyPlug",
     "isStereo": false,
     "name": "My Plugin",
     "hint": "A demo plugin",
     "params": [
       {
         "id": "gain",
         "name": "Gain",
         "type": "int",
         "min": 0,
         "max": 4095
       },
       {
         "id": "bypass",
         "name": "Bypass",
         "type": "bool"
       }
     ]
   }

Step 2: Generate Code
=====================

Run the code generator from the ``generators/`` folder:

.. code-block:: bash

   cd generators
   node generator.js mui-MyPlug.jsn

This creates three files in the current directory:

-  ``ctagSoundProcessorMyPlug.hpp`` --- Header with atomic parameter variables.
-  ``ctagSoundProcessorMyPlug.cpp`` --- Source with parameter mapping in ``knowYourself()``.
-  ``mp-MyPlug.jsn`` --- Default preset file.

Step 3: Move Files
==================

Move the generated files to the correct directories:

.. code-block:: bash

   mv ctagSoundProcessorMyPlug.hpp ../components/ctagSoundProcessor/
   mv ctagSoundProcessorMyPlug.cpp ../components/ctagSoundProcessor/
   mv mp-MyPlug.jsn ../sdcard_image/data/sp/

Step 4: Implement Process()
===========================

Open ``components/ctagSoundProcessor/ctagSoundProcessorMyPlug.cpp`` and implement
your audio processing:

.. code-block:: cpp

   #include "ctagSoundProcessorMyPlug.hpp"
   #include <cmath>

   using namespace CTAG::SP;

   void ctagSoundProcessorMyPlug::Process(const ProcessData &data) {
       // Read parameters (with CV/trigger modulation support)
       MK_FLT_PAR_ABS(fGain, gain, 4095.f, 1.f);
       MK_BOOL_PAR(bBypass, bypass);

       if (bBypass) {
           // Output silence
           for (int i = 0; i < bufSz; i++) {
               data.buf[i * 2 + processCh] = 0.f;
           }
           return;
       }

       // Process audio
       for (int i = 0; i < bufSz; i++) {
           float sample = data.buf[i * 2 + processCh];
           data.buf[i * 2 + processCh] = sample * fGain;
       }
   }

Step 5: Register the Plugin
============================

The plugin must be registered in the factory. Reset the plugin cache so
the system discovers your new plugin on the next boot:

1.  Open ``sdcard_image/data/spm-config.jsn``.
2.  Delete the ``"availableProcessors": [ ... ],`` block.
3.  Save the file.

Step 6: Build and Test
======================

Build the firmware and flash it:

.. code-block:: bash

   idf.py build
   idf.py flash monitor

Or test with the simulator:

.. code-block:: bash

   cd simulator && mkdir -p build && cd build
   cmake .. && make
   ./tbd-sim

Your plugin should now appear in the Web UI plugin selector.

Modifying Parameters Later
==========================

If you add or remove parameters from your MUI file after the initial creation,
use the in-place update mode of the generator:

.. code-block:: bash

   cd generators
   node generator.js MyPlug -i

.. warning::

   The ``-i`` flag overwrites the preset file ``mp-MyPlug.jsn``.
   Back up any custom presets before running this.

Plugin Design Patterns
======================

Self-Contained / Monolithic Plugins
------------------------------------

A plugin can include its own sequencer, arpeggiator, or any internal logic.
The ``Process()`` method has full control over the audio buffer. Use internal
state variables to maintain sequencer position, envelope state, etc.

DSP-Only Plugins
----------------

Simpler plugins focus exclusively on audio processing (filters, effects,
oscillators) and rely on external control from the RP2350 firmware via
MIDI or the ``controlData`` pointer in ``ProcessData``.

Using Samples
-------------

Plugins can access audio samples from the SD card via the ``ctagSampleRom``
helper class. At boot, the system reads WAV files listed in the active sample
and wavetable banks, loads them into PSRAM, and exposes them as indexed slices.

.. code-block:: cpp

   #include "helpers/ctagSampleRom.hpp"

   // In your Init() method:
   sampleRom = std::make_unique<ctagSampleRom>();

   // In your Process() method:
   uint32_t nSlices = sampleRom->GetNumberSlices();
   uint32_t sliceSize = sampleRom->GetSliceSize(sliceIndex);
   sampleRom->ReadSliceAsFloat(buffer, sliceIndex, offset, nSamples);

Key concepts:

- **Wavetable slices** occupy indices 0 to ``GetFirstNonWaveTableSlice() - 1``.
  Each wavetable bank file contains 64 wavetables × 256 samples.
- **Sample slices** start at ``GetFirstNonWaveTableSlice()``. Each slice is one
  WAV file from the active sample bank.
- **Bank switching** is done via ``SetActiveWaveTableBank()`` /
  ``SetActiveSampleBank()`` followed by ``RefreshDataStructure()``.
- Sample data is stored as **16-bit integer** in PSRAM. Use ``ReadSliceAsFloat()``
  to get normalized float output.
- Total PSRAM allocation for samples is configured at build time
  (``CONFIG_MAX_ALLOC_BYTES_PSRAM_SAMPLE_DATA``).

See ``ctagSoundProcessorRompler`` and ``ctagSoundProcessorWTOsc`` for
real-world usage examples.

SD Card Sample Layout
~~~~~~~~~~~~~~~~~~~~~

Samples live in ``/sdcard/tbdsamples/`` on the P4 SD card. The structure is
defined by JSON descriptor files:

- ``sample_rom.jsn`` --- Master index with bank lists and active bank indices
- Bank files (e.g. ``def_smp.jsn``, ``def_wt.jsn``) --- Arrays of sample entries
  with ``filename``, ``path``, ``nsamples``, and optional ``offset``

See ``helpers/ctagSampleRomModel`` for the full data model implementation.
