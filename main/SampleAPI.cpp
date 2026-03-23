/***************
dadamachines TBD-16 — Sample Manager REST API

(c) 2014-2026 Johannes Elias Lohbihler for dadamachines.

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

Part of the dadamachines additions to the CTAG TBD platform.
See LICENSE in the repository root for full terms.

Provided "as is" without any express or implied warranties.
***************/

#include "SampleAPI.hpp"
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

using namespace CTAG::REST;
using namespace rapidjson;

static const char *TAG = "SampleAPI";

// SD card paths
static const char *SAMPLE_ROOT = "/sdcard/tbdsamples";
static const char *SAMPLE_ROM_FILE = "/sdcard/tbdsamples/sample_rom.json";
static const char *CONFIG_ROOT = "/sdcard/data";

// PSRAM capacity constant (~28 MB)
static const uint32_t PSRAM_MAX_BYTES = 29360128;

// Scratch buffer for chunked file reads (allocated from SPIRAM)
#define CHUNK_BUF_SIZE 4096

// ─── Helpers ─────────────────────────────────────────────────

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
    char readBuf[1024];
    FileReadStream is(fp, readBuf, sizeof(readBuf));
    doc.ParseStream(is);
    fclose(fp);
    return !doc.HasParseError();
}

/** Store a JSON Document to a file on SD card */
static bool store_json_file(const char *path, Document &doc) {
    FILE *fp = fopen(path, "w");
    if (!fp) {
        ESP_LOGE(TAG, "Cannot write %s", path);
        return false;
    }
    char writeBuf[1024];
    FileWriteStream os(fp, writeBuf, sizeof(writeBuf));
    Writer<FileWriteStream> writer(os);
    doc.Accept(writer);
    fflush(fp);
    fclose(fp);
    return true;
}

/**
 * Recursively scan a directory for .wav files.
 * Appends to the JSON array `files` in the given allocator.
 * `rel` is the relative path from SAMPLE_ROOT.
 */
