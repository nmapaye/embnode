#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "embnode/hal/hal.hpp"
#include "embnode/metrics/power.hpp"
#include "embnode/ota/ota.hpp"
#include "embnode/telemetry/packet.hpp"

namespace {

bool check(bool condition, const char* message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message);
    }
    return condition;
}

bool packet_contracts() {
    using embnode::telemetry::Frame;
    using embnode::telemetry::PacketType;
    Frame frame{};
    frame.hdr.type = PacketType::Metrics;
    frame.payload = {1, 2, 3, 4};
    std::vector<uint8_t> encoded;
    if (!check(embnode::telemetry::encode(frame, encoded), "valid frame must encode")) {
        return false;
    }
    const std::vector<uint8_t> expected = {0xA5, 0x5A, 0x01, 0x02, 0x04, 0x00,
                                           0x01, 0x02, 0x03, 0x04, 0x5D, 0x31};
    if (!check(encoded == expected, "wire bytes must be stable and little-endian")) {
        return false;
    }

    Frame decoded{};
    if (!check(embnode::telemetry::decode(encoded.data(), encoded.size(), decoded),
               "valid frame must decode") ||
        !check(decoded.payload == frame.payload && decoded.hdr.type == PacketType::Metrics,
               "decoded frame must match input")) {
        return false;
    }

    auto corrupted = encoded;
    corrupted[6] ^= 0xFF;
    auto bad_magic = encoded;
    bad_magic[0] = 0;
    auto bad_version = encoded;
    bad_version[2] = 2;
    auto bad_type = encoded;
    bad_type[3] = 99;
    auto trailing = encoded;
    trailing.push_back(0);
    if (!check(!embnode::telemetry::decode(nullptr, encoded.size(), decoded), "null packet must be rejected") ||
        !check(!embnode::telemetry::decode(encoded.data(), encoded.size() - 1, decoded), "truncated packet must be rejected") ||
        !check(!embnode::telemetry::decode(trailing.data(), trailing.size(), decoded), "trailing bytes must be rejected") ||
        !check(!embnode::telemetry::decode(corrupted.data(), corrupted.size(), decoded), "CRC corruption must be rejected") ||
        !check(!embnode::telemetry::decode(bad_magic.data(), bad_magic.size(), decoded), "bad magic must be rejected") ||
        !check(!embnode::telemetry::decode(bad_version.data(), bad_version.size(), decoded), "unsupported version must be rejected") ||
        !check(!embnode::telemetry::decode(bad_type.data(), bad_type.size(), decoded), "unknown packet type must be rejected")) {
        return false;
    }

    frame.payload.assign(65536, 0xAA);
    encoded.assign(1, 0xFF);
    return check(!embnode::telemetry::encode(frame, encoded) && encoded.empty(),
                 "oversized payload must fail without partial output") &&
           check(embnode::telemetry::encode(frame).empty(),
                 "compatibility encoder must return empty output on failure");
}

bool power_contracts() {
    embnode::metrics::reset();
    embnode::hal::sleep_ms(10);
    auto stats = embnode::metrics::duty_stats();
    if (!check(stats.awake_ms == 10 && stats.asleep_ms == 0,
               "time-zero awake interval must be included")) {
        return false;
    }
    embnode::metrics::mark_sleep();
    embnode::metrics::mark_sleep();
    embnode::hal::deep_sleep_ms(30);
    stats = embnode::metrics::duty_stats();
    if (!check(stats.awake_ms == 10 && stats.asleep_ms == 30,
               "active sleep interval must be included")) {
        return false;
    }
    embnode::metrics::mark_awake();
    embnode::metrics::mark_awake();
    embnode::hal::sleep_ms(10);
    stats = embnode::metrics::duty_stats();
    const float duty = embnode::metrics::duty_cycle_percent();
    const float average = embnode::metrics::average_current_ma(20.0F, 0.1F);
    if (!check(stats.awake_ms == 20 && stats.asleep_ms == 30, "repeated transitions must be idempotent") ||
        !check(std::fabs(duty - 40.0F) < 0.001F, "duty cycle must use live intervals") ||
        !check(std::fabs(average - 8.06F) < 0.001F, "average current must weight live intervals")) {
        return false;
    }
    embnode::metrics::reset();
    stats = embnode::metrics::duty_stats();
    return check(stats.awake_ms == 0 && stats.asleep_ms == 0, "reset must establish a clean baseline");
}

uint64_t ota_now = 0;
uint64_t ota_clock() { return ota_now; }

class FakeBackend final : public embnode::ota::Backend {
public:
    bool available = true;
    bool begin_ok = true;
    bool write_ok = true;
    bool finalize_ok = true;
    uint64_t advance_per_write = 0;
    size_t expected_size = 0;
    std::vector<size_t> writes;
    bool finalized = false;
    bool aborted = false;

    bool supported() const noexcept override { return available; }
    bool begin(size_t image_size) override {
        expected_size = image_size;
        return begin_ok;
    }
    bool write(const uint8_t*, size_t len) override {
        writes.push_back(len);
        ota_now += advance_per_write;
        return write_ok;
    }
    bool finalize() override {
        finalized = true;
        return finalize_ok;
    }
    void abort() noexcept override { aborted = true; }
};

bool ota_contracts() {
    using embnode::ota::Result;
    const uint8_t image[] = {1, 2, 3, 4, 5};
    FakeBackend success;
    ota_now = 0;
    if (!check(embnode::ota::perform_ota(success, image, sizeof(image), 100, 2, ota_clock) == Result::Success,
               "fake backend update must succeed") ||
        !check(success.expected_size == 5 && success.writes == std::vector<size_t>({2, 2, 1}) && success.finalized && !success.aborted,
               "OTA coordinator must stream and finalize exactly once")) {
        return false;
    }

    FakeBackend unsupported;
    unsupported.available = false;
    FakeBackend begin_failure;
    begin_failure.begin_ok = false;
    FakeBackend write_failure;
    write_failure.write_ok = false;
    FakeBackend finalize_failure;
    finalize_failure.finalize_ok = false;
    FakeBackend timeout;
    timeout.advance_per_write = 5;
    ota_now = 0;
    return check(embnode::ota::perform_ota(unsupported, image, sizeof(image), 100, 2, ota_clock) == Result::Unsupported,
                 "unsupported backend must fail closed") &&
           check(embnode::ota::perform_ota(begin_failure, image, sizeof(image), 100, 2, ota_clock) == Result::BeginFailed && begin_failure.aborted,
                 "begin failure must abort") &&
           check(embnode::ota::perform_ota(write_failure, image, sizeof(image), 100, 2, ota_clock) == Result::WriteFailed && write_failure.aborted,
                 "write failure must abort") &&
           check(embnode::ota::perform_ota(finalize_failure, image, sizeof(image), 100, 2, ota_clock) == Result::FinalizeFailed && finalize_failure.aborted,
                 "finalize failure must abort") &&
           check(embnode::ota::perform_ota(timeout, image, sizeof(image), 5, 2, ota_clock) == Result::TimedOut && timeout.aborted,
                 "timeout must abort") &&
           check(!embnode::ota::perform_ota(image, sizeof(image), 100),
                 "compatibility wrapper must not claim unsupported success") &&
           check(embnode::ota::perform_ota(success, nullptr, 0, 100, 2, ota_clock) == Result::InvalidImage,
                 "invalid image must be rejected");
}

}  // namespace

int main() {
    return packet_contracts() && power_contracts() && ota_contracts() ? 0 : 1;
}
