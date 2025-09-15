// BoundedQueue with backpressure policies over FreeRTOS Queue
#pragma once

#include <cstdint>

extern "C" {
#include "FreeRTOS.h"
#include "queue.h"
}

namespace embnode::rtos {

enum class BackpressurePolicy {
    Block,       // Block producer until space
    DropNewest,  // Drop the pushed item when full
    DropOldest   // Overwrite oldest item when full
};

template <typename T>
class BoundedQueue {
public:
    BoundedQueue() = default;
    BoundedQueue(size_t capacity, BackpressurePolicy policy)
        : policy_(policy) {
        q_ = xQueueCreate(capacity, sizeof(T));
    }
    ~BoundedQueue() { if (q_) vQueueDelete(q_); }
    BoundedQueue(BoundedQueue&& o) noexcept { q_ = o.q_; policy_ = o.policy_; o.q_ = nullptr; }
    BoundedQueue& operator=(BoundedQueue&& o) noexcept { if(this!=&o){ if(q_) vQueueDelete(q_); q_=o.q_; policy_=o.policy_; o.q_=nullptr;} return *this; }
    BoundedQueue(const BoundedQueue&) = delete;
    BoundedQueue& operator=(const BoundedQueue&) = delete;

    bool push(const T& item, TickType_t to = portMAX_DELAY) {
        if (!q_) return false;
        switch (policy_) {
            case BackpressurePolicy::Block:
                return xQueueSend(q_, &item, to) == pdTRUE;
            case BackpressurePolicy::DropNewest:
                return xQueueSend(q_, &item, 0) == pdTRUE; // silently drop on full
            case BackpressurePolicy::DropOldest: {
                if (xQueueSend(q_, &item, 0) == pdTRUE) return true;
                // remove oldest and try again
                T tmp; xQueueReceive(q_, &tmp, 0);
                return xQueueSend(q_, &item, 0) == pdTRUE;
            }
        }
        return false;
    }

    // ISR-safe push variant. Returns true if enqueued. If a higher priority
    // task should be woken, sets out_woken to true so caller may yield.
    bool push_from_isr(const T& item, bool* out_woken = nullptr) {
        if (!q_) return false;
        BaseType_t woken = pdFALSE;
        bool ok = xQueueSendFromISR(q_, &item, &woken) == pdTRUE;
        if (out_woken) *out_woken = (woken == pdTRUE);
        return ok;
    }

    bool pop(T& out, TickType_t to = portMAX_DELAY) {
        return q_ && xQueueReceive(q_, &out, to) == pdTRUE;
    }

    size_t size() const { return q_ ? uxQueueMessagesWaiting(q_) : 0; }
    size_t capacity() const { return q_ ? uxQueueSpacesAvailable(q_) + uxQueueMessagesWaiting(q_) : 0; }
    QueueHandle_t handle() const { return q_; }

private:
    QueueHandle_t q_ {nullptr};
    BackpressurePolicy policy_ {BackpressurePolicy::Block};
};

} // namespace embnode::rtos
