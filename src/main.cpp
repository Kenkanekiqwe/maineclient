#include "core/paths.hpp"
#include "core/config.hpp"
#include "ui/main_window.hpp"
#include <fstream>
#include <exception>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
static LONG WINAPI crashHandler(EXCEPTION_POINTERS* info){
  const DWORD code=info&&info->ExceptionRecord?info->ExceptionRecord->ExceptionCode:0;
  const ULONG_PTR address=info&&info->ExceptionRecord?
    reinterpret_cast<ULONG_PTR>(info->ExceptionRecord->ExceptionAddress):0;

  char temp[MAX_PATH]{};
  DWORD n=GetTempPathA(MAX_PATH,temp);
  std::string path=(n&&n<MAX_PATH)?std::string(temp)+"MaineClient_crash.log":"MaineClient_crash.log";

  char line[512]{};
  sprintf_s(line,"MaineClient crashed. Exception=0x%08lX Address=0x%p\r\n",
            static_cast<unsigned long>(code),
            reinterpret_cast<void*>(address));

  HANDLE file=CreateFileA(path.c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,
                          CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
  if(file!=INVALID_HANDLE_VALUE){
    DWORD written=0;
    WriteFile(file,line,static_cast<DWORD>(strlen(line)),&written,nullptr);
    CloseHandle(file);
  }

  MessageBoxA(nullptr,line,"Maine Client - Crash",MB_OK|MB_ICONERROR);
  return EXCEPTION_EXECUTE_HANDLER;
}
#endif

static void writeFatalLog(const std::string& message){
  try{
    auto paths=maine::core::Paths::create();
    paths.ensure();
    std::ofstream log(paths.logs/"launcher.log",std::ios::app);
    log<<"FATAL: "<<message<<"\n";
  }catch(...){}
}

int main(){
#ifdef _WIN32
  SetUnhandledExceptionFilter(crashHandler);
#endif

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
