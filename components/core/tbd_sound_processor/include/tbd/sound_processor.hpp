#pragma once

#define MK_BOOL_PAR(outname, inname) \
    bool outname = inname;\
    if(trig_##inname != -1) outname = data.trig[trig_##inname] == 1 ? false : true;

#define MK_FLT_PAR_ABS(outname, inname, norm, scale) \
    float outname = inname / norm * scale;\
    if(cv_##inname != -1) outname = fabsf(data.cv[cv_##inname]) * scale;

#define MK_FLT_PAR_ABS_ADD(outname, inname, norm, scale) \
    float outname = inname / norm * scale;\
    if(cv_##inname != -1) outname += fabsf(data.cv[cv_##inname]) * scale;    

#define MK_FLT_PAR_ABS_SFT(outname, inname, norm, scale) \
    float outname = inname / norm * scale;\
    if(cv_##inname != -1) outname = (fabsf(data.cv[cv_##inname]) - 0.5f) * 2.f * scale;

#define MK_FLT_PAR(outname, inname, norm, scale) \
    float outname = inname / norm * scale;\
    if(cv_##inname != -1) outname = data.cv[cv_##inname] * scale;

#define MK_INT_PAR_ABS(outname, inname, scale) \
    int outname = inname;\
    if(cv_##inname != -1) outname = static_cast<int>(fabsf(data.cv[cv_##inname]) * scale);

#define MK_INT_PAR(outname, inname, scale) \
    int outname = inname;\
    if(cv_##inname != -1) outname = static_cast<int>(data.cv[cv_##inname] * scale);
    
 #define MK_FLT_PAR_ABS_MIN_MAX(outname, inname, norm, out_min, out_max) \
    float outname = inname/norm * (out_max-out_min)+out_min; \
    if(cv_##inname != -1) outname = fabsf(data.cv[cv_##inname]) * (out_max-out_min)+out_min;   

#define MK_FLT_PAR_ABS_PAN(outname, inname, norm, scale)  \
    float outname = (inname/norm+1.f)/2.f * scale; \
    if(cv_##inname != -1) outname = fabsf(data.cv[cv_##inname]) * scale; 


#include <cinttypes>
#include <tbd/sound_processor/channels.hpp>
#include <tbd/sound_processor/samples.hpp>


namespace tbd::sound_processor {
struct MonoSampleView;

/** @brief input and output data for sound processing
 * 
 *  Processing is invoked for chunks of 32 samples. Each call gets passed the current
 *  binary input values (triggers) and the analogue input values (CVs). 
 * 
 *  - triggers can be used by buttons and other on/off states
 *  - CVs typically represent ranges of input intensities like dials or input values
 *    that relate directly to tone height like analogue control voltages in eurorack 
 *    setups or the complex set of values associated with a MIDI channel.
 * 
 *  @NOTE: Plugins can receive input values either from a dynamic input (trigger or CV)
 *         or plugin properties (typically set manually by users). This mapping is outside
 *         of the plugins control and is done via presets and the TBD UI.
 */
struct ProcessData {
    MonoSampleView left() const { return MonoSampleView(buf + channels::CH_0, TBD_SAMPLES_PER_CHUNK); }
    MonoSampleView right() const { return MonoSampleView(buf + channels::CH_1, TBD_SAMPLES_PER_CHUNK); };
    StereoSampleView both() const { return StereoSampleView(buf, TBD_SAMPLES_PER_CHUNK); };

    /** @brief output data
     * 
     *  The buffer is expected to be populated with the next 32 output sound samples, 
     *  in a 'normalized' [-1, 1] range.
     * 
     *  For mono plugins the output is has a simple time progressive linear layout:
     *  
     *    [t_0, t_1, ..., t_31]
     * 
     *  For stereo plugins the output values are interleaved 
     * 
     *     [l_0, r_0, l_1, r_1, ..., l_31, l_31]
     */
    float *buf;

    /** @brief analogue input values
     *  
     *   The CV inputs have no predetermined order. The plugin defines a set of analogue 
     *   input properties and will receive an up to date mapping on each processing call.
     * 
     *   The order can therefore be arbitrary:
     * 
     *    [prop_312, prop_17, ..., prop_2]
     */
    float *cv;


    uint8_t *trig;
};

struct AnySoundProcessor {
    virtual ~AnySoundProcessor() = default;

    /** plugin initialization
     *
     *  This method hands the plugin a large chunk of preallocated memory that is owned by the plugin for
     *  the entirety of its lifetinme and can be used for any purpose desired. Any other necessary setup
     *  steps can be performed in this method prior to receiving any audio samples.
     */
    virtual void init() = 0;


    virtual bool is_stereo() const = 0;
};

struct SoundProcessor : AnySoundProcessor {
    ~SoundProcessor() override = default;

    /** process an audio sample window
     *  
     *  This is the core functionality of every sound processor and needs to be implemented by plugins.
     *  
     */
    virtual void process(channels::Channels channels, const ProcessData& data) = 0;
};

struct MonoSoundProcessor : AnySoundProcessor {
    ~MonoSoundProcessor() override = default;

    virtual void process_mono(const StereoSampleView& input, const MonoSampleView& output) = 0;
    bool is_stereo() const override { return false; }
};

struct StereoSoundProcessor : AnySoundProcessor {
    ~StereoSoundProcessor() override = default;

    virtual void process_stereo(const StereoSampleView& input, const StereoSampleView& output) = 0;
    bool is_stereo() const override { return true; }
};

}
