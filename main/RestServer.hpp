/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020-2026 by Robert Manzke. All rights reserved.

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

#include "esp_http_server.h"
#include "esp_err.h"

namespace CTAG {
    namespace REST {
        /**
         * HTTP REST Server — v2 API
         *
         * All domain logic lives in dedicated modules:
         *   PluginAPI  (GET/POST /api/v2/plugins)
         *   DeviceAPI  (GET/POST /api/v2/device)
         *   StorageAPI (GET/POST /api/v2/samples)
         *   MacroAPI   (GET/POST /api/v2/macros)
         *
         * RestServer only handles static file serving and
         * registers the 9 URI handlers (4 domains × GET/POST + static).
         */
        class RestServer final {
        public:
            RestServer() = delete;
            static esp_err_t StartRestServer();
        };
    }
}