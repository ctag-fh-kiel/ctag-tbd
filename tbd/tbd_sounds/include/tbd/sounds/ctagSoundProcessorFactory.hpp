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


#pragma once

#include <memory>
#include <iostream>
#include <string>
#include <tbd/sound_processor.hpp>
#include "ctagSPAllocator.hpp"

#include <tbd/sounds/ctagSoundProcessorTDelay.hpp>
#include <tbd/sounds/ctagSoundProcessorVoid.hpp>
#include <tbd/sounds/ctagSoundProcessorDrumRack.hpp>
#include <tbd/sounds/ctagSoundProcessorMacOscDuo.hpp>
#include <tbd/sounds/ctagSoundProcessorTalkbox.hpp>
#include <tbd/sounds/ctagSoundProcessorHihat1.hpp>
#include <tbd/sounds/ctagSoundProcessorCStripM.hpp>
#include <tbd/sounds/ctagSoundProcessorEChorus.hpp>
#include <tbd/sounds/ctagSoundProcessorSubSynth.hpp>
#include <tbd/sounds/ctagSoundProcessorMIVerb.hpp>
#include <tbd/sounds/ctagSoundProcessorSimpleVCA.hpp>
#include <tbd/sounds/ctagSoundProcessorBBeats.hpp>
#include <tbd/sounds/ctagSoundProcessorFVerb.hpp>
#include <tbd/sounds/ctagSoundProcessorTBD03.hpp>
#include <tbd/sounds/ctagSoundProcessorSubbotnik.hpp>
#include <tbd/sounds/ctagSoundProcessorBjorklund.hpp>
#include <tbd/sounds/ctagSoundProcessorMacOsc.hpp>
#include <tbd/sounds/ctagSoundProcessorCDelay.hpp>
#include <tbd/sounds/ctagSoundProcessorRompler.hpp>
#include <tbd/sounds/ctagSoundProcessorDust.hpp>
#include <tbd/sounds/ctagSoundProcessorGVerb.hpp>
#include <tbd/sounds/ctagSoundProcessorClaude.hpp>
#include <tbd/sounds/ctagSoundProcessorPolyPad.hpp>
#include <tbd/sounds/ctagSoundProcessorAntique.hpp>
#include <tbd/sounds/ctagSoundProcessorEveryTrim.hpp>
#include <tbd/sounds/ctagSoundProcessorAPCpp.hpp>
#include <tbd/sounds/ctagSoundProcessorMIPShft.hpp>
#include <tbd/sounds/ctagSoundProcessorMIDifu.hpp>
#include <tbd/sounds/ctagSoundProcessorTBDeep.hpp>
#include <tbd/sounds/ctagSoundProcessorWTOscDuo.hpp>
#include <tbd/sounds/ctagSoundProcessorMIEnsemble.hpp>
#include <tbd/sounds/ctagSoundProcessorMoogFilt.hpp>
#include <tbd/sounds/ctagSoundProcessorRetroactor.hpp>
#include <tbd/sounds/ctagSoundProcessorFormantor.hpp>
#include <tbd/sounds/ctagSoundProcessorVctrSnt.hpp>
#include <tbd/sounds/ctagSoundProcessorTBDings.hpp>
#include <tbd/sounds/ctagSoundProcessorDLoop.hpp>
#include <tbd/sounds/ctagSoundProcessorSineSrc.hpp>
#include <tbd/sounds/ctagSoundProcessorCStrip.hpp>
#include <tbd/sounds/ctagSoundProcessorRecNPlay.hpp>
#include <tbd/sounds/ctagSoundProcessorWTOsc.hpp>
#include <tbd/sounds/ctagSoundProcessorTBDaits.hpp>
#include <tbd/sounds/ctagSoundProcessorFBDlyLine.hpp>
#include <tbd/sounds/ctagSoundProcessorStrampDly.hpp>
#include <tbd/sounds/ctagSoundProcessorGDVerb2.hpp>
#include <tbd/sounds/ctagSoundProcessorMSxxNoise.hpp>
#include <tbd/sounds/ctagSoundProcessorKarpuskl.hpp>
#include <tbd/sounds/ctagSoundProcessorFreakwaves.hpp>
#include <tbd/sounds/ctagSoundProcessorBCSR.hpp>
#include <tbd/sounds/ctagSoundProcessorMIChorus.hpp>
#include <tbd/sounds/ctagSoundProcessorMIVerb2.hpp>
#include <tbd/sounds/ctagSoundProcessorMISVF.hpp>
#include <tbd/sounds/ctagSoundProcessorSpaceFX.hpp>
#include <tbd/sounds/ctagSoundProcessorPNoise.hpp>
#include <tbd/sounds/ctagSoundProcessorGDVerb.hpp>


