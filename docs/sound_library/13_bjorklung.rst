******************
Bjorklund (Stereo)
******************

.. _description-3:

**Description**
~~~~~~~~~~~~~~~

| **Please be careful when using high resonance-levels of the filter.**
| As typical for a diode-ladder filter this can become quite nasty, so
  be sure to try with low volumes first!
| *Please note:* Bjorklund does not offer real stereo processing but the
  processing needs many resources, so it still takes up both slots of
  the TBD.
| Here is a little demo-track done only with Bjorklund, just a bit of
  reverb got added in the DAW:
  https://soundcloud.com/taitekatto/bjorklund-for-tbd

| The track mainly features 2 sounds of Bjorklund played with the
  build-in sequencer, one using an external arpeggiator and one using
  Bjorklund as a synth only.
| But as you’ll see below you can use Bjorklund in even more not so
  obvious ways, for instance to rhythmisize the notes you play. Please
  note that adjusting the main volume can be very important, because the
  instrument has a wide loudness range. For lower volume tones it will
  be suitable to raise the global volume, but be aware that you may have
  to lower it for louder tones to avoid overdrive, unless that is what
  you are looking for.

| Bjorklund uses a Saw-Oscillator, a Pulse Oscillators, a noise-source,
  a Diode Ladder Filter and a wavefolder by Carlos Laguna Ruiz
  implemented in his VULT language, to be found here:
| https://github.com/modlfo/vult

| The idea of Bjorklund is to have a simple yet fun to control
  synthesizer that can be optionally triggered by an internal
  algorithmical engine for rhythmical patterns and melodies. Concerning
  Bjorklunds ideas and implementation of "Euclidean rhythms" please
  refer to: http://cgm.cs.mcgill.ca/~godfried/publications/banff.pdf
| Concerning the concept of mathematical palindromes, which get used to
  generate melodies here, please see:
  https://en.wikipedia.org/wiki/Palindromic_number
| In addition well known palindromic sentences and optional accent
  patterns based on common meters can be applied.
  Bjorklund’s signal-flow is visualized downwards from the top to the
  bottom of the GUI.
| Within the main building blocks many elements can be turned on or off,
  opening up possibilities of different modes of operation which will be
  explained in more detail below.
| Please note that the most important switches are options to disable
  the sequencer and all envelopes except the volume EG.

.. _parameters-2:

**Parameters**
~~~~~~~~~~~~~~

Section 0: Global (Quantizer relative to Master Tune!)

-  Trigger/Clock

-  Beat Divider (Values from 1 to 32 – 32 being the fastest,
      “listening” to each clock-signal!)

-  Use an internal clock n/y (If on, signals coming in via
      Trigger/clock will be ignored!)

-  Clock Speed (Used only if the internal clock is activated!)

-  Master Pitch (map CV here => source for keytracking)

-  Master Tune +- 1200 Cent

-  Quantize CV according to Scale-Correction n/y

-  Scale-Correction (0: Chromatic)
      Various scales are available:
      Chromatic, Major, Slendro, Minor, Pelog, Dorian, Lydian, Phrygian,
      Added fourth and sixth, Octaves and fifth only, Sixth only,
      Pentatonic

-  Master Volume
      

Section 1: Oscillators

-  Saw Pitch

-  Saw Tune

-  Pulse Pitch

-  Pulse Tune

-  Pulse Width

-  PWM n/y

-  PWM Amount

-  PWM Speed
      

Section 2: Sound Mixer

-  Direct out before Filter on Right Channel n/y
      Output will be the mix after the Ringmodulator but before the
      Wavefolder and Filter!

-  Noise volume

-  Saw Oscillator volume

-  Pulse Oscillator volume

-  Oscillators Mix
      

Section 3: Effects & Filter (Distortion/WaveFolder/Ringmodulator/ Diode
Ladder Filter)
- Ring/AM Modulation active for Saw n/y

-  Ring/AM Modulation active for Pulse n/y

-  AM Modulation Boost n/y

-  AM/RingMod Frequency

-  AM/RingMod Amount

-  WaveFolding Amount

-  Cutoff Frequency

-  Resonance amount

-  Filter key tracking (Filter closes or opens more depending on
      note-CV and generated notes)

