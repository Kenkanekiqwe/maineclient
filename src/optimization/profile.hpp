#pragma once
#include "../core/hardware.hpp"
#include <string>
namespace maine::optimization {
enum class Preset { Performance,Balanced,Quality,Custom };
struct Profile {
  Preset preset=Preset::Performance;
  int renderDistance=8,simulationDistance=5,maxFps=0,workerThreads=1;
  bool particles=false,entityCulling=true,dynamicChunkPriority=true,aggressiveMemoryReuse=true;
};
Profile buildProfile(const maine::core::HardwareInfo&,Preset);
std::string presetName(Preset);
}