namespace CTAG {
    namespace SP {
        class ctagSoundProcessorFactory {
        public:
            static ctagSoundProcessor* Create(const std::string& type, ctagSPAllocator::AllocationType const& aType) {
            ctagSoundProcessor* processor {nullptr};
            int ch = 0;
            if(aType == ctagSPAllocator::AllocationType::CH1) ch = 1;
            ctagSPAllocator::PrepareAllocation(aType);

                
                if(type.compare("EChorus") == 0) processor = new ctagSoundProcessorEChorus();
                if(type.compare("WTOsc") == 0) processor = new ctagSoundProcessorWTOsc();
                if(type.compare("Bjorklund") == 0) processor = new ctagSoundProcessorBjorklund();
                if(type.compare("BBeats") == 0) processor = new ctagSoundProcessorBBeats();
                if(type.compare("MIVerb") == 0) processor = new ctagSoundProcessorMIVerb();
                if(type.compare("Karpuskl") == 0) processor = new ctagSoundProcessorKarpuskl();
                if(type.compare("MIEnsemble") == 0) processor = new ctagSoundProcessorMIEnsemble();
                if(type.compare("StrampDly") == 0) processor = new ctagSoundProcessorStrampDly();
                if(type.compare("RecNPlay") == 0) processor = new ctagSoundProcessorRecNPlay();
                if(type.compare("MIDifu") == 0) processor = new ctagSoundProcessorMIDifu();
                if(type.compare("TBDaits") == 0) processor = new ctagSoundProcessorTBDaits();
                if(type.compare("MacOsc") == 0) processor = new ctagSoundProcessorMacOsc();
                if(type.compare("FBDlyLine") == 0) processor = new ctagSoundProcessorFBDlyLine();
                if(type.compare("MISVF") == 0) processor = new ctagSoundProcessorMISVF();
                if(type.compare("MSxxNoise") == 0) processor = new ctagSoundProcessorMSxxNoise();
                if(type.compare("GVerb") == 0) processor = new ctagSoundProcessorGVerb();
                if(type.compare("WTOscDuo") == 0) processor = new ctagSoundProcessorWTOscDuo();
                if(type.compare("Rompler") == 0) processor = new ctagSoundProcessorRompler();
                if(type.compare("Formantor") == 0) processor = new ctagSoundProcessorFormantor();
                if(type.compare("VctrSnt") == 0) processor = new ctagSoundProcessorVctrSnt();
                if(type.compare("BCSR") == 0) processor = new ctagSoundProcessorBCSR();
                if(type.compare("TBDings") == 0) processor = new ctagSoundProcessorTBDings();
                if(type.compare("FVerb") == 0) processor = new ctagSoundProcessorFVerb();
                if(type.compare("Dust") == 0) processor = new ctagSoundProcessorDust();
                if(type.compare("TBDeep") == 0) processor = new ctagSoundProcessorTBDeep();
                if(type.compare("SubSynth") == 0) processor = new ctagSoundProcessorSubSynth();
                if(type.compare("DrumRack") == 0) processor = new ctagSoundProcessorDrumRack();
                if(type.compare("SineSrc") == 0) processor = new ctagSoundProcessorSineSrc();
                if(type.compare("MIChorus") == 0) processor = new ctagSoundProcessorMIChorus();
                if(type.compare("Void") == 0) processor = new ctagSoundProcessorVoid();
                if(type.compare("GDVerb2") == 0) processor = new ctagSoundProcessorGDVerb2();
                if(type.compare("Hihat1") == 0) processor = new ctagSoundProcessorHihat1();
                if(type.compare("GDVerb") == 0) processor = new ctagSoundProcessorGDVerb();
                if(type.compare("TDelay") == 0) processor = new ctagSoundProcessorTDelay();
                if(type.compare("CDelay") == 0) processor = new ctagSoundProcessorCDelay();
                if(type.compare("Claude") == 0) processor = new ctagSoundProcessorClaude();
                if(type.compare("TBD03") == 0) processor = new ctagSoundProcessorTBD03();
                if(type.compare("PolyPad") == 0) processor = new ctagSoundProcessorPolyPad();
                if(type.compare("Freakwaves") == 0) processor = new ctagSoundProcessorFreakwaves();
                if(type.compare("Talkbox") == 0) processor = new ctagSoundProcessorTalkbox();
                if(type.compare("PNoise") == 0) processor = new ctagSoundProcessorPNoise();
                if(type.compare("CStripM") == 0) processor = new ctagSoundProcessorCStripM();
                if(type.compare("MIPShft") == 0) processor = new ctagSoundProcessorMIPShft();
                if(type.compare("DLoop") == 0) processor = new ctagSoundProcessorDLoop();
                if(type.compare("MacOscDuo") == 0) processor = new ctagSoundProcessorMacOscDuo();
                if(type.compare("Antique") == 0) processor = new ctagSoundProcessorAntique();
                if(type.compare("EveryTrim") == 0) processor = new ctagSoundProcessorEveryTrim();
                if(type.compare("Subbotnik") == 0) processor = new ctagSoundProcessorSubbotnik();
                if(type.compare("SimpleVCA") == 0) processor = new ctagSoundProcessorSimpleVCA();
                if(type.compare("MoogFilt") == 0) processor = new ctagSoundProcessorMoogFilt();
                if(type.compare("SpaceFX") == 0) processor = new ctagSoundProcessorSpaceFX();
                if(type.compare("Retroactor") == 0) processor = new ctagSoundProcessorRetroactor();
                if(type.compare("APCpp") == 0) processor = new ctagSoundProcessorAPCpp();
                if(type.compare("CStrip") == 0) processor = new ctagSoundProcessorCStrip();
                if(type.compare("MIVerb2") == 0) processor = new ctagSoundProcessorMIVerb2();
                
                if(nullptr != processor) {
                    processor->Init(ctagSPAllocator::GetRemainingBufferSize(), ctagSPAllocator::GetRemainingBuffer());
                    processor->SetProcessChannel(ch);
                }
                return processor;
            }
        };
    }
}