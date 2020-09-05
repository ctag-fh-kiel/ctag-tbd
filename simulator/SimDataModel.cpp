#include "SimDataModel.hpp"

SimDataModel::SimDataModel() {
    loadJSON(m, MODELJSONFN);
}

void SimDataModel::SetModelJSONString(const string &s) {
   m.Parse(s);
   storeJSON(m, MODELJSONFN);
}

const char * SimDataModel::GetModelJSONCString() {
    json.Clear();
    Writer<StringBuffer> writer(json);
    m.Accept(writer);
    return json.GetString();
}

SimDataModel::~SimDataModel() {

}

const string &SimDataModel::GetKeyValue(const string &key) {
    if(!m.HasMember(key)) return "";
    if(!m[key].IsString()) return "";
    return m[key].GetString();
}

const int SimDataModel::GetArrayElement(const string &key, const int index) {
    if(!m.HasMember(key)) return 0;
    if(!m[key].IsArray()) return 0;
    if(m[key][index].IsString()) return std::stoi(m[key][index].GetString());
    if(m[key][index].IsInt()) return m[key][index].GetInt();
}
