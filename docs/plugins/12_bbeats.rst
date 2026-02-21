*************
BBeats (Mono)
*************

.. _description-2:

Description
~~~~~~~~~~~

This plugin showcases two ByteBeat generators that can be mixed.
ByteBeats are based on algorithms generating eight-bit waveforms. For
details on the concepts please refer to:

*https://nightmachines.tv/the-absolute-beginners-guide-to-coding-bytebeats.html* <https://nightmachines.tv/the-absolute-beginners-guide-to-coding-bytebeats.html>__

Includes ByteBeat algorithms by Matt Wand - make sure to have a
look at his other great work at: https://hot-air.bandcamp.com

A demotrack can be found here:
https://soundcloud.com/taitekatto/bbeats-for-tbd

.. _parameters-1:

Parameters
~~~~~~~~~~

*Section 1: “Beat A Params”*

Here the ByteBeat A can be generated and modified

**BeatA stop:** Mutes this ByteBeat

**BeatA backwards:** Changes the playing direction of this
ByteBeat. Please note that the effect will be only audible with longer,
evolving ByteBeats played at slower speed.

**#1 BeatA select**: Selects the ByteBeat from a list of over 50
algorithms. The first half of the algorithms are different waveforms,
the second half are more rhythmical and experimental ByteBeats. The
first half of the ByteBeats is identical for the BeatA and BeatB
section, whereas the second half is different per section.

The # is given as a selectable modulation target for the optional
Envelope Generators (EGs).

**#2 BeatA pitch**: Sets the playback speed of the ByteBeat. All
way left is minimum speed, all right is maximum speed and features the
ByteBeat with its original speed.

*Section 2: “Beat B Params”*

Here the ByteBeat B can be generated and modified

**BeatB stop:** Mutes this ByteBeat

**BeatB backwards:** Changes the playing direction of this
ByteBeat. Please note that the effect will be only audible with longer,
evolving ByteBeats played at slower speed.

**#3 BeatB select**: Selects the ByteBeat from a list of over 50
algorithms. The first half of the algorithms are different waveforms,
the second half are more rhythmical and experimental ByteBeats. The
first half of the ByteBeats is identical for the BeatA and BeatB
section, whereas the second half is different per section.

The # is given as a selectable modulation target for the optional
Envelope Generators (EGs).

**#4 BeatB pitch**: Sets the playback speed of the ByteBeat. All
way left is minimum speed, all right is maximum speed and features the
ByteBeat with its original speed.

*Section 3: “Beat Mixer”*

Here the ByteBeats can be crossfaded and amplified

**Reset BBeats on stop:** This is a global option deciding if a
ByteBeat will play where it stopped or restart at its beginning. Please
note that the difference will be only audible with longer, evolving
ByteBeats played at slower speed.

**#5 Volume:** Sets the overall volume for the mixture of both
ByteBeats.

The # is given as a selectable modulation target for the optional
Envelope Generators (EGs).

**#6 XFade A<=>B:** Crossfades between both ByteBeats, setting it
to all way left only ByteBeatA will be audible, all way right only
ByteBeatB will be audible. The point where both ByteBeats have identical
volume may differ from algorithm to algorithm of the ByteBeats.

The # is given as a selectable modulation target for the optional
Envelope Generators (EGs).

*Section 4: “EG 1 (linear)”*

A loopable Envelope Generator that can modulate any available target.

Please note that the modulation is always positive and starts at the
current setting of the target.

Linear EGs are better for targets #1-4, logarithmic EGs are better for
targets #5-6, especially to modulate the volume (Target #5). But please
feel free to experiment with other settings as well!

**Target # of any Slider:** Current setting of target gets
modulated (EG off: Target # 0) Targets are exclusive to one EG!
Please note: If you accidentally set two EGs to the same target, only
the last EG will be processed.

**Activate EG:** Triggers the EG when on.

If **Loop EG** is ‘on’, **Activate EG** will toggle the EG from
on to off or off to on.

If **Loop EG** is ‘off’, **Activate EG** will directly trigger
the EG!
Set **Activate EG** to ‘off’, if you don’t need this EG, but still want
to keep the target setting for later.

Please note: If you want to use the EG as an LFO, set **Loop EG**
to ‘on’ first, if **Activate EG** is ‘on’ the loop will be latched,
then!

**Loop EG:** If activated the EG will restart after it finishes.
This can be utilized to use the EG as an LFO with a variable waveform.

**Amount (adding to target):** The EG will modulate the target by
max 200% at max. If the target reaches 100% of its range during the
progression of the EG, the result will be clipped. This can be
particularly useful, if you want to have a fake ADSR type of EG.

**Attack:** Set the attack-phase of the EG.

**Decay:** Set the release-phase of the EG.

*Section 5: “EG 2 (logarithmic)”*

A loopable Envelope Generator that can modulate any available target.

-> For further information please refer to “Section 4”, the
parameters are identical!

*Section 6: “EG 3 (linear)”*

A loopable Envelope Generator that can modulate any available target.

-> For further information please refer to “Section 4”, the
parameters are identical!

*Section 7: “EG 4 (logarithmic)”*

A loopable Envelope Generator that can modulate any available target.

-> For further information please refer to “Section 4”, the
parameters are identical!

.. _usage-patch-ideas-1:

Usage / Patch Ideas
~~~~~~~~~~~~~~~~~~~

ByteBeats can be rather harsh in sound or of a eightbit toy machine
character by principle.

But if you filter the results and/or apply various effects in your
modular, the outcome can change in character quite drastically. Due to
the sometimes raw and yet overtones-rich character of the available
material processing the sound further can lead to rather unexpected
results.

The mechanism as well as the audio-material really lend themselves very
well to experimentation!

Using the loopable EGs you can set up a kind of backing-track with the
GUI and save it as a preset. You then can assign the parameters you want
to modify via Pots, CV and Triggers.

Experimenting with two instances of BBeats can be especially
interesting, you may even construct a self-playing patch this way, if
for instance you send the result of one BBeats instance to a S&H module
and trigger the hold-point of the S&H with the other instance. You may
need additional utility-modules (like an attenuator and a lag-processor
for instance) to make this work properly, though.
