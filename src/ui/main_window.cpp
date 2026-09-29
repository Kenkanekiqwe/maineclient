#include "main_window.hpp"
#include "../core/hardware.hpp"
#include "../core/java_runtime.hpp"
#include "../minecraft/version_downloader.hpp"
#include "../minecraft/launcher.hpp"
#include "../optimization/optimizer.hpp"
#ifdef _WIN32
#include <windows.h>
#endif
#include <sstream>
namespace maine::ui {
int runMainWindow(const maine::core::Paths& paths,maine::core::Config& config) {
  auto h=maine::core::detectHardware(); auto profile=maine::optimization::Optimizer{}.analyze(h);
#ifdef _WIN32
  auto java=maine::core::JavaRuntimeManager(paths).detect();
  std::ostringstream b;
  b<<"Maine Client\n\nMinecraft: "<<config.selectedVersion<<"\nLoader: "<<config.selectedLoader
   <<"\nInstance: "<<config.selectedInstance<<"\nAccount: "<<config.selectedAccount<<" (offline)\n\nCPU: "<<h.cpuName
   <<"\nThreads: "<<h.logicalProcessors<<"\nRAM: "<<h.totalRamMb<<" MB\nGPU: "<<h.gpuName
   <<"\nJava: "<<(java.executable.empty()?"not found":java.executable.string())
   <<"\n\n"<<profile.summary<<"\n\nPreparing Minecraft metadata...";
  std::string error; maine::minecraft::MinecraftLauncher launcher(paths,config);
  maine::minecraft::LaunchRequest account; account.username=config.selectedAccount;
  if(!launcher.prepare(error)) b<<"\n\nDownload error: "<<error;
  else b<<"\n\nMinecraft metadata ready.";
  MessageBoxA(nullptr,b.str().c_str(),"Maine Client",MB_OK|MB_ICONINFORMATION);
#endif
  config.save(paths.config/"settings.json"); return 0;
}
}
