#include "embnode/config/config.hpp"
#include "embnode/logging/log.hpp"
#include "embnode/rtos/task.hpp"
#include "embnode/rtos/bounded_queue.hpp"
#include "embnode/hal/hal.hpp"
#include "embnode/metrics/power.hpp"

extern "C" {
#include "FreeRTOS.h"
#include "task.h"
}

namespace embnode::tasks {
// forward factory functions
struct Sample; // from types.hpp
embnode::rtos::Task start_sampler_task(embnode::rtos::BoundedQueue<Sample>* outQ, UBaseType_t prio);
embnode::rtos::Task start_aggregator_task(embnode::rtos::BoundedQueue<Sample>* inQ,
                                          embnode::rtos::BoundedQueue<embnode::telemetry::Frame>* outQ,
                                          UBaseType_t prio);
embnode::rtos::Task start_comms_task(embnode::rtos::BoundedQueue<embnode::telemetry::Frame>* inQ, UBaseType_t prio);
embnode::rtos::Task start_watchdog_task(UBaseType_t prio);
}

using embnode::rtos::BackpressurePolicy;

extern "C" void app_main();

void app_main() {
    using embnode::rtos::BoundedQueue;
    using Sample = embnode::tasks::Sample;

    // Configure backpressure
    BackpressurePolicy policy = BackpressurePolicy::DropOldest;
    if (embnode::config::kBackpressurePolicy == 0) policy = BackpressurePolicy::Block;
    if (embnode::config::kBackpressurePolicy == 1) policy = BackpressurePolicy::DropNewest;

    static BoundedQueue<Sample> sampleQ(1024, policy);
    static BoundedQueue<embnode::telemetry::Frame> pktQ(16, BackpressurePolicy::DropOldest);

    metrics::mark_awake();

    // Start tasks with priorities: sampler high, aggregator mid, comms lower, watchdog low
    auto tSampler = embnode::tasks::start_sampler_task(&sampleQ, tskIDLE_PRIORITY + 4);
    auto tAgg     = embnode::tasks::start_aggregator_task(&sampleQ, &pktQ, tskIDLE_PRIORITY + 3);
    auto tComms   = embnode::tasks::start_comms_task(&pktQ, tskIDLE_PRIORITY + 2);
    auto tWdog    = embnode::tasks::start_watchdog_task(tskIDLE_PRIORITY + 1);
    (void)tSampler; (void)tAgg; (void)tComms; (void)tWdog;

    // Duty cycling loop: stay awake then deep sleep
    const TickType_t activeTicks = pdMS_TO_TICKS(embnode::config::kActiveWindowMs);
    const TickType_t sleepTicks = pdMS_TO_TICKS(embnode::config::kSleepWindowMs);
    TickType_t last = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&last, activeTicks);
        // Prepare for sleep: in a real build coordinate shutdown/flush
        EMB_LOGI("MAIN", "Entering deep sleep for %u ms", (unsigned)embnode::config::kSleepWindowMs);
        metrics::mark_sleep();
        embnode::hal::deep_sleep_ms(embnode::config::kSleepWindowMs);
        metrics::mark_awake();
        float dc = embnode::metrics::duty_cycle_percent();
        float iavg = embnode::metrics::average_current_ma(embnode::config::kAwakeCurrent_mA,
                                                          embnode::config::kSleepCurrent_mA);
        EMB_LOGI("MAIN", "Woke; duty=%.1f%%, Iavg=%.2f mA", dc, iavg);
    }
}
