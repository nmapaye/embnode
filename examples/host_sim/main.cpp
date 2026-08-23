#include <cstdio>
#include <vector>
#include "embnode/metrics/power.hpp"
#include "embnode/hal/hal.hpp"
#include "embnode/telemetry/packet.hpp"
#include "embnode/config/config.hpp"

using namespace embnode;

int main() {
    // CRC known-vector test ("123456789" -> 0x29B1 for CCITT-FFFF)
    {
        static const uint8_t v[] = { '1','2','3','4','5','6','7','8','9' };
        uint16_t crc = telemetry::crc16_ccitt(v, sizeof(v));
        std::printf("CRC16-CCITT('123456789') = 0x%04X (expected 0x29B1)\n", crc);
        if (crc != 0x29B1) return 10;
    }

    // Simulate awake/sleep cycles and verify duty cycle and average current
    metrics::mark_awake();
    hal::sleep_ms(config::kActiveWindowMs);
    metrics::mark_sleep();
    hal::deep_sleep_ms(config::kSleepWindowMs);
    metrics::mark_awake();

    float dc = metrics::duty_cycle_percent();
    float iavg = metrics::average_current_ma(config::kAwakeCurrent_mA, config::kSleepCurrent_mA);
    std::printf("Duty cycle: %.2f%%\n", dc);
    std::printf("Avg current (modeled): %.2f mA\n", iavg);

    // Build a telemetry frame and round-trip encode/decode
    telemetry::Frame f{};
    f.hdr.type = telemetry::PacketType::Samples;
    f.payload = {1,2,3,4,5,6,7,8};
    auto buf = telemetry::encode(f);

    telemetry::Frame out{};
    bool ok = telemetry::decode(buf.data(), buf.size(), out);
    std::printf("Telemetry decode: %s\n", ok ? "OK" : "FAIL");
    if (!ok) return 1;

    // Simple content check
    bool same = out.payload == f.payload && out.hdr.type == f.hdr.type;
    std::printf("Payload match: %s\n", same ? "YES" : "NO");
    return same ? 0 : 2;
}