static void scan_wav_files(const char *base, const char *rel,
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
            scan_wav_files(base, relPath.c_str(), files, alloc);
        } else if (S_ISREG(st.st_mode)) {
            // Check if .wav extension
            size_t len = strlen(ent->d_name);
            if (len > 4 &&
                (strcasecmp(ent->d_name + len - 4, ".wav") == 0)) {
                // Extract name without extension
                std::string stem(ent->d_name, len - 4);
                // Extract folder path (relative)
                std::string folder;
                if (rel && rel[0]) {
                    folder = rel;
                }

                Value fileObj(kObjectType);
                Value nameVal(stem.c_str(), alloc);
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
            // val contains path like "drums/factory/BD0"
            char decoded[256];
            url_decode(decoded, val, sizeof(decoded));
            std::string wavPath = std::string(SAMPLE_ROOT) + "/" + decoded + ".wav";

            // Try .wav then .WAV
            FILE *fp = fopen(wavPath.c_str(), "r");
            if (!fp) {
                wavPath = std::string(SAMPLE_ROOT) + "/" + decoded + ".WAV";
                fp = fopen(wavPath.c_str(), "r");
            }
            if (!fp) {
                return send_error(req, 404, "Preview not found");
            }

            // Get file size
            fseek(fp, 0, SEEK_END);
            long fsize = ftell(fp);
            fseek(fp, 0, SEEK_SET);

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


        // Handle preview request
        if (httpd_query_key_value(query, "getconfig", val, sizeof(val)) == ESP_OK) {
            free(query);
            // val contains path like "drums/factory/BD0"
            char decoded[256];
            url_decode(decoded, val, sizeof(decoded));
            std::string jsonPath = std::string(CONFIG_ROOT) + "/" + decoded;

            // Try .wav then .WAV
            FILE *fp = fopen(jsonPath.c_str(), "r");
            if (!fp) {
                return send_error(req, 404, "Config file not found");
            }

            // Get file size
            fseek(fp, 0, SEEK_END);
            long fsize = ftell(fp);
            fseek(fp, 0, SEEK_SET);

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

    // Load sample_rom.json
    Document sampleRom;
    if (!load_json_file(SAMPLE_ROM_FILE, sampleRom)) {
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

    // 1) Scan WAV files
    Value files(kArrayType);
    scan_wav_files(SAMPLE_ROOT, "", files, alloc);
    resp.AddMember("files", files, alloc);

    // 1b) Scan all directories (including empty ones)
    Value dirs(kArrayType);
    scan_directories(SAMPLE_ROOT, "", dirs, alloc);
    resp.AddMember("directories", dirs, alloc);

    // 1b) Scan all directories (including empty ones)
    // Value dirs2(kArrayType);
    // scan_directories(ROOT, "", dirs2, alloc);
    // resp.AddMember("rootdirectories", dirs2, alloc);

    Value files2(kArrayType);
    scan_json_files(CONFIG_ROOT, "", files2, alloc);
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
            std::string kitFile = std::string(SAMPLE_ROOT) + "/" +
                                  banks[activeKit].GetString();
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
    std::string dirPath = std::string(SAMPLE_ROOT) + "/" + pathVal;
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

    // Ensure target directory exists
    // std::string dirPath = std::string(CONFIG_ROOT) + "/" + pathVal;
    // // Create directories recursively (simple approach)
    // {
    //     std::string tmp;
    //     for (size_t i = 0; i < dirPath.size(); i++) {
    //         tmp += dirPath[i];
    //         if (dirPath[i] == '/' && i > 0) {
    //             mkdir(tmp.c_str(), 0755);
    //         }
    //     }
    //     mkdir(dirPath.c_str(), 0755);
    // }

    // Write configfile
    std::string filePath = std::string(CONFIG_ROOT) + "/" + pathVal;
    // std::string filePath = dirPath;
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
        std::string oldPath = std::string(SAMPLE_ROOT) + "/" +
                              doc["path"].GetString() + "/" +
                              doc["oldName"].GetString() + ".wav";
        std::string newPath = std::string(SAMPLE_ROOT) + "/" +
                              doc["path"].GetString() + "/" +
                              doc["newName"].GetString() + ".wav";
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
        std::string filePath = std::string(SAMPLE_ROOT) + "/" +
                               doc["path"].GetString() + "/" +
                               doc["filename"].GetString() + ".wav";
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
        std::string filePath = std::string(CONFIG_ROOT) + "/" +
                               doc["path"].GetString();
        if (remove(filePath.c_str()) != 0) {
            ESP_LOGE(TAG, "Delete failed: %s", filePath.c_str());
            return send_error(req, 500, "Delete failed");
        }
        ESP_LOGI(TAG, "Deleted: %s", doc["path"].GetString());
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
        if (!load_json_file(SAMPLE_ROM_FILE, sampleRom)) {
            return send_error(req, 500, "Cannot read sample_rom.json");
        }
        if (!sampleRom.HasMember("smp_banks") || !sampleRom["smp_banks"].IsArray() ||
            (uint32_t)bankIdx >= sampleRom["smp_banks"].GetArray().Size()) {
            return send_error(req, 400, "Invalid bank index");
        }

        // Write kit descriptor
        std::string kitFile = std::string(SAMPLE_ROOT) + "/" +
                              sampleRom["smp_banks"][bankIdx].GetString();
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

            store_json_file(SAMPLE_ROM_FILE, sampleRom);
            ESP_LOGI(TAG, "Saved bank metadata for kit %d", bankIdx);
        }

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
        std::string fullPath = std::string(SAMPLE_ROOT) + "/" + filename;

        // Write kit descriptor
        Document kitDoc;
        if (doc.HasMember("entries") && doc["entries"].IsArray()) {
            kitDoc.CopyFrom(doc["entries"], kitDoc.GetAllocator());
        } else {
            kitDoc.SetArray();
        }
        store_json_file(fullPath.c_str(), kitDoc);

        // Update sample_rom.json
        Document sampleRom;
        if (!load_json_file(SAMPLE_ROM_FILE, sampleRom)) {
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
        store_json_file(SAMPLE_ROM_FILE, sampleRom);

        ESP_LOGI(TAG, "Created kit '%s' -> %s (index %" PRIu32 ")", name.c_str(), filename.c_str(), newIdx);

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

        // Load sample_rom.json
        Document sampleRom;
        if (!load_json_file(SAMPLE_ROM_FILE, sampleRom)) {
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

        // Get kit filename and delete the .json file
        std::string kitFile = std::string(SAMPLE_ROOT) + "/" +
                              sampleRom["smp_banks"][kitIdx].GetString();
        remove(kitFile.c_str());
        ESP_LOGI(TAG, "Deleted kit file: %s", kitFile.c_str());

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
        store_json_file(SAMPLE_ROM_FILE, sampleRom);

        ESP_LOGI(TAG, "Deleted kit at index %d", kitIdx);
        return send_ok(req);
    }

    // ── createFolder ──
    if (action == "createFolder") {
        if (!doc.HasMember("path")) {
            return send_error(req, 400, "Missing folder path");
        }
        std::string dirPath = std::string(SAMPLE_ROOT) + "/" + doc["path"].GetString();
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
        std::string oldDir = std::string(SAMPLE_ROOT) + "/" + doc["oldPath"].GetString();
        std::string newDir = std::string(SAMPLE_ROOT) + "/" + doc["newPath"].GetString();

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
        std::string dirPath = std::string(SAMPLE_ROOT) + "/" + doc["path"].GetString();

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
        if (!load_json_file(SAMPLE_ROM_FILE, sampleRom)) {
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
                std::string kitFile = std::string(SAMPLE_ROOT) + "/" +
                                      banks[ki].GetString();
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

// ─── Public dispatch handlers ────────────────────────────────

esp_err_t SampleAPI::samples_get_handler(httpd_req_t *req) {
    return handle_list(req);
}

esp_err_t SampleAPI::samples_post_handler(httpd_req_t *req) {
    // Dispatch via ?action= query string
    size_t qlen = httpd_req_get_url_query_len(req);
    char action[32] = {0};

    if (qlen > 0) {
        char *query = (char *)malloc(qlen + 1);
        httpd_req_get_url_query_str(req, query, qlen + 1);
        httpd_query_key_value(query, "action", action, sizeof(action));
        free(query);
    }

    if (strcmp(action, "upload") == 0) {
        return handle_upload(req);
    } else if (strcmp(action, "uploadconfig") == 0) {
        return handle_uploadconfig(req);
    } else if (strcmp(action, "uploadwww") == 0) {
        return handle_uploadwww(req);
    } else if (strcmp(action, "manage") == 0) {
        return handle_manage(req);
    } else if (strcmp(action, "reload") == 0) {
        return handle_reload(req);
    }

    ESP_LOGW(TAG, "Unknown POST action: %s", action);
    return send_error(req, 400, "Unknown action parameter");
}
