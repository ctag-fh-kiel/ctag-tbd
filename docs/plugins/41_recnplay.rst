***************
RecNPlay (Mono)
***************


**Description**
~~~~~~~~~~~~~~~

*RecNPlay* combines a synth, a (real time-capable) step sequencer and a
bitcrusher effect, as well as any combination of these use cases.
External input is available as a component passing the complete signal
chain. The sequencer can be synchronized internally or externally. The
bitcrusher provides optionally random modulation per step. Step
positions can be modified, for recording there is an overdub-option
available. The filter is similar to that of a Korg MS-10, thus
interesting drums sounds in combination with whitenoise are possible,
too. The parameters are structured in a manner, that starting with a
preset recording and playback with the TBD hardware alone (without the
web interface) is possible.

To get an idea what can be done with these features and effects, you may
listen here:

https://soundcloud.com/taitekatto/recnplay-for-tbd

**Parameters**

Section 1: Global Stuff (Volume / EG / Trigger)

-  Master Volume

-  EG active n/y

-  Attack

-  Decay

-  Main Trigger (EG and/or next step)

Section 2: Voice (Parameters can't be modified during playback)

-  Voice Volume (0== mute)

-  Pitch Saw - CV V/Oct

-  External <=> Saw

-  Saw <=> Noise

-  Cutoff

-  Resonance

-  Filter Drive Noise

Section 3: Effects / Bitcrusher

-  Bits to Crush (0==off)

-  Downsample Factor

-  S&H ModAmount per Step

Section 4: Stepper (Values of 'Voice' to be recorded)

-  Stepper active n/y

-  Record <-> Playback (Change sets step to one)

-  Current Step (changes on modify)

-  Number of Steps

-  Clock TPM*10 (0==ExtTrig / Max==1/32 at 60.0 BPM)

.. _usage-patch-ideas-7:

**Usage / Patch Ideas**
~~~~~~~~~~~~~~~~~~~~~~~

Normally the patches/play options will be driven by *RecNplay’s*
main building blocks and the use-case on the other side, the main use
cases are:

-  Effect for external input, featuring a filter, a bitcrusher
      and optionally an AD envelope

-  Synth, featuring a sawwave, a bitcrusher, an AD envelope and
      optionally external input

-  Step sequencer modifying the parameters (incl. tuning) of the
      synth and/or external input

Please be aware, that when the step sequencer is in
playback-mode, all Voice-Parameters (Section 2) will be blocked / not
changeable anymore. This is due to the concept that per step “snapshots”
of the then current state of the voice, including tuning will be taken.
To record “empty steps” you can set the voice-volume to zero.

*Using the step sequencer / Record:*

Set “Stepper active” to ‘y’, set “Record ⇔ Playback” to ‘Record’,
optionally set “Current Step” to the position you want to start
recording. Set “Number of Steps” to the number of steps you want to
record. Enable the “Main Trigger” (Global section) once per step to be
recorded.


*Using the step sequencer / Playback:*

Set “Stepper active” to ‘y’, set “Record ⇔ Playback” to
‘Playback’, optionally set “Current Step” to the position you want to
start playback. Set “Number of Steps” to the number of steps you want to
playback. Enable the “Main Trigger” (Global section) once per step to be
played, or use the internal clock by setting it to any value but 0.

**Please note**:

-  Any Change of Record ⇔ Playback will reset the current step to
      0

-  Changes to “Current Step” will only take place on change, so
      to enable 2 if 2 is already set for instance, change to 1 or 3 and
      back.

-  The internal clock for playback will be used, when it’s set to
      any other value than 0

-  Clock values are “Triggers per Minute”, times 10 - so 600
      would be 60 BPMs effectively using quaternotes. 1200 would be 60
      BPM using ⅛ notes and so on.

-  For external sync, simply use the main trigger in play-mode.
      This way you can have uneven triggerings as well, for instance if
      you use a module like MI Grids.

*Common parameter mappings:*

To be able to record in a real-time kind of way you could use the
following mapping:

-  “Pitch Saw” to CV-0 (played with a keyboard for instance)

-  “Main Trigger” to TRIG-0 (triggered by a keyboard for
      instance)

-  “Record ⇔ Playback” to TRIG-1

-  “Saw ⇔ Noise” to CV-1 (controlled with the mod-wheel of a
      keyboard for instance)

-  “Cutoff” to Pot-0

-  “Resonance” to Pot-1

*Storage of sequences:*

Sequences are stored in working memory only. You can modify
individual steps during recording or repeat steps during playback. But
when you load a different plugin or shut down your TBD, the current
sequence will be lost. One trick to partly bypass this is to record
another sequence from your modular synth using *RecNplay,* thus freeing
this sequencer for other duties.
