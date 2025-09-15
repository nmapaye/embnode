// RAII FreeRTOS timer wrapper
#pragma once

#include <cstdint>
#include <functional>

extern "C" {
#include "FreeRTOS.h"
#include "timers.h"
}

namespace embnode::rtos {

class UniqueTimer {
public:
    using Callback = std::function<void()>;

    UniqueTimer() = default;
    UniqueTimer(const char* name, TickType_t periodTicks, bool autoReload, Callback cb)
        : cb_(std::move(cb)) {
        h_ = xTimerCreate(name, periodTicks, autoReload ? pdTRUE : pdFALSE, this, &UniqueTimer::tramp_);
    }
    ~UniqueTimer() { if (h_) xTimerDelete(h_, 0); }
    UniqueTimer(UniqueTimer&& o) noexcept { move_from_(o); }
    UniqueTimer& operator=(UniqueTimer&& o) noexcept { if(this!=&o){ if(h_) xTimerDelete(h_,0); move_from_(o);} return *this; }
    UniqueTimer(const UniqueTimer&) = delete;
    UniqueTimer& operator=(const UniqueTimer&) = delete;

    bool start(TickType_t blockTicks = 0) { return h_ && xTimerStart(h_, blockTicks) == pdPASS; }
    bool stop(TickType_t blockTicks = 0) { return h_ && xTimerStop(h_, blockTicks) == pdPASS; }
    bool reset(TickType_t blockTicks = 0) { return h_ && xTimerReset(h_, blockTicks) == pdPASS; }
    bool changePeriod(TickType_t ticks, TickType_t blockTicks = 0) { return h_ && xTimerChangePeriod(h_, ticks, blockTicks) == pdPASS; }
    TimerHandle_t handle() const { return h_; }

private:
    static void tramp_(TimerHandle_t timer) {
        auto* self = static_cast<UniqueTimer*>(pvTimerGetTimerID(timer));
        if (self && self->cb_) self->cb_();
    }
    void move_from_(UniqueTimer& o) { h_ = o.h_; cb_ = std::move(o.cb_); vTimerSetTimerID(h_, this); o.h_ = nullptr; }

    TimerHandle_t h_ {nullptr};
    Callback cb_ {};
};

} // namespace embnode::rtos

