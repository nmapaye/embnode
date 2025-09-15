#ifdef ESP_PLATFORM

#include "embnode/hal/hal.hpp"
#include "embnode/logging/log.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_sleep.h"

namespace embnode::hal {

uint64_t millis() { return static_cast<uint64_t>(esp_timer_get_time() / 1000ULL); }
void sleep_ms(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }
void deep_sleep_ms(uint32_t ms) {
    EMB_LOGI("HAL", "ESP32 deep sleep %u ms", (unsigned)ms);
    esp_sleep_enable_timer_wakeup(static_cast<uint64_t>(ms) * 1000ULL);
    esp_deep_sleep_start();
}

bool start_dma_sampler(uint32_t sample_rate_hz, size_t frame_samples, SampleReadyCB cb) {
    (void)sample_rate_hz; (void)frame_samples; (void)cb;
    // TODO: Implement via ADC/I2S + DMA for your sensor frontend.
    EMB_LOGW("HAL", "start_dma_sampler not implemented on ESP32 yet");
    return false;
}
void stop_dma_sampler() {}

void watchdog_feed() {
    // Optional: integrate with esp_task_wdt if enabled.
}

bool net_mqtt_publish(const uint8_t* data, size_t len) {
    (void)data; (void)len;
    // TODO: integrate with esp-mqtt and configured broker/topic.
    EMB_LOGW("NET", "MQTT publish not implemented");
    return false;
}
bool net_http_post(const uint8_t* data, size_t len) {
    (void)data; (void)len;
    // TODO: integrate with esp_http_client.
    EMB_LOGW("NET", "HTTP POST not implemented");
    return false;
}

}

#endif // ESP_PLATFORM

