************
WTOsc (Mono)
************

Description
===========

A basic wave-table oscillator using the 1MiB of wavetable ROM at
the beginning of the sample rom section. Users can create and upload
their own wavetables e.g. created with https://waveeditonline.com/ .

This oscillator makes use of the plaits wavetable oscillator
implementation (again, kudos to E. Gillet!!!), but using user wavetable
data from TBD's sample ROM. It allows for Z-morphing within one
wavetable bank (one bank consists of 64 wavetables each 256 samples
large). The 1MiB of wavetable ROM allows to store 32 banks of wavetable
data, i.e. 32 banks \* 64 wavetables \* 256 wavetable size \* 2 bytes
(int16 format) = 1MiB. WTOsc allows to select any of those 32 banks,
whereas Z-morphing is only implemented for within a bank. NOTE: Use the
pitch quantizer to mitigate noise on the pitch CV.

Features of WTOsc include:

- User wavetables

- Wavetable (CV)

- Wavetable bank switching (not per CV as pre slower pre-calculations are required)

- Z-morphing within a bank (EG, LFO, CV)

- Pitch / pitch modulation (EG, LFO, CV), pitch quantization (see miscellaneous information)

- ADSR EG

- Amplitude modulated with EG/LFO/CV

- SVF filter modulated with EG/LFO/CV

Miscellaneous Information
-------------------------

Quantizer Scales (from Braids implementation) used in various plugins such as MacOsc, WTOsc etc., root key based on pitch setting:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

0: Off

1: Semitones

2: Ionian (From midipal/BitT source code)

3: Dorian (From midipal/BitT source code)

4: Phrygian (From midipal/BitT source code)

5: Lydian (From midipal/BitT source code)

6: Mixolydian (From midipal/BitT source code)

7: Aeolian (From midipal/BitT source code)

8: Locrian (From midipal/BitT source code)

9: Blues major (From midipal/BitT source code)

10: Blues minor (From midipal/BitT source code)

11: Pentatonic major (From midipal/BitT source code)

12: Pentatonic minor (From midipal/BitT source code)

13: Folk (From midipal/BitT source code)

14: Japanese (From midipal/BitT source code)

15: Gamelan (From midipal/BitT source code)

16: Gypsy

17: Arabian

18: Flamenco

19: Whole tone (From midipal/BitT source code)

20: pythagorean (From yarns source code)

21: 1_4_eb (From yarns source code)

22: 1_4_e (From yarns source code)

23: 1_4_ea (From yarns source code)

24: bhairav (From yarns source code)

25: gunakri (From yarns source code)

26: marwa (From yarns source code)

27: shree (From yarns source code)

28: purvi (From yarns source code)

29: bilawal (From yarns source code)

30: yaman (From yarns source code)

31: kafi (From yarns source code)

32: bhimpalasree (From yarns source code)

33: darbari (From yarns source code)

34: rageshree (From yarns source code)

35: khamaj (From yarns source code)

36: mimal (From yarns source code)

37: parameshwari (From yarns source code)

38: rangeshwari (From yarns source code)

39: gangeshwari (From yarns source code)

40: kameshwari (From yarns source code)

41: pa\__kafi (From yarns source code)

42: natbhairav (From yarns source code)

43: m_kauns (From yarns source code)

44: bairagi (From yarns source code)

45: b_todi (From yarns source code)

46: chandradeep (From yarns source code)

47: kaushik_todi (From yarns source code)

48: jogeshwari (From yarns source code)

