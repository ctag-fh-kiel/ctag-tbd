***************
SubSynth (Mono)
***************


Description
~~~~~~~~~~~

Implementation of a single sub synth voice, similar noise shaping
as in https://zynaddsubfx.sourceforge.io/, essentially cascaded
bandpasses applied to noise. If all bandpasses are enabled with full
cascade and two channels are assigned with this plugin, the ESP32
processor runs 3*10*2=60 bandpasses in parallel (BiQuad optimized
implementation).
