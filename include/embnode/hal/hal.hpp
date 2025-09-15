// Hardware abstraction hooks (ESP32/STM32 specific impls separate)
#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>

namespace embnode::hal {

// Clock/timing
uint64_t millis();
void sleep_ms(uint32_t ms);

// Deep sleep
void deep_sleep_ms(uint32_t ms);

// DMA sampling: configure and start a continuous or periodic DMA into buffer.
// The callback should be called when a buffer of N samples is ready.
using SampleReadyCB = std::function<void(const int16_t* data, size_t count, uint64_t timestampMs)>;
bool start_dma_sampler(uint32_t sample_rate_hz, size_t frame_samples, SampleReadyCB cb);
void stop_dma_sampler();

// Watchdog
void watchdog_feed();

// Network stubs
bool net_mqtt_publish(const uint8_t* data, size_t len);
bool net_http_post(const uint8_t* data, size_t len);

}

