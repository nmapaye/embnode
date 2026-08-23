#include "embnode/metrics/power.hpp"

#include "embnode/hal/hal.hpp"

namespace embnode::metrics {
namespace {

uint64_t interval_start = 0;
DutyStats accumulated{};
bool awake = true;
bool initialized = false;

void initialize_if_needed() {
    if (!initialized) {
        interval_start = hal::millis();
        awake = true;
        initialized = true;
    }
}

DutyStats snapshot(uint64_t now) {
    initialize_if_needed();
    DutyStats result = accumulated;
    const uint64_t elapsed = now >= interval_start ? now - interval_start : 0;
    if (awake) {
        result.awake_ms += elapsed;
    } else {
        result.asleep_ms += elapsed;
    }
    return result;
}

}  // namespace

void reset() {
    accumulated = {};
    interval_start = hal::millis();
    awake = true;
    initialized = true;
}

void mark_awake() {
    initialize_if_needed();
    if (awake) {
        return;
    }
    const uint64_t now = hal::millis();
    if (now >= interval_start) {
        accumulated.asleep_ms += now - interval_start;
    }
    interval_start = now;
    awake = true;
}

void mark_sleep() {
    initialize_if_needed();
    if (!awake) {
        return;
    }
    const uint64_t now = hal::millis();
    if (now >= interval_start) {
        accumulated.awake_ms += now - interval_start;
    }
    interval_start = now;
    awake = false;
}

DutyStats duty_stats() {
    return snapshot(hal::millis());
}

float duty_cycle_percent() {
    const DutyStats stats = duty_stats();
    const uint64_t total = stats.awake_ms + stats.asleep_ms;
    if (total == 0) {
        return 0.0F;
    }
    return 100.0F * static_cast<float>(stats.awake_ms) / static_cast<float>(total);
}

float average_current_ma(float awake_current_ma, float sleep_current_ma) {
    const float duty_cycle = duty_cycle_percent() / 100.0F;
    return duty_cycle * awake_current_ma + (1.0F - duty_cycle) * sleep_current_ma;
}

}  // namespace embnode::metrics
