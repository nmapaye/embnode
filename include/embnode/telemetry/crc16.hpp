// CRC16-CCITT (0x1021) impl
#pragma once
#include <cstddef>
#include <cstdint>

namespace embnode::telemetry {

uint16_t crc16_ccitt(const uint8_t* data, size_t len, uint16_t seed = 0xFFFF);

}

