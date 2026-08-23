#pragma once

#include <cstddef>
#include <cstdint>

namespace embnode::ota {

enum class Result {
    Success,
    InvalidImage,
    Unsupported,
    BeginFailed,
    WriteFailed,
    FinalizeFailed,
    TimedOut,
};

class Backend {
public:
    virtual ~Backend() = default;
    virtual bool supported() const noexcept = 0;
    virtual bool begin(size_t image_size) = 0;
    virtual bool write(const uint8_t* data, size_t len) = 0;
    virtual bool finalize() = 0;
    virtual void abort() noexcept = 0;
};

using Clock = uint64_t (*)();

// Coordinates a platform backend without defining flash or bootloader behavior.
Result perform_ota(Backend& backend, const uint8_t* image, size_t len,
                   uint32_t timeout_ms, size_t chunk_size = 1024,
                   Clock clock = nullptr);

// Compatibility wrapper. It maps only a completed platform update to true.
bool perform_ota(const uint8_t* image, size_t len, uint32_t timeout_ms);

}  // namespace embnode::ota
