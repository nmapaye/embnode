// Telemetry framing + CRC
#pragma once

#include <cstdint>
#include <array>
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

// Serialize into buffer
std::vector<uint8_t> encode(const Frame& f);

// Deserialize from buffer (validates CRC and magic). Returns true on success.
bool decode(const uint8_t* buf, size_t len, Frame& out);

}

