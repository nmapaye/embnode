#include "embnode/metrics/power.hpp"
#include "embnode/hal/hal.hpp"

namespace embnode::metrics {

static uint64_t awake_start = 0;
static uint64_t sleep_start = 0;
static DutyStats stats;
static bool is_awake = true;

void mark_awake() {
    uint64_t now = hal::millis();
    if (!is_awake) {
        // leaving sleep
        if (sleep_start) stats.asleep_ms += (now - sleep_start);
        awake_start = now;
        is_awake = true;
    }
}

void mark_sleep() {
    uint64_t now = hal::millis();
    if (is_awake) {
        // entering sleep
        if (awake_start) stats.awake_ms += (now - awake_start);
        sleep_start = now;
        is_awake = false;
    }
}

DutyStats duty_stats() { return stats; }

float duty_cycle_percent() {
    uint64_t total = stats.awake_ms + stats.asleep_ms;
    if (total == 0) return 0.f;
    return (100.0f * static_cast<float>(stats.awake_ms)) / static_cast<float>(total);
}

float average_current_ma(float i_awake_ma, float i_sleep_ma) {
    float dc = duty_cycle_percent() / 100.0f;
    return dc * i_awake_ma + (1.0f - dc) * i_sleep_ma;
}

}
