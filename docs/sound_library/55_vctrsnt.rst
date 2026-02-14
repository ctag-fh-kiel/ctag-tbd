****************
VctrSnt (Stereo)
****************


Description
~~~~~~~~~~~

Here is a little demo-track, where all tracks are done with
VctrSnt, only a bit of reverb got added in the DAW:
https://soundcloud.com/taitekatto/vctrsnt-for-tbd

VctrSnt stands for "Vector Synthesizer" and is inspired by the
legendary Sequential Circuits Prophet VS
http://www.vintagesynth.com/sci/pvs.php

This TBD synth is stereo when the oscillators split option or the
optional auto-panner effect is activated, the sound generation itself is
monophonic, though. When only one audio-output is used and panning is on
it will be audible as a kind of tremolo-effect instead, which makes it
mono-compatible, this may be welcome when combined with other modules in
your rack's signal chain. With the oscillators split option in contrast
it's possible to split out one wavetable and one sample oscillator to
each of the stereo outputs, so in this context you have to make sure the
option is off when used as a mono device.

Very much like the Prophet VS, VctrSnt can blend sound between
four oscillators, that's what the name vector-synthesis mainly stands
for.

In contrast to its role model this is not done with a joystick
but with separate controllers/CV per axis.

As with the VS the Oscillators are named A,B,C,D.

On the X-axis the wavetables can be crossfaded as A<->C, on the
Y-axis samples (as an enhancement in contrast of using wavetables only)
can be crossfaded as D<->B.

Similar to the PPG and other wavetable-based instruments, the
waves within the table can be changed manually or via an LFO and is
called "Z-scanning" here.

The volume envelope and the panner can be turned off completely,
this is important in case you want to use a true analogue filter behind
the digital oscillators, which is one of the strong points of vintage
wavetable synths like the Prophet VS and PPG 2 or early samplers like
the EMU II.

VctrSnt borrows a great amount of code from the TBD-Plugins
*WTOsc* and *Rompler,* big thanks to Robert Manzke for implementing
those!

.. _parameters-10:

Parameters
~~~~~~~~~~

**LFO-Types:** 0: Sine, 1: Square-Wave, 2: Half-Sine upwards, 3:
Pseudo-Triangle upwards,

4: Half-Sine downwards, 5: Pseudo-Triangle downwards, 6: Sample & Hold
(Random Values)

**Filter-Types:** 0: off, 1: LowPass, 2: HighPass, 3: BandPass

Section 1: “Voice”

This is the section where you can find “global controls”

- Gate Voice n/y: Triggers the Volume EG (if active) and/or a new sample

- Samples ignore Gate n/y: Samples are not triggered when activated, useful for free running loops

- MasterPitch: Pitch in halftone steps -48 … 48 All oscillators can be pitched and/or be attached 
  to a single CV to play all oscillatorsin tune

- MasterTune: Tuning +/- one octave for all oscillators

- Quantize CV to chromatic tuning n/y: Useful for exact tuning.
  Please note: use MasterTune for calibrating the incoming CV if needed.

- Master Volume: Please start with a setting of about 25% to
  avoid distortion and then adjust to your needs

- Exclude SubOSCs from MasterPitch-CV n/y: Useful to add bourdon (drone)-tones

- Exclude WaveTables from MasterPitch-CV n/y: Useful for instance
  if Wavetables are used with rhythmically Z-scanning.

- Exclude Samples from MasterPitch-CV n/y: Useful for instance if
  you want loops to play “forever” without transpositions.

Section 2: “Vector space (A<->C is X-Axis, D<->B is Y-Axis)”

This is the main mixer

- PWM intensity SubOSCs: Squarewave if set to 0.

- PWM speed SubOSCs: Speed of PWM for the rectangular Sub Oscillators

- SubOsc Noise/PWM A: Choose between Noise and rectangle waves
  for the suboscillators which are added to the Wavetable's signal chain.

- Pitch SubOsc A: Pitch of 3 octaves as -3600 … 3600 Cent / For
  noise you can switch between pink and white noise here instead.
  Please note: if you want to play this via CV, it either has to be mapped
  explicitly or “Exclude SubOSCs from MasterPitch-CV” from the
  master-section has to be off. If it's not assigned to the MasterPitch it
  allows “bourdon”/drone tones.
  When the SubOsc is set to Noise, Pitch below 0 will select pinknoise and
  pitch above zero whitenoise

- SubOscillator Level A: Crossfades between the wavetable and the suboscillator

- SubOsc Noise/PWM C: see above

- Pitch SubOsc C: see above

- SubOscillator Level C: see above

- Vol Wavetable A: Level of Wavetable and it's suboscillator

- Vol Sample Osc B: Level of one sample oscillator, please note
  that max. Level for normalized samples is about 50%

- Vol Wavetable C: see above

- Vol Sample Osc D: see above

- Stereo Split Oscillators n/y: If activated, Sample D and
  Wavetable A (and its suboscillator) will be output at the left channel
  and Sample B and Wavetable C (and its suboscillator)

