#include "main_window.hpp"
#include "../core/hardware.hpp"
#include "../optimization/optimizer.hpp"
#ifdef _WIN32
#include <windows.h>
#endif
#include <sstream>
namespace maine::ui {
int runMainWindow(const maine::core::Paths& paths,maine::core::Config& config) {
  auto h=maine::core::detectHardware(); auto r=maine::optimization::Optimizer{}.analyze(h);
#ifdef _WIN32
  std::ostringstream b;
  b<<"Maine Client\n\nMinecraft: "<<config.selectedVersion
   <<"\nLoader: "<<config.selectedLoader<<"\n\nCPU: "<<h.cpuName
   <<"\nThreads: "<<h.logicalProcessors<<"\nRAM: "<<h.totalRamMb<<" MB\nGPU: "<<h.gpuName
   <<"\n\n"<<r.summary<<"\n\nBootstrap initialized successfully.";
  MessageBoxA(nullptr,b.str().c_str(),"Maine Client",MB_OK|MB_ICONINFORMATION);
#endif
  config.save(paths.config/"settings.json"); return 0;
}
}
