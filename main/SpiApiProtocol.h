/***************
TBD-16 — Macro/Preset System & PicoSeqRack

(c) 2025-2026 Per-Olov Jernberg (possan). https://possan.codes

Licensed under the GNU Lesser General Public License (LGPL 3.0).
https://www.gnu.org/licenses/lgpl-3.0.txt

dadamachines has a commercial license to use this code in the TBD-16 product.
Other commercial use requires a separate license agreement.

Provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#pragma once

// #include "sdkconfig.h"
// #if CONFIG_TBD_USE_RP2350

#include <stdint.h>
#include <MacroDeviceDefinition.hpp>

//
// More commands
//

// machine ids
// none = \0\0\0\0
// machineId == "ab"; // AB\0\0
// machineId == "as"; // AS\0\0
// machineId == "cl"; // CL\0\0
// machineId == "ds"; // DS\0\0
// machineId == "fmb"; // FMB\0
// machineId == "hh1"; // HH1\0
// machineId == "hh2"; // HH2\0
// machineId == "mo"; // MO\0\0
// machineId == "pp"; // PP\0\0
// machineId == "ro"; // RO\0\0
// machineId == "rs"; // RS\0\0
// machineId == "tbd"; // DING
// machineId == "tbdait"; // DAIT
// machineId == "td3"; // TD3\0
// machineId == "wtosc"; // WTO\0
// REV1
// DEL1
// CMP1
// "hh"

// struct SynthDefinitionsHeader {
//     uint8_t num_track_definitions;
//     uint8_t num_machines;
// };

//
// Sample kits
//

enum KitType : uint8_t {
    KitType_Wavetable = 0,
    KitType_Sample = 1,
};

struct GetKitIndexPageRequest {
    uint8_t offset;
    enum KitType type;
};

struct GetKitIndexPageResponse {
    uint8_t offset;
    enum KitType type;
    uint8_t totalKits;
    uint8_t returnedKits;
    struct {
        char id[16];
        char name[16];
        struct {
            char name[16];
        } bank[32];
        struct {
            char name[16];
        } tags[8];
    } kits[2];
};

struct ApplyKitRequest {
    char id[16];
    enum KitType type;
};

// struct GetKitRequest {
//     char id[16];
// };

// struct GetKitResponse {
//     uint8_t totalBanks;
//     struct {
//         char name[16];
//     } bank[32];
// };

//
// Samples within kits
//

struct GetSampleBankIndexRequest {
    uint8_t bankIndex;
};

struct GetSampleBankIndexResponse {
    uint8_t bankIndex;
    uint16_t totalSamples;
    uint16_t returnedSamples;
    struct {
        char id[16];
        char displayName[16];
        uint32_t sampleCount;
    } samples[32];
};

struct GetSampleInfoAndPreviewRequest {
    uint8_t bankIndex;
    uint8_t sliceIndex;
};

struct GetSampleInfoAndPreviewResponse {
    uint8_t bankIndex;
    uint8_t sliceIndex;
    char id[16];
    char displayName[16];
    char path[64]; // maybe not needed?
    uint32_t fileSize;
    uint32_t sampleCount;
    int8_t preview[1024];
};

//
// File access
//

struct FileQueryRequest {
    char path[128];
};

struct FileQueryResponse {
    uint32_t size;
};

// struct __p4_spi_request__file_read_open_ {
//     char path[128];
// };

// struct __p4_spi_response__file_read_open_ {
//     char path[128];
//     uint32_t offset;
//     bool successful;
//     uint32_t size;
// };

struct FileReadBlockRequest {
    char path[128];
    uint32_t offset;
    uint32_t totalSize;
};

struct FileReadBlockResponse {
    char path[128];
    uint32_t offset;
    uint32_t totalSize;
    uint16_t dataLength;
    uint8_t data[1600];
};

// struct __p4_spi_request___file_read_close_ {
//     char path[128];
// };

// struct __p4_spi_response___file_read_close_ {
//     char path[128];
//     uint32_t totalSize;
// };


// struct __p4_spi_request__file_write_open_ {
//     char path[128];
//     uint32_t expectedSize;
// };

// struct __p4_spi_response__file_write_open_ {
//     char path[128];
//     bool successful;
// };

struct FileWriteBlockRequest {
    char path[128];
    uint32_t offset;
    uint32_t totalSize;
    uint16_t dataLength;
    uint8_t data[1600];
};

struct FileWriteBlockResponse {
    char path[128];
    uint32_t offset;
    uint32_t totalSize;
    uint16_t bytesWritten;
};

// struct __p4_spi_request___file_write_close_ {
//     char path[128];
// };

// struct __p4_spi_response___file_write_close_ {
//     char path[128];
//     uint32_t totalSize;
// };

struct FileDeleteRequest {
    char path[128];
};

struct FileDeleteResponse {
    char path[128];
    bool successful;
};

struct FileListPageRequest {
    uint16_t offset;
    char parent[128];
    char extension[8];
};

struct FileListPageResponse {
    uint16_t offset;
    char parent[128];
    char extension[8];
    uint16_t totalFiles;
    uint16_t returnedFiles;
    struct {
        char name[64];
        bool isDirectory;
        uint32_t size;
    } files[8];
};

//
// Projects
//



//
// Misc.
//

struct GetFirmwareInfoResponse {
    char hardwareVersion[16];
    char firmwareVersion[16];
    char firmwareName[16];
    char activeOTAPartition[16];
};









// #endif // CONFIG_TBD_USE_RP2350
