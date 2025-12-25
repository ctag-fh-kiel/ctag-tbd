#include "link.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"

#ifdef CONFIG_ABLETON_LINK
#include "ableton/Link.hpp"
#endif

namespace CTAG{
    namespace LINK{

        // Ableton Link
#ifdef CONFIG_ABLETON_LINK_DEBUG
        static void link_print_task(void* userParam){
            auto link = static_cast<ableton::Link*>(userParam);
            const auto quantum = 4.0;

            while (true){
                const auto sessionState = link->captureAppSessionState();
                const auto numPeers = link->numPeers();
                const auto time = link->clock().micros();
                const auto beats = sessionState.beatAtTime(time, quantum);
                std::cout << std::defaultfloat << "| peers: " << numPeers << " | "
                    << "tempo: " << sessionState.tempo() << " | " << std::fixed
                    << "beats: " << beats << " |" << std::endl;
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
        }
#endif

#ifdef CONFIG_ABLETON_LINK
        static ableton::Link *ableton_link {nullptr};
#endif

        void link::Init(){
#ifdef CONFIG_ABLETON_LINK
            if(ableton_link) return;
            ableton_link = new ableton::Link(120.0);
            ableton_link->enable(true);
#ifdef CONFIG_ABLETON_LINK_DEBUG
            xTaskCreatePinnedToCore(link_print_task, "link_print_task", 4096, ableton_link, 1, NULL, 0);
#endif
            ESP_LOGI("Ableton Link", "Enabled");
#endif
        }

        void link::DeInit(){
#ifdef CONFIG_ABLETON_LINK
            ableton_link->enable(false);
            ableton_link->enableStartStopSync(true);
            delete ableton_link;
            ableton_link = nullptr;
            ESP_LOGI("Ableton Link", "Disabled");
#else
            return;
#endif
        }

IRAM_ATTR void link::GetLinkRtSessionData(link_session_data_t* data){
#ifdef CONFIG_ABLETON_LINK
            if(!ableton_link){
                data->linkActive = false;
                return;
            }
            const auto state = ableton_link->captureAudioSessionState();
            const auto time = ableton_link->clock().micros();
            data->linkActive = true;
            data->numPeers = ableton_link->numPeers();
            data->tempo = static_cast<float>(state.tempo());
            data->quantum = 4.0f;
            data->beat = state.beatAtTime(time, data->quantum);
            data->phase = state.phaseAtTime(time, 1.);
#else
            data->linkActive = false;
#endif
        }

        void link::GetLinkSessionData(link_session_data_t* data){
#ifdef CONFIG_ABLETON_LINK
            if(!ableton_link){
                data->linkActive = false;
                return;
            }
            const auto state = ableton_link->captureAppSessionState();
            const auto time = ableton_link->clock().micros();

            data->linkActive = true;
            data->numPeers = ableton_link->numPeers();
            data->tempo = static_cast<float>(state.tempo());
            data->quantum = 4.0f;
            data->beat = state.beatAtTime(time, data->quantum);
            data->phase = state.phaseAtTime(time, 1.);
#else
            data->linkActive = false;
#endif
        }

        void link::SetLinkTempo(float bpm){
#ifdef CONFIG_ABLETON_LINK
            auto state = ableton_link->captureAppSessionState();
            const auto now = ableton_link->clock().micros();
            state.setTempo(bpm, now);
            ableton_link->commitAppSessionState(state);
#endif
        }
    } // LINK
} // CTAG
