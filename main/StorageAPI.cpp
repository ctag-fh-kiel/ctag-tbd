/***************
dadamachines TBD-16 — Storage & Sample Manager REST API

(c) 2014-2026 Johannes Elias Lohbihler for dadamachines.

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

Part of the dadamachines additions to the CTAG TBD platform.
See LICENSE in the repository root for full terms.

Provided "as is" without any express or implied warranties.
***************/

#include "StorageAPI.hpp"
#include "SPManager.hpp"
#include "sdkconfig.h"
#include <cstring>
#include <string>
#include <vector>
#include <inttypes.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>
#if CONFIG_TBD_USE_SD_CARD
#include "esp_vfs_fat.h"
#endif
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "StorageOverlay.hpp"

using namespace CTAG::REST;
using namespace rapidjson;

static const char *TAG = "StorageAPI";

// SD card paths
static const char *SD_ROOT = "/sdcard";
// Kit JSON overlay: user-created kits go here, factory kits in /factory/kits/
static const char *USER_KITS_DIR = "/sdcard/user/kits";
static const char *FACTORY_KITS_DIR = "/sdcard/factory/kits";
// Config files are accessed via StorageOverlay (factory+user resolution).
// Legacy /data/ path removed — all config reads use overlay, writes go to /user/.

// PSRAM capacity constant (~28 MB)
static const uint32_t PSRAM_MAX_BYTES = 29360128;

// Scratch buffer for chunked file reads (allocated from SPIRAM)
#define CHUNK_BUF_SIZE 4096

// ─── Helpers ─────────────────────────────────────────────────

/**
 * Resolve a kit JSON filename using overlay pattern:
 * 1. /user/kits/{filename} — user-created or user-modified kits
 * 2. /factory/kits/{filename} — factory-shipped kits
 * Returns full path to the kit file.
 */
static std::string resolveKitFile(const std::string &filename) {
    std::string userKit = std::string(USER_KITS_DIR) + "/" + filename;
    if (CTAG::STORAGE::fileExists(userKit)) return userKit;
    std::string factoryKit = std::string(FACTORY_KITS_DIR) + "/" + filename;
    if (CTAG::STORAGE::fileExists(factoryKit)) return factoryKit;
    // Fallback: return factory path even if it doesn't exist yet
    return factoryKit;
}

/**
 * Get the user-writable path for a kit file.
 * Always returns /user/kits/{filename} for writes.
 */
static std::string userKitPath(const std::string &filename) {
    return std::string(USER_KITS_DIR) + "/" + filename;
}

static const char *SAMPLE_ROM_FILENAME = "sample_rom.json";

/**
 * Resolve sample_rom.json via overlay (user → factory → samples legacy).
 */
static std::string resolveSampleRomFile() {
    return resolveKitFile(SAMPLE_ROM_FILENAME);
}

/**
 * Get the user-writable path for sample_rom.json (copy-on-write).
 */
static std::string userSampleRomFile() {
    return userKitPath(SAMPLE_ROM_FILENAME);
}

/**
 * Check if a kit file is a factory kit (lives in /factory/kits/ or /samples/ but NOT /user/kits/).
 */
static bool isFactoryKit(const std::string &filename) {
    std::string userKit = std::string(USER_KITS_DIR) + "/" + filename;
    return !CTAG::STORAGE::fileExists(userKit);
}

/** URL-decode a query value in-place. Only handles %XX and + → space. */
static void url_decode(char *dst, const char *src, size_t dst_size) {
    size_t i = 0;
    while (*src && i < dst_size - 1) {
        if (*src == '%' && src[1] && src[2]) {
            char hex[3] = { src[1], src[2], 0 };
            dst[i++] = (char)strtol(hex, nullptr, 16);
            src += 3;
        } else if (*src == '+') {
            dst[i++] = ' ';
            src++;
        } else {
            dst[i++] = *src++;
        }
    }
    dst[i] = 0;
}

static void set_api_headers(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Connection", "close");
}

/** Send a JSON string as HTTP response */
static esp_err_t send_json(httpd_req_t *req, const char *json) {
    set_api_headers(req);
    httpd_resp_set_type(req, "application/json");
    if (json) httpd_resp_sendstr(req, json);
    else httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

/** Send a simple JSON ok response */
static esp_err_t send_ok(httpd_req_t *req) {
    return send_json(req, "{\"ok\":true}");
}

/** Send a JSON error response */
static esp_err_t send_error(httpd_req_t *req, int code, const char *msg) {
    httpd_resp_set_status(req, code == 404 ? "404 Not Found" : "500 Internal Server Error");
    char buf[128];
    snprintf(buf, sizeof(buf), "{\"error\":\"%s\"}", msg);
    return send_json(req, buf);
}

/** Read POST body into a SPIRAM-allocated buffer. Caller must free(). */
static char *read_post_body(httpd_req_t *req) {
    if (req->content_len == 0) return nullptr;
    char *buf = (char *)heap_caps_malloc(req->content_len + 1, MALLOC_CAP_SPIRAM);
    if (!buf) {
        ESP_LOGE(TAG, "Failed to allocate %d bytes for POST body", req->content_len);
        return nullptr;
    }
    int remaining = req->content_len;
    char *ptr = buf;
    while (remaining > 0) {
        int recv = httpd_req_recv(req, ptr, remaining);
        if (recv <= 0) {
            if (recv == HTTPD_SOCK_ERR_TIMEOUT) {
                continue; // retry on timeout
            }
            heap_caps_free(buf);
            return nullptr;
        }
        ptr += recv;
        remaining -= recv;
    }
    buf[req->content_len] = 0;
    return buf;
}

/** Load a JSON file from SD card into a Document */
static bool load_json_file(const char *path, Document &doc) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        ESP_LOGW(TAG, "Cannot open %s", path);
        return false;
    }
    char *readBuf = (char *)heap_caps_malloc(1024, MALLOC_CAP_SPIRAM);
    if (!readBuf) { fclose(fp); return false; }
    FileReadStream is(fp, readBuf, 1024);
    doc.ParseStream(is);
    fclose(fp);
    heap_caps_free(readBuf);
    return !doc.HasParseError();
}

/** Store a JSON Document to a file on SD card */
static bool store_json_file(const char *path, Document &doc) {
    CTAG::STORAGE::lockStorage();
    FILE *fp = fopen(path, "w");
    if (!fp) {
        CTAG::STORAGE::unlockStorage();
        ESP_LOGE(TAG, "Cannot write %s", path);
        return false;
    }
    char writeBuf[1024];
    FileWriteStream os(fp, writeBuf, sizeof(writeBuf));
    Writer<FileWriteStream> writer(os);
    doc.Accept(writer);
    fflush(fp);
    fclose(fp);
    CTAG::STORAGE::unlockStorage();
    return true;
}

/**
 * Recursively scan a directory for .wav files.
 * Appends to the JSON array `files` in the given allocator.
 * `rel` is the relative path from SAMPLE_ROOT.
 */
static void scan_all_files(const char *base, const char *rel,
                           Value &files, Document::AllocatorType &alloc) {
    std::string dirPath = std::string(base);
    if (rel && rel[0]) {
        dirPath += "/";
        dirPath += rel;
    }

    DIR *dir = opendir(dirPath.c_str());
    if (!dir) return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
        // Skip hidden files
        if (ent->d_name[0] == '.') continue;

        std::string relPath;
        if (rel && rel[0]) {
            relPath = std::string(rel) + "/" + ent->d_name;
        } else {
            relPath = ent->d_name;
        }

        std::string fullPath = std::string(base) + "/" + relPath;
        struct stat st;
        if (stat(fullPath.c_str(), &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            // Recurse into subdirectory
            scan_all_files(base, relPath.c_str(), files, alloc);
        } else if (S_ISREG(st.st_mode)) {
                // Extract folder path (relative)
                std::string folder;
                if (rel && rel[0]) {
                    folder = rel;
                }

                Value fileObj(kObjectType);
                Value nameVal(ent->d_name, alloc);
                Value pathVal(folder.c_str(), alloc);
                fileObj.AddMember("name", nameVal, alloc);
                fileObj.AddMember("path", pathVal, alloc);
                fileObj.AddMember("size", (uint64_t)st.st_size, alloc);
                // mtime in milliseconds (ESP-IDF st_mtime is seconds)
                fileObj.AddMember("mtime", (uint64_t)st.st_mtime * 1000ULL, alloc);
                files.PushBack(fileObj, alloc);
        }
    }
    closedir(dir);
}

/**
 * Recursively scan a directory for .wav files.
 * Appends to the JSON array `files` in the given allocator.
 * `rel` is the relative path from SAMPLE_ROOT.
 */
static void scan_json_files(const char *base, const char *rel,
                           Value &files, Document::AllocatorType &alloc) {
    std::string dirPath = std::string(base);
    if (rel && rel[0]) {
        dirPath += "/";
        dirPath += rel;
    }

    DIR *dir = opendir(dirPath.c_str());
    if (!dir) return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
        // Skip hidden files
        if (ent->d_name[0] == '.') continue;

        std::string relPath;
        if (rel && rel[0]) {
            relPath = std::string(rel) + "/" + ent->d_name;
        } else {
            relPath = ent->d_name;
        }

        std::string fullPath = std::string(base) + "/" + relPath;
        struct stat st;
        if (stat(fullPath.c_str(), &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            // Recurse into subdirectory
            scan_json_files(base, relPath.c_str(), files, alloc);
        } else if (S_ISREG(st.st_mode)) {
            // Check if .wav extension
            size_t len = strlen(ent->d_name);
            if (len > 5 && strcasecmp(ent->d_name + len - 5, ".json") == 0) {
                // Extract name without extension
                // std::string stem(ent->d_name, len - 4);
                // Extract folder path (relative)
                std::string folder;
                if (rel && rel[0]) {
                    folder = rel;
                }

                Value fileObj(kObjectType);
                Value nameVal(ent->d_name, alloc);
                Value pathVal(folder.c_str(), alloc);
                fileObj.AddMember("name", nameVal, alloc);
                fileObj.AddMember("path", pathVal, alloc);
                fileObj.AddMember("size", (uint64_t)st.st_size, alloc);
                // mtime in milliseconds (ESP-IDF st_mtime is seconds)
                fileObj.AddMember("mtime", (uint64_t)st.st_mtime * 1000ULL, alloc);
                files.PushBack(fileObj, alloc);
            }
        }
    }
    closedir(dir);
}

