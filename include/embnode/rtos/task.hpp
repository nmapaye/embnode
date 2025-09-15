// RAII Task wrapper for FreeRTOS
#pragma once

#include <cstdint>
#include <functional>

extern "C" {
#include "FreeRTOS.h"
#include "task.h"
}

namespace embnode::rtos {

class Task {
public:
    using Entry = std::function<void()>;

    Task() = default;

    Task(const char* name,
         UBaseType_t priority,
         uint32_t stackWords,
         Entry entry,
         BaseType_t coreAffinity = tskNO_AFFINITY)
        : entry_(std::move(entry)) {
        create_(name, priority, stackWords, coreAffinity);
    }

    ~Task() { destroy_(); }

    Task(Task&& other) noexcept { move_from_(other); }
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            destroy_();
            move_from_(other);
        }
        return *this;
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    bool valid() const { return handle_ != nullptr; }
    TaskHandle_t handle() const { return handle_; }

    void suspend() { if (handle_) vTaskSuspend(handle_); }
    void resume() { if (handle_) vTaskResume(handle_); }
    void notifyGive() { if (handle_) vTaskNotifyGive(handle_); }

    // Wait for notification with timeout in ticks
    uint32_t waitNotify(uint32_t ticks = portMAX_DELAY) {
        return ulTaskNotifyTake(pdTRUE, ticks);
    }

private:
    static void tramp_(void* arg) {
        Task* self = static_cast<Task*>(arg);
        if (self && self->entry_) self->entry_();
        vTaskDelete(nullptr);
    }

    void create_(const char* name,
                 UBaseType_t priority,
                 uint32_t stackWords,
                 BaseType_t coreAffinity) {
#if (defined(ESP_PLATFORM))
        BaseType_t ok = xTaskCreatePinnedToCore(
            &Task::tramp_, name, stackWords, this, priority, &handle_, coreAffinity);
        if (ok != pdPASS) handle_ = nullptr;
#else
        (void)coreAffinity;
        BaseType_t ok = xTaskCreate(&Task::tramp_, name, stackWords, this, priority, &handle_);
        if (ok != pdPASS) handle_ = nullptr;
#endif
    }

    void destroy_() {
        // Only delete if not current task to avoid deleting self incorrectly.
        if (handle_ && xTaskGetCurrentTaskHandle() != handle_) {
            vTaskDelete(handle_);
        }
        handle_ = nullptr;
    }

    void move_from_(Task& o) {
        handle_ = o.handle_;
        entry_ = std::move(o.entry_);
        o.handle_ = nullptr;
    }

    TaskHandle_t handle_ {nullptr};
    Entry entry_ {};
};

} // namespace embnode::rtos
