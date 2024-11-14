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

#include "esp_http_server.h"
#include "esp_err.h"

namespace CTAG {
namespace REST {

class RestServer final {
public:
    RestServer() = delete;
    static esp_err_t StartRestServer();

private:
    static esp_err_t get_plugins_get_handler(httpd_req_t *req);

    static esp_err_t get_active_plugin_get_handler(httpd_req_t *req);

    static esp_err_t get_params_plugin_get_handler(httpd_req_t *req);

    static esp_err_t set_active_plugin_get_handler(httpd_req_t *req);

    static esp_err_t set_plugin_param_get_handler(httpd_req_t *req);

    static esp_err_t get_presets_get_handler(httpd_req_t *req);

    static esp_err_t save_preset_get_handler(httpd_req_t *req);

    static esp_err_t load_preset_get_handler(httpd_req_t *req);

    static esp_err_t set_configuration_post_handler(httpd_req_t *req);

    static esp_err_t get_configuration_get_handler(httpd_req_t *req);

    static esp_err_t get_calibration_get_handler(httpd_req_t *req);

    static esp_err_t set_calibration_post_handler(httpd_req_t *req);

    static esp_err_t get_preset_json_handler(httpd_req_t *req);

    static esp_err_t favorite_post_handler(httpd_req_t *req);

    static esp_err_t set_preset_json_handler(httpd_req_t *req);

    static esp_err_t reboot_handler(httpd_req_t *req);

    static esp_err_t ota_handler(httpd_req_t *req);

    static esp_err_t srom_handler(httpd_req_t *req);

    static esp_err_t get_iocaps_handler(httpd_req_t *req);
};

}
}