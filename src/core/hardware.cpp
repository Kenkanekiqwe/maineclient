#include "hardware.hpp"
#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <cstring>
#endif
namespace maine::core {
HardwareInfo detectHardware() {
  HardwareInfo h;
#ifdef _WIN32
  SYSTEM_INFO si{}; GetSystemInfo(&si); h.logicalProcessors=si.dwNumberOfProcessors;
  MEMORYSTATUSEX mem{}; mem.dwLength=sizeof(mem);
  if(GlobalMemoryStatusEx(&mem)) h.totalRamMb=mem.ullTotalPhys/(1024ull*1024ull);
  char brand[49]{}; int regs[4]{};
  __cpuid(regs,0x80000000); unsigned maxLeaf=(unsigned)regs[0];
  if(maxLeaf>=0x80000004) {
    __cpuid(regs,0x80000002); std::memcpy(brand,regs,16);
    __cpuid(regs,0x80000003); std::memcpy(brand+16,regs,16);
    __cpuid(regs,0x80000004); std::memcpy(brand+32,regs,16);
    h.cpuName=brand;
  }
  Microsoft::WRL::ComPtr<IDXGIFactory1> factory;
  if(SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
    if(factory->EnumAdapters1(0,&adapter)!=DXGI_ERROR_NOT_FOUND) {
      DXGI_ADAPTER_DESC1 d{};
      if(SUCCEEDED(adapter->GetDesc1(&d))) {
        char name[256]{};
        WideCharToMultiByte(CP_UTF8,0,d.Description,-1,name,sizeof(name),nullptr,nullptr);
        h.gpuName=name;
      }
    }
  }
#endif
  return h;
}
}
