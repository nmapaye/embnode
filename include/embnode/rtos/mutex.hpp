// RAII Mutex wrapper for FreeRTOS
#pragma once

extern "C" {
#include "FreeRTOS.h"
#include "semphr.h"
}

namespace embnode::rtos {

class Mutex {
public:
    Mutex() { h_ = xSemaphoreCreateMutex(); }
    ~Mutex() { if (h_) vSemaphoreDelete(h_); }
    Mutex(Mutex&& o) noexcept { h_ = o.h_; o.h_ = nullptr; }
    Mutex& operator=(Mutex&& o) noexcept { if (this!=&o){ if(h_) vSemaphoreDelete(h_); h_=o.h_; o.h_=nullptr;} return *this; }
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    void lock() { xSemaphoreTake(h_, portMAX_DELAY); }
    void unlock() { xSemaphoreGive(h_); }
    bool try_lock() { return xSemaphoreTake(h_, 0) == pdTRUE; }

    class Guard {
    public:
        explicit Guard(Mutex& m): m_(m) { m_.lock(); }
        ~Guard() { m_.unlock(); }
        Guard(const Guard&) = delete; Guard& operator=(const Guard&) = delete;
    private: Mutex& m_;
    };

    bool valid() const { return h_ != nullptr; }

private:
    SemaphoreHandle_t h_ {nullptr};
};

} // namespace embnode::rtos

