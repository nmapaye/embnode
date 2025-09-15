#include "embnode/hal/hal.hpp"
#include "embnode/logging/log.hpp"

#include <atomic>

namespace embnode::hal {

static std::atomic<uint64_t> fake_ms{0};

uint64_t millis() { return fake_ms.load(); }
void sleep_ms(uint32_t ms) { fake_ms += ms; }
void deep_sleep_ms(uint32_t ms) { EMB_LOGI("HAL", "Deep sleep %u ms (stub)", (unsigned)ms); fake_ms += ms; }

bool start_dma_sampler(uint32_t sample_rate_hz, size_t frame_samples, SampleReadyCB cb) {
    EMB_LOGI("HAL", "Start DMA sampler %u Hz, frame=%u (stub)", (unsigned)sample_rate_hz, (unsigned)frame_samples);
    (void)cb; return true;
}
void stop_dma_sampler() {
    EMB_LOGI("HAL", "Stop DMA sampler (stub)");
}

void watchdog_feed() {}

bool net_mqtt_publish(const uint8_t* data, size_t len) {
    EMB_LOGI("NET", "MQTT publish len=%u (stub)", (unsigned)len);
    (void)data; return true;
}
bool net_http_post(const uint8_t* data, size_t len) {
    EMB_LOGI("NET", "HTTP POST len=%u (stub)", (unsigned)len);
    (void)data; return true;
}

}

