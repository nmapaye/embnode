#include "embnode/ota/ota.hpp"

#include <algorithm>

#include "embnode/hal/hal.hpp"
#include "embnode/logging/log.hpp"

namespace embnode::ota {
namespace {

uint64_t default_clock() {
    return embnode::hal::millis();
}

bool timed_out(uint64_t started_at, uint32_t timeout_ms, Clock clock) {
    const uint64_t now = clock();
    return now < started_at || now - started_at >= timeout_ms;
}

class UnsupportedBackend final : public Backend {
public:
    bool supported() const noexcept override { return false; }
    bool begin(size_t) override { return false; }
    bool write(const uint8_t*, size_t) override { return false; }
    bool finalize() override { return false; }
    void abort() noexcept override {}
};

}  // namespace

Result perform_ota(Backend& backend, const uint8_t* image, size_t len,
                   uint32_t timeout_ms, size_t chunk_size, Clock clock) {
    if (image == nullptr || len == 0 || chunk_size == 0) {
        return Result::InvalidImage;
    }
    if (!backend.supported()) {
        return Result::Unsupported;
    }
    if (timeout_ms == 0) {
        return Result::TimedOut;
    }
    if (clock == nullptr) {
        clock = default_clock;
    }
    const uint64_t started_at = clock();
    if (!backend.begin(len)) {
        backend.abort();
        return Result::BeginFailed;
    }
    if (timed_out(started_at, timeout_ms, clock)) {
        backend.abort();
        return Result::TimedOut;
    }
    for (size_t offset = 0; offset < len;) {
        const size_t write_size = std::min(chunk_size, len - offset);
        if (!backend.write(image + offset, write_size)) {
            backend.abort();
            return Result::WriteFailed;
        }
        offset += write_size;
        if (timed_out(started_at, timeout_ms, clock)) {
            backend.abort();
            return Result::TimedOut;
        }
    }
    if (!backend.finalize()) {
        backend.abort();
        return Result::FinalizeFailed;
    }
    if (timed_out(started_at, timeout_ms, clock)) {
        backend.abort();
        return Result::TimedOut;
    }
    return Result::Success;
}

bool perform_ota(const uint8_t* image, size_t len, uint32_t timeout_ms) {
    UnsupportedBackend backend;
    const Result result = perform_ota(backend, image, len, timeout_ms);
    if (result == Result::Unsupported) {
        EMB_LOGE("OTA", "No platform OTA backend is configured");
    }
    return result == Result::Success;
}

}  // namespace embnode::ota
