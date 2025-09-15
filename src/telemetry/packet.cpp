#include "embnode/telemetry/packet.hpp"
#include <cstring>

namespace embnode::telemetry {

std::vector<uint8_t> encode(const Frame& f) {
    std::vector<uint8_t> buf;
    FrameHeader hdr = f.hdr;
    hdr.length = static_cast<uint16_t>(f.payload.size());
    buf.resize(sizeof(FrameHeader) + hdr.length + sizeof(uint16_t));
    std::memcpy(buf.data(), &hdr, sizeof(hdr));
    if (hdr.length)
        std::memcpy(buf.data() + sizeof(hdr), f.payload.data(), hdr.length);
    uint16_t crc = crc16_ccitt(buf.data(), sizeof(hdr) + hdr.length);
    std::memcpy(buf.data() + sizeof(hdr) + hdr.length, &crc, sizeof(crc));
    return buf;
}

bool decode(const uint8_t* buf, size_t len, Frame& out) {
    if (!buf || len < sizeof(FrameHeader) + sizeof(uint16_t)) return false;
    FrameHeader hdr;
    std::memcpy(&hdr, buf, sizeof(hdr));
    if (hdr.magic1 != 0xA5 || hdr.magic2 != 0x5A) return false;
    size_t total = sizeof(hdr) + hdr.length + sizeof(uint16_t);
    if (len < total) return false;
    uint16_t crc;
    std::memcpy(&crc, buf + sizeof(hdr) + hdr.length, sizeof(crc));
    uint16_t calc = crc16_ccitt(buf, sizeof(hdr) + hdr.length);
    if (crc != calc) return false;

    out.hdr = hdr;
    out.payload.assign(buf + sizeof(hdr), buf + sizeof(hdr) + hdr.length);
    out.crc = crc;
    return true;
}

}

