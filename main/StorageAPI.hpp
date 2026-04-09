/***************
dadamachines TBD-16 — Storage & Sample Manager REST API

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
         * Storage & Sample Manager REST API
         *
         * === Legacy Sample Manager (preserved) ===
         *
         *   GET  /api/v2/samples*           — list files / kits / capacity
         *        ?preview=path/name         — stream a WAV for audio preview
         *        ?kit=N                     — switch active kit before listing
         *
         *   POST /api/v2/samples*
         *        ?action=upload&path=X&filename=Y  — binary WAV upload
         *        ?action=manage                    — JSON body: rename/delete/saveKit/createKit/createFolder
         *        ?action=reload                    — trigger PSRAM reload
         *
         * === Generic Storage API (Phase 6) ===
         *
         *   GET  /api/v2/storage*
         *        ?action=info                      — SD card total/free/per-zone stats
         *        ?action=list&path=X               — recursive directory listing with sizes
         *        ?action=file&path=X               — raw file download
         *
         *   POST /api/v2/storage*
         *        ?action=upload&path=X             — raw file upload (body = file content)
         *        ?action=mkdir&path=X              — create directory
         *        ?action=delete&path=X             — delete file or directory (recursive)
         *        ?action=copy&from=X&to=Y          — server-side file copy
         *        ?action=reload                    — flush caches, re-scan overlay
         *
         * Security: /factory/ read-only, /system/ write-protected, path traversal rejected.
         */
        class StorageAPI final {
        public:
            StorageAPI() = delete;

            // Legacy Sample Manager endpoints
            static esp_err_t samples_get_handler(httpd_req_t *req);
            static esp_err_t samples_post_handler(httpd_req_t *req);

            // Generic Storage API endpoints (Phase 6)
            static esp_err_t storage_get_handler(httpd_req_t *req);
            static esp_err_t storage_post_handler(httpd_req_t *req);
        };
    }
}
