/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include <tbd/sounds/SoundProcessorClaude.hpp>
#include <algorithm>
#include <tbd/logging.hpp>
#include <tbd/heaps.hpp>


namespace heaps = tbd::heaps;

using namespace tbd::sounds;

void SoundProcessorClaude::Process(const sound_processor::ProcessData&data) {

    // setup processor
    clouds::PlaybackMode playbackMode = static_cast<clouds::PlaybackMode>(mode.load());
    processor.set_playback_mode(playbackMode);
    processor.set_quality(quality);
    bool bReverse = reverse;
    if(trig_reverse != -1){
        bReverse = data.trig[trig_reverse] == 1 ? false : true;
    }
    processor.set_reverse(bReverse);
    processor.Prepare();

    // Trigger evaluation
    bool bTrigger = trigger;
    if(trig_trigger != -1){
        bTrigger = data.trig[trig_trigger] == 1 ? false : true;
    }
    if(bTrigger && !preTrigger){
        preTrigger = bTrigger;
        isTriggered = true;
    }else if(!bTrigger && preTrigger){
        preTrigger = bTrigger;
    }else if(bTrigger && preTrigger){
        isTriggered = false;
    }

    clouds::Parameters* p = processor.mutable_parameters();
    p->trigger = isTriggered;
    p->gate = isTriggered;
    bool bFreeze = freeze;
    if(trig_freeze != -1){
        bFreeze = data.trig[trig_freeze] == 1 ? 0 : 1;
    }
    p->freeze = bFreeze;
    float fPosition = position / 4095.f;
    if(cv_position != -1){
        fPosition = fabsf(data.cv[cv_position]);
    }
    p->position = fPosition;
    float fSize = size / 4095.f;
    if(cv_size != -1){
        fSize = fabsf(data.cv[cv_size]);
    }
    if(playbackMode==clouds::PLAYBACK_MODE_STRETCH) fSize *= 0.5f;
    p->size = fSize;
    float fPitch = pitch;
    if(cv_pitch != -1){
        fPitch += 12.f * data.cv[cv_pitch] * 5.f;
    }
    CONSTRAIN(fPitch, -48.f, 48.f)
    p->pitch = fPitch;
    float fDensity = density / 4095.f;
    if(cv_density != -1){
        fDensity = fabsf(data.cv[cv_density]);
    }
    p->density = fDensity;
    float fTexture = texture / 4095.f;
    if(cv_texture != -1){
        fTexture = fabsf(data.cv[cv_texture]);
    }
    p->texture = fTexture;
    float fDryWet = drywet / 4095.f;
    if(cv_drywet != -1){
        fDryWet = fabsf(data.cv[cv_drywet]);
    }
    p->dry_wet = fDryWet;
    float fWidth = width / 4095.f;
    if(cv_width != -1){
        fWidth = fabsf(data.cv[cv_width]);
    }
    p->stereo_spread = fWidth;
    float fFeedback = feedback / 4095.f;
    if(cv_feedback != -1){
        fFeedback = fabsf(data.cv[cv_feedback]);
    }
    p->feedback = fFeedback;
    float fReverb = reverb / 4095.f;
    if(cv_reverb != -1){
        fReverb = fabsf(data.cv[cv_reverb]);
    }
    p->reverb = fReverb;

    processor.Process(data.buf, 32);

}

void SoundProcessorClaude::Init(std::size_t blockSize, void *blockPtr) {
    // memallocs
    block_mem = (uint8_t *) heaps::malloc(memLen, TBD_HEAPS_SPIRAM);
    if(block_mem == NULL){
        TBD_LOGE("Claude", "Cannot alloc ram!");
    }
    memset(block_mem, 0, memLen);

    assert(blockSize >= ccmLen);
    block_ccm = (uint8_t*) blockPtr;
    memset(block_ccm, 0, ccmLen);

    processor.Init(block_mem, memLen, block_ccm, ccmLen);
}

SoundProcessorClaude::~SoundProcessorClaude() {
    heaps::free(block_mem);
}
