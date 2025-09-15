#include "embnode/ota/ota.hpp"
#include "embnode/logging/log.hpp"
#include "embnode/config/config.hpp"

namespace embnode::ota {

bool perform_ota(const uint8_t* image, size_t len, uint32_t timeout_ms) {
    (void)timeout_ms;
    // Stub: integrate with ESP-IDF OTA or STM32 bootloader here.
    if (!image || len == 0) {
        EMB_LOGE("OTA", "Invalid OTA image");
        return false;
    }
    EMB_LOGI("OTA", "Received OTA image len=%u (stub)", (unsigned)len);
    return true;
}

}

