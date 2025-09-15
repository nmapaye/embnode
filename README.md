Embedded Node on FreeRTOS (ESP32/STM32)

Scope
- ESP32/STM32 sensor node: periodic sampling via DMA, task scheduling, queue to comms task, OTA updates, deep-sleep duty cycling.

Implementation checkpoints
- FreeRTOS tasks: sampler (priority high), aggregator, comms (MQTT/HTTP), watchdog.
- C++ wrappers: Task, Mutex, UniqueTimer, BoundedQueue providing RAII semantics over the C API.
- Power profiling: deep sleep hooks, wake latency; jitter budget for sampler.
- Telemetry packet framing + CRC; backpressure handling.

Benchmarks to publish (targets)
- Sampler jitter ≤ 1 ms at 100 Hz, sustained 24h.
- Average current draw ≤ X mA at Y% duty cycle; OTA < 2 min.

Notes
- This repository is scaffold code intended to drop into an ESP-IDF or STM32CubeMX + FreeRTOS environment.
- Hardware-specific hooks live under `include/embnode/hal` and can be implemented per-target.
- Logging defaults to `printf` if platform logging is unavailable.

Quick testing
- Host simulator (no FreeRTOS):
  - Configure and build: `cmake -S embnode -B embnode/build -DEMBNODE_BUILD_FREERTOS_LIB=OFF -DEMBNODE_BUILD_HOST_SIM=ON && cmake --build embnode/build`
  - Run: `./embnode/build/embnode_host_sim`
  - It exercises telemetry encode/decode and duty-cycle power math using the stub HAL.
- ESP-IDF (ESP32):
  - Option A: Use the provided component in `embnode/idf_component/` by copying it into your project's `components/embnode/` folder.
  - Option B: Add this folder as a subdir and call `idf_component_register` yourself.
  - Ensure FreeRTOS headers are available (provided by ESP-IDF) and build with `idf.py build flash monitor`.
  - `app_main()` in `src/main.cpp` starts sampler/aggregator/comms/watchdog tasks.
  - Jitter logs: sampler reports avg/max absolute jitter every minute; warnings when exceeding 1 ms budget.
- STM32 (CubeIDE + FreeRTOS):
  - Add `embnode/include` to include paths and needed `src/*` files to your project.
  - Provide `hal` implementations for DMA sampler, network, and deep sleep.
  - Start `app_main()` from a CMSIS-RTOS thread after scheduler init.

CI
- GitHub Actions workflow builds and runs the host simulator test on pushes/PRs: `.github/workflows/ci.yml`.
# embnode
