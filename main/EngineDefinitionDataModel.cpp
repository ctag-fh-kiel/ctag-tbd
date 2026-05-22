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

#include "EngineDefinitionDataModel.hpp"
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

static void TrackDefinition_Reset(struct TrackDefinition *def) {
    def->index = 0;
    memset(def->name, 0, sizeof(def->name));
    def->midiChannel = 0;
    def->drumNote = 0;
    def->baseCC = 0;
    memset(def->engineIdStr, 0, sizeof(def->engineIdStr));
}


void EngineDefinitionDataModel::Init() {
    synths = (struct SharedEngineDefinition *) heap_caps_malloc(MAX_SYNTHS * sizeof(struct SharedEngineDefinition), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    tracks = (struct TrackDefinition *) heap_caps_malloc(MAX_TRACKS * sizeof(struct TrackDefinition), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);

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

    trackAddDrum("Kick", 9, 0, 36, "");
    trackEngine("nodrum");
    trackEngine("db");
    trackEngine("ab");
    trackEngine("ro");
    trackEngine("extdrum");

    trackAddDrum("Kick2", 9, 40, 37, "");
    trackEngine("nodrum");
    trackEngine("fmb");
    trackEngine("ro");
    trackEngine("extdrum");

    trackAddDrum("Snare", 9, 80, 38, "");
    trackEngine("nodrum");
    trackEngine("ds");
    trackEngine("as");
    trackEngine("do");
    trackEngine("extdrum");

    trackAddDrum("Hat", 10, 0, 36, "");
    trackEngine("nodrum");
    trackEngine("hh1");
    trackEngine("hh2");
    trackEngine("ro");
    trackEngine("extdrum");

    trackAddDrum("Rimshot", 10, 40, 37, "");
    trackEngine("nodrum");
    trackEngine("rs");
    trackEngine("ro");
    trackEngine("extdrum");

    trackAddDrum("Clap", 10, 80, 38, "");
    trackEngine("nodrum");
    trackEngine("cl");
    trackEngine("ro");
    trackEngine("extdrum");

    trackAddDrum("Rompler", 11, 0, 36, "percussion");
    trackEngine("nodrum");
    trackEngine("ro");
    trackEngine("extdrum");

    trackAddDrum("Rompler", 11, 40, 37, "chords");
    trackEngine("nodrum");
    trackEngine("ro");
    trackEngine("extdrum");

    trackAddSynth("Bass", 0, 0, "");
    trackEngine("nosynth");
    trackEngine("td3");
    trackEngine("ro");
    trackEngine("extsynth");

    trackAddSynth("Bass2", 1, 0, "");
    trackEngine("nosynth");
    trackEngine("td3");
    trackEngine("ro");
    trackEngine("extsynth");

    trackAddSynth("Lead", 2, 0, "");
    trackEngine("nosynth");
    trackEngine("mo");
    trackEngine("ro");
    trackEngine("extsynth");

    trackAddSynth("Lead2", 3, 0, "");
    trackEngine("nosynth");
    trackEngine("wtosc");
    trackEngine("mo");
    trackEngine("tbd");
    trackEngine("tbdait");
    trackEngine("ro");
    trackEngine("extsynth");

    trackAddSynth("Rompler", 4, 0, "stabs");
    trackEngine("nosynth");
    trackEngine("ro");
    trackEngine("extsynth");

    trackAddSynth("Rompler", 5, 0, "synths");
    trackEngine("nosynth");
    trackEngine("ro");
    trackEngine("extsynth");

    trackAddSynth("Chords", 6, 0, "");
    trackEngine("nosynth");
    trackEngine("pp");
    trackEngine("tbd");
    trackEngine("ro");
    trackEngine("extsynth");

    trackAddSynth("Input", 7, 0, "");
    trackEngine("nosynth");
    trackEngine("imp");

    trackAddFx("FX1", 12, 0, "");
    trackEngine("nofx");
    trackEngine("fxdelay");

    trackAddFx("FX2", 12, 20, "");
    trackEngine("nofx");
    trackEngine("fxreverb");

    trackAddFx("Master", 12, 40, "");
    trackEngine("nofx");
    trackEngine("fxmaster");

    //
    // Initialize synths
    //

    lastEngine = -1;

    engineAdd("nodrum", "Empty drum", EngineType_Drum);

    engineAdd("nosynth", "Empty synth", EngineType_Synth);

    engineAdd("nofx", "Empty FX", EngineType_FX);

    engineAdd("fxdelay", "FX Delay", EngineType_FX);
    engineCC("time", "Time", 8, 16);
    engineCC("sync", "Sync", 9, 0);
    engineCC("freeze", "Freeze", 10, 0);
    engineCC("tapedig", "Tapedig", 11, 0);
    engineCC("stwid", "Stereo width", 12, 32);
    engineCC("fx2send", "FX2 Send", 13, 0);
    engineCC("feedback", "Feedback", 14, 32);
    engineCC("base", "Base", 15, 0);
    engineCC("width2", "Width 2", 16, 32);
    engineCC("level", "Level", 17, 64);
    engineCC("inputhp", "Input HP", 18, 0);

    engineAdd("fxreverb", "FX Reverb", EngineType_FX);
    engineCC("time", "Time", 8, 64);
    engineCC("lowpass", "Lowpass", 9, 96);
    engineCC("level", "Level", 10, 64);
    engineCC("diffuse", "Diffuse", 11, 89);
    engineCC("predelay", "PreDly", 12, 14);
    engineCC("modulation", "ModRate", 13, 64);
    engineCC("inputgain", "Drive", 14, 64);
    engineCC("tanklevel", "TankLvl", 15, 127);
    engineCC("hp", "HP", 16, 0);

    engineAdd("fxmaster", "FX Master", EngineType_FX);
    engineCC("compthres", "Thresh", 8, 100);
    engineCC("compratio", "Ratio", 9, 32);
    engineCC("compatk", "Attack", 10, 0);
    engineCC("comprel", "Release", 11, 20);
    engineCC("complpf", "LPF", 12, 48);
    engineCC("compgain", "Gain", 13, 0);
    engineCC("compmix", "Mix", 14, 64);
    engineCC("compdlylev", "Dly.Lev", 15, 64);
    engineCC("comprevlev", "Rev.Lev", 16, 64);
    engineCC("summute", "Sum mute", 17, 0);
    engineCC("sumlev", "Sum lev", 18, 64);

    engineAdd("db", "Synth Kick", EngineType_Drum);
    engineNRPM("freq", "Freq", 8, 2048);
    engineCC("tone", "Tone", 9, 64);
    engineCC("decay", "Decay", 10, 32);
    engineCC("dirt", "Dirt", 11, 0);
    engineNRPM("fm-env", "Fm Env", 12, 2048);
    engineNRPM("fm-decay", "Fm Decay", 13, 1024);
    engineCC("fm-accent", "Fm Accent", 14, 8);

    engineAdd("ab", "Analog Bass Drum", EngineType_Drum);
    engineNRPM("freq", "Freq", 8, 2048);
    engineCC("tone", "Tone", 9, 32);
    engineCC("decay", "Decay", 10, 64);
    engineCC("a-fm", "A FM", 11, 256);
    engineNRPM("s-fm", "S FM", 12, 256);
    engineCC("accent", "Accent", 13, 2);

    engineAdd("fmb", "FM Kick", EngineType_Drum);
    engineNRPM("f-b", "FM", 8, 3072);
    engineNRPM("d-b", "DB", 9, 6450);
    engineNRPM("f-m", "FM", 10, 896);
    engineNRPM("d-m", "DM", 11, 0);
    engineNRPM("b-m", "BM", 12, 640);
    engineNRPM("a-f", "AF", 13, 2048);
    engineNRPM("d-f", "DF", 14, 4096);
    engineNRPM("i", "I", 15, 80);
    engineCC("ratiomode", "Ratio Mode", 16, 0);
    engineCC("envsync", "Env Sync", 17, 0);

    engineAdd("ds", "Digital Snare", EngineType_Drum);
    engineCC("freq", "Freq", 8, 32);
    engineCC("tone", "Tone", 9, 16);
    engineCC("decay", "Decay", 10, 16);
    engineCC("noise", "Noise", 11, 32);
    engineCC("accent", "Accent", 12, 32);

    engineAdd("as", "Analog Snare", EngineType_Drum);
    engineCC("freq", "Freq", 8, 32);
    engineCC("tone", "Tone", 9, 16);
    engineCC("decay", "Decay", 10, 16);
    engineCC("noise", "Noise", 11, 32);
    engineCC("accent", "Accent", 12, 32);

    engineAdd("hh1", "Hi-Hat 1", EngineType_Drum);
    engineCC("freq", "Freq", 8, 32);
    engineCC("tone", "Tone", 9, 16);
    engineCC("decay", "Decay", 10, 16);
    engineCC("noise", "Noise", 11, 32);
    engineCC("accent", "Accent", 12, 32);

    engineAdd("hh2", "Hi-Hat 2", EngineType_Drum);
    engineCC("freq", "Freq", 8, 32);
    engineCC("tone", "Tone", 9, 16);
    engineCC("decay", "Decay", 10, 16);
    engineCC("noise", "Noise", 11, 32);
    engineCC("accent", "Accent", 12, 32);

    engineAdd("hh2", "Hi-Hat 2", EngineType_Drum);
    engineCC("freq", "Freq", 8, 32);
    engineCC("tone", "Tone", 9, 16);
    engineCC("decay", "Decay", 10, 16);
    engineCC("noise", "Noise", 11, 32);
    engineCC("accent", "Accent", 12, 32);

    engineAdd("rs", "Rimshot", EngineType_Drum);
    engineCC("freq", "Freq", 8, 32);
    engineCC("tone", "Tone", 9, 16);
    engineCC("decay", "Decay", 10, 16);
    engineCC("noise", "Noise", 11, 32);
    engineCC("accent", "Accent", 12, 32);

    engineAdd("cl", "Clap", EngineType_Drum);
    engineCC("freq", "Freq", 8, 16);
    engineCC("tone", "Tone", 9, 10);
    engineCC("decay", "Decay", 10, 10);
    engineCC("scale", "Scale", 11, 4);
    engineCC("trans", "Trans", 12, 4);

    engineAdd("ro", "Rompler", EngineType_Synth);
    engineCC("bank", "Bank", 8, 0);
    engineCC("slice", "Slice", 9, 0);
    engineCC("start", "Start", 10, 0);
    engineCC("end", "End", 11, 127);
    engineCC("cutoff", "Cutoff", 12, 127);
    engineCC("reso", "Reso", 13, 0);
    engineCC("type", "Type", 14, 0);
    engineCC("bitcr", "Bit.CR", 15, 0);
    engineCC("attack", "Attack", 16, 0);
    engineCC("decay", "Decay", 17, 64);
    engineCC("speed", "Speed", 18, 64);
    engineCC("pitch", "Pitch", 19, 64);
    engineCC("loop", "Loop", 20, 0);
    engineCC("pingpong", "PingPong", 21, 0);
    engineCC("ppstart", "PPStart", 22, 64);
    engineCC("eg2fm", "EG2FM", 23, 0);
    engineCC("tsmode", "TSMode", 24, 0);
    engineCC("tsamt", "TSAmt", 25, 64);

    engineAdd("td3", "TBD03", EngineType_Synth);
    engineCC("shape", "Bank", 8, 0);
    engineNRPM("p0", "P0", 9, 0);
    engineNRPM("vca_d", "VCA D", 10, 8);
    engineNRPM("vcf_d", "VCF D", 11, 8);
    engineNRPM("cutoff", "Cutoff", 12, 64);
    engineNRPM("reso", "Reso", 13, 0);
    engineNRPM("envdec", "EnvDec", 14, 16);
    engineCC("type", "Type", 15, 0);
    engineNRPM("satur", "Satur", 16, 0);
    engineNRPM("drive", "Drive", 17, 0);
    engineCC("slide", "Slide", 18, 0);
    engineCC("accent", "Accent", 19, 0);
    engineNRPM("p1", "P1", 20, 0);
    engineNRPM("p0amt", "P0 Amt", 21, 0);
    engineNRPM("p1amt", "P1 Amt", 22, 0);
    engineNRPM("acclev", "Acc. Lev", 23, 0);
    engineCC("slidelev", "Slide Lev", 24, 0);
    engineCC("synctrig", "Sync Trig", 25, 0);

    engineAdd("mo", "Mono Synth", EngineType_Synth);
    engineCC("shape", "Bank", 8, 0);
    engineNRPM("p0", "P0", 9, 0);
    engineNRPM("p1", "P1", 10, 0);
    engineNRPM("waveshap", "Waveshape", 11, 0);
    engineNRPM("p0a", "P0 A", 12, 0);
    engineNRPM("p1a", "P1 A", 13, 0);
    engineNRPM("fma", "FM A", 14, 0);
    engineCC("qscale", "Q Scale", 15, 32);
    engineNRPM("attack", "Attack", 16, 0);
    engineCC("decay", "Decay", 17, 32);
    engineCC("loopenv", "Loop Env", 18, 0);
    engineCC("decim", "Decim", 19, 0);
    engineCC("bitred", "Bit Red", 20, 0);

    engineAdd("wtosc", "Wavetable Osc", EngineType_Synth);
    engineCC("wavebank", "Bank", 8, 0);
    engineNRPM("wave", "Wave", 9, 0);
    engineNRPM("tune", "Tune", 10, 0);
    engineCC("type", "Type", 11, 0);
    engineNRPM("cutoff", "Cutoff", 12, 127);
    engineNRPM("reso", "Reso", 13, 0);
    engineNRPM("gain", "Gain", 14, 64);
    engineNRPM("attack", "Attack", 15, 0);
    engineNRPM("decay", "Decay", 16, 64);
    engineNRPM("sustain", "Sustain", 17, 0);
    engineNRPM("release", "Release", 18, 16);
    engineNRPM("e2wave", "E2 Wave", 19, 0);
    engineNRPM("e2fm", "E2 FM", 20, 0);
    engineNRPM("e2filt", "E2 Filt", 21, 0);
    engineNRPM("speed", "Speed", 22, 10);
    engineCC("sync", "Sync", 23, 0);
    engineNRPM("l2wave", "L2 Wave", 24, 0);
    engineNRPM("l2am", "L2 AM", 25, 0);
    engineNRPM("l2fm", "L2 FM", 26, 0);
    engineNRPM("l2filt", "L2 Filt", 27, 0);

    engineAdd("pp", "Polypad", EngineType_Synth);
    engineCC("chord", "Chord", 8, 0);
    engineCC("inver", "Inversion", 9, 0);
    engineNRPM("detune", "Detune", 10, 8);
    engineNRPM("cutoff", "Cutoff", 11, 80);
    engineNRPM("reso", "Reso", 12, 0);
    engineCC("type", "Type", 13, 32);
    engineCC("qscale", "Q Scale", 14, 0);
    engineNRPM("attack", "Attack", 15, 0);
    engineNRPM("decay", "Decay", 16, 8);
    engineNRPM("sustain", "Sustain", 17, 8);
    engineNRPM("release", "Release", 18, 16);
    engineNRPM("l1spd", "L1 Speed", 19, 0);
    engineNRPM("l1amt", "L1 Amount", 20, 0);
    engineNRPM("l2spd", "L2 Speed", 21, 0);
    engineNRPM("l2amt", "L2 Amount", 22, 0);
    engineNRPM("efltamt", "E Filter Amount", 23, 0);
    engineCC("l2rand", "L2 Random", 24, 127);
    engineCC("nnotes", "Number of Notes", 25, 32);

    engineAdd("tbd", "TBDings", EngineType_Synth);
    engineCC("model", "Model", 8, 0);
    engineNRPM("freq", "Freq", 9, 8192);
    engineNRPM("struc", "Structure", 10, 8192);
    engineNRPM("pos", "Position", 11, 4900);
    engineNRPM("bright", "Bright", 12, 10000);
    engineNRPM("damp", "Damp", 13, 7000);
    engineCC("chord", "Chord", 14, 0);
    engineCC("poly", "Poly", 15, 1);
    engineNRPM("envsh", "Env Shape", 16, 4000);
    engineNRPM("vela", "Vel Amt", 17, 8000);
    engineNRPM("air", "Air", 18, 0);
    engineCC("pluck", "Pluck", 19, 0);
    engineCC("mtype", "Mod Type", 20, 0);
    engineNRPM("mdpth", "Mod Depth", 21, 0);
    engineNRPM("mrate", "Mod Rate", 22, 8000);
    engineCC("msnap", "Mod Snap", 23, 0);

    engineAdd("tbdait", "TBDaits", EngineType_Synth);
    engineCC("model", "Model", 8, 2);
    engineNRPM("freq", "Freq", 9, 8192);
    engineNRPM("harm", "Harm", 10, 8192);
    engineNRPM("timbre", "Timbre", 11, 8192);
    engineNRPM("morph", "Morph", 12, 8192);
    engineNRPM("decay", "Decay", 13, 13000);
    engineCC("color", "Color", 14, 1);
    engineNRPM("level", "Level", 15, 11600);
    engineNRPM("fmod", "FMod", 16, 0);
    engineNRPM("tmod", "TMod", 17, 0);
    engineNRPM("mmod", "MMod", 18, 0);

    engineAdd("extsynth", "External Synth", EngineType_Synth);
    engineCC("chan", "MIDI Channel", 8, 0);

    engineAdd("extdrum", "External Drum", EngineType_Drum);
    engineCC("chan", "MIDI Channel", 8, 0);
    engineCC("note", "MIDI Note", 9, 36);

    engineAdd("inp", "Audio Input", EngineType_Synth);
    engineNRPM("in_gain", "Gain", 8, 1024);
    engineCC("in_mono", "Mono", 9, 0);
    engineNRPM("in_hp", "HP", 10, 0);
    engineNRPM("in_drive", "Drive", 11, 0);
    engineCC("in_ftype", "FType", 12, 0);
    engineNRPM("in_fcutoff", "FCut", 13, 4095);
    engineNRPM("in_freso", "FReso", 14, 0);
    engineNRPM("in_fenv", "FEnv", 15, 0);
}

void EngineDefinitionDataModel::trackAddDrum(const char *name, int midiChannel, int baseCC, int drumNote, const char *defaultbank) {
    lastTrack ++;
    TrackDefinition *track = &tracks[lastTrack];
    track->index = lastTrack;
    track->type = TRACK_TYPE_DRUM;
    strlcpy(track->name, name, 16);
    track->midiChannel = midiChannel;
    track->baseCC = baseCC;
    track->drumNote = drumNote;
    memset(track->engineIdStr, 0, sizeof(track->engineIdStr));
    // strncpy(tracks[trackIndex].ki0], defaultbank, 15
}

void EngineDefinitionDataModel::trackAddSynth(const char *name, int midiChannel, int baseCC, const char *defaultbank) {
    lastTrack ++;
    TrackDefinition *track = &tracks[lastTrack];
    track->index = lastTrack;
    track->type = TRACK_TYPE_SYNTH;
    strlcpy(track->name, name, 16);
    track->midiChannel = midiChannel;
    track->baseCC = baseCC;
    track->drumNote = -1;
    memset(track->engineIdStr, 0, sizeof(track->engineIdStr));
}

void EngineDefinitionDataModel::trackAddFx(const char *name, int midiChannel, int baseCC, const char *defaultbank) {
    lastTrack ++;
    TrackDefinition *track = &tracks[lastTrack];
    track->index = lastTrack;
    track->type = TRACK_TYPE_FX;
    strlcpy(track->name, name, 16);
    track->midiChannel = midiChannel;
    track->baseCC = baseCC;
    track->drumNote = -1;
    memset(track->engineIdStr, 0, sizeof(track->engineIdStr));
}

void EngineDefinitionDataModel::trackEngine(const char *machineId) {
    int index = 0;
    while(index < MaxTrackDefinitionEngineIds) {
        if (tracks[lastTrack].engineIdStr[index][0] == '\0') {
            strlcpy(tracks[lastTrack].engineIdStr[index], machineId, 16);
            break;
        }
        index ++;
    }
}

void EngineDefinitionDataModel::engineAdd(const char *id, const char *name, enum SharedEngineType type) {
    lastEngine ++;
    struct SharedEngineDefinition *s = &synths[lastEngine];
    strlcpy(s->idStr, id, 16);
    strlcpy(s->name, name, 32);
    s->type = type;
}

void EngineDefinitionDataModel::engineCC(const char *id, const char *name, int ccIndex, int defaultValue) {
    int index = 0;
    while(index < MaxEngineDefinitionParameters) {
        if (synths[lastEngine].parameters[index].id[0] == '\0') {
            strlcpy(synths[lastEngine].parameters[index].id, id, 16);
            strlcpy(synths[lastEngine].parameters[index].name, name, 32);
            synths[lastEngine].parameters[index].type = EngineParameterType_CC;
            synths[lastEngine].parameters[index].relCC = ccIndex;
            synths[lastEngine].parameters[index].defaultValue = defaultValue;
            break;
        }
        index ++;
    }
}

void EngineDefinitionDataModel::engineNRPM(const char *id, const char *name, int ccIndex, int defaultValue) {
    int index = 0;
    while(index < MaxEngineDefinitionParameters) {
        if (synths[lastEngine].parameters[index].id[0] == '\0') {
            strlcpy(synths[lastEngine].parameters[index].id, id, 16);
            strlcpy(synths[lastEngine].parameters[index].name, name, 32);
            synths[lastEngine].parameters[index].type = EngineParameterType_NRPM;
            synths[lastEngine].parameters[index].relCC = ccIndex;
            synths[lastEngine].parameters[index].defaultValue = defaultValue;
            break;
        }
        index ++;
    }
}

int EngineDefinitionDataModel::GetNumberOfSynthDefinitions() {
    int count = 0;
    for(int i=0; i<MAX_SYNTHS; i++) {
        if (synths[i].idStr[0] != '\0') {
            count ++;
        }
    }
    return count;
}

struct SharedEngineDefinition *EngineDefinitionDataModel::GetSynthDefinition(const std::string id) {
    for(int i=0; i<MAX_SYNTHS; i++) {
        if (strcmp(synths[i].idStr, id.c_str()) == 0) {
            return &synths[i];
        }
    }
    return nullptr;
}

struct TrackDefinition *EngineDefinitionDataModel::GetTrackDefinition(int index) {
    for(int i=0; i<MAX_TRACKS; i++) {
        if (tracks[i].index == index) {
            return &tracks[i];
        }
    }
    return nullptr;
}

bool EngineDefinitionDataModel::SerializeIntoJSON(rapidjson::Document &doc) {
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
        struct TrackDefinition *def = &tracks[j];

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
void EngineDefinitionDataModel::WriteListResponse(struct GetEngineDefinitionIdListResponse *r) {
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

void EngineDefinitionDataModel::WriteEngineDefinitionPageResponse(const struct GetEngineDefinitionsPageRequest *request, struct GetEngineDefinitionsPageResponse *response) {
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

static EngineDefinitionDataModel g_enginedef_instance;

EngineDefinitionDataModel *EngineDefinitionDataModel::instance() {
    return &g_enginedef_instance;
}