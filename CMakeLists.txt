cmake_minimum_required(VERSION 3.15)
project(embnode LANGUAGES C CXX)

option(EMBNODE_TARGET_ESP32 "Build for ESP32 (ESP-IDF)" OFF)
option(EMBNODE_TARGET_STM32 "Build for STM32" OFF)
option(EMBNODE_BUILD_FREERTOS_LIB "Build the FreeRTOS-based library" ON)
option(EMBNODE_BUILD_HOST_SIM "Build host-only simulator/tests" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(EMBNODE_BUILD_FREERTOS_LIB)
  add_library(embnode
      src/main.cpp
      src/telemetry/crc16.cpp
      src/telemetry/packet.cpp
      src/tasks/sampler_task.cpp
      src/tasks/aggregator_task.cpp
      src/tasks/comms_task.cpp
      src/tasks/watchdog_task.cpp
      src/ota/ota.cpp
      src/hal/hal_stub.cpp
      src/metrics/power.cpp
  )

  target_include_directories(embnode PUBLIC
      ${CMAKE_CURRENT_LIST_DIR}/include
  )

  target_compile_definitions(embnode PUBLIC
      $<$<BOOL:${EMBNODE_TARGET_ESP32}>:EMBNODE_TARGET_ESP32>
      $<$<BOOL:${EMBNODE_TARGET_STM32}>:EMBNODE_TARGET_STM32>
  )

  # Expect FreeRTOS headers available in the toolchain environment
  # Consumers should link FreeRTOS kernel and platform SDK separately.
endif()

if(EMBNODE_BUILD_HOST_SIM)
  add_executable(embnode_host_sim
      examples/host_sim/main.cpp
      src/telemetry/crc16.cpp
      src/telemetry/packet.cpp
      src/ota/ota.cpp
      src/hal/hal_stub.cpp
      src/metrics/power.cpp
  )
  target_include_directories(embnode_host_sim PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
  enable_testing()
  add_test(NAME host_sim COMMAND embnode_host_sim)
endif()
