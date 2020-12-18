#include "SerialAPI.hpp"
#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <atomic>
#include <cstdint>
#include "SPManager.hpp"
#include "Calibration.hpp"
#include "driver/gpio.h"
#include "driver/uart.h"

using namespace rapidjson;

TaskHandle_t CTAG::SAPI::SerialAPI::hSerialTask = 0;
const char CTAG::SAPI::SerialAPI::stx = 0x02;
const char CTAG::SAPI::SerialAPI::etx = 0x03;

void CTAG::SAPI::SerialAPI::StartSerialAPI() {
    initUART();
    xTaskCreatePinnedToCore(&SerialAPI::serialTask, "serial_task", 4096, 0, tskIDLE_PRIORITY + 4, &hSerialTask, 0);
}

void CTAG::SAPI::SerialAPI::initUART() {
    /* Configure parameters of a UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity    = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 0 // no effect as HW flow disabled
    };
    uart_param_config(UART_NUM_0, &uart_config);
    //uart_set_pin(UART_NUM_0, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
    uart_set_pin(UART_NUM_0,UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,UART_PIN_NO_CHANGE,UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_0, 4096, 1024, 0, NULL, 0);
}

void CTAG::SAPI::SerialAPI::serialTask(void *) {
    char data;
    std::string cmd;
    enum ProtocolStates {
        IDLE = 0x00,
        RCV = 0x01
    };
    ProtocolStates pState = IDLE;
    while(1) {
        //Read data from UART
        int len = uart_read_bytes(UART_NUM_0, (uint8_t *)&data, 1, 500 / portTICK_RATE_MS);
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
        return;
    }
    if(s.find("/api/v1/setPluginParam/") == 0){
        int ch = d["ch"].GetInt();
        int val = d["current"].GetInt();
        string id = d["id"].GetString();
        CTAG::AUDIO::SoundProcessorManager::SetChannelParamValue(ch, id, "current", val);
        return;
    }
    if(s.find("/api/v1/setPluginParamCV/") == 0){
        int ch = d["ch"].GetInt();
        int val = d["cv"].GetInt();
        string id = d["id"].GetString();
        CTAG::AUDIO::SoundProcessorManager::SetChannelParamValue(ch, id, "cv", val);
        return;
    }
    if(s.find("/api/v1/setPluginParamTRIG/") == 0){
        int ch = d["ch"].GetInt();
        int val = d["trig"].GetInt();
        string id = d["id"].GetString();
        CTAG::AUDIO::SoundProcessorManager::SetChannelParamValue(ch, id, "trig", val);
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
        return;
    }
    if(s.find("/api/v1/savePreset/") == 0){
        int ch = d["ch"].GetInt();
        int num = d["number"].GetInt();
        string name = d["name"].GetString();
        CTAG::AUDIO::SoundProcessorManager::ChannelSavePreset(ch, name, num);
        return;
    }
    if(s.find("/api/v1/getConfiguration") == 0){
        sendString(CTAG::AUDIO::SoundProcessorManager::GetCStrJSONConfiguration());
        return;
    }
    if(s.find("/api/v1/reboot") == 0){
        int doCal = d["calibration"].GetInt();
        if (doCal) CTAG::CAL::Calibration::RequestCalibrationOnReboot();
        esp_restart();
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
        sendString(CTAG::CAL::Calibration::GetCStrJSONCalibration());
        return;
    }
    if(s.find("/api/v1/setCalibration") == 0){
        Value calibrationData = d["calibration"].GetObject();
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        calibrationData.Accept(writer);
        CTAG::CAL::Calibration::SetJSONCalibration(buffer.GetString());
        return;
    }
    if(s.find("/api/v1/setConfiguration") == 0){
        Value configData = d["configuration"].GetObject();
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        configData.Accept(writer);
        CTAG::AUDIO::SoundProcessorManager::SetConfigurationFromJSON(buffer.GetString());
        return;
    }
    if(s.find("/api/v1/setPresetData") == 0){
        string id = d["id"].GetString();
        Value presetData = d["data"].GetObject();
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        presetData.Accept(writer);
        CTAG::AUDIO::SoundProcessorManager::SetJSONSoundProcessorPreset(id, buffer.GetString());
        return;
    }
    /*

    otaAPI
     */

}

void CTAG::SAPI::SerialAPI::sendString(const string &s) {
    string cmd = stx + s + etx;
    cout << cmd;
    cout.flush();
}
