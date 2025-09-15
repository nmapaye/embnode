#include "embnode/logging/log.hpp"
#include "embnode/rtos/task.hpp"
#include "embnode/hal/hal.hpp"

extern "C" {
#include "FreeRTOS.h"
#include "task.h"
}

namespace embnode::tasks {

static void watchdog_entry() {
    const TickType_t period = pdMS_TO_TICKS(1000);
    for (;;) {
        hal::watchdog_feed();
        vTaskDelay(period);
    }
}

embnode::rtos::Task start_watchdog_task(UBaseType_t prio) {
    return embnode::rtos::Task("watchdog", prio, 2048, []{ watchdog_entry(); });
}

} // namespace embnode::tasks

