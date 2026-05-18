/***************
TBD-16 — Macro/Preset System & PicoSeqRack

(c) 2025-2026 Per-Olov Jernberg (possan). https://possan.codes

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

dadamachines has a commercial license to use this code in the TBD-16 product.
Other commercial use requires a separate license agreement.

Provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "SynthDefinitionDataModel.hpp"
#include "SynthDefinition.hpp"
#include "TrackDefinition.hpp"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <dirent.h>
#include "esp_log.h"
#include "ctagResources.hpp"
#include "StorageOverlay.hpp"


using namespace CTAG::MACROPRESETS;
using namespace rapidjson;


void SynthDefinitionDataModel::Init() {
    synths = (struct SynthDefinition *) heap_caps_malloc(MAX_SYNTHS * sizeof(struct SynthDefinition), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    tracks = (struct TrackDefinition *) heap_caps_malloc(MAX_TRACKS * sizeof(struct TrackDefinition), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);

    for(int i=0; i<MAX_SYNTHS; i++) {
        CTAG::MACROPRESETS::SynthDefinitionUtils::SynthDefinition_Reset(&synths[i]);
    }

    for(int i=0; i<MAX_TRACKS; i++) {
        CTAG::MACROPRESETS::TrackDefinitionUtils::TrackDefinition_Reset(&tracks[i]);
    }

    //
    // Initialize tracks
    //

    lastTrack = -1;

    addDrumTrack("Kick", 9, 0, 36, "");
    addTrackEngine("nodrum");
    addTrackEngine("db");
    addTrackEngine("ab");
    addTrackEngine("ro");
    addTrackEngine("extdrum");

    addDrumTrack("Kick2", 9, 40, 37, "");
    addTrackEngine("nodrum");
    addTrackEngine("fmb");
    addTrackEngine("ro");
    addTrackEngine("extdrum");

    addDrumTrack("Snare", 9, 80, 38, "");
    addTrackEngine("nodrum");
    addTrackEngine("ds");
    addTrackEngine("as");
    addTrackEngine("do");
    addTrackEngine("extdrum");

    addDrumTrack("Hat", 10, 0, 36, "");
    addTrackEngine("nodrum");
    addTrackEngine("hh1");
    addTrackEngine("hh2");
    addTrackEngine("ro");
    addTrackEngine("extdrum");

    addDrumTrack("Rimshot", 10, 40, 37, "");
    addTrackEngine("nodrum");
    addTrackEngine("rs");
    addTrackEngine("ro");
    addTrackEngine("extdrum");

    addDrumTrack("Clap", 10, 80, 38, "");
    addTrackEngine("nodrum");
    addTrackEngine("cl");
    addTrackEngine("ro");
    addTrackEngine("extdrum");

    addDrumTrack("Rompler", 11, 0, 36, "percussion");
    addTrackEngine("nodrum");
    addTrackEngine("ro");
    addTrackEngine("extdrum");

    addDrumTrack("Rompler", 11, 40, 37, "chords");
    addTrackEngine("nodrum");
    addTrackEngine("ro");
    addTrackEngine("extdrum");

    addSynthTrack("Bass", 0, 0, "");
    addTrackEngine("nosynth");
    addTrackEngine("td3");
    addTrackEngine("ro");
    addTrackEngine("extsynth");

    addSynthTrack("Bass2", 1, 0, "");
    addTrackEngine("nosynth");
    addTrackEngine("td3");
    addTrackEngine("ro");
    addTrackEngine("extsynth");

    addSynthTrack("Lead", 2, 0, "");
    addTrackEngine("nosynth");
    addTrackEngine("mo");
    addTrackEngine("ro");
    addTrackEngine("extsynth");

    addSynthTrack("Lead2", 3, 0, "");
    addTrackEngine("nosynth");
    addTrackEngine("wtosc");
    addTrackEngine("mo");
    addTrackEngine("tbd");
    addTrackEngine("tbdait");
    addTrackEngine("ro");
    addTrackEngine("extsynth");

    addSynthTrack("Rompler", 4, 0, "stabs");
    addTrackEngine("nosynth");
    addTrackEngine("ro");
    addTrackEngine("extsynth");

    addSynthTrack("Rompler", 5, 0, "synths");
    addTrackEngine("nosynth");
    addTrackEngine("ro");
    addTrackEngine("extsynth");

    addSynthTrack("Chords", 6, 0, "");
    addTrackEngine("nosynth");
    addTrackEngine("pp");
    addTrackEngine("tbd");
    addTrackEngine("ro");
    addTrackEngine("extsynth");

    addSynthTrack("Input", 7, 0, "");
    addTrackEngine("nosynth");
    addTrackEngine("imp");

    addFxTrack("FX1", 12, 0, "");
    addTrackEngine("nofx");
    addTrackEngine("fxdelay");

    addFxTrack("FX2", 12, 20, "");
    addTrackEngine("nofx");
    addTrackEngine("fxreverb");

    addFxTrack("Master", 12, 40, "");
    addTrackEngine("nofx");
    addTrackEngine("fxmaster");

    //
    // Initialize synths
    //

    lastEngine = -1;

    addEngine("nodrum", "Empty drum", SynthType_Drum);

    addEngine("nosynth", "Empty synth", SynthType_Synth);

    addEngine("nofx", "Empty FX", SynthType_FX);

    addEngine("fxdelay", "FX Delay", SynthType_FX);
    addEngineParameter("time", "Time", SynthParameterType_CC, 8, 16);
    addEngineParameter("sync", "Sync", SynthParameterType_CC, 9, 0);
    addEngineParameter("freeze", "Freeze", SynthParameterType_CC, 10, 0);
    addEngineParameter("tapedig", "Tapedig", SynthParameterType_CC, 11, 0);
    addEngineParameter("stwid", "Stereo width", SynthParameterType_CC, 12, 32);
    addEngineParameter("fx2send", "FX2 Send", SynthParameterType_CC, 13, 0);
    addEngineParameter("feedback", "Feedback", SynthParameterType_CC, 14, 32);
    addEngineParameter("base", "Base", SynthParameterType_CC, 15, 0);
    addEngineParameter("width2", "Width 2", SynthParameterType_CC, 16, 32);
    addEngineParameter("level", "Level", SynthParameterType_CC, 17, 64);
    addEngineParameter("inputhp", "Input HP", SynthParameterType_CC, 18, 0);

    addEngine("fxreverb", "FX Reverb", SynthType_FX);
    addEngineParameter("time", "Time", SynthParameterType_CC, 8, 64);
    addEngineParameter("lowpass", "Lowpass", SynthParameterType_CC, 9, 96);
    addEngineParameter("level", "Level", SynthParameterType_CC, 10, 64);
    addEngineParameter("diffuse", "Diffuse", SynthParameterType_CC, 11, 89);
    addEngineParameter("predelay", "PreDly", SynthParameterType_CC, 12, 14);
    addEngineParameter("modulation", "ModRate", SynthParameterType_CC, 13, 64);
    addEngineParameter("inputgain", "Drive", SynthParameterType_CC, 14, 64);
    addEngineParameter("tanklevel", "TankLvl", SynthParameterType_CC, 15, 127);
    addEngineParameter("hp", "HP", SynthParameterType_CC, 16, 0);

    addEngine("fxmaster", "FX Master", SynthType_FX);
    addEngineParameter("compthres", "Thresh", SynthParameterType_CC, 8, 100);
    addEngineParameter("compratio", "Ratio", SynthParameterType_CC, 9, 32);
    addEngineParameter("compatk", "Attack", SynthParameterType_CC, 10, 0);
    addEngineParameter("comprel", "Release", SynthParameterType_CC, 11, 20);
    addEngineParameter("complpf", "LPF", SynthParameterType_CC, 12, 48);
    addEngineParameter("compgain", "Gain", SynthParameterType_CC, 13, 0);
    addEngineParameter("compmix", "Mix", SynthParameterType_CC, 14, 64);
    addEngineParameter("compdlylev", "Dly.Lev", SynthParameterType_CC, 15, 64);
    addEngineParameter("comprevlev", "Rev.Lev", SynthParameterType_CC, 16, 64);
    addEngineParameter("summute", "Sum mute", SynthParameterType_CC, 17, 0);
    addEngineParameter("sumlev", "Sum lev", SynthParameterType_CC, 18, 64);

    addEngine("db", "Synth Kick", SynthType_Drum);
    addEngineParameter("freq", "Freq", SynthParameterType_NRPM, 8, 2048);
    addEngineParameter("tone", "Tone", SynthParameterType_CC, 9, 64);
    addEngineParameter("decay", "Decay", SynthParameterType_CC, 10, 32);
    addEngineParameter("dirt", "Dirt", SynthParameterType_CC, 11, 0);
    addEngineParameter("fm-env", "Fm Env", SynthParameterType_NRPM, 12, 2048);
    addEngineParameter("fm-decay", "Fm Decay", SynthParameterType_NRPM, 13, 1024);
    addEngineParameter("fm-accent", "Fm Accent", SynthParameterType_CC, 14, 8);

    addEngine("ab", "Analog Bass Drum", SynthType_Drum);
    addEngineParameter("freq", "Freq", SynthParameterType_NRPM, 8, 2048);
    addEngineParameter("tone", "Tone", SynthParameterType_CC, 9, 32);
    addEngineParameter("decay", "Decay", SynthParameterType_CC, 10, 64);
    addEngineParameter("a-fm", "A FM", SynthParameterType_CC, 11, 256);
    addEngineParameter("s-fm", "S FM", SynthParameterType_NRPM, 12, 256);
    addEngineParameter("accent", "Accent", SynthParameterType_CC, 13, 2);

    addEngine("fmb", "FM Kick", SynthType_Drum);
    addEngineParameter("f-b", "FM", SynthParameterType_NRPM, 8, 3072);
    addEngineParameter("d-b", "DB", SynthParameterType_NRPM, 9, 6450);
    addEngineParameter("f-m", "FM", SynthParameterType_NRPM, 10, 896);
    addEngineParameter("d-m", "DM", SynthParameterType_NRPM, 11, 0);
    addEngineParameter("b-m", "BM", SynthParameterType_NRPM, 12, 640);
    addEngineParameter("a-f", "AF", SynthParameterType_NRPM, 13, 2048);
    addEngineParameter("d-f", "DF", SynthParameterType_NRPM, 14, 4096);
    addEngineParameter("i", "I", SynthParameterType_NRPM, 15, 80);
    addEngineParameter("ratiomode", "Ratio Mode", SynthParameterType_CC, 16, 0);
    addEngineParameter("envsync", "Env Sync", SynthParameterType_CC, 17, 0);

    addEngine("ds", "Digital Snare", SynthType_Drum);
    addEngineParameter("freq", "Freq", SynthParameterType_CC, 8, 32);
    addEngineParameter("tone", "Tone", SynthParameterType_CC, 9, 16);
    addEngineParameter("decay", "Decay", SynthParameterType_CC, 10, 16);
    addEngineParameter("noise", "Noise", SynthParameterType_CC, 11, 32);
    addEngineParameter("accent", "Accent", SynthParameterType_CC, 12, 32);

    addEngine("as", "Analog Snare", SynthType_Drum);
    addEngineParameter("freq", "Freq", SynthParameterType_CC, 8, 32);
    addEngineParameter("tone", "Tone", SynthParameterType_CC, 9, 16);
    addEngineParameter("decay", "Decay", SynthParameterType_CC, 10, 16);
    addEngineParameter("noise", "Noise", SynthParameterType_CC, 11, 32);
    addEngineParameter("accent", "Accent", SynthParameterType_CC, 12, 32);

    addEngine("hh1", "Hi-Hat 1", SynthType_Drum);
    addEngineParameter("freq", "Freq", SynthParameterType_CC, 8, 32);
    addEngineParameter("tone", "Tone", SynthParameterType_CC, 9, 16);
    addEngineParameter("decay", "Decay", SynthParameterType_CC, 10, 16);
    addEngineParameter("noise", "Noise", SynthParameterType_CC, 11, 32);
    addEngineParameter("accent", "Accent", SynthParameterType_CC, 12, 32);

    addEngine("hh2", "Hi-Hat 2", SynthType_Drum);
    addEngineParameter("freq", "Freq", SynthParameterType_CC, 8, 32);
    addEngineParameter("tone", "Tone", SynthParameterType_CC, 9, 16);
    addEngineParameter("decay", "Decay", SynthParameterType_CC, 10, 16);
    addEngineParameter("noise", "Noise", SynthParameterType_CC, 11, 32);
    addEngineParameter("accent", "Accent", SynthParameterType_CC, 12, 32);

    addEngine("hh2", "Hi-Hat 2", SynthType_Drum);
    addEngineParameter("freq", "Freq", SynthParameterType_CC, 8, 32);
    addEngineParameter("tone", "Tone", SynthParameterType_CC, 9, 16);
    addEngineParameter("decay", "Decay", SynthParameterType_CC, 10, 16);
    addEngineParameter("noise", "Noise", SynthParameterType_CC, 11, 32);
    addEngineParameter("accent", "Accent", SynthParameterType_CC, 12, 32);

    addEngine("rs", "Rimshot", SynthType_Drum);
    addEngineParameter("freq", "Freq", SynthParameterType_CC, 8, 32);
    addEngineParameter("tone", "Tone", SynthParameterType_CC, 9, 16);
    addEngineParameter("decay", "Decay", SynthParameterType_CC, 10, 16);
    addEngineParameter("noise", "Noise", SynthParameterType_CC, 11, 32);
    addEngineParameter("accent", "Accent", SynthParameterType_CC, 12, 32);

    addEngine("cl", "Clap", SynthType_Drum);
    addEngineParameter("freq", "Freq", SynthParameterType_CC, 8, 16);
    addEngineParameter("tone", "Tone", SynthParameterType_CC, 9, 10);
    addEngineParameter("decay", "Decay", SynthParameterType_CC, 10, 10);
    addEngineParameter("scale", "Scale", SynthParameterType_CC, 11, 4);
    addEngineParameter("trans", "Trans", SynthParameterType_CC, 12, 4);

    addEngine("ro", "Rompler", SynthType_Synth);
    addEngineParameter("bank", "Bank", SynthParameterType_CC, 8, 0);
    addEngineParameter("slice", "Slice", SynthParameterType_CC, 9, 0);
    addEngineParameter("start", "Start", SynthParameterType_CC, 10, 0);
    addEngineParameter("end", "End", SynthParameterType_CC, 11, 127);
    addEngineParameter("cutoff", "Cutoff", SynthParameterType_CC, 12, 127);
    addEngineParameter("reso", "Reso", SynthParameterType_CC, 13, 0);
    addEngineParameter("type", "Type", SynthParameterType_CC, 14, 0);
    addEngineParameter("bitcr", "Bit.CR", SynthParameterType_CC, 15, 0);
    addEngineParameter("attack", "Attack", SynthParameterType_CC, 16, 0);
    addEngineParameter("decay", "Decay", SynthParameterType_CC, 17, 64);
    addEngineParameter("speed", "Speed", SynthParameterType_CC, 18, 64);
    addEngineParameter("pitch", "Pitch", SynthParameterType_CC, 19, 64);
    addEngineParameter("loop", "Loop", SynthParameterType_CC, 20, 0);
    addEngineParameter("pingpong", "PingPong", SynthParameterType_CC, 21, 0);
    addEngineParameter("ppstart", "PPStart", SynthParameterType_CC, 22, 64);
    addEngineParameter("eg2fm", "EG2FM", SynthParameterType_CC, 23, 0);
    addEngineParameter("tsmode", "TSMode", SynthParameterType_CC, 24, 0);
    addEngineParameter("tsamt", "TSAmt", SynthParameterType_CC, 25, 64);

    addEngine("td3", "TBD03", SynthType_Synth);
    addEngineParameter("shape", "Bank", SynthParameterType_CC, 8, 0);
    addEngineParameter("p0", "P0", SynthParameterType_NRPM, 9, 0);
    addEngineParameter("vca_d", "VCA D", SynthParameterType_NRPM, 10, 8);
    addEngineParameter("vcf_d", "VCF D", SynthParameterType_NRPM, 11, 8);
    addEngineParameter("cutoff", "Cutoff", SynthParameterType_NRPM, 12, 64);
    addEngineParameter("reso", "Reso", SynthParameterType_NRPM, 13, 0);
    addEngineParameter("envdec", "EnvDec", SynthParameterType_NRPM, 14, 16);
    addEngineParameter("type", "Type", SynthParameterType_CC, 15, 0);
    addEngineParameter("satur", "Satur", SynthParameterType_NRPM, 16, 0);
    addEngineParameter("drive", "Drive", SynthParameterType_NRPM, 17, 0);
    addEngineParameter("slide", "Slide", SynthParameterType_CC, 18, 0);
    addEngineParameter("accent", "Accent", SynthParameterType_CC, 19, 0);
    addEngineParameter("p1", "P1", SynthParameterType_NRPM, 20, 0);
    addEngineParameter("p0amt", "P0 Amt", SynthParameterType_NRPM, 21, 0);
    addEngineParameter("p1amt", "P1 Amt", SynthParameterType_NRPM, 22, 0);
    addEngineParameter("acclev", "Acc. Lev", SynthParameterType_NRPM, 23, 0);
    addEngineParameter("slidelev", "Slide Lev", SynthParameterType_CC, 24, 0);
    addEngineParameter("synctrig", "Sync Trig", SynthParameterType_CC, 25, 0);

    addEngine("mo", "Mono Synth", SynthType_Synth);
    addEngineParameter("shape", "Bank", SynthParameterType_CC, 8, 0);
    addEngineParameter("p0", "P0", SynthParameterType_NRPM, 9, 0);
    addEngineParameter("p1", "P1", SynthParameterType_NRPM, 10, 0);
    addEngineParameter("waveshap", "Waveshape", SynthParameterType_NRPM, 11, 0);
    addEngineParameter("p0a", "P0 A", SynthParameterType_NRPM, 12, 0);
    addEngineParameter("p1a", "P1 A", SynthParameterType_NRPM, 13, 0);
    addEngineParameter("fma", "FM A", SynthParameterType_NRPM, 14, 0);
    addEngineParameter("qscale", "Q Scale", SynthParameterType_CC, 15, 32);
    addEngineParameter("attack", "Attack", SynthParameterType_NRPM, 16, 0);
    addEngineParameter("decay", "Decay", SynthParameterType_CC, 17, 32);
    addEngineParameter("loopenv", "Loop Env", SynthParameterType_CC, 18, 0);
    addEngineParameter("decim", "Decim", SynthParameterType_CC, 19, 0);
    addEngineParameter("bitred", "Bit Red", SynthParameterType_CC, 20, 0);

    addEngine("wtosc", "Wavetable Osc", SynthType_Synth);
    addEngineParameter("wavebank", "Bank", SynthParameterType_CC, 8, 0);
    addEngineParameter("wave", "Wave", SynthParameterType_NRPM, 9, 0);
    addEngineParameter("tune", "Tune", SynthParameterType_NRPM, 10, 0);
    addEngineParameter("type", "Type", SynthParameterType_CC, 11, 0);
    addEngineParameter("cutoff", "Cutoff", SynthParameterType_NRPM, 12, 127);
    addEngineParameter("reso", "Reso", SynthParameterType_NRPM, 13, 0);
    addEngineParameter("gain", "Gain", SynthParameterType_NRPM, 14, 64);
    addEngineParameter("attack", "Attack", SynthParameterType_NRPM, 15, 0);
    addEngineParameter("decay", "Decay", SynthParameterType_NRPM, 16, 64);
    addEngineParameter("sustain", "Sustain", SynthParameterType_NRPM, 17, 0);
    addEngineParameter("release", "Release", SynthParameterType_NRPM, 18, 16);
    addEngineParameter("e2wave", "E2 Wave", SynthParameterType_NRPM, 19, 0);
    addEngineParameter("e2fm", "E2 FM", SynthParameterType_NRPM, 20, 0);
    addEngineParameter("e2filt", "E2 Filt", SynthParameterType_NRPM, 21, 0);
    addEngineParameter("speed", "Speed", SynthParameterType_NRPM, 22, 10);
    addEngineParameter("sync", "Sync", SynthParameterType_CC, 23, 0);
    addEngineParameter("l2wave", "L2 Wave", SynthParameterType_NRPM, 24, 0);
    addEngineParameter("l2am", "L2 AM", SynthParameterType_NRPM, 25, 0);
    addEngineParameter("l2fm", "L2 FM", SynthParameterType_NRPM, 26, 0);
    addEngineParameter("l2filt", "L2 Filt", SynthParameterType_NRPM, 27, 0);

    addEngine("pp", "Polypad", SynthType_Synth);
    addEngineParameter("chord", "Chord", SynthParameterType_CC, 8, 0);
    addEngineParameter("inver", "Inversion", SynthParameterType_CC, 9, 0);
    addEngineParameter("detune", "Detune", SynthParameterType_NRPM, 10, 8);
    addEngineParameter("cutoff", "Cutoff", SynthParameterType_NRPM, 11, 80);
    addEngineParameter("reso", "Reso", SynthParameterType_NRPM, 12, 0);
    addEngineParameter("type", "Type", SynthParameterType_CC, 13, 32);
    addEngineParameter("qscale", "Q Scale", SynthParameterType_CC, 14, 0);
    addEngineParameter("attack", "Attack", SynthParameterType_NRPM, 15, 0);
    addEngineParameter("decay", "Decay", SynthParameterType_NRPM, 16, 8);
    addEngineParameter("sustain", "Sustain", SynthParameterType_NRPM, 17, 8);
    addEngineParameter("release", "Release", SynthParameterType_NRPM, 18, 16);
    addEngineParameter("l1spd", "L1 Speed", SynthParameterType_NRPM, 19, 0);
    addEngineParameter("l1amt", "L1 Amount", SynthParameterType_NRPM, 20, 0);
    addEngineParameter("l2spd", "L2 Speed", SynthParameterType_NRPM, 21, 0);
    addEngineParameter("l2amt", "L2 Amount", SynthParameterType_NRPM, 22, 0);
    addEngineParameter("efltamt", "E Filter Amount", SynthParameterType_NRPM, 23, 0);
    addEngineParameter("l2rand", "L2 Random", SynthParameterType_CC, 24, 127);
    addEngineParameter("nnotes", "Number of Notes", SynthParameterType_CC, 25, 32);

    addEngine("tbd", "TBDings", SynthType_Synth);
    addEngineParameter("model", "Model", SynthParameterType_CC, 8, 0);
    addEngineParameter("freq", "Freq", SynthParameterType_NRPM, 9, 8192);
    addEngineParameter("struc", "Structure", SynthParameterType_NRPM, 10, 8192);
    addEngineParameter("pos", "Position", SynthParameterType_NRPM, 11, 4900);
    addEngineParameter("bright", "Bright", SynthParameterType_NRPM, 12, 10000);
    addEngineParameter("damp", "Damp", SynthParameterType_NRPM, 13, 7000);
    addEngineParameter("chord", "Chord", SynthParameterType_CC, 14, 0);
    addEngineParameter("poly", "Poly", SynthParameterType_CC, 15, 1);
    addEngineParameter("envsh", "Env Shape", SynthParameterType_NRPM, 16, 4000);
    addEngineParameter("vela", "Vel Amt", SynthParameterType_NRPM, 17, 8000);
    addEngineParameter("air", "Air", SynthParameterType_NRPM, 18, 0);
    addEngineParameter("pluck", "Pluck", SynthParameterType_CC, 19, 0);
    addEngineParameter("mtype", "Mod Type", SynthParameterType_CC, 20, 0);
    addEngineParameter("mdpth", "Mod Depth", SynthParameterType_NRPM, 21, 0);
    addEngineParameter("mrate", "Mod Rate", SynthParameterType_NRPM, 22, 8000);
    addEngineParameter("msnap", "Mod Snap", SynthParameterType_CC, 23, 0);

    addEngine("tbdait", "TBDaits", SynthType_Synth);
    addEngineParameter("model", "Model", SynthParameterType_CC, 8, 2);
    addEngineParameter("freq", "Freq", SynthParameterType_NRPM, 9, 8192);
    addEngineParameter("harm", "Harm", SynthParameterType_NRPM, 10, 8192);
    addEngineParameter("timbre", "Timbre", SynthParameterType_NRPM, 11, 8192);
    addEngineParameter("morph", "Morph", SynthParameterType_NRPM, 12, 8192);
    addEngineParameter("decay", "Decay", SynthParameterType_NRPM, 13, 13000);
    addEngineParameter("color", "Color", SynthParameterType_CC, 14, 1);
    addEngineParameter("level", "Level", SynthParameterType_NRPM, 15, 11600);
    addEngineParameter("fmod", "FMod", SynthParameterType_NRPM, 16, 0);
    addEngineParameter("tmod", "TMod", SynthParameterType_NRPM, 17, 0);
    addEngineParameter("mmod", "MMod", SynthParameterType_NRPM, 18, 0);

    addEngine("extsynth", "External Synth", SynthType_Synth);
    addEngineParameter("chan", "MIDI Channel", SynthParameterType_CC, 8, 0);

    addEngine("extdrum", "External Drum", SynthType_Drum);
    addEngineParameter("chan", "MIDI Channel", SynthParameterType_CC, 8, 0);
    addEngineParameter("note", "MIDI Note", SynthParameterType_CC, 9, 36);

    addEngine("inp", "Audio Input", SynthType_Synth);
    addEngineParameter("in_gain", "Gain", SynthParameterType_NRPM, 8, 1024);
    addEngineParameter("in_mono", "Mono", SynthParameterType_CC, 9, 0);
    addEngineParameter("in_hp", "HP", SynthParameterType_NRPM, 10, 0);
    addEngineParameter("in_drive", "Drive", SynthParameterType_NRPM, 11, 0);
    addEngineParameter("in_ftype", "FType", SynthParameterType_CC, 12, 0);
    addEngineParameter("in_fcutoff", "FCut", SynthParameterType_NRPM, 13, 4095);
    addEngineParameter("in_freso", "FReso", SynthParameterType_NRPM, 14, 0);
    addEngineParameter("in_fenv", "FEnv", SynthParameterType_NRPM, 15, 0);
}

void SynthDefinitionDataModel::addDrumTrack(const char *name, int midiChannel, int baseCC, int drumNote, const char *defaultbank) {
    lastTrack ++;
    TrackDefinition *track = &tracks[lastTrack];
    track->index = lastTrack;
    strlcpy(track->name, name, 16);
    track->midiChannel = midiChannel;
    track->baseCC = baseCC;
    track->drumNote = drumNote;
    memset(track->macroMachineIds, 0, sizeof(track->macroMachineIds));
    // strncpy(tracks[trackIndex].ki0], defaultbank, 15
}

void SynthDefinitionDataModel::addSynthTrack(const char *name, int midiChannel, int baseCC, const char *defaultbank) {
    lastTrack ++;
    TrackDefinition *track = &tracks[lastTrack];
    track->index = lastTrack;
    strlcpy(track->name, name, 16);
    track->midiChannel = midiChannel;
    track->baseCC = baseCC;
    track->drumNote = -1;
    memset(track->macroMachineIds, 0, sizeof(track->macroMachineIds));
}

void SynthDefinitionDataModel::addFxTrack(const char *name, int midiChannel, int baseCC, const char *defaultbank) {
    lastTrack ++;
    TrackDefinition *track = &tracks[lastTrack];
    track->index = lastTrack;
    strlcpy(track->name, name, 16);
    track->midiChannel = midiChannel;
    track->baseCC = baseCC;
    track->drumNote = -1;
    memset(track->macroMachineIds, 0, sizeof(track->macroMachineIds));
}

void SynthDefinitionDataModel::addEngine(const char *id, const char *name, enum SynthType type) {
    lastEngine ++;
    struct SynthDefinition *s = &synths[lastEngine];
    strlcpy(s->id, id, 16);
    strlcpy(s->name, name, 32);
    s->type = type;
}

void SynthDefinitionDataModel::addTrackEngine(const char *machineId) {
    int index = 0;
    while(index < MaxTrackDefinitionMachineIds) {
        if (tracks[lastTrack].macroMachineIds[index][0] == '\0') {
            strlcpy(tracks[lastTrack].macroMachineIds[index], machineId, 15);
            break;
        }
        index ++;
    }
}

void SynthDefinitionDataModel::addEngineParameter(const char *id, const char *name, enum SynthParameterType type, int ccIndex, int defaultValue) {
    int index = 0;
    while(index < MaxSynthDefinitionParameters) {
        if (synths[lastEngine].parameters[index].id[0] == '\0') {
            strlcpy(synths[lastEngine].parameters[index].id, id, 16);
            strlcpy(synths[lastEngine].parameters[index].name, name, 32);
            synths[lastEngine].parameters[index].type = type;
            synths[lastEngine].parameters[index].cc = ccIndex;
            synths[lastEngine].parameters[index].defaultValue = defaultValue;
            break;
        }
        index ++;
    }
}

int SynthDefinitionDataModel::GetNumberOfSynthDefinitions() {
    int count = 0;
    for(int i=0; i<MAX_SYNTHS; i++) {
        if (synths[i].id[0] != '\0') {
            count ++;
        }
    }
    return count;
}

SynthDefinition *SynthDefinitionDataModel::GetSynthDefinition(const std::string id) {
    for(int i=0; i<MAX_SYNTHS; i++) {
        if (synths[i].id == id) {
            return &synths[i];
        }
    }
    return nullptr;
}

TrackDefinition *SynthDefinitionDataModel::GetTrackDefinition(int index) {
    for(int i=0; i<MAX_TRACKS; i++) {
        if (tracks[i].index == index) {
            return &tracks[i];
        }
    }
    return nullptr;
}

bool SynthDefinitionDataModel::SerializeIntoJSON(rapidjson::Document &doc) {
    // Implementation of SerializeIntoJSON

    Value machinesarray(kArrayType);
    doc.AddMember("machines", machinesarray, doc.GetAllocator());
    for(int j=0; j<MAX_SYNTHS; j++) {
        SynthDefinition *def = &synths[j];

        if (def->id[0] == '\0') continue;

        Value machineObj(kObjectType);

        machineObj.AddMember("id", Value(def->id, doc.GetAllocator()), doc.GetAllocator());
        machineObj.AddMember("name", Value(def->name, doc.GetAllocator()), doc.GetAllocator());

        Value paramsarray(kArrayType);
        machineObj.AddMember("parameters", paramsarray, doc.GetAllocator());
        for(int i=0; i<MaxSynthDefinitionParameters; i++) {
            if (def->parameters[i].id[0] == '\0') {
                continue;
            }
            Value param(kObjectType);
            param.AddMember("id", Value(def->parameters[i].id, doc.GetAllocator()), doc.GetAllocator());
            param.AddMember("name", Value(def->parameters[i].name, doc.GetAllocator()), doc.GetAllocator());
            std::string typestring;
            if (def->parameters[i].type == SynthParameterType_CC) {
                typestring = "cc";
            } else if (def->parameters[i].type == SynthParameterType_NRPM) {
                typestring = "nrpm";
            } else {
                typestring = "none";
            }
            param.AddMember("type", Value(typestring.c_str(), doc.GetAllocator()), doc.GetAllocator());
            param.AddMember("ctrl", def->parameters[i].cc, doc.GetAllocator());
            param.AddMember("def", def->parameters[i].defaultValue, doc.GetAllocator());
            machineObj["parameters"].PushBack(param, doc.GetAllocator());
        }

        doc["machines"].PushBack(machineObj, doc.GetAllocator());
    }

    Value tracksarray(kArrayType);
    doc.AddMember("tracks", tracksarray, doc.GetAllocator());
    for(int j=0; j<MAX_TRACKS; j++) {
        TrackDefinition *def = &tracks[j];

        if (def->name[0] == '\0') continue;

        Value trackObj(kObjectType);

        trackObj.AddMember("index", def->index, doc.GetAllocator());
        trackObj.AddMember("name", Value(def->name, doc.GetAllocator()), doc.GetAllocator());
        trackObj.AddMember("midichannel", def->midiChannel, doc.GetAllocator());
        trackObj.AddMember("drumnote", def->drumNote, doc.GetAllocator());
        trackObj.AddMember("basecc", def->baseCC, doc.GetAllocator());

        Value machinesarray(kArrayType);
        trackObj.AddMember("machines", machinesarray, doc.GetAllocator());
        for(int k=0; k<MaxTrackDefinitionMachineIds; k++) {
            if (def->macroMachineIds[k][0] == '\0') {
                continue;
            }
            trackObj["machines"].PushBack(Value(def->macroMachineIds[k], doc.GetAllocator()), doc.GetAllocator());
        }

        doc["tracks"].PushBack(trackObj, doc.GetAllocator());
    }

    return true;
}

#pragma GCC diagnostic ignored "-Wstringop-truncation"
void SynthDefinitionDataModel::WriteListResponse(struct GetEngineDefinitionIdListResponse *r) {
    r->numEngines = 0;
    for(int i=0; i<MAX_SYNTHS; i++) {
        if (synths[i].id[0] != '\0') {
            strncpy((char *)&r->engineIds[r->numEngines], (const char *)&synths[i].id, 8);
            r->numEngines ++;
            if (r->numEngines >= MAX_SYNTHS)
                break;
        }
    }
}

void SynthDefinitionDataModel::WriteEngineDefinitionPageResponse(const struct GetEngineDefinitionsPageRequest *request, struct GetEngineDefinitionsPageResponse *response) {
    response->offset = request->offset;
    response->totalEngineDefinitions = GetNumberOfSynthDefinitions();
    response->returnedEngineDefinitions = 0;

    for(int k=0; k<MaxEngineDefinitionsPerPage; k++) {
        int i = request->offset + k;
        if (i >= MAX_SYNTHS) {
            break;
        }
        if (synths[i].id[0] == '\0') {
            continue;
        }

        response->engineDefinitions[k].id = 0;
        strcpy( response->engineDefinitions[k].idStr, synths[i].id);
        strncpy(response->engineDefinitions[k].name, synths[i].name, 8);

        for(int p=0; p<MaxSynthDefinitionParameters; p++) {
            // response->engineDefinitions[k].parameters[p].index = synths[i].parameters[p].index;
            response->engineDefinitions[k].parameters[p].type = EngineParameterType_None;
            if (synths[i].parameters[p].type == SynthParameterType_CC) {
                response->engineDefinitions[k].parameters[p].type = EngineParameterType_CC;
            } else if (synths[i].parameters[p].type == SynthParameterType_NRPM) {
                response->engineDefinitions[k].parameters[p].type = EngineParameterType_NRPM;
            }

            response->engineDefinitions[k].parameters[p].defaultValue = synths[i].parameters[p].defaultValue;
            response->engineDefinitions[k].parameters[p].relCC = synths[i].parameters[p].cc;

            strncpy(response->engineDefinitions[k].parameters[p].name, synths[i].parameters[p].name, 32);
        }

        response->returnedEngineDefinitions ++;
    }
}

static SynthDefinitionDataModel g_synthdef_instance;

SynthDefinitionDataModel *SynthDefinitionDataModel::instance() {
    return &g_synthdef_instance;
}