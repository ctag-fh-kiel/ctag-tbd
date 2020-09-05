#pragma once

#include <string>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "ctagDataModelBase.hpp"
#include <iostream>
#include <memory>
#include <vector>

class SimDataModel : public CTAG::SP::ctagDataModelBase {
public:
    SimDataModel();
    ~SimDataModel();
    void SetModelJSONString(const string &);
    const char * GetModelJSONCString();
    const string &GetKeyValue(const string &key);
    const int GetArrayElement(const string &key, const int index);
private:
    const string MODELJSONFN = "../data/cfg_tbd_sim.jsn";
    Document m;
};


