// Node configuration
#pragma once

#include <cstdint>

namespace embnode::config {

// Sampling
static constexpr uint32_t kSampleRateHz = 100; // target rate
static constexpr uint32_t kSamplerJitterBudgetMs = 1; // max jitter target

// Duty cycling
static constexpr uint32_t kActiveWindowMs = 5000; // time awake per cycle
static constexpr uint32_t kSleepWindowMs = 15000; // sleep time per cycle

// Power modeling (user to tune per hardware)
static constexpr float kAwakeCurrent_mA = 40.0f;   // typical active current
static constexpr float kSleepCurrent_mA = 0.15f;   // deep sleep current

// Backpressure policy: 0=Block, 1=DropNewest, 2=DropOldest
static constexpr int kBackpressurePolicy = 2;

// Telemetry
static constexpr size_t kAggregatorBatchSize = 64; // samples per packet

// OTA
static constexpr uint32_t kOtaTimeoutMs = 120000; // 2 minutes

} // namespace embnode::config
