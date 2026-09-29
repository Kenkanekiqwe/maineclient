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
#ifdef _WIN32
extern "C" void MaineSetCrashStage(const char* stage);
#endif
namespace maine::ui {
int runMainWindow(const maine::core::Paths& paths,maine::core::Config& config) {
  #ifdef _WIN32
  MaineSetCrashStage("detectHardware");
  #endif
  auto h=maine::core::detectHardware();
  #ifdef _WIN32
  MaineSetCrashStage("Optimizer::analyze");
  #endif
  auto profile=maine::optimization::Optimizer{}.analyze(h);
#ifdef _WIN32
  MaineSetCrashStage("JavaRuntimeManager::detect");
  auto java=maine::core::JavaRuntimeManager(paths).detect();
  MaineSetCrashStage("Build UI text");
  std::ostringstream b;
  b<<"Maine Client\n\nMinecraft: "<<config.selectedVersion<<"\nLoader: "<<config.selectedLoader
   <<"\nInstance: "<<config.selectedInstance<<"\nAccount: "<<config.selectedAccount<<" (offline)\n\nCPU: "<<h.cpuName
   <<"\nThreads: "<<h.logicalProcessors<<"\nRAM: "<<h.totalRamMb<<" MB\nGPU: "<<h.gpuName
   <<"\nJava: "<<(java.executable.empty()?"not found":java.executable.string())
   <<"\n\n"<<profile.summary<<"\n\nPreparing Minecraft metadata...";
  MaineSetCrashStage("MinecraftLauncher::construct");
  std::string error; maine::minecraft::MinecraftLauncher launcher(paths,config);
  maine::minecraft::LaunchRequest account; account.username=config.selectedAccount;
  MaineSetCrashStage("MinecraftLauncher::prepare");
  if(!launcher.prepare(error)) b<<"\n\nDownload error: "<<error;
  else { MaineSetCrashStage("MinecraftLauncher::launch"); if(!launcher.launch(account,error)) b<<"\n\nLaunch error: "<<error;
  b<<"\n\nMinecraft launched."; }
  MaineSetCrashStage("MessageBox");
  MessageBoxA(nullptr,b.str().c_str(),"Maine Client",MB_OK|MB_ICONINFORMATION);
#endif
  config.save(paths.config/"settings.json"); return 0;
}
}
