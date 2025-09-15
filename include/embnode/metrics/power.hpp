// Power profiling hooks
#pragma once

#include <cstdint>

namespace embnode::metrics {

struct DutyStats {
    uint64_t awake_ms {0};
    uint64_t asleep_ms {0};
};

void mark_awake();
void mark_sleep();
DutyStats duty_stats();
float duty_cycle_percent();
float average_current_ma(float i_awake_ma, float i_sleep_ma);

}
