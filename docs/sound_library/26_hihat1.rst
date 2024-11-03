*************
HiHat1 (Mono)
*************

Description
===========

Simple hihat model based on noise shaping.

Parameters
==========

Section 1
---------

**White/Pink Noise:** Selects the noise source for the hihat
sound. Pink noise is a bit smoother.

**Frequency / Hz:** Sets the noise/hihat frequency. Can be
changed/modulated via CV and assigned to potis.

**Q Factor**: Sharpens the sound with resonance. Smaller values
equal sharper sounds.

**Loudness**: Sets the loudness of the sound.

Section 2: EG Loudness
----------------------

**Enable EG:** If active, the sound volume can be controlled via
an AD envelope.

**Loop EG:** Loops the envelope after the first trigger (set via
the drop-down menu right to **Enable EG**).Turns the plugin into a noise
generator when on and **Enable EG** is off.

**Attack:** Attack of the envelope. Determines how long it takes
for the sound to reach its highest level. Can be modulated via CV and
assigned to potis.

**Decay**: Decay of the envelope. Determines how long it takes
for the sound to die down after reaching its highest level. Can be
modulated via CV and assigned to potis.

Section 3: EG Pitch
-------------------

**Enable EG:** If active, the hihat pitch can be controlled via
an AD envelope.

**Loop EG:** Loops the pitch envelope after the first trigger.

**Amount:** Determines the amount of pitch change. Can be
modulated via CV and assigned to potis.

**Attack:** Attack of the envelope. Can be modulated via CV and
assigned to potis.

**Decay**: Decay of the envelope. Can be modulated via CV and
assigned to potis.

Usage/Patch Ideas
=================

Patch idea 1: Varying laser zap

**Settings:**

-  Frequency/Hz: 516

-  Q Factor: 450

-  Loudness: as you like

-  Enable Loudness EG: On (set to TRIG 0)

-  Loop Loudness EG: Off

-  Attack/Decay: as you like

-  Enable Pitch EG: on (set to TRIG 0)

-  Loop EG: Off

-  Amount: -260

-  Attack: 50

-  Decay: 917

With these settings, you can feed two clock signals into the TRIG
0 and CV 0 inputs to generate interesting laser-style sounds. With a
random source into CV 0 you can change the pitch of the laser as you go.
You could then use the second channel of the TBD to generate another
drum pattern (e.g. a kick with the sine source) and thus get a basic but
possibly ever-evolving rhythm.