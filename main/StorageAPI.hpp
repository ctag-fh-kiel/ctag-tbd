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
         * Unified Storage & Sample Manager REST API
         *
         * Single endpoint: /api/v2/storage*
         *
         * GET  /api/v2/storage*
         *      (no query)                         — bulk sample list + dirs + configs + kits
         *      ?preview=path/name                 — stream a WAV for audio preview
         *      ?getconfig=filename                — overlay-resolved config file
         *      ?kit=N                             — switch active kit before listing
         *      ?action=info                       — SD card total/free/per-zone stats
         *      ?action=list&path=X                — recursive directory listing with sizes
         *      ?action=file&path=X                — raw file download
         *
         * POST /api/v2/storage*
         *      ?action=upload&path=X              — raw file upload (body = file content)
         *      ?action=uploadconfig&path=X        — config file upload (user overlay)
         *      ?action=uploadwww&path=X           — www file upload
         *      ?action=uploadsystem&path=X        — system file upload
         *      ?action=manage                     — JSON body: rename/delete/saveKit/createKit/etc
         *      ?action=mkdir&path=X               — create directory
         *      ?action=delete&path=X              — delete file or directory (recursive)
         *      ?action=copy&from=X&to=Y           — server-side file copy
         *      ?action=reload                     — reload PSRAM from SD card
         *
         * Security: /factory/ read-only, path traversal rejected.
         */
        class StorageAPI final {
        public:
            StorageAPI() = delete;

            static esp_err_t storage_get_handler(httpd_req_t *req);
            static esp_err_t storage_post_handler(httpd_req_t *req);
        };
    }
}
