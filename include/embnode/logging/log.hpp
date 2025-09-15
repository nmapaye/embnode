// Minimal logging macros; map to platform or printf
#pragma once

#include <cstdio>

#if defined(ESP_PLATFORM)
#include "esp_log.h"
#define EMB_LOGI(tag, fmt, ...) ESP_LOGI(tag, fmt, ##__VA_ARGS__)
#define EMB_LOGW(tag, fmt, ...) ESP_LOGW(tag, fmt, ##__VA_ARGS__)
#define EMB_LOGE(tag, fmt, ...) ESP_LOGE(tag, fmt, ##__VA_ARGS__)
#else
#define EMB_LOGI(tag, fmt, ...) std::printf("I (%s): " fmt "\n", tag, ##__VA_ARGS__)
#define EMB_LOGW(tag, fmt, ...) std::printf("W (%s): " fmt "\n", tag, ##__VA_ARGS__)
#define EMB_LOGE(tag, fmt, ...) std::printf("E (%s): " fmt "\n", tag, ##__VA_ARGS__)
#endif

