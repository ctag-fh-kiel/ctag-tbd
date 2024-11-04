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
#include "ctagSoundProcessor.hpp"
#include "ctagSPAllocator.hpp"
{% for header in headers %}
#include <tbd/sounds/{{ header }}>
{%- endfor %}


namespace CTAG {
    namespace SP {
        class ctagSoundProcessorFactory {
        public:
            static ctagSoundProcessor* Create(const std::string& type, ctagSPAllocator::AllocationType const& aType) {
            ctagSoundProcessor* processor {nullptr};
            int ch = 0;
            if(aType == ctagSPAllocator::AllocationType::CH1) ch = 1;
            ctagSPAllocator::PrepareAllocation(aType);

                {% for cls in sound_processors %}
                if(type.compare("{{ cls.name }}") == 0) processor = new {{ cls.cls_name }}();
                {%- endfor %}
                
                if(nullptr != processor) {
                    processor->Init(ctagSPAllocator::GetRemainingBufferSize(), ctagSPAllocator::GetRemainingBuffer());
                    processor->SetProcessChannel(ch);
                }
                return processor;
            }
        };
    }
}
