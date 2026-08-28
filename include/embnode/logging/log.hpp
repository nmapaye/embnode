// Minimal logging macros; map to platform or printf
#pragma once

#include <cstdio>

#if defined(ESP_PLATFORM)
#include "esp_log.h"
#define EMB_LOGI(...) ESP_LOGI(__VA_ARGS__)
#define EMB_LOGW(...) ESP_LOGW(__VA_ARGS__)
#define EMB_LOGE(...) ESP_LOGE(__VA_ARGS__)
#else
namespace embnode::logging {

inline void log(const char* level, const char* tag, const char* message) {
    std::printf("%s (%s): %s\n", level, tag, message);
}

template <typename... Args>
void log(const char* level, const char* tag, const char* format, Args... args) {
    std::printf("%s (%s): ", level, tag);
    std::printf(format, args...);
    std::printf("\n");
}

}  // namespace embnode::logging

#define EMB_LOGI(...) ::embnode::logging::log("I", __VA_ARGS__)
#define EMB_LOGW(...) ::embnode::logging::log("W", __VA_ARGS__)
#define EMB_LOGE(...) ::embnode::logging::log("E", __VA_ARGS__)
#endif
