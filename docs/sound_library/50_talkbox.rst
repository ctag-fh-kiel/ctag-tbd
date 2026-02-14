****************
Talkbox (Stereo)
****************


**Description**
~~~~~~~~~~~~~~~

| *Talkbox* is a vocoder instrument with an internal synth used as the
  so-called carrier-signal. The so called modulator-signal has to be fed
  in via the external input (in-0).
| To get a general idea what the concept of vocoding is about please
  look here:
| https://en.wikipedia.org/wiki/Vocoder
| A real Talkbox is based on a different principle of operation:

https://en.wikipedia.org/wiki/Talk_box

The name talkbox was chosen to point out the (almost) “all-in-one”
nature of this effect/instrument and due to the fact that similar to
vocoder-based instruments from the 70th with built-in synths (by Korg or
Roland for instance) the bands of the vocoder can not be adjusted. The
actual vocoder-code is based on the soundpipe-DSP-Library port of the
VST plugin also named Talkbox by MDA: http://mda.smartelectronix.com/

| Here is a little demo-track done with Subbotnik with quite a bit of
  reverb added in the DAW:
| https://soundcloud.com/taitekatto/talkbox-for-tbd

| Talkbox’s featureset is largely inspired by Korg’s VC10, an early
  vocoder instrument:
| http://www.vintagesynth.com/korg/vc10.php

In contrast to the VC10 *Talkbox* is not truly polyphonic, though.
Instead a selectable interval can be added as a sub oscillator. By using
an external Carrier this limitation can be bypassed. All other features
are available, in addition to the stereo ensemble-effect there even is a
chorus.

**Parameters**

Section 1: Global / Mixer

-  Internal Carrier Pitch (map CV here)

-  Internal Carrier Tune +- 1200 Cent

-  Total Carrier Level

-  Internal Carrier Signal Balance (Synth<=>Noise)

-  Carrier Balance (Internal<=>External [1-in])

-  Carrier Leakage

-  External Carrier Amount

-  Activate Accent Bend n/y

-  Accent Bend (Talkbox to Internal Carrier Pitch)

-  Modulator Level [0-in]

-  Modulator Leakage [0-in]

-  Volume

Section 2: Internal Carrier

-  Vibrato active n/y

-  Vibrato Speed

-  Vibrato Depth

-  PWM active n/y

-  PWM Speed

-  PWM Depth

-  SubOsc Tuning (A) +- 1200 Cent

-  SubOsc switch Tuning (A)<=>(B)

-  SubOsc Tuning (B) +- 1200 Cent

Section 3: FX

-  FX active n/y

-  Ensemble / Chorus

-  FX Depth

-  FX Amount

.. _usage-patch-ideas-11:

**Usage / Patch Ideas**
~~~~~~~~~~~~~~~~~~~~~~~

Normally the patches/play options will be driven by *Talkbox’s*
main building blocks, for instance:

-  Changing the pitch of the vocoded signal via CV, typically
      using an external keyboard.

-  Changing pitch of the sub oscillator by using the “switch
      tuning” option

-  Mix in an external carrier, for instance to provide additional
      polyphony or different waves

-  Accent Bend as a special effect, to be temporarily enabled via
      the on/off switch

-  Vibrato as a special effect, to be temporarily enabled via the
      on/off switch

The most common usage of vocoding obviously is to use human voice
as the modulator. But also rhythmical material like drum beats can be
very interesting when used as a modulator.
If you want to blend in the modulator signal along with the vocoded
signal you can do that using the “Modulator Leakage” parameter. Also a
bit of the unprocessed carrier-signal can be nice.

