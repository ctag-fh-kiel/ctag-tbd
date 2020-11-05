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

#include <cstdint>

namespace CTAG {
    namespace SP {
        namespace HELPERS {
            class ctagADEnv {
            public:
                float Process();

                void SetSampleRate(float fs_hz);

                void SetAttack(float a_s);

                void SetDecay(float d_s);

                void Trigger();

                void SetModeLin();

                void SetModeExp();

                void SetLoop(bool l);

                bool GetLoop();

                bool GetIsRunning();

            private:
                enum class EnvStateType : uint32_t {
                    STATE_IDLE,
                    STATE_ATTACK,
                    STATE_DECAY,
                };
                enum class EnvModeType : uint32_t {
                    MODE_LIN,
                    MODE_LOG
                };
                EnvStateType envState = EnvStateType::STATE_IDLE;
                EnvModeType envMode = EnvModeType::MODE_LOG;
                float envAccum = 0.f;
                float envMaxLength = 44100.0f / 32.f / 10.f; // 10s at 44100Hz fs and 32 buf size
                float attack = 0.5f;
                float decay = 0.5f;
                float fSample = 0.f, tSample = 0.f;
                bool loop = false;
            };
        }
    }
}

