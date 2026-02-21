***************
Karpuskl (Mono)
***************


Description
~~~~~~~~~~~

This plugin features the combination of a *Plucked String Synth*,
a *Fuzz/Sustainer*, a *Ringmodulator*, a *Volume-EG* and a *CV Pitch
Tuner* with *Quantizer*. Any of these components can be switched
off/muted when convenient.
A demo-track can be found here:
https://soundcloud.com/taitekatto/karpuskl-for-tbd

The String Synthesizer is based on the algorithm by Kevin Karplus
und Alex Strong and also features different ways to play chords. Chords
can consist of 3 or 4 notes and either contain the main tone and a fifth
or various standard chord structures.

The latter logic is based on on the Stradella bass system as used
with accordions: https://en.wikipedia.org/wiki/Stradella_bass_system
The Ring Modulator is inspired by Bob Moog's original design as also
used with the Moogerfooger MF-102.

.. _parameters-5:

Parameters
~~~~~~~~~~

*Section 1: “Plucked String Synth”*

Here the tone generation can be modified and various chord options can
be selected.

**Single Notes/Chords:** Option to switch chords on/off.

**Power Chords/Stradella:** Option to select either chords
consisting of the fundamental and fifth[s] and octaves or chords with
Stradella-logic.
Stradella-chords feature different chord-structures on five available
octaves of the keyboard:

Here are the chords for the C notes, they will be transposed for
any other note accordingly:

1) C-E-G (C major)

2) G-Eb-C (C minor)

3) Bb-E-C (C dominant 7th)

4) Eb-C-A (C diminished 7th)

5) C-E-G# (C augmented triad)

Please note: Power-chords use perfect fights, Stradella-chords
are in well tempered tuning.

**Four Note Chords**: Adds a fifth for Power-chords or an octave
(in some cases a formerly missing fundamental) for Stradella-chords.

**Octave up for Stradella**: All notes of the chord will be one
octave higher as usual

**Octave down for Stradella**: All notes of the chord will be one
octave higher as usual
Please note: obviously if both transposition options are selected, the
pitch will be back to normal!

**Trigger**: This triggers a note or a chord of the String
Synthesizer.

**Frequency**: Adjust the pitch of a note or a chord of the
String Synthesizer.
Please note: If the “Chromatic Quantize\ **”** is enabled the pitch will
be quantized to well tempered notes of the chromatic scale. Otherwise
the frequency is continuous.

**Sustain**: The “size of the body” of our virtual String
instrument.

**Stiffness**: The fabric of the “strings” of our virtual String
instrument.
Please note: you can obtain similar results concerning the duration of
the tone by decreasing “Sustain” or increasing “Stiffness”, but similar
to a natural instrument the character of the tone will differ.

*Section 2: “Ring Modulator"”*

This section features a Ring Modulator with a built-in pitch-LFO.

**Drive Pre Modulator:** Selects if the Fuzz/Sustainer will be
applied on the original tone or only after the Ring Modulator.

**Drive Fuzz+Sustainer:** Intensity of the Fuzz/Sustainer effect.
Please note: at high settings and especially in combination with ring
modulation the tone may sound “forever” (if no volume EG is applied to
it).

**Modulator Gain**: Adjusts the signal-level to be sent to the
Ring Modulator

**Modulator Mix**: Mixes the dry tone (with optional fuzz) and
the ring modulated signal in a range from 0-10.
Please note: The Ringmodulator will be turned off when set to zero, the
dry signal will be not audible anymore when set to 10.

**Modulator Frequency**: Frequency of the sinus carrier signal of
the Ring Modulator.
0.6 - 80 Hz with range-setting “Lo”, 30-4000 Hz with setting “Hi”.

**Modulator Freq Lo/Hi**: Select slower or faster range of
frequencies.

**LFO Amount**: Adjusts the amount of an optional LFO modulating
the pitch of the Ring Modulator’s carrier-signal in a range of 0-10.
Please note: The LFO is turned off when the amount is set to zero or
will circle around the current setting from (almost) minimum to maximum
when set to 10.

**LFO Rate**: 0.1-25 Hz.

**LFO Sine/Square**: Option to switch the LFO’s sinus wave to a
square signal.

*Section 3: “VCA+Envelope”*

Here you can adjust the overall gain and decide if you want an envelope
generator applied to it.

**Master Gain:** The maxim level of the VCA / the overall
volume.

**Volume EG on/off:** Will be triggered along with the triggering
of the string synth when set to on.

**Attack**: Attack time from 0-5 seconds, logarithmic range of
slider / pot / CV.

**Decay**: Decay time from 0-20 seconds, logarithmic range of
slider / pot / CV.

**EG Loop off/on**: EG will loop when set to on.

*Section 4: “Pitch+Quantizer”*

Incoming CV that is assigned to the Plucked String Synth’s Frequency can
be adjusted here.

**Adjust Pitch CV:** Apply an optional offset to the incoming CV,
as assigned to the String Synth’s frequency from -50 to 50 Cent in terms
of chromatic notes.

Please note: this also may be used to add an offset in order to
minimize rounding errors in combination with the “Chromatic Quantize”
option.

**Chromatic Quantize:** Option to adjust the incoming CV, as
assigned to the String Synth’s frequency to well tempered chromatic
tuning.

.. _usage-patch-ideas-5:

Usage / Patch Ideas
~~~~~~~~~~~~~~~~~~~

Caution: combinations of a high carrier signal of the Ringmodulator with
a fast LFO can cause quite loud tones. The same may be true for high
fuzz settings.

It’s a good idea to start using the string synth alone to familiarize
yourself with it’s tones and chord options. You should set the trigger
to a CV-trigger-in and the Frequency to a CV-in of the TBD.

Apply a long attack of the EG on sustaining notes to get sounds that are
quite different to a plucked string instrument.

Experiment with the Ringmodulator and it’s LFO to get “spaced out”
tones.
