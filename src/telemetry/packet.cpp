#include "embnode/telemetry/packet.hpp"

#include <limits>
#include <utility>

namespace embnode::telemetry {
namespace {

constexpr size_t kHeaderSize = 6;
constexpr size_t kCrcSize = 2;

bool valid_type(PacketType type) {
    switch (type) {
        case PacketType::Samples:
        case PacketType::Metrics:
        case PacketType::Log:
            return true;
    }
    return false;
}

void put_u16_le(std::vector<uint8_t>& buffer, size_t offset, uint16_t value) {
    buffer[offset] = static_cast<uint8_t>(value & 0xFFU);
    buffer[offset + 1] = static_cast<uint8_t>((value >> 8U) & 0xFFU);
}

uint16_t get_u16_le(const uint8_t* buffer, size_t offset) {
    return static_cast<uint16_t>(buffer[offset]) |
           static_cast<uint16_t>(static_cast<uint16_t>(buffer[offset + 1]) << 8U);
}

}  // namespace

bool encode(const Frame& frame, std::vector<uint8_t>& out) {
    out.clear();
    if (frame.hdr.magic1 != 0xA5 || frame.hdr.magic2 != 0x5A || frame.hdr.version != 1 ||
        !valid_type(frame.hdr.type) || frame.payload.size() > std::numeric_limits<uint16_t>::max()) {
        return false;
    }

    const auto payload_size = static_cast<uint16_t>(frame.payload.size());
    out.resize(kHeaderSize + payload_size + kCrcSize);
    out[0] = frame.hdr.magic1;
    out[1] = frame.hdr.magic2;
    out[2] = frame.hdr.version;
    out[3] = static_cast<uint8_t>(frame.hdr.type);
    put_u16_le(out, 4, payload_size);
    for (size_t index = 0; index < frame.payload.size(); ++index) {
        out[kHeaderSize + index] = frame.payload[index];
    }
    const uint16_t crc = crc16_ccitt(out.data(), kHeaderSize + payload_size);
    put_u16_le(out, kHeaderSize + payload_size, crc);
    return true;
}

std::vector<uint8_t> encode(const Frame& frame) {
    std::vector<uint8_t> out;
    (void)encode(frame, out);
    return out;
}

bool decode(const uint8_t* buffer, size_t len, Frame& out) {
    if (buffer == nullptr || len < kHeaderSize + kCrcSize) {
        return false;
    }
    const auto type = static_cast<PacketType>(buffer[3]);
    if (buffer[0] != 0xA5 || buffer[1] != 0x5A || buffer[2] != 1 || !valid_type(type)) {
        return false;
    }
    const uint16_t payload_size = get_u16_le(buffer, 4);
    const size_t expected_size = kHeaderSize + static_cast<size_t>(payload_size) + kCrcSize;
    if (len != expected_size) {
        return false;
    }
    const uint16_t stored_crc = get_u16_le(buffer, kHeaderSize + payload_size);
    const uint16_t calculated_crc = crc16_ccitt(buffer, kHeaderSize + payload_size);
    if (stored_crc != calculated_crc) {
        return false;
    }

    Frame decoded{};
    decoded.hdr.magic1 = buffer[0];
    decoded.hdr.magic2 = buffer[1];
    decoded.hdr.version = buffer[2];
    decoded.hdr.type = type;
    decoded.hdr.length = payload_size;
    decoded.payload.assign(buffer + kHeaderSize, buffer + kHeaderSize + payload_size);
    decoded.crc = stored_crc;
    out = std::move(decoded);
    return true;
}

}  // namespace embnode::telemetry
