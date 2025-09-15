#include "embnode/config/config.hpp"
#include "embnode/logging/log.hpp"
#include "embnode/rtos/task.hpp"
#include "embnode/rtos/bounded_queue.hpp"
#include "embnode/telemetry/packet.hpp"
#include "embnode/tasks/types.hpp"

extern "C" {
#include "FreeRTOS.h"
#include "task.h"
}

namespace embnode::tasks {

using Sample = embnode::tasks::Sample;
using SampleQueue = embnode::rtos::BoundedQueue<Sample>;
using PacketQueue = embnode::rtos::BoundedQueue<embnode::telemetry::Frame>;

static void aggregator_entry(SampleQueue* inQ, PacketQueue* outQ) {
    std::vector<uint8_t> payload;
    payload.reserve(config::kAggregatorBatchSize * sizeof(Sample));
    uint32_t count = 0;
    for (;;) {
        Sample s{};
        if (!inQ->pop(s, portMAX_DELAY)) continue;
        const uint8_t* p = reinterpret_cast<const uint8_t*>(&s);
        payload.insert(payload.end(), p, p + sizeof(Sample));
        if (++count >= config::kAggregatorBatchSize) {
            embnode::telemetry::Frame f{};
            f.hdr.type = embnode::telemetry::PacketType::Samples;
            f.payload = std::move(payload);
            payload.clear();
            payload.reserve(config::kAggregatorBatchSize * sizeof(Sample));
            count = 0;
            outQ->push(f, 0);
        }
    }
}

embnode::rtos::Task start_aggregator_task(SampleQueue* inQ, PacketQueue* outQ, UBaseType_t prio) {
    return embnode::rtos::Task("aggregator", prio, 4096, [=]{ aggregator_entry(inQ, outQ); });
}

} // namespace embnode::tasks
