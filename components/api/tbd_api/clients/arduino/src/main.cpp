#include <Arduino.h>
#include <tbd/client/tbd_client.hpp>

#if 1

TBDClient tbd_client(TBD_CLIENT_UART_ID, TBD_CLIENT_RX_PIN, TBD_CLIENT_TX_PIN);

void setup() {
    TBD_LOGI("tbd_client_demo", "setting up TBD client demo");

    #if TBD_CLIENT_LOG_BAUD_RATE
        Serial.begin(TBD_CLIENT_LOG_BAUD_RATE);
    #endif

    tbd_client.begin(TBD_CLIENT_BAUD_RATE);
    TBD_LOGI("tbd_client_demo", "log level %i", TBD_LOG_LEVEL);
    TBD_LOGD("tbd_client_demo", "setup done");
}

void loop() {
    tbd_client.process_incoming();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

#else

HardwareSerial uart(2);

void setup() {
    TBD_LOGI("tbd_client_demo", "setting up TBD client demo");
    uart.setPins(TBD_CLIENT_RX_PIN, TBD_CLIENT_TX_PIN);
    uart.begin(TBD_CLIENT_BAUD_RATE);
#if TBD_CLIENT_LOG_BAUD_RATE
    Serial.begin(TBD_CLIENT_LOG_BAUD_RATE);
#endif
    TBD_LOGI("tbd_client_demo", "setup done");
}

void loop() {
    Serial.print("have bytes: ");
    Serial.print(uart.available());
    Serial.print("\r\n");
    Serial.flush();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

#endif