/***************
TBD-16 — Macro/Preset System & PicoSeqRack
RackTBDings: FaseAcht-inspired Rings (Mutable Instruments) wrapper for the rack.

(c) 2026 dadamachines / Johannes Lohbihler. https://dadamachines.com
Licensed under the GNU Lesser General Public License (LGPL 3.0).
***************/

#include "RackSynth.hpp"
#include "RackTBDings.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"
#include "stmlib/dsp/dsp.h"
#include "esp_heap_caps.h"
#include <cmath>
#include <new>

using namespace CTAG::SP;

// Reverb buffer size matches Rings' FxEngine<32768, FORMAT_16_BIT>: 32768 uint16_t = 64 KB.
static constexpr size_t kReverbBufferWords = 32768;

// §4.5 ENVELOPE knob morph — 5-point lookup the user can interpolate between.
// Each row: { attack_s, decay_s, sustain_norm, release_s }.
static const float kEnvShapeTable[5][4] = {
    {0.001f, 0.05f, 0.0f, 0.05f},  // Fastest A, Fast D, no sustain
    {0.001f, 0.40f, 0.0f, 0.20f},  // Fastest A, Slower D, no sustain
    {0.001f, 0.10f, 1.0f, 0.005f}, // Fastest A, Sustain, Fastest R
    {0.20f,  0.20f, 1.0f, 0.40f},  // Slower A, Sustain, Slower R
    {1.5f,   0.50f, 1.0f, 1.5f},   // Slowest A, Sustain, Slowest R
};

// §4.6 QUANTIZE harmonic snap — ratios of the played note frequency.
static const float kHarmonicRatios[8] = {0.5f, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 8.f};

// Silence-gate tail — how many blocks of "no reason to make audio" before we
// skip Rings entirely. 5 s at 1378 Hz block rate ≈ 6890 blocks; rounded up to
// clean number. Enough for a long ringing decay, short enough that an idle
// track between sequencer patterns doesn't bleed audio.
static constexpr int kSilenceTailBlocks = 7000;
// AIR is "audibly engaged" above this cv threshold; below it, Air is treated
// as off so the silence gate can engage. Well above LSB noise but well below
// audibly meaningful AIR. (Approximately 1% of full-scale.)
static constexpr int kAirActivityThreshold = 40;

static inline float midiNoteToHz(int note) {
    return 440.f * powf(2.f, (note - 69) / 12.f);
}

// §4.4 Triangle wavefolder. Above unity, output reflects between -1 and +1.
// Bounded iteration count + NaN/Inf guard so a wild Rings transient (e.g.
// during the very first block after enable) cannot lock the audio task in
// an unbounded reflection loop.
static inline float triangleFold(float x) {
    if (!std::isfinite(x)) return 0.f;
    int safety = 32;
    while (x > 1.f && safety-- > 0)  x = 2.f - x;
    safety = 32;
    while (x < -1.f && safety-- > 0) x = -2.f - x;
    return x;
}

