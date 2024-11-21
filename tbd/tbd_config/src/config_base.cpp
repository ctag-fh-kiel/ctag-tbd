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
#include <tbd/config_base.hpp>

#include "rapidjson/filewritestream.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/filereadstream.h"
#include <cstdio>
#include <regex>
#include <tbd/heaps.hpp>


#define MB_BUF_SZ 4096

namespace heaps = tbd::heaps;

namespace tbd::config {

void ConfigBase::loadJSON(rapidjson::Document &d, const std::string &fn) {
    d.GetAllocator().Clear();
    // TBD_LOGD("JSON", "read buffer");
    //FILE*
    fp = fopen(fn.c_str(), "r");
    if (fp == NULL) {
        // TBD_LOGE("JSON", "could not open file %s", fn.c_str());
        return;
    }
    //char readBuffer[512];
//    TBD_LOGE("JSON", "read stream");
    rapidjson::FileReadStream is(fp, buffer, MB_BUF_SZ);
//    TBD_LOGE("JSON", "trying to parse");
    d.ParseStream(is);
    fclose(fp);

    // error checks and reading backup if error
    // THIS IS AN IMPORTANT BUG FIX
    // WHEN ESP IS LOOSING POWER DURING FLASH WRITE, IMPORTANT JSON FILES CAN GET CORRUPTED
    // IF THIS HAPPENS DATA IS PULLED FROM THE INITIAL BACKUP FILE STRUCTURE
    // THE BACKUP FILE STRUCTURE IS GENERATED DURING CMAKE BUILD FROM THE ORIGINAL
    // CONTENTS OF SPIFFS_IMAGE/DATA
    // IF THIS HAPPENS, ALL CONTENTS OF THE AFFECTED FILE ARE RESET TO FACTORY DEFAULT
    if(d.HasParseError()){
        std::string backup_file_name = std::regex_replace(fn, std::regex("data"), "dbup");
        // TBD_LOGE("JSON", "File %s has a parse error!", fn.c_str());
        // TBD_LOGE("JSON", "Trying to replace with backup file %s", backup_file_name.c_str());
        fp = fopen(backup_file_name.c_str(), "r");
        if (fp == NULL) {
            // TBD_LOGE("JSON", "Could not open file %s", backup_file_name.c_str());
            return;
        }
        //char readBuffer[512];
//    TBD_LOGE("JSON", "read stream");
        rapidjson::FileReadStream is(fp, buffer, MB_BUF_SZ);
//    TBD_LOGE("JSON", "trying to parse");
        d.ParseStream(is);
        fclose(fp);
        // if backup parsing was successful, copy backuped data to defective file
        if(!d.HasParseError()){
            storeJSON(d, fn);
        }else{
            // TBD_LOGE("JSON", "FATAL ERROR: Could not recover from backup file %s", backup_file_name.c_str());
        }
    }
}

void ConfigBase::storeJSON(rapidjson::Document &d, const std::string &fn) {
    //FILE*
    fp = fopen(fn.c_str(), "w"); // non-Windows use "w"
    if (fp == NULL) {
        // TBD_LOGE("JSON", "could not open file %s", fn.c_str());
        return;
    }
    //char writeBuffer[512];
    rapidjson::FileWriteStream os(fp, buffer, MB_BUF_SZ);
    rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
    d.Accept(writer);
    fflush(fp);
    fclose(fp);
}

void ConfigBase::printJSON(rapidjson::Value &v) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    v.Accept(writer);
    // TBD_LOGW("Model", "JSON string %s", buffer.GetString());
}

ConfigBase::ConfigBase() {
    buffer = (char *) heaps::malloc(MB_BUF_SZ, TBD_HEAPS_SPIRAM);
    // if (buffer == nullptr) TBD_LOGE("Model Base", "Fatal: Out of mem!");
}

ConfigBase::~ConfigBase() {
    heaps::free(buffer);
}

}