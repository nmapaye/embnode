#include "embnode/config/config.hpp"
#include "embnode/rtos/task.hpp"
#include "embnode/rtos/bounded_queue.hpp"
#include "embnode/tasks/types.hpp"
#include "embnode/hal/hal.hpp"
#include "embnode/logging/log.hpp"

extern "C" {
#include "FreeRTOS.h"
#include "task.h"
}

namespace embnode::tasks {

using Sample = embnode::tasks::Sample;
using SampleQueue = embnode::rtos::BoundedQueue<Sample>;

static void sampler_entry(SampleQueue* outQ) {
    const TickType_t periodTicks = pdMS_TO_TICKS(1000 / config::kSampleRateHz);
    TickType_t nextWake = xTaskGetTickCount();
    uint64_t start_ms = hal::millis();
    uint32_t produced = 0;
    float max_abs_jitter_ms = 0.0f;
    double sum_abs_jitter_ms = 0.0;

    for (;;) {
        vTaskDelayUntil(&nextWake, periodTicks);
        // Simulated read via HAL; replace with DMA callback integration.
        Sample s{hal::millis(), static_cast<int16_t>(produced & 0x7FFF)};
        outQ->push(s, 0);
        ++produced;

        // jitter measurement
        uint64_t elapsed = hal::millis() - start_ms;
        float ideal_ms = (1000.0f * produced) / config::kSampleRateHz;
        float jitter_ms = static_cast<float>(elapsed) - ideal_ms;
        float abs_jitter = jitter_ms >= 0 ? jitter_ms : -jitter_ms;
        if (abs_jitter > max_abs_jitter_ms) max_abs_jitter_ms = abs_jitter;
        sum_abs_jitter_ms += abs_jitter;
        if (abs_jitter > config::kSamplerJitterBudgetMs) {
            EMB_LOGW("SAMPLER", "Jitter %.2f ms exceeds budget", jitter_ms);
        }
        if ((produced % (config::kSampleRateHz * 60)) == 0) { // every ~minute
            float avg_abs = static_cast<float>(sum_abs_jitter_ms / produced);
            EMB_LOGI("SAMPLER", "Jitter avg_abs=%.3f ms max_abs=%.3f ms samples=%u",
                    avg_abs, max_abs_jitter_ms, (unsigned)produced);
        }
    }
}

embnode::rtos::Task start_sampler_task(SampleQueue* outQ, UBaseType_t prio) {
    return embnode::rtos::Task("sampler", prio, 4096, [outQ]{ sampler_entry(outQ); });
}

} // namespace embnode::tasks
