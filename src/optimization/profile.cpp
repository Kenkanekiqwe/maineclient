#include "profile.hpp"
#include <algorithm>
namespace maine::optimization {
Profile buildProfile(const maine::core::HardwareInfo& h,Preset preset) {
  Profile p; p.preset=preset;
  p.workerThreads=std::clamp((int)h.logicalProcessors-2,1,12);
  if(preset==Preset::Balanced){p.renderDistance=12;p.simulationDistance=8;p.particles=true;}
  else if(preset==Preset::Quality){p.renderDistance=20;p.simulationDistance=12;p.particles=true;p.entityCulling=false;}
  else if(preset==Preset::Performance){p.renderDistance=8;p.simulationDistance=5;p.particles=false;}
  return p;
}
std::string presetName(Preset p) {
  switch(p){case Preset::Performance:return "Performance";case Preset::Balanced:return "Balanced";case Preset::Quality:return "Quality";case Preset::Custom:return "Custom";}
  return "Unknown";
}
}
