#include "core/paths.hpp"
#include "core/config.hpp"
#include "ui/main_window.hpp"
#include <fstream>
#include <exception>
#ifdef _WIN32
#include <windows.h>
#endif

static void writeFatalLog(const std::string& message){
  try{
    auto paths=maine::core::Paths::create();
    paths.ensure();
    std::ofstream log(paths.logs/"launcher.log",std::ios::app);
    log<<"FATAL: "<<message<<"\n";
  }catch(...){}
}

int main() {
  try{
    auto paths=maine::core::Paths::create();
    paths.ensure();
    auto config=maine::core::Config::load(paths.config/"settings.json");
    return maine::ui::runMainWindow(paths,config);
  }catch(const std::exception& ex){
    writeFatalLog(ex.what());
#ifdef _WIN32
    MessageBoxA(nullptr,ex.what(),"Maine Client - Fatal Error",MB_OK|MB_ICONERROR);
#endif
    return 1;
  }catch(...){
    writeFatalLog("Unknown fatal error");
#ifdef _WIN32
    MessageBoxA(nullptr,"Unknown fatal error. See logs\\launcher.log","Maine Client - Fatal Error",MB_OK|MB_ICONERROR);
#endif
    return 1;
  }
}
