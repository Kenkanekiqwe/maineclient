#pragma once
#include <cstdint>
#include <string>
namespace maine::core {
struct HardwareInfo { std::string cpuName,gpuName; std::uint32_t logicalProcessors=1; std::uint64_t totalRamMb=0; };
HardwareInfo detectHardware();
}
