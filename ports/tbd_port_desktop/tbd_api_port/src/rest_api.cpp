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

#include <tbd/api/port/rest_api.hpp>
#include <algorithm>
#include <tbd/storage/resources.hpp>
#include <fstream>
#include <vector>
#include <server_http.hpp>

#include <tbd/favorites.hpp>
#include <tbd/sound_manager.hpp>

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
using SoundProcessorManager = tbd::audio::SoundProcessorManager;
using Favorites = tbd::Favorites;
namespace fs = tbd::storage::filesystem;

namespace {

constexpr char* tag = "rest_api";

HttpServer server;
std::thread server_thread;

}

namespace tbd::api {

void RestApi::begin(const RestApiParams& params) {
    // HTTP-server at port 8080 using 1 thread
    // Unless you do more heavy non-threaded processing in the resources,
    // 1 thread is usually faster than several threads
    server.config.port = params.port;

    server.resource["^/api/v1/favorites/getAll"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        // Retrieve std::string:
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        response->write(Favorites::GetAllFavorites(), header);
    };

    server.resource["^/api/v1/favorites/store/([0-9])$"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response,
                                                                       std::shared_ptr<HttpServer::Request> request) {
        // Retrieve std::string:
        int fav = std::stoi(request->path_match[1].str());
        std::string content = request->content.string();
        Favorites::StoreFavorite(fav, content);
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/favorites/recall/([0-9])$"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response,
                                                                        std::shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int fav = std::stoi(request->path_match[1].str());
        Favorites::ActivateFavorite(fav);
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/setPresetData/(.+)"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        std::string id = request->path_match[1].str();
        std::string content = request->content.string();
        SoundProcessorManager::SetCStrJSONSoundProcessorPreset(id.c_str(), content.c_str());
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/getPresetData/(.+)"]["GET"] = [&](std::shared_ptr<HttpServer::Response> response,
                                                                std::shared_ptr<HttpServer::Request> request) {
        std::string id = request->path_match[1].str();
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        response->write(string(SoundProcessorManager::GetCStrJSONSoundProcessorPresets(id)), header);
    };

    server.resource["^/api/v1/srom/getSize$"]["POST"] = [](std::shared_ptr<HttpServer::Response> response,
                                                        std::shared_ptr<HttpServer::Request> request) {
        response->write(to_string(1024*1024*5));
    };

    server.resource["^/api/v1/getPlugins$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                                        std::shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        auto content = request->content.string();
        response->write(SoundProcessorManager::GetCStrJSONSoundProcessors());
    };

    server.resource["^/api/v1/getIOCaps$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                                        std::shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        auto content = request->content.string();
        std::string const s("{\"t\":[\"TRIG0\", \"TRIG1\"], \"cv\":[\"CV0\",\"CV1\",\"POT0\",\"POT1\"]}");
        response->write(s);
    };

    server.resource["^/api/v1/getActivePlugin/([0-1])$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                                                     std::shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        response->write("{\"id\":\"" + SoundProcessorManager::GetStringID(ch) + "\"}");
    };

    server.resource["^/api/v1/getPluginParams/([0-1])$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                                                     std::shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        response->write(SoundProcessorManager::GetCStrJSONActivePluginParams(ch));
    };

    server.resource["^/api/v1/setActivePlugin/([0-1])$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                                                     std::shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        auto query_fields = request->parse_query_string();
        for (auto &field: query_fields) {
            if (field.first == "id") {
                SoundProcessorManager::SetSoundProcessorChannel(ch, field.second);
            }
        }
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/setPluginParam/([0-1])$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                                                    std::shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        auto query_fields = request->parse_query_string();
        std::string id, what;
        int value;
        for (auto &field: query_fields) {
            if (field.first == "id") {
                id = field.second;
            } else {
                what = field.first;
                value = std::stoi(field.second);
            }
        }
        SoundProcessorManager::SetChannelParamValue(ch, id, what, value);
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/setPluginParamCV/([0-1])$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                                                      std::shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        auto query_fields = request->parse_query_string();
        std::string id, what;
        int value;
        for (auto &field: query_fields) {
            if (field.first == "id") {
                id = field.second;
            } else {
                what = field.first;
                value = std::stoi(field.second);
            }
        }
        SoundProcessorManager::SetChannelParamValue(ch, id, what, value);
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/setPluginParamTRIG/([0-1])$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                                                        std::shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        auto query_fields = request->parse_query_string();
        std::string id, what;
        int value;
        for (auto &field: query_fields) {
            if (field.first == "id") {
                id = field.second;
            } else {
                what = field.first;
                value = std::stoi(field.second);
            }
        }
        SoundProcessorManager::SetChannelParamValue(ch, id, what, value);
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/getPresets/([0-1])$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                                                std::shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        response->write(SoundProcessorManager::GetCStrJSONGetPresets(ch), header);
    };

    server.resource["^/api/v1/loadPreset/([0-1])$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                                                std::shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        auto query_fields = request->parse_query_string();
        int number = 0;
        for (auto &field: query_fields) {
            if (field.first == "number") {
                number = std::stoi(field.second);
            }
        }
        SoundProcessorManager::ChannelLoadPreset(ch, number);
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/savePreset/([0-1])$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                                                std::shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        auto query_fields = request->parse_query_string();
        std::string name;
        int number;
        for (auto &field: query_fields) {
            if (field.first == "number") {
                number = std::stoi(field.second);
            } else if (field.first == "name") {
                name = field.second;
            }
        }
        SoundProcessorManager::ChannelSavePreset(ch, name, number);
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    // FIXME: process params
    // server.resource["^/ctrl-set"]["POST"] = [](std::shared_ptr<HttpServer::Response> response,
    //                                            std::shared_ptr<HttpServer::Request> request) {
    //     SoundProcessorManager::SetProcessParams(request->content.string());
    //     response->write(SimpleWeb::StatusCode::success_ok);
    // };
    //
    // server.resource["^/ctrl-get"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
    //                                           std::shared_ptr<HttpServer::Request> request) {
    //     response->write(SoundProcessorManager::GetProcessParams());
    // };


    // Default GET-example. If no other matches, this anonymous function will be called.
    // Will respond with content in the web/-directory, and its subdirectories.
    // Default file: index.html
    // Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server
    // I.E. static web files
    server.resource["^/ctrl"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                          std::shared_ptr<HttpServer::Request> request) {
        try {
            auto web_root_path = storage::get_fs_path("www");
            if (!web_root_path) {
                TBD_LOGE(tag, "static web data root does not exist");
                response->write(SimpleWeb::StatusCode::client_error_not_found);
                return;
            }

            auto path = canonical(*web_root_path / "ui.html");

            // Check if path is within web_root_path
            auto path_starts_with_root = equal(web_root_path->begin(), web_root_path->end(), path.begin());
            auto root_path_depth = distance(web_root_path->begin(), web_root_path->end());
            auto path_depth = distance(path.begin(), path.end());
            if (root_path_depth > path_depth || !path_starts_with_root)
                throw invalid_argument("path must be within root path");

            SimpleWeb::CaseInsensitiveMultimap header;

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
                    read_and_send(const std::shared_ptr<HttpServer::Response> &response, const std::shared_ptr<ifstream> &ifs) {
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
                                        std::cerr << "Connection interrupted" << endl;
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
    server.default_resource["GET"] = [](std::shared_ptr<HttpServer::Response> response,
                                        std::shared_ptr<HttpServer::Request> request) {
        try {
            auto web_root_path = storage::get_fs_path("www");
            if (!web_root_path) {
                TBD_LOGE(tag, "static web data root does not exist");
                response->write(SimpleWeb::StatusCode::client_error_not_found);
                return;
            }

            auto path = canonical(
                *web_root_path / fs::path(request->path).relative_path());

            // Check if path is within web_root_path
            auto path_starts_with_root = equal(web_root_path->begin(), web_root_path->end(), path.begin());
            auto root_path_depth = distance(web_root_path->begin(), web_root_path->end());
            auto path_depth = distance(path.begin(), path.end());
            if (root_path_depth > path_depth || !path_starts_with_root)
                throw invalid_argument("path must be within root path");

            if (is_directory(path))
                path /= "index.html";

            SimpleWeb::CaseInsensitiveMultimap header;

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
                    read_and_send(const std::shared_ptr<HttpServer::Response> &response, const std::shared_ptr<ifstream> &ifs) {
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
                                        std::cerr << "Connection interrupted" << std::endl;
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

    server.on_error = [](std::shared_ptr<HttpServer::Request> /*request*/, const SimpleWeb::error_code & /*ec*/) {
        // Handle errors here
        // Note that connection timeouts will also call this handle with ec set to SimpleWeb::errc::operation_canceled
    };

    // Start server and receive assigned port when server is listening for requests
    promise<unsigned short> server_port;
    thread st([&server_port]() {
        // Start server
        server.start([&server_port](unsigned short port) {
            server_port.set_value(port);
        });
    });
    server_thread = std::move(st);
    cout << "Server listening on port " << server_port.get_future().get() << std::endl << std::endl;
}

void RestApi::end() {
    server.stop();
    server_thread.join();
}

}