- Crossfade Wavetables A<->C: This is the X-axis of our “vector stick”

- Crossfade Samples D<->B: This is the Y-axis of our “vector stick”

Section 3: “Vector Modulators”

Here it's possible to automatically modulate the Crossfaders of
the “Vector Space”

- LFO 1 WaveTable Xfade n/y: turn on LFO 1

- LFO 1 Type WT Xfade: Choose LFO-type for Xfading of wavetables

- LFO 1 WaveTable range: Amount of Xfade modulation

- LFO 1 WaveTable Speed: Speed of LFO 1, modulating X-Fading of the wavetable oscillators

- LFO 2 WaveTable Xfade n/y: enable LFO 2 for WT Xfade or SubOSC A Modulation

- WT LFO 2 modulates SubOscXfade A n/y: If active modulate crossfade WT A <-> SubOSC A

- LFO 2 Type WT Xfade: see above

- LFO 2 WaveTable range: see above

- LFO 2 WaveTable Speed: see above

- LFO 1 Sample Xfade n/y: see above

- LFO 1 Type Sample Xfade: see above

- LFO 1 Sample range: see above

- LFO 1 Sample Speed: see above

- LFO 2 Sample Xfade n/y: enable LFO 2 for WT Xfade or SubOSC A Modulation

- WT LFO 2 modulates SubOscXfade C n/y: If active modulate crossfade WT C <-> SubOSC C

- LFO 2 Type Sample Xfade: see above

- LFO 2 Sample range: see above

- LFO 2 Sample Speed: see above

Section 4: “Wavetable A”

One of two wavetable oscillators

- Wavetable A: Select from up to 32 wavetables

- Scan Wavetable A: select wave from 256 waves in table

- Pitch A: Pitch in halftone steps -48 … 48

- Tune A: Tuning +/- one octave

- Automatic Scanning A n/y: Turn LFO for Z-scanning on/off

- LFO Z-Scan Type A: Select LFO-wave type (LFO-Types see above)

- LFO Z-Scan Amount A: Range of Z-scanning

- LFO Z-Scan Speed A: LFO Speed:

- Filter Mode A: Filter-Types see above

- Filter Cutoff A: Cutoff Frequency

- Filter Resonance A: Resonance amount

- Filter LFO on A n/y: Turn LFO for filter modulation on/off

- LFO Filter Type A: Select LFO-wave type (LFO-Types see above)

- Filter LFO Amount A: Range of LFO-modulation on filter

- Filter LFO Speed A: Speed of LFO-modulation on filter

Section 5: “Sample Oscillator B”

One of two sample oscillators

- Bank B: Bank selection, each bank contains 32 sample slices

- Slice B: Selection of 32 slices, slices are stored sequentially
  in sample rom

- Playback Speed B: Playback speed -200% … 200%

- Pitch B: Pitch in halftone steps -48 … 48

- Tune B: Tuning +/- one octave

- Start Position B: Start sample position (relative to slice
  length) of slice playback

- Length B: Playback length (relative to slice length)

- Loop B off/on: Loops sample between loop position end end position

- Ping-Pong B: Sample loop ping pong mode

- Loop Position B: Sample loop position relative to length parameter

- Filter Mode B: Filter-Types see above

- Filter Cutoff B: Cutoff Frequency

- Filter Resonance B: Resonance amount

- Filter LFO on B n/y: Turns LFO for filter modulation on/off,
  Waveform always is sine

- Filter LFO Amount B: LFO modulation level on filter cutoff

- Filter LFO Speed B: Frequency of LFO for filter modulation

Section 5: “Wavetable C”

For details please refer to: Section 4: “Wavetable A”

- Wavetable C

- Scan Wavetable C

- Pitch C

- Tune C

- Automatic Scanning C n/y

- LFO Z-Scan Type C

- LFO Z-Scan Amount C

- LFO Z-Scan Speed C
 
- Filter Mode C

- Filter Cutoff C

- Filter Resonance C

- Filter LFO on C n/y

- LFO Filter Type C

- Filter LFO Amount C

- Filter LFO Speed C

Section 6: “Sample Oscillator D”

For details please refer to: Section 5: “Sample Oscillator D”

- Bank D

- Slice D

- Playback Speed D

- Pitch D

- Tune D

- Start Position D

- Length D

- Loop D off/on

- Ping-Pong D

- Loop Position D

- Filter Mode D

- Filter Cutoff D

- Filter Resonance D

- Filter LFO on D n/y

- Filter LFO Amount D

- Filter LFO Speed D

Section 7: “Frequency Modulation”

Applies frequency modulation to all possible oscillators with
some options

- Freq Modulation active n/y: Only active when set to on

- Freq Mod excludes SubOSCs n/y: Avoid frequency modulation on
  Sub oscillators

- Freq Mod excludes WaveTableOSCs n/y: Avoid frequency modulation
  on Wavetables

- Freq Mod excludes SampleOSCs n/y: Avoid frequency modulation on
  samples

