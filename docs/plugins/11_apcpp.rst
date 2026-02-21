************
APCpp (Mono)
************

.. _description-1:

Description
~~~~~~~~~~~

APCpp stands for "Atari Punk Console plus plus", i.e. the digital
implementation of a popular electronics circuit with enhancements.

A little demotrack can be found here:
https://soundcloud.com/taitekatto/apcpp-for-tbd

As with many simple Oscillators, the original hardware design is
based on timerchips (two NE555 or one 556)
https://de.wikipedia.org/wiki/NE555

To learn more about the original APC circuit please refer to:
https://sdiy.info/wiki/Atari_Punk_Console

Here you can watch a video about how to build an (enhanced) APC
in hardware and how it basically sounds:
https://www.youtube.com/watch?v=1wTc4tqBpnw&feature=youtu.be

The enhancements for TBD are optional pitch-modulation,
pulse-width-modulation, amplitude/ring-modulation, sinus waves for the
oscillators and a volume-envelope. Details are explained with the
parameters below.

Parameters
~~~~~~~~~~

*Section 1: “Atari-Punk-Console Oscillators / AM or PWM”*

This is the section where you can find the two main oscillators and
basic options on how to interact with them. Oscillators 1 and 2 are
always modulating each other.

**MOD 1 Frequency:** Frequency of the optional modulator for OSC
1.

**Frequency Osc 1:** Frequency of OSC 1 - remember: this
oscillator will always be modulated with the other Oscillator.

**MOD 1 off/on:** Apply modulation to OSC 1 or not.

**MOD 1 is PWM n/y:** You can choose between a kind of Pulse
Width Modulation and Amplitude Modulation for OSC 1. PWM is applied with
a sine wave, AM is applied with a rectangle here. The latter mode can
result in interesting cut-effects to the sound. When the carrier wave is
a sine the “PWM” is similar to a phase-modulation.

**Smooth Oscillator 1 n/y:** Typically for the APC the oscillator
uses a rectangle wave and is modulated with the other oscillator with a
kind of logical “not and” operation. In smoothed mode a sine wave is
used instead and modulation with the other oscillator is amplitude
modulation, also known as ring-modulation in case when the second
oscillator is set to a sine wave.

**Smooth Oscillator 2 n/y:** See right above: “Smooth Oscillator
1 n/y”

**MOD 2 is PWM n/y:** You can choose between a kind of Pulse
Width Modulation and Amplitude Modulation for OSC 2. PWM is applied with
a sine wave, AM is applied with a rectangle here. The latter mode can
result in interesting cut-effects to the sound. When the carrier wave is
a sine the “PWM” is similar to a phase-modulation.

**MOD 2 off/on:** Apply modulation to OSC 2 or not.

**Frequency Osc 2:** Frequency of OSC 2 - remember: this
oscillator always will be modulated with the other Oscillator.

**MOD 2 Frequency:** Frequency of the optional modulator for OSC
2.

*Section 2: “Frequency Modulation”*

Here additional pitch-modulation can be applied per oscillator

**FreqMod 1 Amount / 0 to disable:** When turned all the way to
the left no pitch modulation will be applied to oscillator 1, all the
way to the right up/down of a perfect fifth will be added in pitch.

**Frequency-Modulation 1 Speed:** Very slow to low audio-range
frequency.

**FreqMod 1 Square-Wave n/y:** If set to on, oscillator 1 will be
pitch-modulated by a square wave, a sine wave is used otherwise.

**FreqMod 2 Square-Wave n/y:** If set to on, oscillator 2 will be
pitch-modulated by a square wave, a sine wave is used otherwise.

**Frequency-Modulation 2 Speed:** Very slow to low audio-range
frequency.

**FreqMod 2 Amount / 0 to disable:** All the way to the left no
pitch modulation will be applied to oscillator 2, all the way to the
right up/down of a perfect fifth will be added in pitch.

*Section 3: “VCA / Envelope”*

Here the overall volume can be adjusted and an optional, loopable
envelope generator applied to it.

**Volume Amount:** General volume and maximum amount of the
volume envelope.

**Please note:** High settings, depending on the source-material
may lead to distortion or even digital clipping. This is intentional to
be able to amplify quieter sounds and also can be used in connection
with the EG to obtain sounds that stay on a certain level for a longer
period of time.

If in doubt, setting the volume to about the middle-position
should be fine.

**Use Envelope n/y:** If activated the sound will be only audible
when the volume envelope is triggered

**Trigger Envelope:** Triggers the envelope, which will loop if
required.

**Volume Attack:** Modifies the time it takes to reach the peak
in volume.

**Volume Decay:** Modifies the time for the sound to decay after
the peak in volume had been reached.

**Envelope Loop n/y:** If on the envelope will trigger itself
after it has reached its end.

Usage / Patch Ideas
~~~~~~~~~~~~~~~~~~~

This plugin is intended to let you experiment with all kinds of weird
sounds.

To try out the original APC algorithm you should turn all options and
modulations off, first.

Among other things, the APCpp can achieve self-playing sounds similar to
those known from early SciFi movies like “Forbidden Planet”. For such
sounds using the built in smooth options and adding echo and reverb or
even a shimmer-verb is a good idea.

In general it’s always a good point to start with the two main
oscillators alone. Remember that they are V/Oct. compatible, so in
principle you can play tuned melodies. But the tuning is highly
dependent on the modulation settings and the two oscillators modulate
each other all the time, too, so some notes may be in tune, whereas
others will not be, again experimenting is encouraged.

After you have set up the basic oscillators, choosing the suitable
modulation may be the next step. In some cases, especially with
“smoothed oscillators” and reverb, you may find sounds that remind you
of “robotic vowels” when you change modulation frequencies on the fly.

If you want to make percussive sounds it’s good to make very dense basic
sounds and to trigger them with a short envelope. On the other hand you
can choose varying pad-like sounds by making basic sounds with slow
evolution and use an envelope with a slow attack and a long decay.

It’s a good idea to attach the frequency of the main oscillators to
CV-in and to use a CV-trigger for the envelope. For the pots of the TBD
for instance tweaking the frequency or amounts of modulators can be
interesting.