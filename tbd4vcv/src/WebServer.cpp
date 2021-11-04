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

#include "WebServer.hpp"
//#include "SimSPManager.hpp"
#include <algorithm>
#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>


using namespace std;
using namespace boost::property_tree;


void WebServer::Start(const int port) {
    // HTTP-server at port 8080 using 1 thread
    // Unless you do more heavy non-threaded processing in the resources,
    // 1 thread is usually faster than several threads
    server.config.port = port;

    server.resource["^/api/v1/srom/getSize$"]["POST"] = [](shared_ptr<HttpServer::Response> response,
                                                        shared_ptr<HttpServer::Request> request) {
        response->write(to_string(1024*1024*5));
    };

    server.resource["^/api/v1/getPlugins$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                        shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        auto content = request->content.string();
        response->write(currentSPManager->GetCStrJSONSoundProcessors());
    };

    server.resource["^/api/v1/getIOCaps$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                        shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        auto content = request->content.string();
        string const s("{\"t\":[\"TRIG0\", \"TRIG1\"], \"cv\":[\"CV0\",\"CV1\",\"POT0\",\"POT1\"]}");
        response->write(s);
    };

    server.resource["^/api/v1/getActivePlugin/([0-1])$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                                     shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        response->write("{\"id\":\"" + currentSPManager->GetStringID(ch) + "\"}");
    };

    server.resource["^/api/v1/getPluginParams/([0-1])$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                                     shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        response->write(currentSPManager->GetCStrJSONActivePluginParams(ch));
    };

    server.resource["^/api/v1/setActivePlugin/([0-1])$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                                     shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        auto query_fields = request->parse_query_string();
        for (auto &field: query_fields) {
            if (field.first == "id") {
                currentSPManager->SetSoundProcessorChannel(ch, field.second);
            }
        }
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/setPluginParam/([0-1])$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                                    shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
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
        currentSPManager->SetChannelParamValue(ch, id, what, value);
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/setPluginParamCV/([0-1])$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                                      shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
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
        currentSPManager->SetChannelParamValue(ch, id, what, value);
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/setPluginParamTRIG/([0-1])$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                                        shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
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
        currentSPManager->SetChannelParamValue(ch, id, what, value);
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/getPresets/([0-1])$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                                shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        response->write(currentSPManager->GetCStrJSONGetPresets(ch));
    };

    server.resource["^/api/v1/loadPreset/([0-1])$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                                shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        auto query_fields = request->parse_query_string();
        int number = 0;
        for (auto &field: query_fields) {
            if (field.first == "number") {
                number = std::stoi(field.second);
            }
        }
        currentSPManager->ChannelLoadPreset(ch, number);
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/savePreset/([0-1])$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                                shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
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
        currentSPManager->ChannelSavePreset(ch, name, number);
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    // Default GET-example. If no other matches, this anonymous function will be called.
    // Will respond with content in the web/-directory, and its subdirectories.
    // Default file: index.html
    // Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server
    // I.E. static web files
    server.default_resource["GET"] = [](shared_ptr<HttpServer::Response> response,
                                        shared_ptr<HttpServer::Request> request) {
        try {
            auto web_root_path = boost::filesystem::canonical("plugins/tbd4vcv/spiffs_image/www");
            auto path = boost::filesystem::canonical(web_root_path / request->path);
            // Check if path is within web_root_path
            if (distance(web_root_path.begin(), web_root_path.end()) > distance(path.begin(), path.end()) ||
                !equal(web_root_path.begin(), web_root_path.end(), path.begin()))
                throw invalid_argument("path must be within root path");
            if (boost::filesystem::is_directory(path))
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
    thread st([&]() {
        // Start server
        server.start([&server_port](unsigned short port) {
            server_port.set_value(port);
        });
    });
    server_thread = std::move(st);
    cout << "Server listening on port " << server_port.get_future().get() << endl << endl;
    isRunning = true;
}

void WebServer::Stop() {
    if(!isRunning) return;
    server.stop();
    server_thread.join();
}

void WebServer::SetCurrentSPManager(CTAG::AUDIO::SPManager *manager) {
    std::cerr << "Setting current SP Manager " << manager << std::endl;
    currentSPManager = manager;
}
