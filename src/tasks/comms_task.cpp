#include "embnode/logging/log.hpp"
#include "embnode/rtos/task.hpp"
#include "embnode/rtos/bounded_queue.hpp"
#include "embnode/telemetry/packet.hpp"
#include "embnode/hal/hal.hpp"

extern "C" {
#include "FreeRTOS.h"
#include "task.h"
}

namespace embnode::tasks {

using PacketQueue = embnode::rtos::BoundedQueue<embnode::telemetry::Frame>;

static void comms_entry(PacketQueue* inQ) {
    for (;;) {
        embnode::telemetry::Frame f{};
        if (!inQ->pop(f, portMAX_DELAY)) continue;
        auto buf = embnode::telemetry::encode(f);
        // choose transport runtime; stub uses MQTT first, fallback HTTP
        if (!hal::net_mqtt_publish(buf.data(), buf.size())) {
            (void)hal::net_http_post(buf.data(), buf.size());
        }
    }
}

embnode::rtos::Task start_comms_task(PacketQueue* inQ, UBaseType_t prio) {
    return embnode::rtos::Task("comms", prio, 6144, [=]{ comms_entry(inQ); });
}

} // namespace embnode::tasks

