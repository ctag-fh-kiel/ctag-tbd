#include "SerialAPI.hpp"
#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <atomic>
#include <cstdint>
#include <tbd/sound_manager.hpp>
#include <tbd/favourites.hpp>
#include "driver/uart.h"
#include <tbd/version.hpp>

#if TBD_CALIBRATION
    #include "Calibration.hpp"
#endif

#if TDB_ADC
    #include "driver/gpio.h"
#endif

using namespace rapidjson;

TaskHandle_t CTAG::SAPI::SerialAPI::hSerialTask = 0;
const char CTAG::SAPI::SerialAPI::stx = 0x02;
const char CTAG::SAPI::SerialAPI::etx = 0x03;

void CTAG::SAPI::SerialAPI::StartSerialAPI() {
    initUART();
    xTaskCreatePinnedToCore(&SerialAPI::serialTask, "serial_task", 4096*2, 0, tskIDLE_PRIORITY + 4, &hSerialTask, 0);
}

void CTAG::SAPI::SerialAPI::initUART() {
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

void CTAG::SAPI::SerialAPI::serialTask(void *) {
    char data;
    // TODO: using a string here for storing cmd is bad, when large data is received, and a large plugin is loaded
    // TODO: one may run out of memory
    // TODO: fix this with heap_caps_malloc
    // TODO: currently Void preset is loaded from web-ui before a backup is send to the TBD
    // TODO: but Void may not exist in a custom firmware
    std::string cmd;
    enum ProtocolStates {
        IDLE = 0x00,
        RCV = 0x01
    };
    ProtocolStates pState = IDLE;
    while(1) {
        //Read data from UART
        int len = uart_read_bytes(UART_NUM_0, (uint8_t *)&data, 1, 500 / portTICK_PERIOD_MS);
        if(len != 0){
            if(data == stx){
                // start of text
                pState = RCV;
                cmd = "";
            }else if(data == etx){
                // end of text
                processAPICommand(cmd);
                pState = IDLE;
            }else if(pState == RCV){
                cmd.push_back(data);
            }
        }/*else{
            if(pState != RCV){
                // send heart beat with enquire sign
                uart_write_bytes(UART_NUM_0, &enq, 1);
            }
        }*/
    }
}

void CTAG::SAPI::SerialAPI::processAPICommand(const string &cmd) {
    Document d;
    d.Parse(cmd);
    if(d.HasParseError() || !d.IsObject() || !d.HasMember("cmd")){
        sendString("{\"error\":\"" + cmd + "\"}");
        return;
    }
    string s(d["cmd"].GetString());
    if(s == "/api/v1/getPlugins"){
        string s = CTAG::AUDIO::SoundProcessorManager::GetCStrJSONSoundProcessors();
        sendString(s);
        return;
    }
    if(s.find("/api/v1/getActivePlugin/") == 0){
        int ch = d["ch"].GetInt();
        sendString("{\"id\":\"" + CTAG::AUDIO::SoundProcessorManager::GetStringID(ch) + "\"}");
        return;
    }
    if(s.find("/api/v1/getPluginParams/") == 0){
        int ch = d["ch"].GetInt();
        sendString(CTAG::AUDIO::SoundProcessorManager::GetCStrJSONActivePluginParams(ch));
        return;
    }
    if(s.find("/api/v1/setActivePlugin/") == 0){
        int ch = d["ch"].GetInt();
        string id = d["id"].GetString();
        CTAG::AUDIO::SoundProcessorManager::SetSoundProcessorChannel(ch, id);
        sendString("{}");
        return;
    }
    if(s.find("/api/v1/setPluginParam/") == 0){
        int ch = d["ch"].GetInt();
        int val = d["current"].GetInt();
        string id = d["id"].GetString();
        CTAG::AUDIO::SoundProcessorManager::SetChannelParamValue(ch, id, "current", val);
        sendString("{}");
        return;
    }
    if(s.find("/api/v1/setPluginParamCV/") == 0){
        int ch = d["ch"].GetInt();
        int val = d["cv"].GetInt();
        string id = d["id"].GetString();
        CTAG::AUDIO::SoundProcessorManager::SetChannelParamValue(ch, id, "cv", val);
        sendString("{}");
        return;
    }
    if(s.find("/api/v1/setPluginParamTRIG/") == 0){
        int ch = d["ch"].GetInt();
        int val = d["trig"].GetInt();
        string id = d["id"].GetString();
        CTAG::AUDIO::SoundProcessorManager::SetChannelParamValue(ch, id, "trig", val);
        sendString("{}");
        return;
    }
    if(s.find("/api/v1/getPresets/") == 0){
        int ch = d["ch"].GetInt();
        sendString(CTAG::AUDIO::SoundProcessorManager::GetCStrJSONGetPresets(ch));
        return;
    }
    if(s.find("/api/v1/loadPreset/") == 0){
        int ch = d["ch"].GetInt();
        int num = d["number"].GetInt();
        CTAG::AUDIO::SoundProcessorManager::ChannelLoadPreset(ch, num);
        sendString("{}");
        return;
    }
    if(s.find("/api/v1/savePreset/") == 0){
        int ch = d["ch"].GetInt();
        int num = d["number"].GetInt();
        string name = d["name"].GetString();
        CTAG::AUDIO::SoundProcessorManager::ChannelSavePreset(ch, name, num);
        sendString("{}");
        return;
    }
    if(s.find("/api/v1/getConfiguration") == 0){
        sendString(CTAG::AUDIO::SoundProcessorManager::GetCStrJSONConfiguration());
        return;
    }
    if(s.find("/api/v1/reboot") == 0){
#if TBD_CALIBRATION
        int doCal = d["calibration"].GetInt();
        if (doCal) CTAG::CAL::Calibration::RequestCalibrationOnReboot();
#endif
        sendString("{}");
        esp_restart();
        // no return
    }
    if(s.find("/api/v1/getPresetData") == 0){
        string pluginID = d["id"].GetString();
        const char *json = CTAG::AUDIO::SoundProcessorManager::GetCStrJSONSoundProcessorPresets(string(pluginID));
        if (json != NULL)
            sendString(json);
        else
            sendString("{}");
        return;
    }
    if(s.find("/api/v1/getCalibration") == 0){
#if TBD_CALIBRATION
        sendString(CTAG::CAL::Calibration::GetCStrJSONCalibration());
#endif
        return;
    }
    if(s.find("/api/v1/getIOCaps") == 0){
        sendString(device_capabilities);
        return;
    }
    if(s.find("/api/v1/setCalibration") == 0){
        Value calibrationData = d["calibration"].GetObject();
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        calibrationData.Accept(writer);
#if TBD_CALIBRATION
        CTAG::CAL::Calibration::SetJSONCalibration(buffer.GetString());
#endif
        sendString("{}");
        return;
    }
    if(s.find("/api/v1/setConfiguration") == 0){
        Value configData = d["configuration"].GetObject();
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        configData.Accept(writer);
        CTAG::AUDIO::SoundProcessorManager::SetConfigurationFromJSON(buffer.GetString());
        sendString("{}");
        return;
    }
    if(s.find("/api/v1/favorites/getAll") == 0) {
        sendString(FAV::Favorites::GetAllFavorites().c_str());
        return;
    }
    if(s.find("/api/v1/favorites/store") == 0) {
        Value data = d["data"].GetObject();
        int fav = d["fav"].GetInt();
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        data.Accept(writer);
        CTAG::FAV::Favorites::StoreFavorite(fav, buffer.GetString());
        sendString("{}");
        return;
    }
    if(s.find("/api/v1/favorites/recall") == 0) {
        int fav = d["fav"].GetInt();
        FAV::Favorites::ActivateFavorite(fav);
        sendString("{}");
        return;
    }
    if(s.find("/api/v1/setPresetData") == 0){
        string id = d["id"].GetString();
        Value presetData = d["data"].GetObject();
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        presetData.Accept(writer);
        CTAG::AUDIO::SoundProcessorManager::SetCStrJSONSoundProcessorPreset(id.c_str(), buffer.GetString());
        sendString("{\"id\":\"" + id + "\"}");
        return;
    }
    /*

    otaAPI
     */
    // sample rom API
    if(s.find("/api/v1/srom/getSize") == 0){
        sendString(to_string(CONFIG_SAMPLE_ROM_SIZE));
        return;
    }
    // TODO: implement sample rom write with serial api, it is super slow...
    /*
    if(s.find("/api/v1/srom/erase") == 0){
        CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
        // erase flash / lengthy operation
        ESP_LOGI("SERIAL", "Erasing flash start %d, size %d!", CONFIG_SAMPLE_ROM_START_ADDRESS, CONFIG_SAMPLE_ROM_SIZE);
        //ESP_ERROR_CHECK(spi_flash_erase_range(CONFIG_SAMPLE_ROM_START_ADDRESS, CONFIG_SAMPLE_ROM_SIZE));
        CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();
        sendString("{}");
        return;
    }
    if(s.find("/api/v1/srom/upRaw") == 0){
        int size = d["size"].GetInt();
        // erase flash / lengthy operation
        ESP_LOGI("SERIAL", "Receiving srom BLOB size %d bytes", size);
        sendString("{}");
        int cnt = 0, len;
        char data;

        do{
            len = uart_read_bytes(UART_NUM_0, (uint8_t *)&data, 1, 500 / portTICK_RATE_MS);
            cnt += len;
        }while(cnt != size);

        sendString("{}");
        ESP_LOGI("SERIAL", "Received %d bytes", cnt);

        return;
    }
    */
}

void CTAG::SAPI::SerialAPI::sendString(const string &s) {
    string cmd = stx + s + etx;
    cout << cmd;
    cout.flush();
}

void CTAG::SAPI::SerialAPI::sendString(const char* s) {
    size_t strLen = strlen(s);
    write(STDOUT_FILENO, &stx, 1);
    write(STDOUT_FILENO, s, strLen);
    write(STDOUT_FILENO, &etx, 1);
    // flush stdout
    write(STDOUT_FILENO, NULL, 0);
}
