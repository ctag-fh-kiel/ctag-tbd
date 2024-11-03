******************
Formantor (Stereo)
******************

Description
~~~~~~~~~~~

Here is a little demo-track, where all sounds are from with
Formantor, only a bit of reverb got added and some mastering was done in
the DAW: https://soundcloud.com/taitekatto/formantor-for-tbd

The track mainly features feedbacklike tones as well as some of
the typical formant based sounds on which the name of the instrument
obviously is based on.

The idea of Formantor is to have a simple yet fun to control
synthesizer combining a Phase Distortion oscillator, a Squarewave
oscillator with optional pulse width modulation, a vowel-filter, a
resonator, a tremolo and a volume EG.

The basic signal-flow is similar to classical synths like a Mini
Moog with sections like OSC->Filter->LFO->VCA/EG

Yet every of those components is used in a bit more
unconventional way. This applies to the features that can be controlled
as well as the sounds that can be achieved. You can have chant-like
vowels as well as quite extreme overdrive / feedback loaded sounds and
interesting modulations on various aspects of the sound including sample
and hold modulation. Also lush and slowly evolving tones are possible.
There are no stereo-effects or similar, instead the stereo-option is
used to provide a possibility for direct out of the
voices/oscillators-mix to allow additional treatment with other
modules.

Please note that adjusting the main volume can be very important,
because the instrument has a wide loudness range. For lower tones it
will be suitable to raise the global volume, but be aware that you may
have to lower it for louder tones to avoid overdrive, unless that is
what you are looking for.

Formantor uses a resonant comb-filter, three state variable
bandpass-filters and a Phase Distortion synth by Carlos Laguna Ruiz
implemented in his VULT language, the PD-synth can be found here:

`*https://github.com/modlfo/teensy-vult-example* <https://github.com/modlfo/teensy-vult-example>`__

The code originally was intended as an add-on to the Teensy Audio
library and got modified to be used with the TBD along with the
aforementioned filters from VULT-examples by using the VULT compiler.
For more details on the topic please look here:
https://github.com/modlfo/vult

For the formant-filter Open Source code by
alex@smartelectronix.com and got used, as found here:
https://www.musicdsp.org/en/latest/Filters/110-formant-filter.html
Also interpolations on that code by Antonio Grazioli got used:
https://github.com/antoniograzioli/Autodafe


Parameters
~~~~~~~~~~

Section 1: “Global (Quantizer relative to Master Tune!)”

-  Gate/Trigger n/y

-  Master Pitch (map CV here)

-  Master Tune +- 1200 Cent
      (Adjust to your needs, especially in combination with
      formant-selection by notes or if CV is quantized to half-tones as
      below)

-  Quantize CV to chromatic tuning n/y

-  Master Volume (Please lower in case you encounter unwanted
      distortion)

Section 2: “Voices”

-  Voices direct out (out-1) n/y
      (This may be welcome if you want to treat the oscillators with
      other modules as well)

-  PD Pitch

-  PD Tune

-  PD PitchMod Amount

-  Phase Distortion Amount (see description of “simulating a
      resonant filter” here:
      https://en.wikipedia.org/wiki/Phase_distortion_synthesis)

-  PD WaveFolding Amount (Wave Folder effect on the
      PD-Oscillator)

-  SQW Pitch

-  SQW Tune

-  PD-PitchMod and PWM speed

-  PWM intensity for SQW Osc

Section 3: “AM / Mixer”

-  Amplitude Modulation active n/y

-  PD Osc<->AM PD with Sine from SQW

-  SQW Osc<->AM SQW with PD

-  PD Oscillator volume

-  SQW Oscillator volume

Section 4: “Formant-Filter ('Key logic': Keys trigger the
formants (A..E.I..O..U / R1-R12)”

-  Formant Filter n/y

-  'Key logic' on n/y

-  Formant selection (A..E.I..O..U / R1-R12)

-  New random formant for selection n/y

-  Formant Filter Amount

-  Resonator active (switch to random formants) n/y

-  Resonator Frequency

-  Resonator Tone

-  Resonator Q

-  Resonator Amount

Section 5: “Tremolo”

-  Tremolo active n/y

-  Enable only with ADSR Gate n/y

-  Attack

-  Release

-  Tremolo uses SQW n/y

-  Tremolo Speed

-  Tremolo Amount

-  Tremolo after formant filter n/y

-  PD-Mod uses SQW n/y

-  Modulate PD Tone Amount

-  S&H-Modulate Resonator Amount

-  S&H-Modulate PD Tone Amount

Section 6: “Volume Envelope”

-  Volume envelope active n/y

-  Attack

-  Decay

-  ADSR n/y

-  Sustain

-  Release

-  Slow EG n/y

-  Envelope to Phase PD-OSC


Usage / Patch Ideas
~~~~~~~~~~~~~~~~~~~

1. Standalone sound generator, playing the formants - optionally
via the keyboard

1. Standalone sound generator, using the Resonator, note that
there are very interesting feedback-sounds possible when the resonance
is high.

1. Combination of formants and Resonator. Note that in this
combination you can select new random formants for the currently
selected formant (by slider on GUI or via keyboard)

1. Use the Tremolo creatively with any of the combinations above,
including Sample&Hold on PD amount and similar. Note that the tremolo
can be delayed via Attack and Release once it’s activated/deactivated
via a separate button.

1. Use the Volume envelope, including changing the PD-amount in
time, similar to a filter-envelope.

1. Use the PD-Oscillator with slow pitch-modulation in
combination with slow PWM on the Square Wave Oscillator and just a bit
of the resonator for lush, slowly evolving tones, for instance to play
slow melodies.

1. Use Formantor additionally as a pure Oscillator source via
secondary audio output “out-1”. In combination with the options from
above via “out-0” interesting combinations are possible, especially when
you connect additional modules to “out-1”.
