/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2024 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "helpers/ctagTimer.hpp"

namespace CTAG {
    namespace SP {
        namespace HELPERS {
            void ctagTimer::SetSampleRate(float fs) {
                delta_time = 1.f / fs;
            }
            void ctagTimer::SetTimeout(float t_ms) {
                current_time = 0.f;
                timeout = t_ms / 1000.f;
                expired = false;
            }
            void ctagTimer::SetRepeat(bool rpt = true) {
                repeat = rpt;
            }
            bool ctagTimer::isRepeat() {
                return repeat;
            }
            void ctagTimer::Tick() {
                if(expired) return;
                current_time += delta_time;
                // call callback if time is up
                if (current_time >= timeout) {
                    if(repeat) current_time = 0.f;
                    else expired = true;
                    if (timeout_cb != nullptr) {
                        timeout_cb();
                    }
                }
            }
            void ctagTimer::SetTimeoutCallback(std::function<void()> cb) {
                timeout_cb = std::move(cb);
            }
            bool ctagTimer::isExpired() {
                return expired;
            }
        } // HELPERS
    } // SP
} // CTAG