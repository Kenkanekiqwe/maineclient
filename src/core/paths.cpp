#include "paths.hpp"
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif
namespace maine::core {
Paths Paths::create() {
#ifdef _WIN32
  PWSTR raw=nullptr; std::filesystem::path base;
  if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData,0,nullptr,&raw))) { base=raw; CoTaskMemFree(raw); }
  else base=std::filesystem::temp_directory_path();
#else
  std::filesystem::path base=".";
#endif
  Paths p; p.root=base/"MaineClient";
  p.instances=p.root/"instances"; p.versions=p.root/"versions";
  p.libraries=p.root/"libraries"; p.assets=p.root/"assets";
  p.runtimes=p.root/"runtimes"; p.logs=p.root/"logs"; p.config=p.root/"config";
  return p;
}
void Paths::ensure() const {
  for(const auto& p:{root,instances,versions,libraries,assets,runtimes,logs,config})
    std::filesystem::create_directories(p);
}
}