/**
 * Scan a single directory for JSON files, using a fixed path prefix for results.
 * Unlike scan_json_files, the "path" field is set to pathPrefix for top-level
 * files and "pathPrefix/rel" for nested files.
 */
static void scan_json_with_prefix(const char *dir, const char *rel,
                                  const char *pathPrefix,
                                  Value &files, Document::AllocatorType &alloc) {
    std::string dirPath = std::string(dir);
    if (rel && rel[0]) { dirPath += "/"; dirPath += rel; }

    DIR *d = opendir(dirPath.c_str());
    if (!d) return;

    struct dirent *ent;
    while ((ent = readdir(d)) != nullptr) {
        if (ent->d_name[0] == '.') continue;
        std::string relPath;
        if (rel && rel[0]) { relPath = std::string(rel) + "/" + ent->d_name; }
        else { relPath = ent->d_name; }

        std::string fullPath = std::string(dir) + "/" + relPath;
        struct stat st;
        if (stat(fullPath.c_str(), &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            scan_json_with_prefix(dir, relPath.c_str(), pathPrefix, files, alloc);
        } else if (S_ISREG(st.st_mode)) {
            size_t len = strlen(ent->d_name);
            if (len > 5 && strcasecmp(ent->d_name + len - 5, ".json") == 0) {
                // Build path: "prefix" or "prefix/rel_folder"
                std::string folder(pathPrefix);
                if (rel && rel[0]) { folder += "/"; folder += rel; }

                Value fileObj(kObjectType);
                Value nameVal(ent->d_name, alloc);
                Value pathVal(folder.c_str(), alloc);
                fileObj.AddMember("name", nameVal, alloc);
                fileObj.AddMember("path", pathVal, alloc);
                fileObj.AddMember("size", (uint64_t)st.st_size, alloc);
                fileObj.AddMember("mtime", (uint64_t)st.st_mtime * 1000ULL, alloc);
                files.PushBack(fileObj, alloc);
            }
        }
    }
    closedir(d);
}

/**
 * Scan overlay config directories (merged factory+user) for JSON files.
 * Returns entries with path = "subdir" or "subdir/nested", matching
 * the overlay subdir names (presets, macros, plugins, config, trackdefaults).
 * User files shadow same-named factory files.
 */
static void scan_overlay_configs(Value &files, Document::AllocatorType &alloc) {
    using namespace CTAG::STORAGE;
    static const char *SUBDIRS[] = {
        DIR_PRESETS, DIR_MACROS, DIR_PLUGINS, DIR_CONFIG, DIR_TRACKDEFAULTS, nullptr
    };
    for (int i = 0; SUBDIRS[i]; i++) {
        const char *subdir = SUBDIRS[i];
        // Collect user-layer relative paths first (these shadow factory)
        std::vector<std::string> userPaths;
        std::string userDir = userPath() + "/" + subdir;
        // Scan user zone with subdir as path prefix
        int beforeUser = files.Size();
        scan_json_with_prefix(userDir.c_str(), "", subdir, files, alloc);
        // Record relative paths found in user zone for dedup
        for (rapidjson::SizeType j = beforeUser; j < files.Size(); j++) {
            std::string p = files[j]["path"].GetString();
            p += "/";
            p += files[j]["name"].GetString();
            userPaths.push_back(p);
        }
        // Scan factory zone, skip entries already found in user
        int beforeFactory = files.Size();
        scan_json_with_prefix((factoryPath() + "/" + subdir).c_str(), "", subdir, files, alloc);
        // Remove factory entries that shadow user entries
        // Walk backwards to safely erase
        for (int j = (int)files.Size() - 1; j >= beforeFactory; j--) {
            std::string p = files[j]["path"].GetString();
            p += "/";
            p += files[j]["name"].GetString();
            bool shadowed = false;
            for (const auto &up : userPaths) {
                if (up == p) { shadowed = true; break; }
            }
            if (shadowed) {
                // Remove by swapping with last element
                files[j] = files[files.Size() - 1];
                files.PopBack();
            }
        }
    }
}

/** Scan directory tree and collect all directory paths (relative) */
static void scan_directories(const char *base, const char *rel,
                             Value &dirs, Document::AllocatorType &alloc) {
    std::string dirPath = std::string(base);
    if (rel && rel[0]) {
        dirPath += "/";
        dirPath += rel;
    }

    DIR *dir = opendir(dirPath.c_str());
    if (!dir) return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (ent->d_name[0] == '.') continue;

        std::string relPath;
        if (rel && rel[0]) {
            relPath = std::string(rel) + "/" + ent->d_name;
        } else {
            relPath = ent->d_name;
        }

        std::string fullPath = std::string(base) + "/" + relPath;
        struct stat st;
        if (stat(fullPath.c_str(), &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            Value dirVal(relPath.c_str(), alloc);
            dirs.PushBack(dirVal, alloc);
            // Recurse
            scan_directories(base, relPath.c_str(), dirs, alloc);
        }
    }
    closedir(dir);
}

/** Compute capacity from kit entries */
static uint32_t compute_used_bytes(Document &kitDoc) {
    uint32_t bytes = 0;
    if (kitDoc.IsArray()) {
        for (auto &v : kitDoc.GetArray()) {
            if (v.HasMember("nsamples") && v["nsamples"].IsUint()) {
                bytes += v["nsamples"].GetUint() * 2; // 16-bit mono
            }
        }
    }
    return bytes;
}

// ─── GET /api/v2/samples (list) ──────────────────────────────
//
// Query params:
//   ?kit=N        — switch active kit index before listing
//   ?preview=path/name — serve a WAV file for audio preview
//
static esp_err_t handle_list(httpd_req_t *req) {
    ESP_LOGI(TAG, "samples_list_handler");

    // Parse query string
    size_t qlen = httpd_req_get_url_query_len(req);
    char *query = nullptr;
    char val[256] = {0};

    if (qlen > 0) {
        query = (char *)malloc(qlen + 1);
        httpd_req_get_url_query_str(req, query, qlen + 1);

        // Handle preview request
        if (httpd_query_key_value(query, "preview", val, sizeof(val)) == ESP_OK) {
            free(query);
            // val contains path like "samples/factory/drums/BD0"
            char decoded[256];
            url_decode(decoded, val, sizeof(decoded));
            // Try exact path first (may include extension), then .wav/.WAV
            std::string wavPath = std::string(SD_ROOT) + "/" + decoded;
            FILE *fp = fopen(wavPath.c_str(), "r");
            if (!fp) {
                wavPath = std::string(SD_ROOT) + "/" + decoded + ".wav";
                fp = fopen(wavPath.c_str(), "r");
            }
            if (!fp) {
                wavPath = std::string(SD_ROOT) + "/" + decoded + ".WAV";
                fp = fopen(wavPath.c_str(), "r");
            }
            if (!fp) {
                return send_error(req, 404, "Preview not found");
            }

            httpd_resp_set_type(req, "audio/wav");
            httpd_resp_set_hdr(req, "Connection", "close");
            httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
            httpd_resp_set_hdr(req, "Cache-Control", "no-cache");

            // Stream the file in chunks
            char *chunk = (char *)heap_caps_malloc(CHUNK_BUF_SIZE, MALLOC_CAP_SPIRAM);
            if (!chunk) {
                fclose(fp);
                return send_error(req, 500, "Out of memory");
            }
            size_t read_bytes;
            do {
                read_bytes = fread(chunk, 1, CHUNK_BUF_SIZE, fp);
                if (read_bytes > 0) {
                    if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                        fclose(fp);
                        heap_caps_free(chunk);
                        httpd_resp_send_chunk(req, NULL, 0);
                        return ESP_FAIL;
                    }
                }
            } while (read_bytes > 0);

            fclose(fp);
            heap_caps_free(chunk);
            httpd_resp_send_chunk(req, NULL, 0);
            return ESP_OK;
        }

        // Handle file download / raw fetch request
        // ?download=path — serves any file with Content-Disposition: attachment
        // ?fetch=path    — serves file inline (for JSON viewer etc.)
        if (httpd_query_key_value(query, "download", val, sizeof(val)) == ESP_OK ||
            httpd_query_key_value(query, "fetch", val, sizeof(val)) == ESP_OK) {
            bool isDownload = (httpd_query_key_value(query, "download", val, sizeof(val)) == ESP_OK);
            free(query);
            char decoded[256];
            url_decode(decoded, val, sizeof(decoded));
            std::string filePath = std::string(SD_ROOT) + "/" + decoded;
            FILE *fp = fopen(filePath.c_str(), "r");
            if (!fp) {
                return send_error(req, 404, "File not found");
            }

            fseek(fp, 0, SEEK_END);
            long fsize = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            // Determine Content-Type from extension
            std::string ext;
            size_t dotPos = filePath.rfind('.');
            if (dotPos != std::string::npos) ext = filePath.substr(dotPos);
            const char *ctype = "application/octet-stream";
            if (ext == ".json")      ctype = "application/json";
            else if (ext == ".txt")  ctype = "text/plain";
            else if (ext == ".wav" || ext == ".WAV") ctype = "audio/wav";
            else if (ext == ".html") ctype = "text/html";
            else if (ext == ".css")  ctype = "text/css";
            else if (ext == ".js")   ctype = "application/javascript";

            httpd_resp_set_type(req, ctype);
            httpd_resp_set_hdr(req, "Connection", "close");
            httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
            httpd_resp_set_hdr(req, "Cache-Control", "no-cache");

            // Content-Length for proper download progress
            char lenBuf[16];
            snprintf(lenBuf, sizeof(lenBuf), "%ld", fsize);
            httpd_resp_set_hdr(req, "Content-Length", lenBuf);

            if (isDownload) {
                // Extract filename for Content-Disposition
                std::string fname(decoded);
                size_t lastSlash = fname.rfind('/');
                if (lastSlash != std::string::npos) fname = fname.substr(lastSlash + 1);
                std::string disposition = "attachment; filename=\"" + fname + "\"";
                httpd_resp_set_hdr(req, "Content-Disposition", disposition.c_str());
            }

            char *chunk = (char *)heap_caps_malloc(CHUNK_BUF_SIZE, MALLOC_CAP_SPIRAM);
            if (!chunk) {
                fclose(fp);
                return send_error(req, 500, "Out of memory");
            }
            size_t read_bytes;
            do {
                read_bytes = fread(chunk, 1, CHUNK_BUF_SIZE, fp);
                if (read_bytes > 0) {
                    if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                        fclose(fp);
                        heap_caps_free(chunk);
                        httpd_resp_send_chunk(req, NULL, 0);
                        return ESP_FAIL;
                    }
                }
            } while (read_bytes > 0);

            fclose(fp);
            heap_caps_free(chunk);
            httpd_resp_send_chunk(req, NULL, 0);
            return ESP_OK;
        }


        // Handle preview request
        if (httpd_query_key_value(query, "getconfig", val, sizeof(val)) == ESP_OK) {
            free(query);
            // val contains path like "drums/factory/BD0"
            char decoded[256];
            url_decode(decoded, val, sizeof(decoded));
            // Parse subdir/filename from path (e.g. "presets/myfile.json")
            std::string decodedStr(decoded);
            std::string configSubdir, configFilename;
            size_t slashPos = decodedStr.find('/');
            if (slashPos != std::string::npos) {
                configSubdir = decodedStr.substr(0, slashPos);
                configFilename = decodedStr.substr(slashPos + 1);
            } else {
                configFilename = decodedStr;
            }
            std::string jsonPath;
            if (!configSubdir.empty()) {
                jsonPath = CTAG::STORAGE::resolveFile(configSubdir.c_str(), configFilename);
            } else {
                // No subdir — check root of user/ then factory/ (e.g. synthdefinitions.json)
                std::string userRoot = CTAG::STORAGE::userPath() + "/" + configFilename;
                if (CTAG::STORAGE::fileExists(userRoot)) {
                    jsonPath = userRoot;
                } else {
                    std::string factoryRoot = CTAG::STORAGE::factoryPath() + "/" + configFilename;
                    if (CTAG::STORAGE::fileExists(factoryRoot)) {
                        jsonPath = factoryRoot;
                    }
                }
            }
            if (jsonPath.empty()) {
                return send_error(req, 404, "Config file not found");
            }

            FILE *fp = fopen(jsonPath.c_str(), "r");
            if (!fp) {
                return send_error(req, 404, "Config file not found");
            }

            httpd_resp_set_type(req, "application/json");
            httpd_resp_set_hdr(req, "Connection", "close");
            httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
            httpd_resp_set_hdr(req, "Cache-Control", "no-cache");

            // Stream the file in chunks
            char *chunk = (char *)heap_caps_malloc(CHUNK_BUF_SIZE, MALLOC_CAP_SPIRAM);
            if (!chunk) {
                fclose(fp);
                return send_error(req, 500, "Out of memory");
            }
            size_t read_bytes;
            do {
                read_bytes = fread(chunk, 1, CHUNK_BUF_SIZE, fp);
                if (read_bytes > 0) {
                    if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                        fclose(fp);
                        heap_caps_free(chunk);
                        httpd_resp_send_chunk(req, NULL, 0);
                        return ESP_FAIL;
                    }
                }
            } while (read_bytes > 0);

            fclose(fp);
            heap_caps_free(chunk);
            httpd_resp_send_chunk(req, NULL, 0);
            return ESP_OK;
        }
    }

    // Load sample_rom.json via overlay (user → factory → samples)
    Document sampleRom;
    if (!load_json_file(resolveSampleRomFile().c_str(), sampleRom)) {
        if (query) free(query);
        return send_error(req, 500, "Cannot read sample_rom.json");
    }

    // Handle kit switching: ?kit=N
    int activeKit = 0;
    if (sampleRom.HasMember("active_smp_bank") && sampleRom["active_smp_bank"].IsUint()) {
        activeKit = sampleRom["active_smp_bank"].GetUint();
    }

    if (query) {
        char kitVal[16] = {0};
        if (httpd_query_key_value(query, "kit", kitVal, sizeof(kitVal)) == ESP_OK) {
            int idx = atoi(kitVal);
            if (idx >= 0 && sampleRom.HasMember("smp_banks") &&
                (uint32_t)idx < sampleRom["smp_banks"].GetArray().Size()) {
                activeKit = idx;
                // Update active index in document (don't persist — just in-memory for response)
                sampleRom["active_smp_bank"].SetUint(activeKit);
            }
        }
        free(query);
    }

    // Build response document
    Document resp(kObjectType);
    auto &alloc = resp.GetAllocator();

    // 1) Scan all files from entire SD card
    Value files(kArrayType);
    scan_all_files(SD_ROOT, "", files, alloc);
    resp.AddMember("files", files, alloc);

    // 1b) Scan all directories (including empty ones)
    Value dirs(kArrayType);
    scan_directories(SD_ROOT, "", dirs, alloc);
    resp.AddMember("directories", dirs, alloc);

    // 1b) Scan all directories (including empty ones)
    // Value dirs2(kArrayType);
    // scan_directories(ROOT, "", dirs2, alloc);
    // resp.AddMember("rootdirectories", dirs2, alloc);

    Value files2(kArrayType);
    scan_overlay_configs(files2, alloc);
    resp.AddMember("configfiles", files2, alloc);

    // 2) Kits metadata (entire sample_rom object)
    Value kitsObj(kObjectType);
    // Copy relevant fields from sampleRom
    if (sampleRom.HasMember("smp_banks")) {
        Value arr(sampleRom["smp_banks"], alloc);
        kitsObj.AddMember("smp_banks", arr, alloc);
    }
    if (sampleRom.HasMember("smp_bank_names")) {
        Value arr(sampleRom["smp_bank_names"], alloc);
        kitsObj.AddMember("smp_bank_names", arr, alloc);
    }
    if (sampleRom.HasMember("smp_bank_tags")) {
        Value arr(sampleRom["smp_bank_tags"], alloc);
        kitsObj.AddMember("smp_bank_tags", arr, alloc);
    }
    if (sampleRom.HasMember("smp_bank_meta")) {
        Value arr(sampleRom["smp_bank_meta"], alloc);
        kitsObj.AddMember("smp_bank_meta", arr, alloc);
    }
    kitsObj.AddMember("active_smp_bank", activeKit, alloc);
    resp.AddMember("kits", kitsObj, alloc);

    // 3) Active kit entries
    Value kitEntries(kArrayType);
    if (sampleRom.HasMember("smp_banks") && sampleRom["smp_banks"].IsArray()) {
        auto &banks = sampleRom["smp_banks"];
        if ((uint32_t)activeKit < banks.GetArray().Size()) {
            std::string kitFile = resolveKitFile(banks[activeKit].GetString());
            Document kitDoc;
            if (load_json_file(kitFile.c_str(), kitDoc) && kitDoc.IsArray()) {
                Value entriesCopy(kitDoc, alloc);
                kitEntries = entriesCopy.Move();
            }
        }
    }

    // 4) Capacity
    Value capacityObj(kObjectType);
    capacityObj.AddMember("psram_max_bytes", (unsigned)PSRAM_MAX_BYTES, alloc);
    // Compute used bytes from kit entries (skip null entries)
    uint32_t usedBytes = 0;
    if (kitEntries.IsArray()) {
        for (auto &v : kitEntries.GetArray()) {
            if (v.IsObject() && v.HasMember("nsamples") && v["nsamples"].IsUint()) {
                usedBytes += v["nsamples"].GetUint() * 2;
            }
        }
    }
    capacityObj.AddMember("active_bank_bytes", (unsigned)usedBytes, alloc);

    // SD card storage info via esp_vfs_fat_info
