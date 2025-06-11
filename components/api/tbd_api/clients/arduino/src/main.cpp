#include <Arduino.h>
#include <tbd/client/tbd_client.hpp>


TBDClient tbd_client({
    TBD_CLIENT_UART_ID,
    TBD_CLIENT_RX_PIN,
    TBD_CLIENT_TX_PIN,
    TBD_CLIENT_BAUD_RATE,
});


void setup() {
    TBD_LOGI("tbd_client_demo", "setting up TBD client demo");

    #if TBD_CLIENT_LOG_BAUD_RATE
        Serial.begin(TBD_CLIENT_LOG_BAUD_RATE);
    #endif

    tbd_client.begin();
    tbd_client.event.on_some_trigger_event = []() {
        TBD_LOGI("tbd_client_demo", "got some trigger");
    };
    TBD_LOGI("tbd_client_demo", "log level %i", TBD_LOG_LEVEL);
    TBD_LOGD("tbd_client_demo", "setup done");
}

int step = 0;
int cmd_delay = 0;

void loop() {
    tbd_client.process_incoming();
    if (cmd_delay >= 3000) {
        tbd_client.rpc.add_value(12, [](tbd::uint_par res) {
           TBD_LOGI("demo", "add returned %i", res);
        });
        cmd_delay = 0;
    }
    cmd_delay++;

    vTaskDelay(10 / portTICK_PERIOD_MS);
}
