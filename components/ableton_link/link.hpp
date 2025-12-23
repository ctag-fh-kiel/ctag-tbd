/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2025 by Robert Manzke. All rights reserved.

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

namespace CTAG{
    namespace LINK{
        struct link_session_data_t{
            // --- Link Active ---
            bool linkActive;
            uint32_t numPeers;

            // --- Tempo / structure ---
            float tempo;     // BPM
            float quantum;   // beats per bar, set to four

            // --- Musical position ---
            double beat;      // absolute beat position
            double phase;     // beat phase [0..1)
        };
        class link{
        public:
            // real time link session data
            link() = delete;
            static void Init();
            static void DeInit();
            // non blocking, NOT thread safe
            static void GetLinkRtSessionData(link_session_data_t *data);
            // possibly blocking, thread safe
            static void GetLinkSessionData(link_session_data_t *data);
            // possibly blocking, thread safe
            static void SetLinkTempo(float bpm);
        };
    } // LINK
} // CTAG