-  Filter Leakage (let unfiltered sounds through)
      

Section 4: Bjorklund patterns / Sequencer

-  PatternSequencer off/on

-  Reset Sequencer off/on (Click before starting a sync-source)

-  Bjorklund patterns inactive n/y

-  Bjorklund Pattern

-  Bjorklund Shift (Roundshift the relative first entry of the
      pattern in relation to beat)

-  Palindrome melodies inactive n/y

-  Palindrome select

-  Palindrome rootkey

-  Accent patterns inactive n/y

-  Sync Accents with Bjorklund patterns n/y
      (This will reset the accent patterns once a Bjorklund pattern
      loops)

-  Accent Beat Divider (Values from 1 to 32 – 32 evaluates each
      clock-signal!)

-  Accent Destination (see numbers of EGs)

-  Bjorklund instead of metrical for accents n/y (Uses Bjorklund
      patterns for accents, too)

-  Accent patterns select

-  Accent Shift (Roundshift the relative first entry of the
      pattern in relation to beat)

-  Accent amount (can be positive or negative)
      

Section 5: (0) Volume Envelope

-  Volume envelope active n/y

-  Invert Volume envelope n/y

-  Volume Attack

-  Volume Decay

-  Volume Envelope Amount

-  Volume Envelope loop n/y

Section 6: Disable everything downwards from here on for parameter
reduction?

-  Use Volume Envelope only ('easy edit mode') n/y
      (Disables EGs 1-6 as below, effectively internally each EG will be
      set to “active: no”)
      **Please note:** This will only be reflected inside the
      soundengine, not on the GUI!
      So if you want to change EG settings on EGs 1-6 make sure this
      option is turned off!

Section 7: (1) Noise Envelope

-  Noise envelope active n/y

-  Invert Noise envelope n/y

-  Noise Attack

-  Noise Decay

-  Noise envelope amount

-  Noise envelope loop n/y

Section 8: (2) Oscillators Mix Envelope

-  OscillatorMix envelope active n/y

-  Invert OscillatorMix envelope n/y

-  OscillatorMix Attack

-  OscillatorMix Decay

-  OscillatorMix envelope amount

-  OscillatorMix envelope loop n/y

Section 9: (3) RingModulator Envelope

-  RingMod envelope active n/y

-  Invert RingMod envelope n/y

-  RingMod Attack

-  RingMod Decay

-  RingMod envelope amount

-  RingMod envelope loop n/y
      

Section 10: (4) WaveFolder Envelope

-  WaveFolder envelope active n/y

-  Invert WaveFolder envelope n/y

-  WaveFolder Attack

-  WaveFolder Decay

-  WaveFolder envelope amount

-  WaveFolder envelope loop n/y

Section 11: (5) Filter Cutoff Envelope

-  Filter envelope active n/y

-  Invert Filter envelope n/y

-  FilterAttack

-  Filter Decay

-  Filter envelope amount

-  Filter envelope loop n/y

Section 12: (6) Filter Leakage Envelope

-  FilterLeak envelope active n/y

-  Invert FilterLeak envelope n/y

-  FilterLeak Attack

-  FilterLeak Decay

-  FilterLeak envelope amount

-  FilterLeak envelope loop n/y

.. _usage-patch-ideas-2:

**Usage / Patch Ideas**
~~~~~~~~~~~~~~~~~~~~~~~

First of all you may think about the current use-case, set the
basic options and then fine-tune the settings to your liking.

*Common use-cases are:* 

1. Standalone sound generator / “classical synth”, optionally
controlled via the keyboard

2. Synthesizer played by the internal sequencer.
The sequencer can have its own tempo or can be synchronized by an
external clock. To adjust note-length a beat-divider is available.
Optionally the melodies can be transposed via CV-in.

3. Rhythmisation of incoming notes.

4. Triggering palindromic melody-patterns by an external trigger,
note by note.

5. Scale-correction on the sequencer melodies and/or notes played
via CV.

6. Apply synchronized accents according to popular
beats/signatures.

7. Use the right output to drive different modules of your
modular system.

After you have set the options according to the use-case, it’s
probably best to pitch the oscillators, adjust their mixer-levels and
then apply the Envelope-Generators needed.
Feel free to experiment – you can’t break anything!