#if CONFIG_TBD_USE_SD_CARD
    uint64_t sdTotal = 0, sdFree = 0;
    if (esp_vfs_fat_info("/sdcard", &sdTotal, &sdFree) == ESP_OK) {
        // Use double to avoid uint32 overflow for large SD cards
        capacityObj.AddMember("sd_total_bytes", (double)sdTotal, alloc);
        capacityObj.AddMember("sd_free_bytes",  (double)sdFree,  alloc);
    }
#endif

    resp.AddMember("capacity", capacityObj, alloc);

    resp.AddMember("active_kit_entries", kitEntries, alloc);

    // Serialize to string
    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    resp.Accept(writer);

    return send_json(req, sb.GetString());
}

// ─── POST /api/v2/samples?action=upload ──────────────────────
//
// Query params: ?path=drums/user&filename=mysample
// Body: raw WAV binary
//
static esp_err_t handle_upload(httpd_req_t *req) {
    ESP_LOGI(TAG, "samples_upload_handler, content_len=%d", req->content_len);

    // Parse query params
    size_t qlen = httpd_req_get_url_query_len(req);
    char query_buf[256] = {0};
    char pathVal[128] = "user";
    char filenameVal[64] = {0};

    if (qlen > 0) {
        httpd_req_get_url_query_str(req, query_buf, sizeof(query_buf));
        char raw[128];
        if (httpd_query_key_value(query_buf, "path", raw, sizeof(raw)) == ESP_OK) {
            url_decode(pathVal, raw, sizeof(pathVal));
        }
        if (httpd_query_key_value(query_buf, "filename", raw, sizeof(raw)) == ESP_OK) {
            url_decode(filenameVal, raw, sizeof(filenameVal));
        }
    }

    if (filenameVal[0] == 0) {
        snprintf(filenameVal, sizeof(filenameVal), "sample_%lu", (unsigned long)esp_log_timestamp());
    }

    // Ensure target directory exists
    std::string dirPath = std::string(SD_ROOT) + "/" + pathVal;
    // Create directories recursively (simple approach)
    {
        std::string tmp;
        for (size_t i = 0; i < dirPath.size(); i++) {
            tmp += dirPath[i];
            if (dirPath[i] == '/' && i > 0) {
                mkdir(tmp.c_str(), 0755);
            }
        }
        mkdir(dirPath.c_str(), 0755);
    }

    // Write WAV file
    std::string filePath = dirPath + "/" + filenameVal + ".wav";
    FILE *fp = fopen(filePath.c_str(), "w");
    if (!fp) {
        ESP_LOGE(TAG, "Cannot create file: %s", filePath.c_str());
        return send_error(req, 500, "Cannot create file");
    }

    // Receive and write in chunks
    char *chunk = (char *)heap_caps_malloc(CHUNK_BUF_SIZE, MALLOC_CAP_SPIRAM);
    if (!chunk) {
        fclose(fp);
        return send_error(req, 500, "Out of memory");
    }

    int remaining = req->content_len;
    int total_written = 0;
    while (remaining > 0) {
        int toRead = remaining > CHUNK_BUF_SIZE ? CHUNK_BUF_SIZE : remaining;
        int recv = httpd_req_recv(req, chunk, toRead);
        if (recv <= 0) {
            if (recv == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            ESP_LOGE(TAG, "Upload recv error");
            fclose(fp);
            heap_caps_free(chunk);
            remove(filePath.c_str());
            return send_error(req, 500, "Upload receive error");
        }
        fwrite(chunk, 1, recv, fp);
        remaining -= recv;
        total_written += recv;
    }

    fflush(fp);
    fclose(fp);
    heap_caps_free(chunk);

    ESP_LOGI(TAG, "Uploaded %s (%d bytes)", filePath.c_str(), total_written);

    // Return OK with file info
    char resp[256];
    snprintf(resp, sizeof(resp),
             "{\"ok\":true,\"name\":\"%s\",\"path\":\"%s\",\"size\":%d}",
             filenameVal, pathVal, total_written);
    return send_json(req, resp);
}


static esp_err_t handle_uploadconfig(httpd_req_t *req) {
    ESP_LOGI(TAG, "samples_uploadconfig_handler, content_len=%d", req->content_len);

    // Parse query params
    size_t qlen = httpd_req_get_url_query_len(req);
    char query_buf[256] = {0};
    char pathVal[128] = "user";
    char filenameVal[64] = {0};

    if (qlen > 0) {
        httpd_req_get_url_query_str(req, query_buf, sizeof(query_buf));
        char raw[128];
        if (httpd_query_key_value(query_buf, "path", raw, sizeof(raw)) == ESP_OK) {
            url_decode(pathVal, raw, sizeof(pathVal));
        } else {
            return send_error(req, 400, "Bad request");
        }
    }

    if (filenameVal[0] == 0) {
        snprintf(filenameVal, sizeof(filenameVal), "sample_%lu", (unsigned long)esp_log_timestamp());
    }

    // Write configfile to user overlay dir
    // pathVal is "subdir/filename" (e.g. "macros/mydef.json")
    std::string pathStr(pathVal);
    size_t slashPos = pathStr.find('/');
    std::string filePath;
    if (slashPos != std::string::npos) {
        std::string subdir = pathStr.substr(0, slashPos);
        std::string fname = pathStr.substr(slashPos + 1);
        filePath = CTAG::STORAGE::userFilePath(subdir.c_str(), fname);
        // Ensure parent directory exists
        std::string parentDir = CTAG::STORAGE::userPath() + "/" + subdir;
        mkdir(parentDir.c_str(), 0755);
    } else {
        // No subdir — write to user/config/
        filePath = CTAG::STORAGE::userFilePath(CTAG::STORAGE::DIR_CONFIG, pathStr);
    }
    CTAG::STORAGE::lockStorage();
    FILE *fp = fopen(filePath.c_str(), "w");
    if (!fp) {
        CTAG::STORAGE::unlockStorage();
        ESP_LOGE(TAG, "Cannot create file: %s", filePath.c_str());
            return send_error(req, 500, "Cannot create file");
    }

    // Receive and write in chunks
    char *chunk = (char *)heap_caps_malloc(CHUNK_BUF_SIZE, MALLOC_CAP_SPIRAM);
    if (!chunk) {
        fclose(fp);
        CTAG::STORAGE::unlockStorage();
        return send_error(req, 500, "Out of memory");
    }

    int remaining = req->content_len;
    int total_written = 0;
    while (remaining > 0) {
        int toRead = remaining > CHUNK_BUF_SIZE ? CHUNK_BUF_SIZE : remaining;
        int recv = httpd_req_recv(req, chunk, toRead);
        if (recv <= 0) {
            if (recv == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            ESP_LOGE(TAG, "Upload recv error");
            fclose(fp);
            CTAG::STORAGE::unlockStorage();
            heap_caps_free(chunk);
            remove(filePath.c_str());
            return send_error(req, 500, "Upload receive error");
        }
        fwrite(chunk, 1, recv, fp);
        remaining -= recv;
        total_written += recv;
    }

    fflush(fp);
    fclose(fp);
    CTAG::STORAGE::unlockStorage();
    heap_caps_free(chunk);

    ESP_LOGI(TAG, "Uploaded %s (%d bytes)", filePath.c_str(), total_written);

    // Return OK with file info
    char resp[256];
    snprintf(resp, sizeof(resp),
             "{\"ok\":true,\"name\":\"%s\",\"path\":\"%s\",\"size\":%d}",
             filenameVal, pathVal, total_written);
    return send_json(req, resp);
}

// ─── POST /api/v2/samples?action=uploadwww ───────────────────
//
// Query params: ?path=js/macro-bundle.js.gz  (relative to /sdcard/www/)
// Body: raw file data (binary)
//
static esp_err_t handle_uploadwww(httpd_req_t *req) {
    ESP_LOGI(TAG, "handle_uploadwww, content_len=%d", req->content_len);

    size_t qlen = httpd_req_get_url_query_len(req);
    char query_buf[256] = {0};
    char pathVal[128] = {0};

    if (qlen > 0) {
        httpd_req_get_url_query_str(req, query_buf, sizeof(query_buf));
        char raw[128];
        if (httpd_query_key_value(query_buf, "path", raw, sizeof(raw)) == ESP_OK) {
            url_decode(pathVal, raw, sizeof(pathVal));
        } else {
            return send_error(req, 400, "Missing path parameter");
        }
    } else {
        return send_error(req, 400, "Missing query parameters");
    }

    // Path traversal protection
    if (strstr(pathVal, "..") != nullptr) {
        return send_error(req, 400, "Invalid path");
    }

    // Construct full path and ensure parent directories exist
    std::string filePath = std::string("/sdcard/www/") + pathVal;
    std::string dirPath = filePath.substr(0, filePath.find_last_of('/'));
    {
        std::string tmp;
        for (size_t i = 0; i < dirPath.size(); i++) {
            tmp += dirPath[i];
            if (dirPath[i] == '/' && i > 0) {
                mkdir(tmp.c_str(), 0755);
            }
        }
        mkdir(dirPath.c_str(), 0755);
    }

    FILE *fp = fopen(filePath.c_str(), "w");
    if (!fp) {
        ESP_LOGE(TAG, "Cannot create file: %s", filePath.c_str());
        return send_error(req, 500, "Cannot create file");
    }

    char *chunk = (char *)heap_caps_malloc(CHUNK_BUF_SIZE, MALLOC_CAP_SPIRAM);
    if (!chunk) {
        fclose(fp);
        return send_error(req, 500, "Out of memory");
    }

    int remaining = req->content_len;
    int total_written = 0;
    while (remaining > 0) {
        int toRead = remaining > CHUNK_BUF_SIZE ? CHUNK_BUF_SIZE : remaining;
        int recv = httpd_req_recv(req, chunk, toRead);
        if (recv <= 0) {
            if (recv == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            ESP_LOGE(TAG, "Upload recv error");
            fclose(fp);
            heap_caps_free(chunk);
            remove(filePath.c_str());
            return send_error(req, 500, "Upload receive error");
        }
        fwrite(chunk, 1, recv, fp);
        remaining -= recv;
        total_written += recv;
    }

    fflush(fp);
    fclose(fp);
    heap_caps_free(chunk);

    ESP_LOGI(TAG, "Uploaded www file %s (%d bytes)", filePath.c_str(), total_written);

    char resp[256];
    snprintf(resp, sizeof(resp),
             "{\"ok\":true,\"path\":\"%s\",\"size\":%d}",
             pathVal, total_written);
    return send_json(req, resp);
}

// ─── POST /api/v2/samples?action=uploadsystem ────────────────
//
// Query params: ?path=firmware/pico-firmware.bin  (relative to /sdcard/system/)
// Body: raw file data (binary)
//
static esp_err_t handle_uploadsystem(httpd_req_t *req) {
    ESP_LOGI(TAG, "handle_uploadsystem, content_len=%d", req->content_len);

    size_t qlen = httpd_req_get_url_query_len(req);
    char query_buf[256] = {0};
    char pathVal[128] = {0};

    if (qlen > 0) {
        httpd_req_get_url_query_str(req, query_buf, sizeof(query_buf));
        char raw[128];
        if (httpd_query_key_value(query_buf, "path", raw, sizeof(raw)) == ESP_OK) {
            url_decode(pathVal, raw, sizeof(pathVal));
        } else {
            return send_error(req, 400, "Missing path parameter");
        }
    } else {
        return send_error(req, 400, "Missing query parameters");
    }

    // Path traversal protection
    if (strstr(pathVal, "..") != nullptr) {
        return send_error(req, 400, "Invalid path");
    }

    // Construct full path and ensure parent directories exist
    std::string filePath = std::string("/sdcard/system/") + pathVal;
    std::string dirPath = filePath.substr(0, filePath.find_last_of('/'));
    {
        std::string tmp;
        for (size_t i = 0; i < dirPath.size(); i++) {
            tmp += dirPath[i];
            if (dirPath[i] == '/' && i > 0) {
                mkdir(tmp.c_str(), 0755);
            }
        }
        mkdir(dirPath.c_str(), 0755);
    }

    FILE *fp = fopen(filePath.c_str(), "w");
    if (!fp) {
        ESP_LOGE(TAG, "Cannot create file: %s", filePath.c_str());
        return send_error(req, 500, "Cannot create file");
    }

    char *chunk = (char *)heap_caps_malloc(CHUNK_BUF_SIZE, MALLOC_CAP_SPIRAM);
    if (!chunk) {
        fclose(fp);
        return send_error(req, 500, "Out of memory");
    }

    int remaining = req->content_len;
    int total_written = 0;
    while (remaining > 0) {
        int toRead = remaining > CHUNK_BUF_SIZE ? CHUNK_BUF_SIZE : remaining;
        int recv = httpd_req_recv(req, chunk, toRead);
        if (recv <= 0) {
            if (recv == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            ESP_LOGE(TAG, "Upload recv error");
            fclose(fp);
            heap_caps_free(chunk);
            remove(filePath.c_str());
            return send_error(req, 500, "Upload receive error");
        }
        fwrite(chunk, 1, recv, fp);
        remaining -= recv;
        total_written += recv;
    }

    fflush(fp);
    fclose(fp);
    heap_caps_free(chunk);

    ESP_LOGI(TAG, "Uploaded system file %s (%d bytes)", filePath.c_str(), total_written);

    char resp[256];
    snprintf(resp, sizeof(resp),
             "{\"ok\":true,\"path\":\"%s\",\"size\":%d}",
             pathVal, total_written);
    return send_json(req, resp);
}

// ─── POST /api/v2/storage?action=uploadfactory ───────────────
//
// Query params: ?path=presets/mypreset.json  (relative to /sdcard/factory/)
// Body: raw file data (binary)
//
static esp_err_t handle_uploadfactory(httpd_req_t *req) {
    ESP_LOGI(TAG, "handle_uploadfactory, content_len=%d", req->content_len);

    size_t qlen = httpd_req_get_url_query_len(req);
    char query_buf[256] = {0};
    char pathVal[128] = {0};

    if (qlen > 0) {
        httpd_req_get_url_query_str(req, query_buf, sizeof(query_buf));
        char raw[128];
        if (httpd_query_key_value(query_buf, "path", raw, sizeof(raw)) == ESP_OK) {
            url_decode(pathVal, raw, sizeof(pathVal));
        } else {
            return send_error(req, 400, "Missing path parameter");
        }
    } else {
        return send_error(req, 400, "Missing query parameters");
    }

    // Path traversal protection
    if (strstr(pathVal, "..") != nullptr) {
        return send_error(req, 400, "Invalid path");
    }

    // Construct full path and ensure parent directories exist
    std::string filePath = std::string("/sdcard/factory/") + pathVal;
    std::string dirPath = filePath.substr(0, filePath.find_last_of('/'));
    {
        std::string tmp;
        for (size_t i = 0; i < dirPath.size(); i++) {
            tmp += dirPath[i];
            if (dirPath[i] == '/' && i > 0) {
                mkdir(tmp.c_str(), 0755);
            }
        }
        mkdir(dirPath.c_str(), 0755);
    }

    CTAG::STORAGE::lockStorage();
    FILE *fp = fopen(filePath.c_str(), "w");
    if (!fp) {
        CTAG::STORAGE::unlockStorage();
        ESP_LOGE(TAG, "Cannot create file: %s", filePath.c_str());
        return send_error(req, 500, "Cannot create file");
    }

    char *chunk = (char *)heap_caps_malloc(CHUNK_BUF_SIZE, MALLOC_CAP_SPIRAM);
    if (!chunk) {
        fclose(fp);
        CTAG::STORAGE::unlockStorage();
        return send_error(req, 500, "Out of memory");
    }

    int remaining = req->content_len;
    int total_written = 0;
    while (remaining > 0) {
        int toRead = remaining > CHUNK_BUF_SIZE ? CHUNK_BUF_SIZE : remaining;
        int recv = httpd_req_recv(req, chunk, toRead);
        if (recv <= 0) {
            if (recv == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            ESP_LOGE(TAG, "Upload recv error");
            fclose(fp);
            CTAG::STORAGE::unlockStorage();
            heap_caps_free(chunk);
            remove(filePath.c_str());
            return send_error(req, 500, "Upload receive error");
        }
        fwrite(chunk, 1, recv, fp);
        remaining -= recv;
        total_written += recv;
    }

    fflush(fp);
    fclose(fp);
    CTAG::STORAGE::unlockStorage();
    heap_caps_free(chunk);

    ESP_LOGI(TAG, "Uploaded factory file %s (%d bytes)", filePath.c_str(), total_written);

    char resp[256];
    snprintf(resp, sizeof(resp),
             "{\"ok\":true,\"path\":\"%s\",\"size\":%d}",
             pathVal, total_written);
    return send_json(req, resp);
}

// ─── POST /api/v2/samples?action=manage ──────────────────────
//
// Body: JSON with { "action": "rename"|"delete"|"saveKit"|"createKit"|"createFolder", ... }
//
static esp_err_t handle_manage(httpd_req_t *req) {
    ESP_LOGI(TAG, "samples_manage_handler");

    char *body = read_post_body(req);
    if (!body) {
        return send_error(req, 500, "Cannot read body");
    }

    Document doc;
    doc.Parse(body);
    heap_caps_free(body);

    if (doc.HasParseError() || !doc.IsObject() || !doc.HasMember("action")) {
        return send_error(req, 400, "Invalid JSON");
    }

    std::string action = doc["action"].GetString();
    ESP_LOGI(TAG, "manage action: %s", action.c_str());

    // ── rename ──
    if (action == "rename") {
        if (!doc.HasMember("path") || !doc.HasMember("oldName") || !doc.HasMember("newName")) {
            return send_error(req, 400, "Missing rename fields");
        }
        std::string oldPath = std::string(SD_ROOT) + "/" +
                              doc["path"].GetString() + "/" +
                              doc["oldName"].GetString();
        std::string newPath = std::string(SD_ROOT) + "/" +
                              doc["path"].GetString() + "/" +
                              doc["newName"].GetString();
        if (rename(oldPath.c_str(), newPath.c_str()) != 0) {
            ESP_LOGE(TAG, "Rename failed: %s -> %s", oldPath.c_str(), newPath.c_str());
            return send_error(req, 500, "Rename failed");
        }
        ESP_LOGI(TAG, "Renamed: %s -> %s", doc["oldName"].GetString(), doc["newName"].GetString());
        return send_ok(req);
    }

    // ── delete ──
    if (action == "delete") {
        if (!doc.HasMember("path") || !doc.HasMember("filename")) {
            return send_error(req, 400, "Missing delete fields");
        }
        std::string filePath = std::string(SD_ROOT) + "/" +
                               doc["path"].GetString() + "/" +
                               doc["filename"].GetString();
        if (remove(filePath.c_str()) != 0) {
            ESP_LOGE(TAG, "Delete failed: %s", filePath.c_str());
            return send_error(req, 500, "Delete failed");
        }
        ESP_LOGI(TAG, "Deleted: %s/%s", doc["path"].GetString(), doc["filename"].GetString());
        return send_ok(req);
    }

    // ── deleteconfig ──
    if (action == "deleteconfig") {
        if (!doc.HasMember("path")) {
            return send_error(req, 400, "Missing delete fields");
        }
        // Only delete from user dir (factory files are immutable)
        std::string deletePath(doc["path"].GetString());
        size_t slashPos = deletePath.find('/');
        std::string filePath;
        if (slashPos != std::string::npos) {
            std::string subdir = deletePath.substr(0, slashPos);
            std::string fname = deletePath.substr(slashPos + 1);
            filePath = CTAG::STORAGE::userFilePath(subdir.c_str(), fname);
        } else {
            filePath = CTAG::STORAGE::userFilePath(CTAG::STORAGE::DIR_CONFIG, deletePath);
        }
        CTAG::STORAGE::lockStorage();
        int rc = remove(filePath.c_str());
        CTAG::STORAGE::unlockStorage();
        if (rc != 0) {
            ESP_LOGE(TAG, "Delete failed: %s", filePath.c_str());
            return send_error(req, 500, "Delete failed");
        }
        ESP_LOGI(TAG, "Deleted config: %s", doc["path"].GetString());
        return send_ok(req);
    }

    // ── saveKit ──
    if (action == "saveKit") {
        if (!doc.HasMember("bankIndex") || !doc.HasMember("entries")) {
            return send_error(req, 400, "Missing saveKit fields");
        }
        int bankIdx = doc["bankIndex"].GetInt();

        // Load sample_rom to get the kit filename
        Document sampleRom;
        if (!load_json_file(resolveSampleRomFile().c_str(), sampleRom)) {
            return send_error(req, 500, "Cannot read sample_rom.json");
        }
        if (!sampleRom.HasMember("smp_banks") || !sampleRom["smp_banks"].IsArray() ||
            (uint32_t)bankIdx >= sampleRom["smp_banks"].GetArray().Size()) {
            return send_error(req, 400, "Invalid bank index");
        }

        // Write kit descriptor — always to /user/kits/ (overlay write)
        std::string kitFilename = sampleRom["smp_banks"][bankIdx].GetString();
        std::string kitFile = userKitPath(kitFilename);
        Document kitDoc;
        kitDoc.CopyFrom(doc["entries"], kitDoc.GetAllocator());
        store_json_file(kitFile.c_str(), kitDoc);
        ESP_LOGI(TAG, "Saved kit %d to %s", bankIdx, kitFile.c_str());

        // Save bank metadata if provided
        if (doc.HasMember("banksMeta") && doc["banksMeta"].IsArray()) {
            if (!sampleRom.HasMember("smp_bank_meta")) {
                Value meta(kArrayType);
                sampleRom.AddMember("smp_bank_meta", meta, sampleRom.GetAllocator());
            }
            auto &metaArr = sampleRom["smp_bank_meta"];
            while (metaArr.GetArray().Size() <= (uint32_t)bankIdx) {
                Value empty(kObjectType);
                metaArr.PushBack(empty, sampleRom.GetAllocator());
            }
            // Set banks metadata for this kit
            Value metaObj(kObjectType);
            Value banksMeta(doc["banksMeta"], sampleRom.GetAllocator());
            metaObj.AddMember("banks", banksMeta, sampleRom.GetAllocator());
            metaArr[bankIdx] = metaObj.Move();

            store_json_file(userSampleRomFile().c_str(), sampleRom);
            ESP_LOGI(TAG, "Saved bank metadata for kit %d", bankIdx);
        }

        CTAG::AUDIO::SoundProcessorManager::webuiChangeCounter++;
        return send_ok(req);
    }

    // ── createKit ──
    if (action == "createKit") {
        if (!doc.HasMember("name")) {
            return send_error(req, 400, "Missing kit name");
        }
        std::string name = doc["name"].GetString();

        // Generate safe filename
        std::string safeName;
        for (char c : name) {
            if (isalnum(c) || c == '_') safeName += tolower(c);
            else if (c == ' ' || c == '-') safeName += '_';
        }
        if (safeName.empty()) safeName = "unnamed";
        std::string filename = safeName + ".json";
        std::string fullPath = userKitPath(filename);

        // Write kit descriptor to /user/kits/
        Document kitDoc;
        if (doc.HasMember("entries") && doc["entries"].IsArray()) {
            kitDoc.CopyFrom(doc["entries"], kitDoc.GetAllocator());
        } else {
            kitDoc.SetArray();
        }
        store_json_file(fullPath.c_str(), kitDoc);

        // Update sample_rom.json
        Document sampleRom;
        if (!load_json_file(resolveSampleRomFile().c_str(), sampleRom)) {
            return send_error(req, 500, "Cannot read sample_rom.json");
        }

        // Add to smp_banks array
        if (sampleRom.HasMember("smp_banks") && sampleRom["smp_banks"].IsArray()) {
            Value fn(filename.c_str(), sampleRom.GetAllocator());
            sampleRom["smp_banks"].PushBack(fn, sampleRom.GetAllocator());
        }
        // Add to smp_bank_names
        if (sampleRom.HasMember("smp_bank_names") && sampleRom["smp_bank_names"].IsArray()) {
            Value nm(name.c_str(), sampleRom.GetAllocator());
            sampleRom["smp_bank_names"].PushBack(nm, sampleRom.GetAllocator());
        }
        // Add to smp_bank_tags
        if (sampleRom.HasMember("smp_bank_tags") && sampleRom["smp_bank_tags"].IsArray()) {
            Value tags(kArrayType);
            sampleRom["smp_bank_tags"].PushBack(tags, sampleRom.GetAllocator());
        }

        // Get new index
        uint32_t newIdx = sampleRom["smp_banks"].GetArray().Size() - 1;

        // Update active bank
        sampleRom["active_smp_bank"].SetUint(newIdx);
        store_json_file(userSampleRomFile().c_str(), sampleRom);

        ESP_LOGI(TAG, "Created kit '%s' -> %s (index %" PRIu32 ")", name.c_str(), filename.c_str(), newIdx);

        CTAG::AUDIO::SoundProcessorManager::webuiChangeCounter++;
        // Return with new kit index
        char resp[128];
        snprintf(resp, sizeof(resp), "{\"ok\":true,\"newKitIndex\":%" PRIu32 "}", newIdx);
        return send_json(req, resp);
    }

    // ── deleteKit ──
    if (action == "deleteKit") {
        if (!doc.HasMember("kitIndex")) {
            return send_error(req, 400, "Missing kitIndex");
        }
        int kitIdx = doc["kitIndex"].GetInt();

        // Load sample_rom.json via overlay
        Document sampleRom;
        if (!load_json_file(resolveSampleRomFile().c_str(), sampleRom)) {
            return send_error(req, 500, "Cannot read sample_rom.json");
        }
        if (!sampleRom.HasMember("smp_banks") || !sampleRom["smp_banks"].IsArray() ||
            (uint32_t)kitIdx >= sampleRom["smp_banks"].GetArray().Size()) {
            return send_error(req, 400, "Invalid kit index");
        }
        // Prevent deleting the last kit
        if (sampleRom["smp_banks"].GetArray().Size() <= 1) {
            return send_error(req, 400, "Cannot delete the last kit");
        }

        // Get kit filename and delete the .json file (only from user dir)
        std::string kitFilename = sampleRom["smp_banks"][kitIdx].GetString();
        std::string kitFile = resolveKitFile(kitFilename);
        // Only delete if it's a user kit (not factory)
        if (!isFactoryKit(kitFilename)) {
            remove(kitFile.c_str());
            ESP_LOGI(TAG, "Deleted user kit file: %s", kitFile.c_str());
        } else {
            ESP_LOGI(TAG, "Kit %s is factory — file preserved, removing from manifest only", kitFilename.c_str());
        }

        // Remove from smp_banks array
        auto &banks = sampleRom["smp_banks"];
        Value newBanks(kArrayType);
        for (uint32_t i = 0; i < banks.GetArray().Size(); i++) {
            if ((int)i != kitIdx) {
                Value v(banks[i], sampleRom.GetAllocator());
                newBanks.PushBack(v, sampleRom.GetAllocator());
            }
        }
        banks = newBanks.Move();

        // Remove from smp_bank_names
        if (sampleRom.HasMember("smp_bank_names") && sampleRom["smp_bank_names"].IsArray()) {
            auto &names = sampleRom["smp_bank_names"];
            Value newNames(kArrayType);
            for (uint32_t i = 0; i < names.GetArray().Size(); i++) {
                if ((int)i != kitIdx) {
                    Value v(names[i], sampleRom.GetAllocator());
                    newNames.PushBack(v, sampleRom.GetAllocator());
                }
            }
            names = newNames.Move();
        }

        // Remove from smp_bank_tags
        if (sampleRom.HasMember("smp_bank_tags") && sampleRom["smp_bank_tags"].IsArray()) {
            auto &tags = sampleRom["smp_bank_tags"];
            Value newTags(kArrayType);
            for (uint32_t i = 0; i < tags.GetArray().Size(); i++) {
                if ((int)i != kitIdx) {
                    Value v(tags[i], sampleRom.GetAllocator());
                    newTags.PushBack(v, sampleRom.GetAllocator());
                }
            }
            tags = newTags.Move();
        }

        // Remove from smp_bank_meta
        if (sampleRom.HasMember("smp_bank_meta") && sampleRom["smp_bank_meta"].IsArray()) {
            auto &meta = sampleRom["smp_bank_meta"];
            Value newMeta(kArrayType);
            for (uint32_t i = 0; i < meta.GetArray().Size(); i++) {
                if ((int)i != kitIdx) {
                    Value v(meta[i], sampleRom.GetAllocator());
                    newMeta.PushBack(v, sampleRom.GetAllocator());
                }
            }
            meta = newMeta.Move();
        }

        // Set active bank to 0
        sampleRom["active_smp_bank"].SetUint(0);
        store_json_file(userSampleRomFile().c_str(), sampleRom);

        ESP_LOGI(TAG, "Deleted kit at index %d", kitIdx);
        CTAG::AUDIO::SoundProcessorManager::webuiChangeCounter++;
        return send_ok(req);
    }

    // ── createFolder ──
    if (action == "createFolder") {
        if (!doc.HasMember("path")) {
            return send_error(req, 400, "Missing folder path");
        }
        std::string dirPath = std::string(SD_ROOT) + "/" + doc["path"].GetString();
        // Create recursively
        std::string tmp;
        for (size_t i = 0; i < dirPath.size(); i++) {
            tmp += dirPath[i];
            if (dirPath[i] == '/' && i > 0) {
                mkdir(tmp.c_str(), 0755);
            }
        }
        mkdir(dirPath.c_str(), 0755);
        ESP_LOGI(TAG, "Created folder: %s", doc["path"].GetString());
        return send_ok(req);
    }

    // ── renameFolder ──
    if (action == "renameFolder") {
        if (!doc.HasMember("oldPath") || !doc.HasMember("newPath")) {
            return send_error(req, 400, "Missing renameFolder fields");
        }
        std::string oldDir = std::string(SD_ROOT) + "/" + doc["oldPath"].GetString();
        std::string newDir = std::string(SD_ROOT) + "/" + doc["newPath"].GetString();

        // Verify old path exists and is a directory
        struct stat st;
        if (stat(oldDir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
            return send_error(req, 404, "Folder not found");
        }
        // Check new path doesn't already exist
        if (stat(newDir.c_str(), &st) == 0) {
            return send_error(req, 400, "Target folder already exists");
        }
        if (rename(oldDir.c_str(), newDir.c_str()) != 0) {
            ESP_LOGE(TAG, "Rename folder failed: %s -> %s", oldDir.c_str(), newDir.c_str());
            return send_error(req, 500, "Rename folder failed");
        }
        ESP_LOGI(TAG, "Renamed folder: %s -> %s", doc["oldPath"].GetString(), doc["newPath"].GetString());
        return send_ok(req);
    }

    // ── deleteFolder ──
    if (action == "deleteFolder") {
        if (!doc.HasMember("path")) {
            return send_error(req, 400, "Missing folder path");
        }
        std::string dirPath = std::string(SD_ROOT) + "/" + doc["path"].GetString();

        // Verify path exists and is a directory
        struct stat st;
        if (stat(dirPath.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
            return send_error(req, 404, "Folder not found");
        }

        // Recursively delete directory contents using BFS to collect,
        // then delete files first, then directories deepest-first.
        std::vector<std::string> allFiles;
        std::vector<std::string> allDirs;
        std::vector<std::string> queue;
        queue.push_back(dirPath);
        size_t qi = 0;
        while (qi < queue.size()) {
            std::string current = queue[qi++];
            allDirs.push_back(current);
            DIR *dir = opendir(current.c_str());
            if (!dir) continue;
            struct dirent *ent;
            while ((ent = readdir(dir)) != nullptr) {
                if (ent->d_name[0] == '.') continue;
                std::string child = current + "/" + ent->d_name;
                struct stat cst;
                if (stat(child.c_str(), &cst) == 0) {
                    if (S_ISDIR(cst.st_mode)) {
                        queue.push_back(child);
                    } else {
                        allFiles.push_back(child);
                    }
                }
            }
            closedir(dir);
        }
        // Delete all files first
        for (const auto &f : allFiles) {
            if (remove(f.c_str()) != 0) {
                ESP_LOGW(TAG, "Failed to remove file: %s", f.c_str());
            }
        }
        // Delete directories in reverse BFS order (deepest first)
        for (auto it = allDirs.rbegin(); it != allDirs.rend(); ++it) {
            if (rmdir(it->c_str()) != 0) {
                ESP_LOGW(TAG, "Failed to rmdir: %s (errno %d)", it->c_str(), errno);
            }
        }

        // Verify the target directory was actually removed
        struct stat post_st;
        if (stat(dirPath.c_str(), &post_st) == 0) {
            ESP_LOGE(TAG, "deleteFolder: directory still exists after deletion: %s", dirPath.c_str());
            return send_error(req, 500, "Failed to delete folder");
        }

        ESP_LOGI(TAG, "Deleted folder: %s", doc["path"].GetString());
        return send_ok(req);
    }

    // ── checkFileRefs — scan ALL kits for references to a file ──
    if (action == "checkFileRefs") {
        if (!doc.HasMember("path") || !doc.HasMember("filename")) {
            return send_error(req, 400, "Missing path or filename");
        }
        std::string targetPath = doc["path"].GetString();
        std::string targetFile = doc["filename"].GetString();

        Document sampleRom;
        if (!load_json_file(resolveSampleRomFile().c_str(), sampleRom)) {
            return send_error(req, 500, "Cannot read sample_rom.json");
        }

        Document resp(kObjectType);
        auto &alloc = resp.GetAllocator();
        Value refs(kArrayType);

        if (sampleRom.HasMember("smp_banks") && sampleRom["smp_banks"].IsArray()) {
            auto &banks = sampleRom["smp_banks"];
            bool hasNames = sampleRom.HasMember("smp_bank_names") &&
                            sampleRom["smp_bank_names"].IsArray();
            for (unsigned ki = 0; ki < banks.GetArray().Size(); ki++) {
                std::string kitFile = resolveKitFile(banks[ki].GetString());
                Document kitDoc;
                if (!load_json_file(kitFile.c_str(), kitDoc) || !kitDoc.IsArray()) continue;

                for (unsigned ei = 0; ei < kitDoc.GetArray().Size(); ei++) {
                    auto &entry = kitDoc[ei];
                    if (!entry.IsObject()) continue;
                    if (!entry.HasMember("filename") || !entry.HasMember("path")) continue;
                    if (std::string(entry["filename"].GetString()) == targetFile &&
                        std::string(entry["path"].GetString()) == targetPath) {
                        Value ref(kObjectType);
                        ref.AddMember("kitIndex", (unsigned)ki, alloc);
                        // Kit name
                        if (hasNames && ki < sampleRom["smp_bank_names"].GetArray().Size()) {
                            Value n(sampleRom["smp_bank_names"][ki], alloc);
                            ref.AddMember("kitName", n, alloc);
                        }
                        ref.AddMember("slotIndex", (unsigned)ei, alloc);
                        refs.PushBack(ref, alloc);
                    }
                }
            }
        }
        resp.AddMember("refs", refs, alloc);

        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        resp.Accept(writer);
        return send_json(req, sb.GetString());
    }

    ESP_LOGW(TAG, "Unknown action: %s", action.c_str());
    return send_error(req, 400, "Unknown action");
}

// ─── POST /api/v2/samples?action=reload ──────────────────────
//
// Triggers PSRAM reload from SD card.
// This is the "heavy" operation that mutes audio briefly.
//
static esp_err_t handle_reload(httpd_req_t *req) {
    ESP_LOGI(TAG, "samples_reload_handler — reloading PSRAM");

    // Read and discard body (if any)
    char discard[64];
    while (httpd_req_recv(req, discard, sizeof(discard)) > 0) {}

    // Disable plugin processing, refresh, re-enable
    CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
    CTAG::AUDIO::SoundProcessorManager::RefreshSampleRom();
    CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();

    ESP_LOGI(TAG, "PSRAM reload complete");
    return send_ok(req);
}

// ═══════════════════════════════════════════════════════════════════
//  UNIFIED STORAGE REST API
//
//  All storage, sample, and config operations under /api/v2/storage*
//
//  GET  (no query)                → bulk sample list + dirs + configs + kits
//  GET  ?preview=X                → stream WAV for audio preview
//  GET  ?getconfig=X              → overlay-resolved config file
//  GET  ?kit=N                    → switch active kit before listing
//  GET  ?action=info              → SD card stats
//  GET  ?action=list&path=X       → recursive file listing
//  GET  ?action=file&path=X       → raw file download
//
//  POST ?action=upload&path=X     → file upload (generic or sample WAV)
//  POST ?action=uploadconfig      → config file upload (user overlay)
//  POST ?action=uploadwww         → www file upload
//  POST ?action=uploadsystem      → system file upload
//  POST ?action=uploadfactory     → factory file upload
//  POST ?action=manage            → JSON body: rename/delete/kit ops
//  POST ?action=mkdir&path=X      → create directory
//  POST ?action=delete&path=X     → delete file/dir
//  POST ?action=copy&from=X&to=Y  → server-side copy
//  POST ?action=reload            → reload PSRAM from SD card
// ═══════════════════════════════════════════════════════════════════

// ─── Security: path validation ───────────────────────────────────

/**
 * Validate and sanitize a storage path.
 * Rejects path traversal (..), absolute paths outside /sdcard, and null bytes.
 * Returns the full absolute path on success, or empty string on failure.
 */
static std::string sanitize_storage_path(const char *relPath) {
    if (!relPath || relPath[0] == '\0') return "";

    std::string rel(relPath);

    // Reject null bytes
    if (rel.find('\0') != std::string::npos) return "";

    // Reject path traversal
    if (rel.find("..") != std::string::npos) return "";

    // Strip leading slash if present
    if (!rel.empty() && rel[0] == '/') rel = rel.substr(1);

    // Must start with a known zone
    if (rel.rfind("factory/", 0) != 0 &&
        rel.rfind("user/", 0) != 0 &&
        rel.rfind("system/", 0) != 0 &&
        rel.rfind("samples/", 0) != 0 &&
        rel.rfind("www/", 0) != 0 &&
        rel != "factory" && rel != "user" && rel != "system" &&
        rel != "samples" && rel != "www") {
        return "";
    }

    return std::string(SD_ROOT) + "/" + rel;
}

/**
 * Check if a path is in a read-only zone.
 * /factory/ and /www/ are immutable; /system/ is write-protected.
 */
static bool is_read_only_path(const std::string &absPath) {
    const std::string factory = std::string(SD_ROOT) + "/factory";
    const std::string www     = std::string(SD_ROOT) + "/www";
    const std::string system  = std::string(SD_ROOT) + "/system";
    return absPath.rfind(factory, 0) == 0 ||
           absPath.rfind(www, 0) == 0 ||
           absPath.rfind(system, 0) == 0;
}

// ─── Helpers ─────────────────────────────────────────────────────

/** Recursively compute the total size of all files under a directory. */
static uint64_t dir_total_size(const std::string &dirPath) {
    uint64_t total = 0;
    DIR *d = opendir(dirPath.c_str());
    if (!d) return 0;
    struct dirent *entry;
    while ((entry = readdir(d)) != nullptr) {
        if (entry->d_name[0] == '.') continue;
        std::string child = dirPath + "/" + entry->d_name;
        struct stat st;
        if (stat(child.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                total += dir_total_size(child);
            } else {
                total += st.st_size;
            }
        }
    }
    closedir(d);
    return total;
}

/** Recursively list files under a directory, writing JSON array entries. */
static void list_files_recursive(const std::string &baseDir, const std::string &relPrefix,
                                  Writer<StringBuffer> &writer, int depth) {
    if (depth > 16) return; // safety limit
    DIR *d = opendir(baseDir.c_str());
    if (!d) return;
    struct dirent *entry;
    while ((entry = readdir(d)) != nullptr) {
        if (entry->d_name[0] == '.') continue;
        std::string absChild = baseDir + "/" + entry->d_name;
        std::string relChild = relPrefix.empty()
            ? std::string(entry->d_name)
            : relPrefix + "/" + entry->d_name;
        struct stat st;
        if (stat(absChild.c_str(), &st) != 0) continue;

        writer.StartObject();
        writer.Key("path"); writer.String(relChild.c_str());
        writer.Key("size"); writer.Uint64(S_ISDIR(st.st_mode) ? 0 : st.st_size);
        writer.Key("dir");  writer.Bool(S_ISDIR(st.st_mode));
        writer.Key("mtime"); writer.Int64((int64_t)st.st_mtime);
        writer.EndObject();

        if (S_ISDIR(st.st_mode)) {
            list_files_recursive(absChild, relChild, writer, depth + 1);
        }
    }
    closedir(d);
}

/** Recursively delete a directory and all contents. Max depth 16. */
static bool delete_recursive(const std::string &path, int depth = 0) {
    if (depth > 16) return false;
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return false;

    if (!S_ISDIR(st.st_mode)) {
        return remove(path.c_str()) == 0;
    }

    DIR *d = opendir(path.c_str());
    if (!d) return false;
    bool ok = true;
    struct dirent *entry;
    while ((entry = readdir(d)) != nullptr) {
        if (entry->d_name[0] == '.') continue;
        std::string child = path + "/" + entry->d_name;
        if (!delete_recursive(child, depth + 1)) ok = false;
    }
    closedir(d);
    if (rmdir(path.c_str()) != 0) ok = false;
    return ok;
}

/** Recursively copy a directory tree. */
static bool copy_recursive(const std::string &src, const std::string &dst, int depth = 0) {
    if (depth > 16) return false;
    struct stat st;
    if (stat(src.c_str(), &st) != 0) return false;

    if (!S_ISDIR(st.st_mode)) {
        // Regular file — copy contents
        return CTAG::STORAGE::copyFile(src, dst);
    }

    // Directory — create and recurse
    mkdir(dst.c_str(), 0755);
    DIR *d = opendir(src.c_str());
    if (!d) return false;
    bool ok = true;
    struct dirent *entry;
    while ((entry = readdir(d)) != nullptr) {
        if (entry->d_name[0] == '.') continue;
        if (!copy_recursive(src + "/" + entry->d_name, dst + "/" + entry->d_name, depth + 1))
            ok = false;
    }
    closedir(d);
    return ok;
}

/** Ensure parent directory exists (create intermediate dirs). */
static void ensure_parent_dir(const std::string &filePath) {
    for (size_t pos = 1; pos < filePath.size(); pos++) {
        if (filePath[pos] == '/') {
            std::string sub = filePath.substr(0, pos);
            struct stat st;
            if (stat(sub.c_str(), &st) != 0) {
                mkdir(sub.c_str(), 0755);
            }
        }
    }
}

// ─── GET /api/v2/storage ─────────────────────────────────────────

esp_err_t StorageAPI::storage_get_handler(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Connection", "close");

    size_t qlen = httpd_req_get_url_query_len(req);

    // No query params → legacy bulk sample listing
    if (qlen == 0) return handle_list(req);

    // Extract action= using heap to keep stack clean for handle_list()
    char action[32] = {0};
    {
        char *qbuf = (char *)malloc(qlen + 1);
        if (qbuf) {
            httpd_req_get_url_query_str(req, qbuf, qlen + 1);
            httpd_query_key_value(qbuf, "action", action, sizeof(action));
            free(qbuf);
        }
    }

    // No action= param → delegate to legacy handler (handles preview, getconfig, kit)
    if (action[0] == '\0') return handle_list(req);

    // Action branches below don't call handle_list — use heap for query buffer
    char *query = (char *)malloc(qlen + 1);
    if (!query) return send_error(req, 500, "Out of memory");
    httpd_req_get_url_query_str(req, query, qlen + 1);

    // ── action=info ──
    if (strcmp(action, "info") == 0) {
        free(query); // not needed for info
        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        writer.StartObject();

#if CONFIG_TBD_USE_SD_CARD
        uint64_t sdTotal = 0, sdFree = 0;
        if (esp_vfs_fat_info(SD_ROOT, &sdTotal, &sdFree) == ESP_OK) {
            writer.Key("total"); writer.Double((double)sdTotal);
            writer.Key("free");  writer.Double((double)sdFree);
        }

        // Per-zone breakdown
        writer.Key("zones"); writer.StartObject();
        const char *zones[] = {"factory", "user", "system", "samples", "www", nullptr};
        for (int i = 0; zones[i]; i++) {
            std::string zoneDir = std::string(SD_ROOT) + "/" + zones[i];
            if (CTAG::STORAGE::isDirectory(zoneDir)) {
                writer.Key(zones[i]); writer.Double((double)dir_total_size(zoneDir));
            }
        }
        writer.EndObject(); // zones
#endif
        writer.EndObject();

        httpd_resp_set_type(req, "application/json");
        return httpd_resp_sendstr(req, sb.GetString());
    }

    // ── action=list ──
    if (strcmp(action, "list") == 0) {
        char pathVal[256] = {0};
        httpd_query_key_value(query, "path", pathVal, sizeof(pathVal));
        free(query);

        std::string absPath = sanitize_storage_path(pathVal);
        if (absPath.empty()) return send_error(req, 400, "Invalid path");
        if (!CTAG::STORAGE::isDirectory(absPath))
            return send_error(req, 404, "Directory not found");

        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        writer.StartObject();
        writer.Key("path"); writer.String(pathVal);
        writer.Key("files"); writer.StartArray();
        list_files_recursive(absPath, "", writer, 0);
        writer.EndArray();
        writer.EndObject();

        httpd_resp_set_type(req, "application/json");
        return httpd_resp_sendstr(req, sb.GetString());
    }

    // ── action=file (download) ──
    if (strcmp(action, "file") == 0) {
        char pathVal[256] = {0};
        httpd_query_key_value(query, "path", pathVal, sizeof(pathVal));
        free(query);

        std::string absPath = sanitize_storage_path(pathVal);
        if (absPath.empty()) return send_error(req, 400, "Invalid path");

        struct stat st;
        if (stat(absPath.c_str(), &st) != 0 || S_ISDIR(st.st_mode))
            return send_error(req, 404, "File not found");

        FILE *f = fopen(absPath.c_str(), "rb");
        if (!f) return send_error(req, 500, "Cannot open file");

        // Determine content type
        const char *ct = "application/octet-stream";
        if (absPath.size() > 5 && absPath.substr(absPath.size() - 5) == ".json") ct = "application/json";
        else if (absPath.size() > 4 && absPath.substr(absPath.size() - 4) == ".txt") ct = "text/plain";
        else if (absPath.size() > 4 && absPath.substr(absPath.size() - 4) == ".wav") ct = "audio/wav";
        httpd_resp_set_type(req, ct);

        // Stream file in chunks (SPIRAM to avoid stack overflow)
        char *buf = (char *)heap_caps_malloc(CHUNK_BUF_SIZE, MALLOC_CAP_SPIRAM);
        if (!buf) {
            fclose(f);
            return send_error(req, 500, "Out of memory");
        }
        size_t n;
        while ((n = fread(buf, 1, CHUNK_BUF_SIZE, f)) > 0) {
            if (httpd_resp_send_chunk(req, buf, n) != ESP_OK) {
                fclose(f);
                heap_caps_free(buf);
                httpd_resp_send_chunk(req, nullptr, 0);
                return ESP_FAIL;
            }
        }
        fclose(f);
        heap_caps_free(buf);
        httpd_resp_send_chunk(req, nullptr, 0); // end chunked response
        return ESP_OK;
    }

    free(query);
    return send_error(req, 400, "Unknown action");
}

// ─── POST /api/v2/storage ────────────────────────────────────────

esp_err_t StorageAPI::storage_post_handler(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Connection", "close");

    size_t qlen = httpd_req_get_url_query_len(req);
    if (qlen == 0) return send_error(req, 400, "Missing query parameters");

    // Use heap for query buffer to keep stack clean (8 KB httpd limit)
    char *query = (char *)malloc(qlen + 1);
    if (!query) return send_error(req, 500, "Out of memory");
    httpd_req_get_url_query_str(req, query, qlen + 1);

    char action[32] = {0};
    httpd_query_key_value(query, "action", action, sizeof(action));

    // ── action=upload ──
    if (strcmp(action, "upload") == 0) {
        // Legacy sample upload: ?action=upload&path=X&filename=Y (WAV file to /sdcard/samples/)
        char filenameCheck[16] = {0};
        httpd_query_key_value(query, "filename", filenameCheck, sizeof(filenameCheck));
        if (filenameCheck[0] != '\0') {
            free(query);
            return handle_upload(req);
        }

        // Generic storage upload: ?action=upload&path=X (raw file to /sdcard/<path>)
        char pathVal[256] = {0};
        httpd_query_key_value(query, "path", pathVal, sizeof(pathVal));
        free(query);

        std::string absPath = sanitize_storage_path(pathVal);
        if (absPath.empty()) return send_error(req, 400, "Invalid path");
        if (is_read_only_path(absPath)) return send_error(req, 403, "Path is read-only");

        // Reject files larger than 50 MB
        if (req->content_len > 50 * 1024 * 1024) return send_error(req, 413, "File too large (50 MB max)");

        ESP_LOGI(TAG, "storage upload: %s (%d bytes)", pathVal, req->content_len);

        CTAG::STORAGE::lockStorage();

        ensure_parent_dir(absPath);

        std::string tmpPath = absPath + ".tmp";
        FILE *f = fopen(tmpPath.c_str(), "wb");
        if (!f) {
            CTAG::STORAGE::unlockStorage();
            return send_error(req, 500, "Cannot create file");
        }

        // Receive body in chunks (SPIRAM to avoid stack overflow)
        int remaining = req->content_len;
        char *buf = (char *)heap_caps_malloc(CHUNK_BUF_SIZE, MALLOC_CAP_SPIRAM);
        if (!buf) {
            fclose(f);
            CTAG::STORAGE::unlockStorage();
            return send_error(req, 500, "Out of memory");
        }
        bool ok = true;
        while (remaining > 0) {
            int toRead = remaining > (int)CHUNK_BUF_SIZE ? (int)CHUNK_BUF_SIZE : remaining;
            int received = httpd_req_recv(req, buf, toRead);
            if (received <= 0) { ok = false; break; }
            if ((int)fwrite(buf, 1, received, f) != received) { ok = false; break; }
            remaining -= received;
        }
        fclose(f);
        heap_caps_free(buf);

        if (!ok) {
            remove(tmpPath.c_str());
            CTAG::STORAGE::unlockStorage();
            return send_error(req, 500, "Upload failed");
        }

        // Atomic rename
        remove(absPath.c_str());
        if (rename(tmpPath.c_str(), absPath.c_str()) != 0) {
            remove(tmpPath.c_str());
            CTAG::STORAGE::unlockStorage();
            return send_error(req, 500, "Failed to finalize file");
        }

        CTAG::STORAGE::unlockStorage();
        return send_ok(req);
    }

    // ── action=mkdir ──
    if (strcmp(action, "mkdir") == 0) {
        char pathVal[256] = {0};
        httpd_query_key_value(query, "path", pathVal, sizeof(pathVal));
        free(query);

        std::string absPath = sanitize_storage_path(pathVal);
        if (absPath.empty()) return send_error(req, 400, "Invalid path");
        if (is_read_only_path(absPath)) return send_error(req, 403, "Path is read-only");

        CTAG::STORAGE::lockStorage();
        ensure_parent_dir(absPath);
        int rc = mkdir(absPath.c_str(), 0755);
        CTAG::STORAGE::unlockStorage();

        if (rc != 0 && errno != EEXIST) return send_error(req, 500, "Failed to create directory");
        return send_ok(req);
    }

    // ── action=delete ──
    if (strcmp(action, "delete") == 0) {
        char pathVal[256] = {0};
        httpd_query_key_value(query, "path", pathVal, sizeof(pathVal));
        free(query);

        std::string absPath = sanitize_storage_path(pathVal);
        if (absPath.empty()) return send_error(req, 400, "Invalid path");
        if (is_read_only_path(absPath)) return send_error(req, 403, "Path is read-only");

        ESP_LOGI(TAG, "storage delete: %s", pathVal);

        CTAG::STORAGE::lockStorage();
        bool ok = delete_recursive(absPath);
        CTAG::STORAGE::unlockStorage();

        if (!ok) return send_error(req, 500, "Delete failed");
        return send_ok(req);
    }

    // ── action=copy ──
    if (strcmp(action, "copy") == 0) {
        char fromVal[256] = {0}, toVal[256] = {0};
        httpd_query_key_value(query, "from", fromVal, sizeof(fromVal));
        httpd_query_key_value(query, "to", toVal, sizeof(toVal));
        free(query);

        std::string srcPath = sanitize_storage_path(fromVal);
        std::string dstPath = sanitize_storage_path(toVal);
        if (srcPath.empty() || dstPath.empty()) return send_error(req, 400, "Invalid path");
        if (is_read_only_path(dstPath)) return send_error(req, 403, "Destination is read-only");

        ESP_LOGI(TAG, "storage copy: %s -> %s", fromVal, toVal);

        CTAG::STORAGE::lockStorage();
        ensure_parent_dir(dstPath);
        bool ok = copy_recursive(srcPath, dstPath);
        CTAG::STORAGE::unlockStorage();

        if (!ok) return send_error(req, 500, "Copy failed");
        return send_ok(req);
    }

    // ── action=reload ──
    if (strcmp(action, "reload") == 0) {
        free(query);
        return handle_reload(req);
    }

    // ── Legacy sample actions (merged from /api/v2/samples) ──
    if (strcmp(action, "uploadconfig") == 0) {
        free(query);
        return handle_uploadconfig(req);
    }
    if (strcmp(action, "uploadwww") == 0) {
        free(query);
        return handle_uploadwww(req);
    }
    if (strcmp(action, "uploadsystem") == 0) {
        free(query);
        return handle_uploadsystem(req);
    }
    if (strcmp(action, "uploadfactory") == 0) {
        free(query);
        return handle_uploadfactory(req);
    }
    if (strcmp(action, "manage") == 0) {
        free(query);
        return handle_manage(req);
    }

    free(query);
    return send_error(req, 400, "Unknown action");
}
