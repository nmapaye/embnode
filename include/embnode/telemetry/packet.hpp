// Telemetry framing + CRC
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include "embnode/telemetry/crc16.hpp"

namespace embnode::telemetry {

enum class PacketType : uint8_t {
    Samples = 1,
    Metrics = 2,
    Log = 3,
};

struct FrameHeader {
    uint8_t magic1 {0xA5};
    uint8_t magic2 {0x5A};
    uint8_t version {1};
    PacketType type {PacketType::Samples};
    uint16_t length {0}; // payload length
} __attribute__((packed));

struct Frame {
    FrameHeader hdr {};
    std::vector<uint8_t> payload; // arbitrary payload
    uint16_t crc {0};
};

// Serialize into a stable little-endian wire format. Returns false and clears
// out when the header or payload cannot be represented.
bool encode(const Frame& f, std::vector<uint8_t>& out);

// Compatibility wrapper. Returns an empty buffer on validation failure.
std::vector<uint8_t> encode(const Frame& f);

// Deserialize an exact frame (validates version, type, length, CRC, and magic).
bool decode(const uint8_t* buf, size_t len, Frame& out);

}