void RackTBDings::Init(const PickSeqRackInitData *initdata) {
    // Init is called once at PicoSeqRack construction time, before the audio
    // task starts. heap_caps_malloc here is safe.
    if (!reverb_buffer) {
        reverb_buffer = (uint16_t*)heap_caps_malloc(
            kReverbBufferWords * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
        assert(reverb_buffer != nullptr);
    }
    if (!part) {
        void *mem = heap_caps_malloc(sizeof(rings::Part), MALLOC_CAP_SPIRAM);
        assert(mem != nullptr);
        part = new (mem) rings::Part();
    }
    if (!strummer) {
        void *mem = heap_caps_malloc(sizeof(rings::Strummer), MALLOC_CAP_SPIRAM);
        assert(mem != nullptr);
        strummer = new (mem) rings::Strummer();
    }

    strummer->Init(0.01f, 44100.f / BUF_SZ);
    part->Init(reverb_buffer);
    part->set_polyphony(1);
    part->set_model(rings::RESONATOR_MODEL_MODAL);
    last_polyphony = 1;
    last_model = rings::RESONATOR_MODEL_MODAL;

    paramAD.SetSampleRate(44100.f / BUF_SZ);
    paramAD.SetModeLin();
    paramAD.SetAttack(kEnvShapeTable[0][0]);
    paramAD.SetDecay(kEnvShapeTable[0][1]);

    // Sensible patch defaults (mid-range Rings).
    patch.structure = 0.5f;
    patch.brightness = 0.5f;
    patch.damping = 0.5f;
    patch.position = 0.3f;

    // §4.6 FaseAcht mod accumulator
    mod_phase = 0.f;

    // ---- CC registrations (8..24) ----
    // CC layout matches synthdef order so idx == ctrl - 8 invariant holds
    // sequentially across all 17 params (clean 4/4/4/4/1 group layout).
    initdata->rack->registerParamAndCC(initdata, "model",   8,  [&](int v){ reson_model = v; });
    initdata->rack->registerParamAndCC(initdata, "freq",    9,  [&](int v){ freq_par = v; });
    initdata->rack->registerParamAndCC(initdata, "struc",   10, [&](int v){ structure = v; });
    initdata->rack->registerParamAndCC(initdata, "pos",     11, [&](int v){ position = v; });
    initdata->rack->registerParamAndCC(initdata, "bright",  12, [&](int v){ brightness = v; });
    initdata->rack->registerParamAndCC(initdata, "damp",    13, [&](int v){ damping = v; });
    initdata->rack->registerParamAndCC(initdata, "chord",   14, [&](int v){ chord_par = v; });
    initdata->rack->registerParamAndCC(initdata, "poly",    15, [&](int v){ poly_par = v; });
    initdata->rack->registerParamAndCC(initdata, "envsh",   16, [&](int v){ env_shape = v; });
    initdata->rack->registerParamAndCC(initdata, "vela",    17, [&](int v){ vel_amount = v; });
    initdata->rack->registerParamAndCC(initdata, "air",     18, [&](int v){ air_blend = v; });
    initdata->rack->registerParamAndCC(initdata, "pluck",   19, [&](int v){ pluck_cc = v; });
    initdata->rack->registerParamAndCC(initdata, "mtype",   20, [&](int v){ mod_type = v; });
    initdata->rack->registerParamAndCC(initdata, "mdpth",   21, [&](int v){ mod_depth = v; });
    initdata->rack->registerParamAndCC(initdata, "mrate",   22, [&](int v){ mod_rate = v; });
    initdata->rack->registerParamAndCC(initdata, "msnap",   23, [&](int v){ mod_snap = v; });
    initdata->rack->registerParamAndCC(initdata, "easter",  24, [&](int v){ easter_par = v; });

    enabled = false;
}

void RackTBDings::noteOn(uint8_t note, uint8_t vel) {
    midi_note = note;
    midi_freq_hz = midiNoteToHz(note);
    pending_strum = true;
    pending_velocity = vel;
    note_held = true;
    // §4.6 AM Pitch Env retrigger — on each note, jump to 2× note freq.
    pitch_env_ratio = 2.f;
    paramAD.Trigger();
}

void RackTBDings::noteOff(uint8_t note, uint8_t vel) {
    (void)note;
    (void)vel;
    note_held = false;
}

void RackTBDings::Process(const PicoSeqRackProcessData &data) {
    (void)data;
    if (!enabled) return;

    // Snapshot atomics once per block — derive everything else locally.
    const int i_model     = reson_model;
    const int i_freq      = freq_par;
    const int i_structure = structure;
    const int i_bright    = brightness;
    const int i_damping   = damping;
    const int i_position  = position;
    const int i_chord     = chord_par;
    const int i_poly      = poly_par;
    const int i_easter    = easter_par;
    const int i_envsh     = env_shape;
    const int i_velamt    = vel_amount;
    const int i_air       = air_blend;
    const int i_mtype     = mod_type;
    const int i_mdpth     = mod_depth;
    const int i_mrate     = mod_rate;
    const int i_msnap     = mod_snap;
    const int i_pluck     = pluck_cc;

    // §5.4 PLUCK — rising-edge trigger only (CCs are levels, not events).
    // TBD-16's CC handler (PicoSeqRack.cpp:845) maps wire 0..127 → 0..4096
    // so the boolean threshold is mid-scale 2047, NOT 64.
    if (i_pluck > 2047 && prev_pluck_cc <= 2047) {
        pending_strum = true;
        // Pluck does NOT retrigger paramAD — FaseAcht §5.4: above-50% envelope
        // sustains the strum, below-50% lets it decay; either way the
        // envelope itself is not re-attacked.
    }
    prev_pluck_cc = i_pluck;

    // ---- SILENCE GATE -----------------------------------------------------
    // The plugin must be silent until the sequencer (or the user) explicitly
    // asks for audio. "Audio activity" is any of: held note, pending strum,
    // running envelope, AIR turned meaningfully up. If none of these, we tick
    // a tail counter; once it exceeds kSilenceTailBlocks we emit zeros and
    // skip Rings entirely. This:
    //   - prevents continuous self-resonance / hum between sequencer steps
    //   - lets a strummed note ring out naturally before muting (the tail
    //     covers the longest user-set Damping decay)
    //   - saves DSP cycles when the track is enabled but momentarily idle
    const bool any_activity = note_held
                           || pending_strum
                           || paramAD.GetIsRunning()
                           || (i_air > kAirActivityThreshold);
    if (any_activity) {
        silence_tail_blocks = 0;
    } else {
        silence_tail_blocks++;
        if (silence_tail_blocks > kSilenceTailBlocks) {
            std::fill_n(tbd_out_stereo, BUF_SZ * 2, 0.f);
            return;
        }
    }
    // ----------------------------------------------------------------------

    // §4.5 Envelope morph — pick two endpoints and interpolate.
    {
        float x = i_envsh / 4095.f * 4.f; // 0..4 → indexes 0..4
        if (x < 0.f) x = 0.f;
        if (x > 4.f) x = 4.f;
        int lo = (int)x;
        int hi = lo < 4 ? lo + 1 : 4;
        float t = x - (float)lo;
        float a = kEnvShapeTable[lo][0] * (1.f - t) + kEnvShapeTable[hi][0] * t;
        float d = kEnvShapeTable[lo][1] * (1.f - t) + kEnvShapeTable[hi][1] * t;
        // Sustain & release encoded but ctagADEnv is AD-only — sustain handled
        // by re-triggering / damping bias inside Rings, release via decay.
        paramAD.SetAttack(a);
        paramAD.SetDecay(d);
    }

    // §4.4 Velocity → wavefolder drive amount. Note-on velocity scales it.
    const float drive = (i_velamt / 4095.f) * 4.f * (pending_velocity / 127.f);

    // §4.6 mod params
    const float depth = i_mdpth / 4095.f;
    const float rate_norm = i_mrate / 4095.f; // 0..1

    // Resonator model — Rings caps to 6 models. Snap once per block.
    // Easter on overrides to SYMPATHETIC_STRING_QUANTIZED (model 4) — cheap
    // one-toggle alternate timbre, the closest analogue to standalone
    // TBDings' string_synth easter path without a second large DSP alloc.
    // Otherwise the user's Model knob selection is honored exactly.
    //
    // Note re Chord knob: Rings only consumes performance_state.chord in
    // SymQ mode (see part.cc:361). In every other model Chord is silent.
    // The fix is documentation — pick Model=SymQ (or turn Easter on) to
    // use Chord. We tried auto-engaging SymQ when Chord crossed a
    // threshold, but that silently overrode the Model knob and broke the
    // user's mental model — worse UX than chord-only-in-SymQ.
    {
        rings::ResonatorModel m;
        if (i_easter > 2047) {
            m = rings::RESONATOR_MODEL_SYMPATHETIC_STRING_QUANTIZED;
        } else {
            m = (rings::ResonatorModel)((i_model * 6) / 4096);
            if (m < rings::RESONATOR_MODEL_MODAL) m = rings::RESONATOR_MODEL_MODAL;
            if (m >= rings::RESONATOR_MODEL_LAST) m = (rings::ResonatorModel)(rings::RESONATOR_MODEL_LAST - 1);
        }
        if (m != last_model) {
            part->set_model(m);
            last_model = m;
        }
    }

    // Internal polyphony 1 / 2 / 4 — slice 0..4095 into 3 even bands.
    {
        int slot = (i_poly * 3) / 4096;     // 0, 1, 2
        if (slot < 0) slot = 0;
        if (slot > 2) slot = 2;
        int p = 1 << slot;                  // 1, 2, 4
        if (p != last_polyphony) {
            part->set_polyphony(p);
            last_polyphony = p;
        }
    }

    // Patch fields. The standalone TBDings exposed mod_brightness/damping/
    // position/structure as user-controlled CC depths. We replaced those with
    // the §4.6 MOD TYPE output stage — but a tiny fixed envelope-on-brightness
    // is still applied here so notes have natural "voice" character (the
    // resonator opens up on attack, closes on decay) without making the patch
    // feel static. Without this the resonator sounded plastic.
    const float fAD = paramAD.Process();
    patch.brightness = i_bright / 4095.f + fAD * 0.15f;
    patch.damping    = i_damping / 4095.f;
    patch.position   = i_position / 4095.f;
    patch.structure  = i_structure / 4095.f;
    CONSTRAIN(patch.brightness, 0.f, 1.f);
    CONSTRAIN(patch.damping,    0.f, 1.f);
    CONSTRAIN(patch.position,   0.f, 1.f);
    CONSTRAIN(patch.structure,  0.f, 1.f);

    // Note pitch — fine-tune detune ±2 semis around the played note. Closer to
    // Rings' canonical ±1 semi attenuverter feel; lets the user comma-tune
    // resonators against each other or chase a microtonal interval without
    // the knob behaving like a coarse pitch fader. Mid-scale (cv ≈ 2048) = 0.
    {
        float bias_semis = (i_freq / 4095.f - 0.5f) * 4.f;
        performance_state.note = (float)midi_note + bias_semis;
        CONSTRAIN(performance_state.note, 0.f, 96.f);
    }
    // Tonic is the chord root for sympathetic-string mode — track the played note
    // so chord intervals voice in-key.
    performance_state.tonic = (float)(midi_note % 12) + 24.f;
    CONSTRAIN(performance_state.tonic, 0.f, 60.f);
    // Small envelope-driven FM on the played note — adds a subtle pitch bloom
    // on each strum (≤±0.6 semis). Restores some of the standalone TBDings
    // mod_frequency life without exposing yet another knob. fAD decays from 1
    // toward 0 over the envelope; multiplying by ±0.6 keeps it musical.
    performance_state.fm = fAD * 0.6f;
    performance_state.internal_exciter = true;
    // internal_strum MUST be false in the rack: with it true, Rings' strummer
    // overwrites our explicit performance_state.strum with note-change
    // detection — so repeated triggers on the same MIDI note from the
    // sequencer never strum. Setting it false lets noteOn() drive
    // performance_state.strum directly via pending_strum.
    performance_state.internal_strum = false;
    performance_state.internal_note = false;

    // Chord index 0..(kNumChords-1) — multiply by kNumChords so the full
    // 0..4095 input range covers all 11 chords (was -1, dropped the last).
    performance_state.chord = (i_chord * rings::kNumChords) / 4096;
    CONSTRAIN(performance_state.chord, 0, rings::kNumChords - 1);

    // Strum signal — Rings only fires on the rising edge.
    performance_state.strum = pending_strum;
    pending_strum = false;

    // Easter mode (string-synth alternative). For v1 we just feed it as a flag —
    // a future iteration could swap to string_synth.Process when easter is on.
    // Currently leave Rings' modal/string synth paths active (set_model handles it).
    (void)i_easter;

    // §4.2 AIR — feed continuous low-level noise into Rings' exciter input.
    // Without this, the resonator only speaks on a strum; with AIR up it hums
    // at its tuned pitch continuously, mimicking FaseAcht's open-pickup
    // behavior where ambient noise leaks through and excites the steel.
    //
    // The same `in[]` buffer is also fed to the strummer for onset detection,
    // so high AIR can interact with strum onsets — which is musically right.
    float in[BUF_SZ];
    float out[BUF_SZ];
    float aux[BUF_SZ];
    {
        const float air = i_air / 4095.f;
        // 0.10 chosen so AIR is a deliberate effect, not always-loud. At full
        // AIR the noise drives Rings' narrow excitation filter band (Q=1.5 in
        // internal_exciter mode) hard enough to clearly hum the resonator at
        // pitch — but a small accidental Air value (e.g. 5%) won't overpower
        // the sequencer's struck notes.
        const float air_gain = air * 0.10f;
        for (int i = 0; i < BUF_SZ; i++) {
            // simple LCG → [-1, +1)
            rng_state = rng_state * 1664525u + 1013904223u;
            float n = (int32_t)rng_state * (1.f / 2147483648.f);
            in[i] = n * air_gain;
        }
    }

    strummer->Process(in, BUF_SZ, &performance_state);
    part->Process(performance_state, patch, in, out, aux, BUF_SZ);

    // Guard: Rings can produce NaN/Inf during initial transients before any
    // strum has primed its filters. Replace with 0 to keep downstream DSP
    // bounded — without this the wavefolder + AM stages can lock the audio
    // task or produce loud DC.
    for (int i = 0; i < BUF_SZ; i++) {
        if (!std::isfinite(out[i])) out[i] = 0.f;
        if (!std::isfinite(aux[i])) aux[i] = 0.f;
    }

    // §4.4 wavefolder applied at output for harmonic grit, scaled by velocity
    // and the §4.4 Vel Amt knob.
    if (drive > 0.01f) {
        for (int i = 0; i < BUF_SZ; i++) {
            out[i] = triangleFold(out[i] * (1.f + drive));
            aux[i] = triangleFold(aux[i] * (1.f + drive));
        }
    }

    // §4.6 MOD TYPE — 0=Tremolo, 1=AM Pitch Env, 2=AM Polyphonic.
    // i_mtype arrives as 0..4095 (TBD-16 CC handler scales wire 0..127 to that
    // range), so we have to slice it to 0..2 — switching on the raw value
    // would always fall through to default.
    if (depth > 0.001f) {
        const float two_pi = 6.2831853f;
        const float sr = 44100.f;
        int mtype = (i_mtype * 3) / 4096;
        if (mtype < 0) mtype = 0;
        if (mtype > 2) mtype = 2;

        float mod_freq_hz = 0.f;
        switch (mtype) {
            case 0: { // Tremolo: 0.1 .. 20 Hz
                mod_freq_hz = 0.1f + rate_norm * 19.9f;
                break;
            }
            case 1: { // AM Pitch Env — frequency dives from 2× note → 1× note
                // on each strum. Rate knob controls how fast it falls
                // (0.05 .. 1.0 semitones-per-block exponential decay).
                const float decay = 0.0005f + rate_norm * 0.02f;
                pitch_env_ratio = 1.f + (pitch_env_ratio - 1.f) * (1.f - decay);
                if (pitch_env_ratio < 1.f) pitch_env_ratio = 1.f;
                mod_freq_hz = midi_freq_hz * pitch_env_ratio;
                break;
            }
            case 2: { // AM Polyphonic: pitch-relative, optional harmonic snap
                if (i_msnap > 2047) {
                    int idx = (int)(rate_norm * 7.999f);
                    if (idx < 0) idx = 0;
                    if (idx > 7) idx = 7;
                    mod_freq_hz = midi_freq_hz * kHarmonicRatios[idx];
                } else {
                    // continuous: 0.5× .. 8× note freq
                    mod_freq_hz = midi_freq_hz * (0.5f + rate_norm * 7.5f);
                }
                break;
            }
        }

        const float phase_inc = two_pi * mod_freq_hz / sr;
        for (int i = 0; i < BUF_SZ; i++) {
            float lfo = sinf(mod_phase);
            mod_phase += phase_inc;
            if (mod_phase >= two_pi) mod_phase -= two_pi;
            const float am = 1.f - depth + depth * lfo;
            out[i] *= am;
            aux[i] *= am;
        }
    }

    // Soft-clip post-AM to bound combined drive + AM levels.
    for (int i = 0; i < BUF_SZ; i++) {
        tbd_out_stereo[i * 2 + 0] = stmlib::SoftClip(out[i]);
        tbd_out_stereo[i * 2 + 1] = stmlib::SoftClip(aux[i]);
    }
}
