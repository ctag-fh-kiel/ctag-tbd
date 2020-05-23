

#pragma once

#include "esp_http_server.h"
#include "esp_err.h"

namespace CTAG{
    namespace REST{
        class RestServer{
            public:
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
                static esp_err_t set_preset_json_handler(httpd_req_t *req);
                static esp_err_t reboot_handler(httpd_req_t *req);
                static esp_err_t ota_handler(httpd_req_t *req);
        };
    }
}