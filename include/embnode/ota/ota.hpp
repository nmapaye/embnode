// OTA update stub
#pragma once

#include <cstddef>
#include <cstdint>

namespace embnode::ota {

// Begin OTA update with a data stream. Platform should handle image verification and swap.
// Returns true on success, false on timeout/failure.
bool perform_ota(const uint8_t* image, size_t len, uint32_t timeout_ms);

}

