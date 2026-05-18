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


static void SynthParameter_Reset(struct SharedEngineDefinitionParameter *param) {
    memset(param->id, 0, sizeof(param->id));
    memset(param->name, 0, sizeof(param->name));
    param->type = EngineParameterType_None;
    param->defaultValue = 0;
    param->relCC = 0;
}

static void SynthDefinition_Reset(struct SharedEngineDefinition *def) {
    memset(def->idStr, 0, sizeof(def->idStr));
    memset(def->name, 0, sizeof(def->name));
    def->type = EngineType_None;
    for(int pi=0; pi<MaxEngineDefinitionParameters; pi++) {
        SynthParameter_Reset(&def->parameters[pi]);
    }
}

static void TrackDefinition_Reset(struct SharedTrackDefinition *def) {
    def->index = 0;
    memset(def->name, 0, sizeof(def->name));
    def->midiChannel = 0;
    def->drumNote = 0;
    def->baseCC = 0;
    memset(def->engineIdStr, 0, sizeof(def->engineIdStr));
}


void SynthDefinitionDataModel::Init() {
    synths = (struct SharedEngineDefinition *) heap_caps_malloc(MAX_SYNTHS * sizeof(struct SharedEngineDefinition), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    tracks = (struct SharedTrackDefinition *) heap_caps_malloc(MAX_TRACKS * sizeof(struct SharedTrackDefinition), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);

    for(int i=0; i<MAX_SYNTHS; i++) {
        SynthDefinition_Reset(&synths[i]);
    }

    for(int i=0; i<MAX_TRACKS; i++) {
        TrackDefinition_Reset(&tracks[i]);
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

    addEngine("nodrum", "Empty drum", EngineType_Drum);

    addEngine("nosynth", "Empty synth", EngineType_Synth);

    addEngine("nofx", "Empty FX", EngineType_FX);

    addEngine("fxdelay", "FX Delay", EngineType_FX);
    addEngineParameter("time", "Time", EngineParameterType_CC, 8, 16);
    addEngineParameter("sync", "Sync", EngineParameterType_CC, 9, 0);
    addEngineParameter("freeze", "Freeze", EngineParameterType_CC, 10, 0);
    addEngineParameter("tapedig", "Tapedig", EngineParameterType_CC, 11, 0);
    addEngineParameter("stwid", "Stereo width", EngineParameterType_CC, 12, 32);
    addEngineParameter("fx2send", "FX2 Send", EngineParameterType_CC, 13, 0);
    addEngineParameter("feedback", "Feedback", EngineParameterType_CC, 14, 32);
    addEngineParameter("base", "Base", EngineParameterType_CC, 15, 0);
    addEngineParameter("width2", "Width 2", EngineParameterType_CC, 16, 32);
    addEngineParameter("level", "Level", EngineParameterType_CC, 17, 64);
    addEngineParameter("inputhp", "Input HP", EngineParameterType_CC, 18, 0);

    addEngine("fxreverb", "FX Reverb", EngineType_FX);
    addEngineParameter("time", "Time", EngineParameterType_CC, 8, 64);
    addEngineParameter("lowpass", "Lowpass", EngineParameterType_CC, 9, 96);
    addEngineParameter("level", "Level", EngineParameterType_CC, 10, 64);
    addEngineParameter("diffuse", "Diffuse", EngineParameterType_CC, 11, 89);
    addEngineParameter("predelay", "PreDly", EngineParameterType_CC, 12, 14);
    addEngineParameter("modulation", "ModRate", EngineParameterType_CC, 13, 64);
    addEngineParameter("inputgain", "Drive", EngineParameterType_CC, 14, 64);
    addEngineParameter("tanklevel", "TankLvl", EngineParameterType_CC, 15, 127);
    addEngineParameter("hp", "HP", EngineParameterType_CC, 16, 0);

    addEngine("fxmaster", "FX Master", EngineType_FX);
    addEngineParameter("compthres", "Thresh", EngineParameterType_CC, 8, 100);
    addEngineParameter("compratio", "Ratio", EngineParameterType_CC, 9, 32);
    addEngineParameter("compatk", "Attack", EngineParameterType_CC, 10, 0);
    addEngineParameter("comprel", "Release", EngineParameterType_CC, 11, 20);
    addEngineParameter("complpf", "LPF", EngineParameterType_CC, 12, 48);
    addEngineParameter("compgain", "Gain", EngineParameterType_CC, 13, 0);
    addEngineParameter("compmix", "Mix", EngineParameterType_CC, 14, 64);
    addEngineParameter("compdlylev", "Dly.Lev", EngineParameterType_CC, 15, 64);
    addEngineParameter("comprevlev", "Rev.Lev", EngineParameterType_CC, 16, 64);
    addEngineParameter("summute", "Sum mute", EngineParameterType_CC, 17, 0);
    addEngineParameter("sumlev", "Sum lev", EngineParameterType_CC, 18, 64);

    addEngine("db", "Synth Kick", EngineType_Drum);
    addEngineParameter("freq", "Freq", EngineParameterType_NRPM, 8, 2048);
    addEngineParameter("tone", "Tone", EngineParameterType_CC, 9, 64);
    addEngineParameter("decay", "Decay", EngineParameterType_CC, 10, 32);
    addEngineParameter("dirt", "Dirt", EngineParameterType_CC, 11, 0);
    addEngineParameter("fm-env", "Fm Env", EngineParameterType_NRPM, 12, 2048);
    addEngineParameter("fm-decay", "Fm Decay", EngineParameterType_NRPM, 13, 1024);
    addEngineParameter("fm-accent", "Fm Accent", EngineParameterType_CC, 14, 8);

    addEngine("ab", "Analog Bass Drum", EngineType_Drum);
    addEngineParameter("freq", "Freq", EngineParameterType_NRPM, 8, 2048);
    addEngineParameter("tone", "Tone", EngineParameterType_CC, 9, 32);
    addEngineParameter("decay", "Decay", EngineParameterType_CC, 10, 64);
    addEngineParameter("a-fm", "A FM", EngineParameterType_CC, 11, 256);
    addEngineParameter("s-fm", "S FM", EngineParameterType_NRPM, 12, 256);
    addEngineParameter("accent", "Accent", EngineParameterType_CC, 13, 2);

    addEngine("fmb", "FM Kick", EngineType_Drum);
    addEngineParameter("f-b", "FM", EngineParameterType_NRPM, 8, 3072);
    addEngineParameter("d-b", "DB", EngineParameterType_NRPM, 9, 6450);
    addEngineParameter("f-m", "FM", EngineParameterType_NRPM, 10, 896);
    addEngineParameter("d-m", "DM", EngineParameterType_NRPM, 11, 0);
    addEngineParameter("b-m", "BM", EngineParameterType_NRPM, 12, 640);
    addEngineParameter("a-f", "AF", EngineParameterType_NRPM, 13, 2048);
    addEngineParameter("d-f", "DF", EngineParameterType_NRPM, 14, 4096);
    addEngineParameter("i", "I", EngineParameterType_NRPM, 15, 80);
    addEngineParameter("ratiomode", "Ratio Mode", EngineParameterType_CC, 16, 0);
    addEngineParameter("envsync", "Env Sync", EngineParameterType_CC, 17, 0);

    addEngine("ds", "Digital Snare", EngineType_Drum);
    addEngineParameter("freq", "Freq", EngineParameterType_CC, 8, 32);
    addEngineParameter("tone", "Tone", EngineParameterType_CC, 9, 16);
    addEngineParameter("decay", "Decay", EngineParameterType_CC, 10, 16);
    addEngineParameter("noise", "Noise", EngineParameterType_CC, 11, 32);
    addEngineParameter("accent", "Accent", EngineParameterType_CC, 12, 32);

    addEngine("as", "Analog Snare", EngineType_Drum);
    addEngineParameter("freq", "Freq", EngineParameterType_CC, 8, 32);
    addEngineParameter("tone", "Tone", EngineParameterType_CC, 9, 16);
    addEngineParameter("decay", "Decay", EngineParameterType_CC, 10, 16);
    addEngineParameter("noise", "Noise", EngineParameterType_CC, 11, 32);
    addEngineParameter("accent", "Accent", EngineParameterType_CC, 12, 32);

    addEngine("hh1", "Hi-Hat 1", EngineType_Drum);
    addEngineParameter("freq", "Freq", EngineParameterType_CC, 8, 32);
    addEngineParameter("tone", "Tone", EngineParameterType_CC, 9, 16);
    addEngineParameter("decay", "Decay", EngineParameterType_CC, 10, 16);
    addEngineParameter("noise", "Noise", EngineParameterType_CC, 11, 32);
    addEngineParameter("accent", "Accent", EngineParameterType_CC, 12, 32);

    addEngine("hh2", "Hi-Hat 2", EngineType_Drum);
    addEngineParameter("freq", "Freq", EngineParameterType_CC, 8, 32);
    addEngineParameter("tone", "Tone", EngineParameterType_CC, 9, 16);
    addEngineParameter("decay", "Decay", EngineParameterType_CC, 10, 16);
    addEngineParameter("noise", "Noise", EngineParameterType_CC, 11, 32);
    addEngineParameter("accent", "Accent", EngineParameterType_CC, 12, 32);

    addEngine("hh2", "Hi-Hat 2", EngineType_Drum);
    addEngineParameter("freq", "Freq", EngineParameterType_CC, 8, 32);
    addEngineParameter("tone", "Tone", EngineParameterType_CC, 9, 16);
    addEngineParameter("decay", "Decay", EngineParameterType_CC, 10, 16);
    addEngineParameter("noise", "Noise", EngineParameterType_CC, 11, 32);
    addEngineParameter("accent", "Accent", EngineParameterType_CC, 12, 32);

    addEngine("rs", "Rimshot", EngineType_Drum);
    addEngineParameter("freq", "Freq", EngineParameterType_CC, 8, 32);
    addEngineParameter("tone", "Tone", EngineParameterType_CC, 9, 16);
    addEngineParameter("decay", "Decay", EngineParameterType_CC, 10, 16);
    addEngineParameter("noise", "Noise", EngineParameterType_CC, 11, 32);
    addEngineParameter("accent", "Accent", EngineParameterType_CC, 12, 32);

    addEngine("cl", "Clap", EngineType_Drum);
    addEngineParameter("freq", "Freq", EngineParameterType_CC, 8, 16);
    addEngineParameter("tone", "Tone", EngineParameterType_CC, 9, 10);
    addEngineParameter("decay", "Decay", EngineParameterType_CC, 10, 10);
    addEngineParameter("scale", "Scale", EngineParameterType_CC, 11, 4);
    addEngineParameter("trans", "Trans", EngineParameterType_CC, 12, 4);

    addEngine("ro", "Rompler", EngineType_Synth);
    addEngineParameter("bank", "Bank", EngineParameterType_CC, 8, 0);
    addEngineParameter("slice", "Slice", EngineParameterType_CC, 9, 0);
    addEngineParameter("start", "Start", EngineParameterType_CC, 10, 0);
    addEngineParameter("end", "End", EngineParameterType_CC, 11, 127);
    addEngineParameter("cutoff", "Cutoff", EngineParameterType_CC, 12, 127);
    addEngineParameter("reso", "Reso", EngineParameterType_CC, 13, 0);
    addEngineParameter("type", "Type", EngineParameterType_CC, 14, 0);
    addEngineParameter("bitcr", "Bit.CR", EngineParameterType_CC, 15, 0);
    addEngineParameter("attack", "Attack", EngineParameterType_CC, 16, 0);
    addEngineParameter("decay", "Decay", EngineParameterType_CC, 17, 64);
    addEngineParameter("speed", "Speed", EngineParameterType_CC, 18, 64);
    addEngineParameter("pitch", "Pitch", EngineParameterType_CC, 19, 64);
    addEngineParameter("loop", "Loop", EngineParameterType_CC, 20, 0);
    addEngineParameter("pingpong", "PingPong", EngineParameterType_CC, 21, 0);
    addEngineParameter("ppstart", "PPStart", EngineParameterType_CC, 22, 64);
    addEngineParameter("eg2fm", "EG2FM", EngineParameterType_CC, 23, 0);
    addEngineParameter("tsmode", "TSMode", EngineParameterType_CC, 24, 0);
    addEngineParameter("tsamt", "TSAmt", EngineParameterType_CC, 25, 64);

    addEngine("td3", "TBD03", EngineType_Synth);
    addEngineParameter("shape", "Bank", EngineParameterType_CC, 8, 0);
    addEngineParameter("p0", "P0", EngineParameterType_NRPM, 9, 0);
    addEngineParameter("vca_d", "VCA D", EngineParameterType_NRPM, 10, 8);
    addEngineParameter("vcf_d", "VCF D", EngineParameterType_NRPM, 11, 8);
    addEngineParameter("cutoff", "Cutoff", EngineParameterType_NRPM, 12, 64);
    addEngineParameter("reso", "Reso", EngineParameterType_NRPM, 13, 0);
    addEngineParameter("envdec", "EnvDec", EngineParameterType_NRPM, 14, 16);
    addEngineParameter("type", "Type", EngineParameterType_CC, 15, 0);
    addEngineParameter("satur", "Satur", EngineParameterType_NRPM, 16, 0);
    addEngineParameter("drive", "Drive", EngineParameterType_NRPM, 17, 0);
    addEngineParameter("slide", "Slide", EngineParameterType_CC, 18, 0);
    addEngineParameter("accent", "Accent", EngineParameterType_CC, 19, 0);
    addEngineParameter("p1", "P1", EngineParameterType_NRPM, 20, 0);
    addEngineParameter("p0amt", "P0 Amt", EngineParameterType_NRPM, 21, 0);
    addEngineParameter("p1amt", "P1 Amt", EngineParameterType_NRPM, 22, 0);
    addEngineParameter("acclev", "Acc. Lev", EngineParameterType_NRPM, 23, 0);
    addEngineParameter("slidelev", "Slide Lev", EngineParameterType_CC, 24, 0);
    addEngineParameter("synctrig", "Sync Trig", EngineParameterType_CC, 25, 0);

    addEngine("mo", "Mono Synth", EngineType_Synth);
    addEngineParameter("shape", "Bank", EngineParameterType_CC, 8, 0);
    addEngineParameter("p0", "P0", EngineParameterType_NRPM, 9, 0);
    addEngineParameter("p1", "P1", EngineParameterType_NRPM, 10, 0);
    addEngineParameter("waveshap", "Waveshape", EngineParameterType_NRPM, 11, 0);
    addEngineParameter("p0a", "P0 A", EngineParameterType_NRPM, 12, 0);
    addEngineParameter("p1a", "P1 A", EngineParameterType_NRPM, 13, 0);
    addEngineParameter("fma", "FM A", EngineParameterType_NRPM, 14, 0);
    addEngineParameter("qscale", "Q Scale", EngineParameterType_CC, 15, 32);
    addEngineParameter("attack", "Attack", EngineParameterType_NRPM, 16, 0);
    addEngineParameter("decay", "Decay", EngineParameterType_CC, 17, 32);
    addEngineParameter("loopenv", "Loop Env", EngineParameterType_CC, 18, 0);
    addEngineParameter("decim", "Decim", EngineParameterType_CC, 19, 0);
    addEngineParameter("bitred", "Bit Red", EngineParameterType_CC, 20, 0);

    addEngine("wtosc", "Wavetable Osc", EngineType_Synth);
    addEngineParameter("wavebank", "Bank", EngineParameterType_CC, 8, 0);
    addEngineParameter("wave", "Wave", EngineParameterType_NRPM, 9, 0);
    addEngineParameter("tune", "Tune", EngineParameterType_NRPM, 10, 0);
    addEngineParameter("type", "Type", EngineParameterType_CC, 11, 0);
    addEngineParameter("cutoff", "Cutoff", EngineParameterType_NRPM, 12, 127);
    addEngineParameter("reso", "Reso", EngineParameterType_NRPM, 13, 0);
    addEngineParameter("gain", "Gain", EngineParameterType_NRPM, 14, 64);
    addEngineParameter("attack", "Attack", EngineParameterType_NRPM, 15, 0);
    addEngineParameter("decay", "Decay", EngineParameterType_NRPM, 16, 64);
    addEngineParameter("sustain", "Sustain", EngineParameterType_NRPM, 17, 0);
    addEngineParameter("release", "Release", EngineParameterType_NRPM, 18, 16);
    addEngineParameter("e2wave", "E2 Wave", EngineParameterType_NRPM, 19, 0);
    addEngineParameter("e2fm", "E2 FM", EngineParameterType_NRPM, 20, 0);
    addEngineParameter("e2filt", "E2 Filt", EngineParameterType_NRPM, 21, 0);
    addEngineParameter("speed", "Speed", EngineParameterType_NRPM, 22, 10);
    addEngineParameter("sync", "Sync", EngineParameterType_CC, 23, 0);
    addEngineParameter("l2wave", "L2 Wave", EngineParameterType_NRPM, 24, 0);
    addEngineParameter("l2am", "L2 AM", EngineParameterType_NRPM, 25, 0);
    addEngineParameter("l2fm", "L2 FM", EngineParameterType_NRPM, 26, 0);
    addEngineParameter("l2filt", "L2 Filt", EngineParameterType_NRPM, 27, 0);

    addEngine("pp", "Polypad", EngineType_Synth);
    addEngineParameter("chord", "Chord", EngineParameterType_CC, 8, 0);
    addEngineParameter("inver", "Inversion", EngineParameterType_CC, 9, 0);
    addEngineParameter("detune", "Detune", EngineParameterType_NRPM, 10, 8);
    addEngineParameter("cutoff", "Cutoff", EngineParameterType_NRPM, 11, 80);
    addEngineParameter("reso", "Reso", EngineParameterType_NRPM, 12, 0);
    addEngineParameter("type", "Type", EngineParameterType_CC, 13, 32);
    addEngineParameter("qscale", "Q Scale", EngineParameterType_CC, 14, 0);
    addEngineParameter("attack", "Attack", EngineParameterType_NRPM, 15, 0);
    addEngineParameter("decay", "Decay", EngineParameterType_NRPM, 16, 8);
    addEngineParameter("sustain", "Sustain", EngineParameterType_NRPM, 17, 8);
    addEngineParameter("release", "Release", EngineParameterType_NRPM, 18, 16);
    addEngineParameter("l1spd", "L1 Speed", EngineParameterType_NRPM, 19, 0);
    addEngineParameter("l1amt", "L1 Amount", EngineParameterType_NRPM, 20, 0);
    addEngineParameter("l2spd", "L2 Speed", EngineParameterType_NRPM, 21, 0);
    addEngineParameter("l2amt", "L2 Amount", EngineParameterType_NRPM, 22, 0);
    addEngineParameter("efltamt", "E Filter Amount", EngineParameterType_NRPM, 23, 0);
    addEngineParameter("l2rand", "L2 Random", EngineParameterType_CC, 24, 127);
    addEngineParameter("nnotes", "Number of Notes", EngineParameterType_CC, 25, 32);

    addEngine("tbd", "TBDings", EngineType_Synth);
    addEngineParameter("model", "Model", EngineParameterType_CC, 8, 0);
    addEngineParameter("freq", "Freq", EngineParameterType_NRPM, 9, 8192);
    addEngineParameter("struc", "Structure", EngineParameterType_NRPM, 10, 8192);
    addEngineParameter("pos", "Position", EngineParameterType_NRPM, 11, 4900);
    addEngineParameter("bright", "Bright", EngineParameterType_NRPM, 12, 10000);
    addEngineParameter("damp", "Damp", EngineParameterType_NRPM, 13, 7000);
    addEngineParameter("chord", "Chord", EngineParameterType_CC, 14, 0);
    addEngineParameter("poly", "Poly", EngineParameterType_CC, 15, 1);
    addEngineParameter("envsh", "Env Shape", EngineParameterType_NRPM, 16, 4000);
    addEngineParameter("vela", "Vel Amt", EngineParameterType_NRPM, 17, 8000);
    addEngineParameter("air", "Air", EngineParameterType_NRPM, 18, 0);
    addEngineParameter("pluck", "Pluck", EngineParameterType_CC, 19, 0);
    addEngineParameter("mtype", "Mod Type", EngineParameterType_CC, 20, 0);
    addEngineParameter("mdpth", "Mod Depth", EngineParameterType_NRPM, 21, 0);
    addEngineParameter("mrate", "Mod Rate", EngineParameterType_NRPM, 22, 8000);
    addEngineParameter("msnap", "Mod Snap", EngineParameterType_CC, 23, 0);

    addEngine("tbdait", "TBDaits", EngineType_Synth);
    addEngineParameter("model", "Model", EngineParameterType_CC, 8, 2);
    addEngineParameter("freq", "Freq", EngineParameterType_NRPM, 9, 8192);
    addEngineParameter("harm", "Harm", EngineParameterType_NRPM, 10, 8192);
    addEngineParameter("timbre", "Timbre", EngineParameterType_NRPM, 11, 8192);
    addEngineParameter("morph", "Morph", EngineParameterType_NRPM, 12, 8192);
    addEngineParameter("decay", "Decay", EngineParameterType_NRPM, 13, 13000);
    addEngineParameter("color", "Color", EngineParameterType_CC, 14, 1);
    addEngineParameter("level", "Level", EngineParameterType_NRPM, 15, 11600);
    addEngineParameter("fmod", "FMod", EngineParameterType_NRPM, 16, 0);
    addEngineParameter("tmod", "TMod", EngineParameterType_NRPM, 17, 0);
    addEngineParameter("mmod", "MMod", EngineParameterType_NRPM, 18, 0);

    addEngine("extsynth", "External Synth", EngineType_Synth);
    addEngineParameter("chan", "MIDI Channel", EngineParameterType_CC, 8, 0);

    addEngine("extdrum", "External Drum", EngineType_Drum);
    addEngineParameter("chan", "MIDI Channel", EngineParameterType_CC, 8, 0);
    addEngineParameter("note", "MIDI Note", EngineParameterType_CC, 9, 36);

    addEngine("inp", "Audio Input", EngineType_Synth);
    addEngineParameter("in_gain", "Gain", EngineParameterType_NRPM, 8, 1024);
    addEngineParameter("in_mono", "Mono", EngineParameterType_CC, 9, 0);
    addEngineParameter("in_hp", "HP", EngineParameterType_NRPM, 10, 0);
    addEngineParameter("in_drive", "Drive", EngineParameterType_NRPM, 11, 0);
    addEngineParameter("in_ftype", "FType", EngineParameterType_CC, 12, 0);
    addEngineParameter("in_fcutoff", "FCut", EngineParameterType_NRPM, 13, 4095);
    addEngineParameter("in_freso", "FReso", EngineParameterType_NRPM, 14, 0);
    addEngineParameter("in_fenv", "FEnv", EngineParameterType_NRPM, 15, 0);
}

void SynthDefinitionDataModel::addDrumTrack(const char *name, int midiChannel, int baseCC, int drumNote, const char *defaultbank) {
    lastTrack ++;
    SharedTrackDefinition *track = &tracks[lastTrack];
    track->index = lastTrack;
    track->type = TRACK_TYPE_DRUM;
    strlcpy(track->name, name, 16);
    track->midiChannel = midiChannel;
    track->baseCC = baseCC;
    track->drumNote = drumNote;
    memset(track->engineIdStr, 0, sizeof(track->engineIdStr));
    // strncpy(tracks[trackIndex].ki0], defaultbank, 15
}

void SynthDefinitionDataModel::addSynthTrack(const char *name, int midiChannel, int baseCC, const char *defaultbank) {
    lastTrack ++;
    SharedTrackDefinition *track = &tracks[lastTrack];
    track->index = lastTrack;
    track->type = TRACK_TYPE_SYNTH;
    strlcpy(track->name, name, 16);
    track->midiChannel = midiChannel;
    track->baseCC = baseCC;
    track->drumNote = -1;
    memset(track->engineIdStr, 0, sizeof(track->engineIdStr));
}

void SynthDefinitionDataModel::addFxTrack(const char *name, int midiChannel, int baseCC, const char *defaultbank) {
    lastTrack ++;
    SharedTrackDefinition *track = &tracks[lastTrack];
    track->index = lastTrack;
    track->type = TRACK_TYPE_FX;
    strlcpy(track->name, name, 16);
    track->midiChannel = midiChannel;
    track->baseCC = baseCC;
    track->drumNote = -1;
    memset(track->engineIdStr, 0, sizeof(track->engineIdStr));
}

void SynthDefinitionDataModel::addEngine(const char *id, const char *name, enum SharedEngineType type) {
    lastEngine ++;
    struct SharedEngineDefinition *s = &synths[lastEngine];
    strlcpy(s->idStr, id, 16);
    strlcpy(s->name, name, 32);
    s->type = type;
}

void SynthDefinitionDataModel::addTrackEngine(const char *machineId) {
    int index = 0;
    while(index < MaxTrackDefinitionEngineIds) {
        if (tracks[lastTrack].engineIdStr[index][0] == '\0') {
            strlcpy(tracks[lastTrack].engineIdStr[index], machineId, 16);
            break;
        }
        index ++;
    }
}

void SynthDefinitionDataModel::addEngineParameter(const char *id, const char *name, enum SharedEngineParameterType type, int ccIndex, int defaultValue) {
    int index = 0;
    while(index < MaxEngineDefinitionParameters) {
        if (synths[lastEngine].parameters[index].id[0] == '\0') {
            strlcpy(synths[lastEngine].parameters[index].id, id, 16);
            strlcpy(synths[lastEngine].parameters[index].name, name, 32);
            synths[lastEngine].parameters[index].type = type;
            synths[lastEngine].parameters[index].relCC = ccIndex;
            synths[lastEngine].parameters[index].defaultValue = defaultValue;
            break;
        }
        index ++;
    }
}

int SynthDefinitionDataModel::GetNumberOfSynthDefinitions() {
    int count = 0;
    for(int i=0; i<MAX_SYNTHS; i++) {
        if (synths[i].idStr[0] != '\0') {
            count ++;
        }
    }
    return count;
}

struct SharedEngineDefinition *SynthDefinitionDataModel::GetSynthDefinition(const std::string id) {
    for(int i=0; i<MAX_SYNTHS; i++) {
        if (strcmp(synths[i].idStr, id.c_str()) == 0) {
            return &synths[i];
        }
    }
    return nullptr;
}

struct SharedTrackDefinition *SynthDefinitionDataModel::GetTrackDefinition(int index) {
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
        struct SharedEngineDefinition *def = &synths[j];

        if (def->idStr[0] == '\0') continue;

        Value machineObj(kObjectType);

        machineObj.AddMember("id", Value(def->idStr, doc.GetAllocator()), doc.GetAllocator());
        machineObj.AddMember("name", Value(def->name, doc.GetAllocator()), doc.GetAllocator());

        Value paramsarray(kArrayType);
        machineObj.AddMember("parameters", paramsarray, doc.GetAllocator());
        for(int i=0; i<MaxEngineDefinitionParameters; i++) {
            if (def->parameters[i].id[0] == '\0') {
                continue;
            }
            Value param(kObjectType);
            param.AddMember("id", Value(def->parameters[i].id, doc.GetAllocator()), doc.GetAllocator());
            param.AddMember("name", Value(def->parameters[i].name, doc.GetAllocator()), doc.GetAllocator());
            std::string typestring;
            if (def->parameters[i].type == EngineParameterType_CC) {
                typestring = "cc";
            } else if (def->parameters[i].type == EngineParameterType_NRPM) {
                typestring = "nrpm";
            } else {
                typestring = "none";
            }
            param.AddMember("type", Value(typestring.c_str(), doc.GetAllocator()), doc.GetAllocator());
            param.AddMember("ctrl", def->parameters[i].relCC, doc.GetAllocator());
            param.AddMember("def", def->parameters[i].defaultValue, doc.GetAllocator());
            machineObj["parameters"].PushBack(param, doc.GetAllocator());
        }

        doc["machines"].PushBack(machineObj, doc.GetAllocator());
    }

    Value tracksarray(kArrayType);
    doc.AddMember("tracks", tracksarray, doc.GetAllocator());
    for(int j=0; j<MAX_TRACKS; j++) {
        struct SharedTrackDefinition *def = &tracks[j];

        if (def->name[0] == '\0') continue;

        Value trackObj(kObjectType);

        trackObj.AddMember("index", def->index, doc.GetAllocator());
        trackObj.AddMember("name", Value(def->name, doc.GetAllocator()), doc.GetAllocator());
        trackObj.AddMember("midichannel", def->midiChannel, doc.GetAllocator());
        trackObj.AddMember("drumnote", def->drumNote, doc.GetAllocator());
        trackObj.AddMember("basecc", def->baseCC, doc.GetAllocator());

        Value machinesarray(kArrayType);
        trackObj.AddMember("machines", machinesarray, doc.GetAllocator());
        for(int k=0; k<MaxTrackDefinitionEngineIds; k++) {
            if (def->engineIdStr[k][0] == '\0') {
                continue;
            }
            trackObj["machines"].PushBack(Value(def->engineIdStr[k], doc.GetAllocator()), doc.GetAllocator());
        }

        doc["tracks"].PushBack(trackObj, doc.GetAllocator());
    }

    return true;
}

#pragma GCC diagnostic ignored "-Wstringop-truncation"
void SynthDefinitionDataModel::WriteListResponse(struct GetEngineDefinitionIdListResponse *r) {
    r->numEngines = 0;
    for(int i=0; i<MAX_SYNTHS; i++) {
        if (synths[i].idStr[0] != '\0') {
            strncpy((char *)&r->engineIds[r->numEngines], (const char *)&synths[i].idStr, 8);
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

        struct SharedEngineDefinition *def = &synths[i];
        if (def->idStr[0] == '\0') {
            continue;
        }

        strcpy( response->engineDefinitions[k].idStr, def->idStr);
        strncpy(response->engineDefinitions[k].name, def->name, 8);

        for(int p=0; p<MaxEngineDefinitionParameters; p++) {
            response->engineDefinitions[k].parameters[p].type = EngineParameterType_None;
            if (def->parameters[p].type == EngineParameterType_CC) {
                response->engineDefinitions[k].parameters[p].type = EngineParameterType_CC;
            } else if (def->parameters[p].type == EngineParameterType_NRPM) {
                response->engineDefinitions[k].parameters[p].type = EngineParameterType_NRPM;
            }

            response->engineDefinitions[k].parameters[p].defaultValue = def->parameters[p].defaultValue;
            response->engineDefinitions[k].parameters[p].relCC = def->parameters[p].relCC;

            strncpy(response->engineDefinitions[k].parameters[p].name, def->parameters[p].name, 32);
        }

        response->returnedEngineDefinitions ++;
    }
}

static SynthDefinitionDataModel g_synthdef_instance;

SynthDefinitionDataModel *SynthDefinitionDataModel::instance() {
    return &g_synthdef_instance;
}