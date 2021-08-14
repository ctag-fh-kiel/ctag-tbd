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
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace rapidjson;
using namespace boost::property_tree;

void WebServer::Start(const unsigned short port, const string &serialPort) {
    // HTTP-server at port 8080 using 1 thread
    // Unless you do more heavy non-threaded processing in the resources,
    // 1 thread is usually faster than several threads
    server.config.port = port;

    serial = std::make_unique<Serial>(serialPort, 115200);
/*
 * all get requests regex
    server.resource["^/api/v1/.*$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                         shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        auto content = request->content.string();
        string cmd = "{\"cmd\":\"" + request->path + "\"}";
        serial->writeString(cmd);
        response->write(serial->readString());
//        response->write(SimSPManager::GetCStrJSONSoundProcessors());
    };
*/
    server.resource["^/api/v1/getPlugins$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                        shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        auto content = request->content.string();
        string cmd = "{\"cmd\":\"" + request->path + "\"}";
        serial->writeString(cmd);
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        response->write(serial->readString(), header);
    };

    server.resource["^/api/v1/getActivePlugin/([0-1])$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                                     shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        string cmd = "{\"cmd\":\"" + request->path + "\", \"ch\":" + to_string(ch) + "}";
        serial->writeString(cmd);
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        response->write(serial->readString(), header);
    };


    server.resource["^/api/v1/getPluginParams/([0-1])$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                                     shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        string cmd = "{\"cmd\":\"" + request->path + "\", \"ch\":" + to_string(ch) + "}";
        serial->writeString(cmd);
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        response->write(serial->readString(), header);
    };

    server.resource["^/api/v1/setActivePlugin/([0-1])$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                                     shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        auto query_fields = request->parse_query_string();
        for (auto &field: query_fields) {
            if (field.first == "id") {
                string cmd = "{\"cmd\":\"" + request->path + "\", \"ch\":" + to_string(ch) + ", \"id\": \"" + field.second + "\"}";
                serial->writeString(cmd);
            }
        }
        serial->readString();
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
        string cmd = "{\"cmd\":\"" + request->path + "\", \"ch\":" + to_string(ch) + ", \"id\": \"" + id +
                "\", \"current\":" + to_string(value) + "}";
        serial->writeString(cmd);
        serial->readString();
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
        string cmd = "{\"cmd\":\"" + request->path + "\", \"ch\":" + to_string(ch) + ", \"id\": \"" + id +
                     "\", \"cv\":" + to_string(value) + "}";
        serial->writeString(cmd);
        serial->readString();
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
        string cmd = "{\"cmd\":\"" + request->path + "\", \"ch\":" + to_string(ch) + ", \"id\": \"" + id +
                     "\", \"trig\":" + to_string(value) + "}";
        serial->writeString(cmd);
        serial->readString();
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/getPresets/([0-1])$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                                shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int ch = std::stoi(request->path_match[1].str());
        string cmd = "{\"cmd\":\"" + request->path + "\", \"ch\":" + to_string(ch) + "}";
        serial->writeString(cmd);
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        response->write(serial->readString(), header);
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
        string cmd = "{\"cmd\":\"" + request->path + "\", \"ch\":" + to_string(ch) +
                ", \"number\": " + to_string(number) + "}";
        serial->writeString(cmd);
        serial->readString();
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
        string cmd = "{\"cmd\":\"" + request->path + "\", \"ch\":" + to_string(ch) +
                     ", \"number\": " + to_string(number) +
                     ", \"name\":\"" + name + "\"}";
        serial->writeString(cmd);
        serial->readString();
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/getConfiguration$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                         shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        auto content = request->content.string();
        string cmd = "{\"cmd\":\"" + request->path + "\"}";
        serial->writeString(cmd);
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        response->write(serial->readString(), header);
    };

    server.resource["^/api/v1/getIOCaps"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                               shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        auto content = request->content.string();
        string cmd = "{\"cmd\":\"" + request->path + "\"}";
        serial->writeString(cmd);
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        response->write(serial->readString(), header);
    };

    server.resource["^/api/v1/reboot$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                                 shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        auto query_fields = request->parse_query_string();
        int doCal = 0;
        for (auto &field: query_fields) {
            if (field.first == "calibration") {
                doCal = std::stoi(field.second);
            }
        }
        string cmd = "{\"cmd\":\"" + request->path + "\"" +
                     ", \"calibration\": " + to_string(doCal) + "}";
        serial->writeString(cmd);
        serial->readString();
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/getPresetData/(.+)"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                     shared_ptr<HttpServer::Request> request) {
        string id = request->path_match[1].str();
        string cmd = "{\"cmd\":\"" + request->path + "\"" +
                     ", \"id\": \"" + id + "\"}";
        serial->writeString(cmd);
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        response->write(serial->readString(), header);
    };

    server.resource["^/api/v1/getCalibration$"]["GET"] = [&](shared_ptr<HttpServer::Response> response,
                                                         shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        auto content = request->content.string();
        string cmd = "{\"cmd\":\"" + request->path + "\"}";
        serial->writeString(cmd);
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        response->write(serial->readString(), header);
    };

    server.resource["^/api/v1/setCalibration$"]["POST"] = [&](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        string content = request->content.string();
        string cmd = "{\"cmd\":\"" + request->path + "\", " +
                "\"calibration\": " + content + "}";

        serial->writeString(cmd);
        serial->readString();
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/setConfiguration$"]["POST"] = [&](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        string content = request->content.string();
        string cmd = "{\"cmd\":\"" + request->path + "\", " +
                     "\"configuration\": " + content + "}";
        cout << cmd << endl;
        serial->writeString(cmd);
        serial->readString();
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/favorites/getAll"]["POST"] = [&](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        string content = request->content.string();
        string cmd = "{\"cmd\":\"" + request->path + "\"}";
        cout << cmd << endl;
        serial->writeString(cmd);
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "application/json");
        response->write(serial->readString(), header);
    };

    server.resource["^/api/v1/favorites/store/([0-9])$"]["POST"] = [&](shared_ptr<HttpServer::Response> response,
                                                                      shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int fav = std::stoi(request->path_match[1].str());
        string content = request->content.string();
        string cmd = "{\"cmd\":\"" + request->path + "\", " +
                     "\"fav\": " + to_string(fav) + "," +
                     "\"data\": " + content + "}";
        cout << cmd << endl;
        serial->writeString(cmd);
        serial->readString();
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/favorites/recall/([0-9])$"]["POST"] = [&](shared_ptr<HttpServer::Response> response,
                                                                      shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        int fav = std::stoi(request->path_match[1].str());
        string content = request->content.string();
        string cmd = "{\"cmd\":\"" + request->path + "\"" +
                     ", \"fav\":" + to_string(fav) + "}";
        cout << cmd << endl;
        serial->writeString(cmd);
        serial->readString();
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/setPresetData/(.+)"]["POST"] = [&](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        string id = request->path_match[1].str();
        string content = request->content.string();
        string cmd = "{\"cmd\":\"" + request->path + "\", " +
                     "\"id\": \"" + id + "\"," +
                     "\"data\": " + content + "}";

        serial->writeString(cmd);
        serial->readString();
        response->write(SimpleWeb::StatusCode::success_ok);
    };

    server.resource["^/api/v1/srom/getSize$"]["POST"] = [&](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        string id = request->path_match[1].str();
        string content = request->content.string();
        string cmd = "{\"cmd\":\"/api/v1/srom/getSize\"}";
        SimpleWeb::CaseInsensitiveMultimap header;
        header.emplace("Content-Type", "text/plain");
        serial->writeString(cmd);
        response->write(serial->readString(), header);
    };
    // TODO API for serial sample rom writing
    /*
    server.resource["^/api/v1/srom/erase$"]["POST"] = [&](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        string cmd = "{\"cmd\":\"/api/v1/srom/erase\"}";

        serial->writeString(cmd);
        serial->readString();
        response->write(SimpleWeb::StatusCode::success_ok);
    };
    server.resource["^/api/v1/srom/upRaw"]["POST"] = [&](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
        // Retrieve string:
        string id = request->path_match[1].str();
        int size = request->content.size();
        string cmd = "{\"cmd\":\"/api/v1/srom/upRaw\", \"size\":";
        cmd += std::to_string(size);
        cmd += "}";

        serial->writeString(cmd);
        serial->readString();
        cerr << "Writing blob" << endl;
        serial->writeBLOB(request->content.string());
        serial->readString();

        response->write(SimpleWeb::StatusCode::success_ok);
    };
*/
    // Default GET-example. If no other matches, this anonymous function will be called.
    // Will respond with content in the web/-directory, and its subdirectories.
    // Default file: index.html
    // Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server
    // I.E. static web files
    server.default_resource["GET"] = [](shared_ptr<HttpServer::Response> response,
                                        shared_ptr<HttpServer::Request> request) {
        try {
            auto web_root_path = boost::filesystem::canonical("www");
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

string WebServer::CreateSerialRequest(const string &cmd) {
    Document d;
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    return buffer.GetString();
}
