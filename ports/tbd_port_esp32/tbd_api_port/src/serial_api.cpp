#include <tbd/api/common/serial_api.hpp>

#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <atomic>
#include <cstdint>
#include <tbd/sound_manager.hpp>
#include <tbd/favorites.hpp>
#include "driver/uart.h"
#include <tbd/version.hpp>

#if TBD_CALIBRATION
    #include <tbd/calibration.hpp>
#endif

#if TDB_ADC
    #include "driver/gpio.h"
#endif

using namespace rapidjson;

namespace {

void init_uart() {
    /* Configure parameters of a UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config;
    uart_config.baud_rate = 115200;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity    = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.rx_flow_ctrl_thresh = 0; // no effect as HW flow disabled
    uart_config.source_clk = UART_SCLK_DEFAULT;

    uart_param_config(UART_NUM_0, &uart_config);
    //uart_set_pin(UART_NUM_0, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
    uart_set_pin(UART_NUM_0,UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,UART_PIN_NO_CHANGE,UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_0, 4096, 1024, 0, NULL, 0);
}

struct SerialApiImpl{
    uint32_t do_begin();
    uint32_t do_work();
    uint32_t do_cleanup();

private:
    void send_string(const std::string& s);
    void send_string(const char* s);
    void process_command(const std::string& cmd);

    enum ProtocolStates {
        IDLE = 0x00,
        RCV = 0x01
    } pState = IDLE;

    std::string _cmd;
    char _data;

    static const char _stx = 0x02;
    static const char _etx = 0x03;
};

// module task instance
tbd::system::ModuleTask<SerialApiImpl, tbd::system::CpuCore::system, 4096 * 2> serial_api("serial_api");

void SerialApiImpl::send_string(const std::string& s) {
    string cmd = _stx + s + _etx;
    cout << cmd;
    cout.flush();
}

void SerialApiImpl::send_string(const char* s) {
    size_t strLen = strlen(s);
    write(STDOUT_FILENO, &_stx, 1);
    write(STDOUT_FILENO, s, strLen);
    write(STDOUT_FILENO, &_etx, 1);
    // flush stdout
    write(STDOUT_FILENO, NULL, 0);
}


uint32_t SerialApiImpl::do_begin() {
    init_uart();
}

uint32_t SerialApiImpl::do_work() {
    char data;
    // TODO: using a string here for storing cmd is bad, when large data is received, and a large plugin is loaded
    // TODO: one may run out of memory
    // TODO: fix this with tbd_heaps_malloc
    // TODO: currently Void preset is loaded from web-ui before a backup is send to the TBD
    // TODO: but Void may not exist in a custom firmware
    //Read data from UART
    int len = uart_read_bytes(UART_NUM_0, (uint8_t *)&data, 1, 500 / portTICK_PERIOD_MS);
    if(len != 0){
        if(data == _stx){
            // start of text
            pState = RCV;
            _cmd.clear();
        }else if(data == _etx){
            // end of text
            process_command(_cmd);
            pState = IDLE;
        }else if(pState == RCV){
            _cmd.push_back(data);
        }
    }/*else{
        if(pState != RCV){
            // send heart beat with enquire sign
            uart_write_bytes(UART_NUM_0, &enq, 1);
        }
    }*/
}

void SerialApiImpl::process_command(const std::string &cmd) {
    Document d;
    d.Parse(cmd);
    if(d.HasParseError() || !d.IsObject() || !d.HasMember("cmd")){
        send_string("{\"error\":\"" + cmd + "\"}");
        return;
    }
    string s(d["cmd"].GetString());
    if(s == "/api/v1/getPlugins"){
        string s = tbd::audio::SoundProcessorManager::GetCStrJSONSoundProcessors();
        send_string(s);
        return;
    }
    if(s.find("/api/v1/getActivePlugin/") == 0){
        int ch = d["ch"].GetInt();
        send_string("{\"id\":\"" + tbd::audio::SoundProcessorManager::GetStringID(ch) + "\"}");
        return;
    }
    if(s.find("/api/v1/getPluginParams/") == 0){
        int ch = d["ch"].GetInt();
        send_string(tbd::audio::SoundProcessorManager::GetCStrJSONActivePluginParams(ch));
        return;
    }
    if(s.find("/api/v1/setActivePlugin/") == 0){
        int ch = d["ch"].GetInt();
        string id = d["id"].GetString();
        tbd::audio::SoundProcessorManager::SetSoundProcessorChannel(ch, id);
        send_string("{}");
        return;
    }
    if(s.find("/api/v1/setPluginParam/") == 0){
        int ch = d["ch"].GetInt();
        int val = d["current"].GetInt();
        string id = d["id"].GetString();
        tbd::audio::SoundProcessorManager::SetChannelParamValue(ch, id, "current", val);
        send_string("{}");
        return;
    }
    if(s.find("/api/v1/setPluginParamCV/") == 0){
        int ch = d["ch"].GetInt();
        int val = d["cv"].GetInt();
        string id = d["id"].GetString();
        tbd::audio::SoundProcessorManager::SetChannelParamValue(ch, id, "cv", val);
        send_string("{}");
        return;
    }
    if(s.find("/api/v1/setPluginParamTRIG/") == 0){
        int ch = d["ch"].GetInt();
        int val = d["trig"].GetInt();
        string id = d["id"].GetString();
        tbd::audio::SoundProcessorManager::SetChannelParamValue(ch, id, "trig", val);
        send_string("{}");
        return;
    }
    if(s.find("/api/v1/getPresets/") == 0){
        int ch = d["ch"].GetInt();
        send_string(tbd::audio::SoundProcessorManager::GetCStrJSONGetPresets(ch));
        return;
    }
    if(s.find("/api/v1/loadPreset/") == 0){
        int ch = d["ch"].GetInt();
        int num = d["number"].GetInt();
        tbd::audio::SoundProcessorManager::ChannelLoadPreset(ch, num);
        send_string("{}");
        return;
    }
    if(s.find("/api/v1/savePreset/") == 0){
        int ch = d["ch"].GetInt();
        int num = d["number"].GetInt();
        string name = d["name"].GetString();
        tbd::audio::SoundProcessorManager::ChannelSavePreset(ch, name, num);
        send_string("{}");
        return;
    }
    if(s.find("/api/v1/getConfiguration") == 0){
        send_string(tbd::audio::SoundProcessorManager::GetCStrJSONConfiguration());
        return;
    }
    if(s.find("/api/v1/reboot") == 0){
#if TBD_CALIBRATION
        int doCal = d["calibration"].GetInt();
        if (doCal) tbd::Calibration::RequestCalibrationOnReboot();
#endif
        send_string("{}");
        esp_restart();
        // no return
    }
    if(s.find("/api/v1/getPresetData") == 0){
        string pluginID = d["id"].GetString();
        const char *json = tbd::audio::SoundProcessorManager::GetCStrJSONSoundProcessorPresets(string(pluginID));
        if (json != NULL)
            send_string(json);
        else
            send_string("{}");
        return;
    }
    if(s.find("/api/v1/getCalibration") == 0){
#if TBD_CALIBRATION
        send_string(tbd::Calibration::GetCStrJSONCalibration());
#endif
        return;
    }
    if(s.find("/api/v1/getIOCaps") == 0){
        send_string(tbd::sysinfo::device_capabilities);
        return;
    }
    if(s.find("/api/v1/setCalibration") == 0){
        Value calibrationData = d["calibration"].GetObject();
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        calibrationData.Accept(writer);
#if TBD_CALIBRATION
        tbd::Calibration::SetJSONCalibration(buffer.GetString());
#endif
        send_string("{}");
        return;
    }
    if(s.find("/api/v1/setConfiguration") == 0){
        Value configData = d["configuration"].GetObject();
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        configData.Accept(writer);
        tbd::audio::SoundProcessorManager::SetConfigurationFromJSON(buffer.GetString());
        send_string("{}");
        return;
    }
    if(s.find("/api/v1/favorites/getAll") == 0) {
        send_string(tbd::Favorites::GetAllFavorites().c_str());
        return;
    }
    if(s.find("/api/v1/favorites/store") == 0) {
        Value data = d["data"].GetObject();
        int fav = d["fav"].GetInt();
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        data.Accept(writer);
        tbd::Favorites::StoreFavorite(fav, buffer.GetString());
        send_string("{}");
        return;
    }
    if(s.find("/api/v1/favorites/recall") == 0) {
        int fav = d["fav"].GetInt();
        tbd::Favorites::ActivateFavorite(fav);
        send_string("{}");
        return;
    }
    if(s.find("/api/v1/setPresetData") == 0){
        string id = d["id"].GetString();
        Value presetData = d["data"].GetObject();
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        presetData.Accept(writer);
        tbd::audio::SoundProcessorManager::SetCStrJSONSoundProcessorPreset(id.c_str(), buffer.GetString());
        send_string("{\"id\":\"" + id + "\"}");
        return;
    }
    /*

    otaAPI
     */
    // sample rom API
    if(s.find("/api/v1/srom/getSize") == 0){
        send_string(to_string(CONFIG_SAMPLE_ROM_SIZE));
        return;
    }
    // TODO: implement sample rom write with serial api, it is super slow...
    /*
    if(s.find("/api/v1/srom/erase") == 0){
        tbd::audio::SoundProcessorManager::DisablePluginProcessing();
        // erase flash / lengthy operation
        ESP_LOGI("SERIAL", "Erasing flash start %d, size %d!", CONFIG_SAMPLE_ROM_START_ADDRESS, CONFIG_SAMPLE_ROM_SIZE);
        //ESP_ERROR_CHECK(spi_flash_erase_range(CONFIG_SAMPLE_ROM_START_ADDRESS, CONFIG_SAMPLE_ROM_SIZE));
        tbd::audio::SoundProcessorManager::EnablePluginProcessing();
        send_string("{}");
        return;
    }
    if(s.find("/api/v1/srom/upRaw") == 0){
        int size = d["size"].GetInt();
        // erase flash / lengthy operation
        ESP_LOGI("SERIAL", "Receiving srom BLOB size %d bytes", size);
        send_string("{}");
        int cnt = 0, len;
        char data;

        do{
            len = uart_read_bytes(UART_NUM_0, (uint8_t *)&data, 1, 500 / portTICK_RATE_MS);
            cnt += len;
        }while(cnt != size);

        send_string("{}");
        ESP_LOGI("SERIAL", "Received %d bytes", cnt);

        return;
    }
    */
}

}

namespace tbd::api {

void SerialApi::


}