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

/* OTA call sequence:
 * first call InitiateOTA (stops audio thread etc.)
 * then call PostHandlerSPIFFS (reads SPIFFS image into SPIRAM)
 * then call PostHandlerApp (reads new app onto OTA partition and swaps for next boot)
 * then call PostHandlerFlashCommit (writes SPIFFS image to flash and finishes full OTA update)
 * */


#pragma once

#include <cstdint>
#include "esp_http_server.h"
#include "esp_err.h"

namespace CTAG {
    namespace OTA {
        class OTAManager final {
        public:
            OTAManager() = delete;
            static esp_err_t InitiateOTA(httpd_req_t *req);

            static esp_err_t PostHandlerSPIFFS(httpd_req_t *req);

            static esp_err_t PostHandlerApp(httpd_req_t *req);

            static esp_err_t PostHandlerFlashCommit(httpd_req_t *req);

        private:
            static void cleanup();

            static esp_err_t flashSPIFFS();

            static char *largeBuf;
            static char *smallBuf;
            static bool hasMemSPIFFS;
            static bool hasFlashAppImage;
            static uint32_t spiffsImageSize;
        };
    }
}