- Freq Modulation Type: LFO-Types see above - Type 6 (Sample and
  Hold can be interesting here)

- Modulation Intensity: Amount of frequency modulation

- Modulation Speed: Speed of frequency modulation

- Apply 'analogue errors': Adds random to intensity and speed,
  useful for a kind of “vintage effect”

Section 8: “Volume Envelope”

A volume envelope for all oscillators. For more sophisticated
applications please make use of one or two (in stereo split-mode) VCAs
with envelopes as separate modules.

- Volume EG active n/y: Will be triggered/gated by master trigger
  if active (“Gate Voice y/n”)

- Attack: Attack time of Volume envelope

- Decay: Decay time of Volume envelope

- Use ADSR-logic n/y: If off the envelope can be triggered, for
  instance when you are using a trigger sequence. If set to on you can
  play notes that keep on playing until you release the key, and have an
  optional release phase, as it's standard with most synthesizers.

- Sustain: Loudness level to be kept after the Attack/Decay phase.

- Release: Final stage of the envelope, to enable a soft fading tone.

Section 9: “Panner/Tremolo”

Optional stereo panning of tremolo if only one output is used.

- Panner on n/y: Turns auto-panning off/on

- Panner Intensity: Amount of panning-effect

- Panner Frequency: Speed of panning-effect

.. _usage-patch-ideas-12:

Usage / Patch Ideas
~~~~~~~~~~~~~~~~~~~

*Content needed:*

Please be aware that you have to make sure that wavetables and
samples are loaded to the TBDs ROM space, in order to make proper use of
*VctrSnt*. In order to use the sample rom, you can upload from the edit
sample rom page of the web-GUI. Either use your own samples and
wavetables or the factory content (sample-rom.tbd) as explained here:
factory sample
data <https://github.com/ctag-fh-kiel/ctag-tbd/blob/master/sample_rom/readme.md>__

*Keeping the balance right:*

Another important topic is that VctrSnt is designed in a way that
it can handle samples of low volume. The downside of this is that you
may experience unwanted aliasing or even distortion if you amplify your
samples too much or turn up the master volume too high. As a rule of
thumb, you should begin with a volume setting of about 50% for the
sample oscillators B and D in the “vectorspace” and a master volume of
about 25%. To get a decent balance in sound it's often better to lower
the parts that are too loud instead of increasing parts that are too
quiet. After that you can still increase the levels to find the perfect
“sweet spot” in terms of dynamics on the one hand side and no distortion
on the other hand side. There is no compressor integrated, only a simple
limiter that should prevent severe digital clipping.

Please note that normally you would control the overall pitch of
VctrSnt via the MasterPitch, but it's possible to exclude sections from
this. To play only one sample via CV for instance you could exclude
Samples from the Master pitch and add the Samples Pitch for Sample D to
the identical CV-input, then all oscillators apart from Sample B would
be controlled via the MasterPitch!

*Managing VctrSnt's parameters:*

Because of the complexity of this instrument quite many options
and values have to be set.

Therefore it's a good idea to start with one section (Wavetable
or Sample) only and add more as you move on. To do so, it's a good idea
to mute some aspects first by lowering the volume in the so-called
“Vector Space”. After that you can scroll down and focus on the section
you want to change, sections are sized in a way that they should fit
into the visible portion of the window then.

*This plugin can be used in 3 major scenarios:*

1. As a standalone instrument, utilizing all of its filters,
modulators and the volume envelope.
The use cases themselves may vary very much, though. In principle
everything from percussive voices, melody-voices or more padlike sounds
are possible. For instance the voice-option “Samples ignore Gate” can be
very interesting here, because then samples are not retriggered and
notes can vary a lot when a sample is looping with each trigger.

Please note that the envelope can operate in two modes: AD or
ADSR. With the AD-mode the mechanism to trigger a new voice or envelope
is based on triggering instead of gates, thus trigger/pattern sequencers
like for instance the Mutable Instruments Grids can be used without
problems here.

2. As a standalone sound generator of more soundscape / drone
like “self-playing” textures. In this scenario the volume envelope
typically would be disabled.

3. As a kind of “Complex Oscillator” module, with other modules
like real analogue filters and alike in a signal-chain behind it.

*Come, play with me:*

All modulators (for the vector-faders, filters, pitch, Z-Scan,
panning and so on) have on/off options that again can be put to action
using a trigger signal. This way modifications in sound easily can be
put on or off, for instance also with a trigger sequencer.

To make full usage of the many modifications possible it's
recommended to modify the sliders on the GUI itself. Also a MIDI-option
to modify the GUI elements will come in handy here.

Of course this can lead to interesting presets that can be turned
into playable patches with the TBD module alone afterwards.

Of course attaching a real joystick that can send out CV will
give you the closest feel to a real VS, but then there is no CV left to
play VctrSnt pitched. But as explained above this does not have to be a
real disadvantage. Anyways, experimentation is the key! You also may
find videos about classic instruments like the PPG 2.x or the SCI
Prophet VS inspiring for that!
