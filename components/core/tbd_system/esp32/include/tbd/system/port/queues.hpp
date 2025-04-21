#pragma once

#include <optional>
#include <portmacro.h>
#include <freertos/queue.h>

namespace tbd::system {

template<typename ItemT, uint8_t queue_size = 10>
struct Queue {

    Queue() {
        _handle = xQueueCreate(queue_size, sizeof(ItemT));
    }

    ~Queue() {
        vQueueDelete(_handle);
    }

    void push(const ItemT& item) {
        xQueueSend(_handle, &item, portMAX_DELAY);
    }

    /** @brief take oldest element from queue
     *
     *  If no value is available the task sleeps until an element beomces available
     *  or operation times out.
     *
     *  @arg wait_time_ms   time to wait in ms, 0 means wait indefinitely
     */
    std::optional<ItemT> pop(uint32_t wait_time_ms = 0) {
        auto wait_time = wait_time_ms == 0 ? portMAX_DELAY : wait_time_ms;
        ItemT item;
        if (!xQueueReceive(_handle, &item, wait_time_ms)) {
            TBD_LOGD("tbd_system", "queue timed out");
            return {};
        }
        return item;
    }

    /** @brief take oldest element from queue
     *
     */
    std::optional<ItemT> try_pop() {
        ItemT item;
        if (!xQueueReceive(_handle, &item, 0)) {
            return {};
        }
        return item;
    }

private:
    QueueHandle_t _handle;
};

}
