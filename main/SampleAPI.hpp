/***************
dadamachines TBD-16 — Sample Manager REST API

(c) 2014-2026 Johannes Elias Lohbihler for dadamachines.

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

Part of the dadamachines additions to the CTAG TBD platform.
See LICENSE in the repository root for full terms.

Provided "as is" without any express or implied warranties.
***************/

#pragma once

#include "esp_http_server.h"
#include "esp_err.h"

namespace CTAG {
    namespace REST {
        /**
         * Sample Manager REST API
         *
         * Two URI handlers, consolidated per lead-dev guidance to stay within
         * max_uri_handlers = 20.  Actions are dispatched via query strings:
         *
         *   GET  /api/v2/samples*           — list files / kits / capacity
         *        ?preview=path/name         — stream a WAV for audio preview
         *        ?kit=N                     — switch active kit before listing
         *
         *   POST /api/v2/samples*
         *        ?action=upload&path=X&filename=Y  — binary WAV upload
         *        ?action=manage                    — JSON body: rename/delete/saveKit/createKit/createFolder
         *        ?action=reload                    — trigger PSRAM reload
         */
        class SampleAPI final {
        public:
            SampleAPI() = delete;

            static esp_err_t samples_get_handler(httpd_req_t *req);
            static esp_err_t samples_post_handler(httpd_req_t *req);
        };
    }
}
