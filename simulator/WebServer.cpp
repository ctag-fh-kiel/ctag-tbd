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

#include "WebServer.hpp"
#include "SimSPManager.hpp"
#include <algorithm>
#include <boost/filesystem.hpp>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>


using namespace std;
using namespace CTAG::AUDIO;
using namespace boost::property_tree;

// MIME type helper for static file serving
static string getMimeType(const string& path) {
    static const unordered_map<string, string> mimeTypes = {
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".mjs", "application/javascript"},
        {".json", "application/json"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".ico", "image/x-icon"},
        {".woff", "font/woff"},
        {".woff2", "font/woff2"},
        {".ttf", "font/ttf"},
        {".otf", "font/otf"},
        {".wav", "audio/wav"},
        {".mp3", "audio/mpeg"},
        {".xml", "application/xml"},
        {".txt", "text/plain"},
        {".jsn", "application/json"},
    };
    auto dot = path.rfind('.');
    if (dot != string::npos) {
        string ext = path.substr(dot);
        transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        auto it = mimeTypes.find(ext);
        if (it != mimeTypes.end()) return it->second;
    }
    return "application/octet-stream";
}


void WebServer::Start() {
    // HTTP-server at port 8080 using 1 thread
    // Unless you do more heavy non-threaded processing in the resources,
    // 1 thread is usually faster than several threads
    server.config.port = 8080;

    server.resource["^/api/v1/favorites/getAll"]["POST"] = [&](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        response->write(SimSPManager::GetAllFavorites(), header);
    };

    server.resource["^/api/v1/favorites/store/([0-9])$"]["POST"] = [&](shared_ptr<HttpServer::Response> response,
                                                                       shared_ptr<HttpServer::Request> request) {
        try {
            int fav = std::stoi(request->path_match[1].str());
            string content = request->content.string();
            SimSPManager::StoreFavorite(fav, content);
            response->write(SimpleWeb::StatusCode::success_ok);
        } catch (const exception &e) {
            cerr << "[WebServer] favorites/store error: " << e.what() << endl;
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
                            string("Internal error: ") + e.what());
        }
    };

    server.resource["^/api/v1/favorites/recall/([0-9])$"]["POST"] = [&](shared_ptr<HttpServer::Response> response,
                                                                        shared_ptr<HttpServer::Request> request) {
        try {
            int fav = std::stoi(request->path_match[1].str());
            SimSPManager::ActivateFavorite(fav);
            response->write(SimpleWeb::StatusCode::success_ok);
        } catch (const exception &e) {
            cerr << "[WebServer] favorites/recall error: " << e.what() << endl;
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
                            string("Internal error: ") + e.what());
        }
    };

    server.resource["^/api/v1/setPresetData/(.+)"]["POST"] = [&](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        string id = request->path_match[1].str();
        string content = request->content.string();
        SimSPManager::SetCStrJSONSoundProcessorPreset(id.c_str(), content.c_str());
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/getPresetData/(.+)"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                                shared_ptr<HttpServer::Request> request) {
        string id = request->path_match[1].str();
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        response->write(string(SimSPManager::GetCStrJSONSoundProcessorPresets(id)), header);
    };

    server.resource["^/api/v1/srom/getSize$"]["POST"] = [](shared_ptr<HttpServer::Response> response,
                                                        shared_ptr<HttpServer::Request> request) {
        response->write(to_string(1024*1024*5));
    };

    server.resource["^/api/v1/getPlugins$"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                                        shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        auto content = request->content.string();
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        response->write(SimSPManager::GetCStrJSONSoundProcessors(), header);
    };

    server.resource["^/api/v1/getIOCaps$"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                                        shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        auto content = request->content.string();
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        // Full IOCaps matching firmware IOCapabilities.hpp (platform dada)
        string const s(
            "{\"HWV\":\"simulator\",\"FWV\":\"v0.9.5-sim\",\"p\":\"dada\","
            "\"t\":["
            "\"A_NOTE\",\"A_VELO\",\"A_P_PROG\",\"A_P_AT\","
            "\"B_NOTE\",\"B_VELO\",\"B_P_PROG\",\"B_P_AT\","
            "\"C_NOTE\",\"C_VELO\",\"C_P_PROG\",\"C_P_AT\","
            "\"D_NOTE\",\"D_VELO\",\"D_P_PROG\",\"D_P_AT\","
            "\"A_75_P_C1\",\"A_76_P_C#1\",\"A_77_P_D1\",\"A_78_P_D#1\","
            "\"B_75_P_E1\",\"B_76_P_F1\",\"B_77_P_F#1\",\"B_78_P_G1\","
            "\"C_75_P_G#1\",\"C_76_P_A1\",\"C_77_P_A#1\",\"C_78_P_B1\","
            "\"D_75_P_C2\",\"D_76_P_C#2\",\"D_77_P_D2\",\"D_78_P_D#2\","
            "\"G_AT\",\"G_FX1_12\",\"G_FX2_13\",\"G_SUST_64\",\"G_PORT_65\",\"G_SSTN_66\",\"G_SOFT_67\",\"G_HOLD_69\","
            "\"ET_41\",\"ET_42\",\"ET_43\",\"ET_44\",\"ET_45\",\"ET_46\",\"ET_47\",\"ET_48\",\"ET_49\",\"ET_50\","
            "\"ET_51\",\"ET_52\",\"ET_53\",\"ET_54\",\"ET_55\",\"ET_56\",\"ET_57\",\"ET_58\",\"ET_59\",\"ET_60\""
            "],"
            "\"cv\":["
            "\"A_NOTE\",\"A_VELO\",\"A_P_BANK\",\"A_P_SBNK\",\"A_P_PRG\",\"A_P_PB\",\"A_P_PB_LG\",\"A_P_AT\",\"A_P_MW_1\",\"A_P_BC_2\","
            "\"B_NOTE\",\"B_VELO\",\"B_P_BANK\",\"B_P_SBNK\",\"B_P_PRG\",\"B_P_PB\",\"B_P_PB_LG\",\"B_P_AT\",\"B_P_MW_1\",\"B_P_BC_2\","
            "\"C_NOTE\",\"C_VELO\",\"C_P_BANK\",\"C_P_SBNK\",\"C_P_PRG\",\"C_P_PB\",\"C_P_PB_LG\",\"C_P_AT\",\"C_P_MW_1\",\"C_P_BC_2\","
            "\"D_NOTE\",\"D_VELO\",\"D_P_BANK\",\"D_P_SBNK\",\"D_P_PRG\",\"D_P_PB\",\"D_P_PB_LG\",\"D_P_AT\",\"D_P_MW_1\",\"D_P_BC_2\","
            "\"A_P_RES_71\",\"A_P_REL_72\",\"A_P_ATK_73\",\"A_P_CUT_74\","
            "\"B_P_RES_71\",\"B_P_REL_72\",\"B_P_ATK_73\",\"B_P_CUT_74\","
            "\"C_P_RES_71\",\"C_P_REL_72\",\"C_P_ATK_73\",\"C_P_CUT_74\","
            "\"D_P_RES_71\",\"D_P_REL_72\",\"D_P_ATK_73\",\"D_P_CUT_74\","
            "\"G_PB\",\"G_PB_LG\",\"G_AT\",\"G_MW_1\",\"G_BC_2\",\"G_FOOT_4\",\"G_DAT_6\",\"G_VOL_7\",\"G_BAL_8\",\"G_PAN_10\","
            "\"G_XPR_11\",\"G_FX1_12\",\"G_FX2_13\",\"G_SUST_64\",\"G_PORT_65\",\"G_SOST_66\",\"G_SOFT_67\",\"G_HOLD_69\""
            "]}"
        );
        response->write(s, header);
    };

    // Configuration endpoints
    server.resource["^/api/v1/getConfiguration$"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                                              shared_ptr<HttpServer::Request> request) {
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        auto cfg = SimSPManager::GetCStrJSONConfiguration();
        if (cfg) {
            response->write(string(cfg), header);
        } else {
            // Return empty config if none exists
            response->write(string("{}"), header);
        }
    };

    server.resource["^/api/v1/setConfiguration$"]["POST"] = [](shared_ptr<HttpServer::Response> response,
                                                               shared_ptr<HttpServer::Request> request) {
        string content = request->content.string();
        SimSPManager::SetConfigurationFromJSON(content);
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    // Reboot — no-op in simulator
    server.resource["^/api/v1/reboot$"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                                    shared_ptr<HttpServer::Request> request) {
        cout << "[Simulator] Reboot requested — ignoring (not a real device)" << endl;
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    // Sample Manager stubs — return minimal valid JSON so Sample Manager doesn't crash
    server.resource["^/api/v1/samples"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                                    shared_ptr<HttpServer::Request> request) {
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        // Return minimal but correctly-shaped response matching what the WebUI expects:
        // { files, directories, kits, active_kit_entries, capacity }
        string const s(
            "{\"files\":[],\"directories\":[],"
            "\"kits\":{\"smp_banks\":[],\"smp_bank_names\":[],\"smp_bank_tags\":[],\"smp_bank_meta\":[],\"active_smp_bank\":0},"
            "\"active_kit_entries\":[],"
            "\"capacity\":{\"psram_max_bytes\":29360128,\"active_bank_bytes\":0,\"sd_total_bytes\":32000000000,\"sd_free_bytes\":28500000000}}"
        );
        response->write(s, header);
    };

    server.resource["^/api/v1/samples"]["POST"] = [](shared_ptr<HttpServer::Response> response,
                                                     shared_ptr<HttpServer::Request> request) {
        // All sample POST operations — stub with OK
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/getActivePlugin/([0-1])$"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                                                     shared_ptr<HttpServer::Request> request) {
        try {
            int ch = std::stoi(request->path_match[1].str());
            SimpleWeb::CaseInsensitiveMultimap header;
            header.emplace("Content-Type", "application/json");
            response->write("{\"id\":\"" + CTAG::AUDIO::SimSPManager::GetStringID(ch) + "\"}", header);
        } catch (const exception &e) {
            cerr << "[WebServer] getActivePlugin error: " << e.what() << endl;
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
                            string("Internal error: ") + e.what());
        }
    };

    server.resource["^/api/v1/getPluginParams/([0-1])$"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                                                     shared_ptr<HttpServer::Request> request) {
        try {
            int ch = std::stoi(request->path_match[1].str());
            SimpleWeb::CaseInsensitiveMultimap header;
            header.emplace("Content-Type", "application/json");
            response->write(SimSPManager::GetCStrJSONActivePluginParams(ch), header);
        } catch (const exception &e) {
            cerr << "[WebServer] getPluginParams error: " << e.what() << endl;
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
                            string("Internal error: ") + e.what());
        }
    };

    server.resource["^/api/v1/setActivePlugin/([0-1])$"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                                                     shared_ptr<HttpServer::Request> request) {
        try {
            int ch = std::stoi(request->path_match[1].str());
            auto query_fields = request->parse_query_string();
            for (auto &field: query_fields) {
                if (field.first == "id") {
                    SimSPManager::SetSoundProcessorChannel(ch, field.second);
                }
            }
            response->write(SimpleWeb::StatusCode::success_ok);
        } catch (const exception &e) {
            cerr << "[WebServer] setActivePlugin error: " << e.what() << endl;
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
                            string("Internal error: ") + e.what());
        }
    };

    server.resource["^/api/v1/setPluginParam/([0-1])$"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                                                    shared_ptr<HttpServer::Request> request) {
        try {
            int ch = std::stoi(request->path_match[1].str());
            auto query_fields = request->parse_query_string();
            string id, what;
            int value;
            for (auto &field: query_fields) {
                if (field.first == "id") {
                    id = field.second;
                } else {
                    what = field.first;
                    value = std::stoi(field.second);
                }
            }
            SimSPManager::SetChannelParamValue(ch, id, what, value);
            response->write(SimpleWeb::StatusCode::success_ok);
        } catch (const exception &e) {
            cerr << "[WebServer] setPluginParam error: " << e.what() << endl;
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
                            string("Internal error: ") + e.what());
        }
    };

    server.resource["^/api/v1/setPluginParamCV/([0-1])$"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                                                      shared_ptr<HttpServer::Request> request) {
        try {
            int ch = std::stoi(request->path_match[1].str());
            auto query_fields = request->parse_query_string();
            string id, what;
            int value;
            for (auto &field: query_fields) {
                if (field.first == "id") {
                    id = field.second;
                } else {
                    what = field.first;
                    value = std::stoi(field.second);
                }
            }
            SimSPManager::SetChannelParamValue(ch, id, what, value);
            response->write(SimpleWeb::StatusCode::success_ok);
        } catch (const exception &e) {
            cerr << "[WebServer] setPluginParamCV error: " << e.what() << endl;
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
                            string("Internal error: ") + e.what());
        }
    };

    server.resource["^/api/v1/setPluginParamTRIG/([0-1])$"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                                                        shared_ptr<HttpServer::Request> request) {
        try {
            int ch = std::stoi(request->path_match[1].str());
            auto query_fields = request->parse_query_string();
            string id, what;
            int value;
            for (auto &field: query_fields) {
                if (field.first == "id") {
                    id = field.second;
                } else {
                    what = field.first;
                    value = std::stoi(field.second);
                }
            }
            SimSPManager::SetChannelParamValue(ch, id, what, value);
            response->write(SimpleWeb::StatusCode::success_ok);
        } catch (const exception &e) {
            cerr << "[WebServer] setPluginParamTRIG error: " << e.what() << endl;
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
                            string("Internal error: ") + e.what());
        }
    };

    server.resource["^/api/v1/getPresets/([0-1])$"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                                                shared_ptr<HttpServer::Request> request) {
        try {
            int ch = std::stoi(request->path_match[1].str());
            SimpleWeb::CaseInsensitiveMultimap header;
            header.emplace("Content-Type", "application/json");
            response->write(SimSPManager::GetCStrJSONGetPresets(ch), header);
        } catch (const exception &e) {
            cerr << "[WebServer] getPresets error: " << e.what() << endl;
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
                            string("Internal error: ") + e.what());
        }
    };

    server.resource["^/api/v1/loadPreset/([0-1])$"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                                                shared_ptr<HttpServer::Request> request) {
        try {
            int ch = std::stoi(request->path_match[1].str());
            auto query_fields = request->parse_query_string();
            int number = 0;
            for (auto &field: query_fields) {
                if (field.first == "number") {
                    number = std::stoi(field.second);
                }
            }
            SimSPManager::ChannelLoadPreset(ch, number);
            response->write(SimpleWeb::StatusCode::success_ok);
        } catch (const exception &e) {
            cerr << "[WebServer] loadPreset error: " << e.what() << endl;
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
                            string("Internal error: ") + e.what());
        }
    };

    server.resource["^/api/v1/savePreset/([0-1])$"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                                                shared_ptr<HttpServer::Request> request) {
        try {
            int ch = std::stoi(request->path_match[1].str());
            auto query_fields = request->parse_query_string();
            string name;
            int number;
            for (auto &field: query_fields) {
                if (field.first == "number") {
                    number = std::stoi(field.second);
                } else if (field.first == "name") {
                    name = field.second;
                }
            }
            SimSPManager::ChannelSavePreset(ch, name, number);
            response->write(SimpleWeb::StatusCode::success_ok);
        } catch (const exception &e) {
            cerr << "[WebServer] savePreset error: " << e.what() << endl;
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
                            string("Internal error: ") + e.what());
        }
    };

    server.resource["^/ctrl-set"]["POST"] = [](shared_ptr<HttpServer::Response> response,
                                               shared_ptr<HttpServer::Request> request) {
        SimSPManager::SetProcessParams(request->content.string());
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/ctrl-get"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                              shared_ptr<HttpServer::Request> request) {
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        response->write(SimSPManager::GetProcessParams(), header);
    };


    // Default GET-example. If no other matches, this anonymous function will be called.
    // Will respond with content in the web/-directory, and its subdirectories.
    // Default file: index.html
    // Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server
    // I.E. static web files
    server.resource["^/ctrl"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                          shared_ptr<HttpServer::Request> request) {
        try {
            auto web_root_path = boost::filesystem::canonical("../www");
            auto path = boost::filesystem::canonical(web_root_path / "ui.html");
            // Check if path is within web_root_path
            if (distance(web_root_path.begin(), web_root_path.end()) > distance(path.begin(), path.end()) ||
                !equal(web_root_path.begin(), web_root_path.end(), path.begin()))
                throw invalid_argument("path must be within root path");

            SimpleWeb::CaseInsensitiveMultimap header;
            header.emplace("Content-Type", "text/html");

            // Uncomment the following line to enable Cache-Control
            // header.emplace("Cache-Control", "max-age=86400");


            auto ifs = make_shared<ifstream>();
            ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);

            if (*ifs) {
                auto length = ifs->tellg();
                ifs->seekg(0, ios::beg);

                header.emplace("Content-Length", to_string(length));
                response->write(header);

                // Trick to define a recursive function within this scope (for example purposes)
                class FileServer {
                public:
                    static void
                    read_and_send(const shared_ptr<HttpServer::Response> &response, const shared_ptr<ifstream> &ifs) {
                        // Read and send 128 KB at a time
                        static vector<char> buffer(131072); // Safe when server is running on one thread
                        streamsize read_length;
                        if ((read_length = ifs->read(&buffer[0], static_cast<streamsize>(buffer.size())).gcount()) >
                            0) {
                            response->write(&buffer[0], read_length);
                            if (read_length == static_cast<streamsize>(buffer.size())) {
                                response->send([response, ifs](const SimpleWeb::error_code &ec) {
                                    if (!ec)
                                        read_and_send(response, ifs);
                                    else
                                        cerr << "Connection interrupted" << endl;
                                });
                            }
                        }
                    }
                };
                FileServer::read_and_send(response, ifs);
            } else
                throw invalid_argument("could not read file");
        }
        catch (const exception &e) {
            response->write(SimpleWeb::StatusCode::client_error_bad_request,
                            "Could not open path " + request->path + ": " + e.what());
        }
    };

    // Default GET-example. If no other matches, this anonymous function will be called.
    // Will respond with content in the web/-directory, and its subdirectories.
    // Default file: index.html
    // Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server
    // I.E. static web files
    server.default_resource["GET"] = [](shared_ptr<HttpServer::Response> response,
                                        shared_ptr<HttpServer::Request> request) {
        try {
            auto web_root_path = boost::filesystem::canonical("../../sdcard_image/www");
            auto path = boost::filesystem::canonical(web_root_path / request->path);
            // Check if path is within web_root_path
            if (distance(web_root_path.begin(), web_root_path.end()) > distance(path.begin(), path.end()) ||
                !equal(web_root_path.begin(), web_root_path.end(), path.begin()))
                throw invalid_argument("path must be within root path");
            if (boost::filesystem::is_directory(path))
                path /= "index.html";

            SimpleWeb::CaseInsensitiveMultimap header;
            header.emplace("Content-Type", getMimeType(path.string()));

            // Uncomment the following line to enable Cache-Control
            // header.emplace("Cache-Control", "max-age=86400");


            auto ifs = make_shared<ifstream>();
            ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);

            if (*ifs) {
                auto length = ifs->tellg();
                ifs->seekg(0, ios::beg);

                header.emplace("Content-Length", to_string(length));
                response->write(header);

                // Trick to define a recursive function within this scope (for example purposes)
                class FileServer {
                public:
                    static void
                    read_and_send(const shared_ptr<HttpServer::Response> &response, const shared_ptr<ifstream> &ifs) {
                        // Read and send 128 KB at a time
                        static vector<char> buffer(131072); // Safe when server is running on one thread
                        streamsize read_length;
                        if ((read_length = ifs->read(&buffer[0], static_cast<streamsize>(buffer.size())).gcount()) >
                            0) {
                            response->write(&buffer[0], read_length);
                            if (read_length == static_cast<streamsize>(buffer.size())) {
                                response->send([response, ifs](const SimpleWeb::error_code &ec) {
                                    if (!ec)
                                        read_and_send(response, ifs);
                                    else
                                        cerr << "Connection interrupted" << endl;
                                });
                            }
                        }
                    }
                };
                FileServer::read_and_send(response, ifs);
            } else
                throw invalid_argument("could not read file");
        }
        catch (const exception &e) {
            response->write(SimpleWeb::StatusCode::client_error_bad_request,
                            "Could not open path " + request->path + ": " + e.what());
        }
    };

    server.on_error = [](shared_ptr<HttpServer::Request> /*request*/, const SimpleWeb::error_code & /*ec*/) {
        // Handle errors here
        // Note that connection timeouts will also call this handle with ec set to SimpleWeb::errc::operation_canceled
    };

    // Start server and receive assigned port when server is listening for requests
    promise<unsigned short> server_port;
    thread st([this, &server_port]() {
        // Start server
        server.start([&server_port](unsigned short port) {
            server_port.set_value(port);
        });
    });
    server_thread = std::move(st);
    cout << "Server listening on port " << server_port.get_future().get() << endl << endl;
}

void WebServer::Stop() {
    server.stop();
    server_thread.join();
}
