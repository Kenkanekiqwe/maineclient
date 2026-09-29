#include "java_runtime.hpp"
#include "../minecraft/download.hpp"
#include <windows.h>
#include <cstdio>
#include <array>
#include <regex>
#include <cstdlib>
#include <system_error>
#include <vector>

namespace maine::core {

static std::string runVersion(const std::filesystem::path& exe){
  std::string cmd="""+exe.string()+"" -version 2>&1";
  FILE* p=_popen(cmd.c_str(),"r"); if(!p)return{};
  std::array<char,512> b{}; std::string out;
  while(fgets(b.data(),(int)b.size(),p))out+=b.data();
  _pclose(p);
  return out;
}

int JavaRuntimeManager::parseMajor(const std::string& v){
  std::regex r(R"(version\s+\"(\d+)(?:\.(\d+))?)");
  std::smatch m;
  if(!std::regex_search(v,m,r))return 0;
  int a=std::stoi(m[1].str());
  if(a==1&&m[2].matched)return std::stoi(m[2].str());
  return a;
}

JavaRuntime JavaRuntimeManager::detect(int requiredMajor) const{
  std::vector<std::filesystem::path> candidates;

  // MaineClient-managed runtimes are checked first.
  if(requiredMajor>0){
    const auto root=paths_.runtimes/("java-"+std::to_string(requiredMajor));
    if(std::filesystem::exists(root)){
      std::error_code ec;
      for(std::filesystem::recursive_directory_iterator it(root,ec),end;it!=end&&!ec;it.increment(ec)){
        if(it->is_regular_file(ec)&&it->path().filename()=="java.exe"){
          const auto text=runVersion(it->path());
          const int major=parseMajor(text);
          if(major==requiredMajor)return{it->path(),major,text,true};
        }
      }
    }
  }

  char* home=nullptr; size_t homeLen=0;
  if(_dupenv_s(&home,&homeLen,"JAVA_HOME")==0&&home){
    candidates.push_back(std::filesystem::path(home)/"bin/java.exe");
    free(home);
  }
  candidates.push_back("java.exe");

  for(const auto& p:candidates){
    auto text=runVersion(p);
    int major=parseMajor(text);
    if(major&&(!requiredMajor||major==requiredMajor))
      return{p,major,text,false};
  }
  return{};
}

static bool runTarExtract(const std::filesystem::path& archive,const std::filesystem::path& destination,std::string& error){
  std::filesystem::create_directories(destination);
  std::string cmd="tar -xf ""+archive.string()+"" -C ""+destination.string()+""";
  STARTUPINFOA si{}; si.cb=sizeof(si);
  PROCESS_INFORMATION pi{};
  std::vector<char> buffer(cmd.begin(),cmd.end());
  buffer.push_back('\0');

  if(!CreateProcessA(nullptr,buffer.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&si,&pi)){
    error="Cannot start Windows tar: "+std::to_string(GetLastError());
    return false;
  }

  WaitForSingleObject(pi.hProcess,INFINITE);
  DWORD code=1;
  GetExitCodeProcess(pi.hProcess,&code);
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

  if(code!=0){
    error="Windows tar failed with exit code "+std::to_string(code);
    return false;
  }
  return true;
}

JavaRuntime JavaRuntimeManager::ensure(int requiredMajor) const{
  if(requiredMajor<=0)return detect();

  auto existing=detect(requiredMajor);
  if(!existing.executable.empty())return existing;

  const auto installRoot=paths_.runtimes/("java-"+std::to_string(requiredMajor));
  const auto archive=paths_.runtimes/("java-"+std::to_string(requiredMajor)+".zip");
  const auto staging=paths_.runtimes/("java-"+std::to_string(requiredMajor)+"-staging");

  std::error_code ec;
  std::filesystem::remove_all(staging,ec);
  std::filesystem::create_directories(paths_.runtimes);

  // Eclipse Temurin's official Adoptium API provides the current GA Windows x64 JDK.
  // We use JDK rather than a system installation so the launcher remains self-contained.
  const std::string url=
    "https://api.adoptium.net/v3/binary/latest/"+
    std::to_string(requiredMajor)+
    "/ga/windows/x64/jdk/hotspot/normal/eclipse";

  auto result=maine::minecraft::Downloader::file(url,archive);
  if(!result.ok)return{};

  std::string error;
  if(!runTarExtract(archive,staging,error)){
    std::filesystem::remove(archive,ec);
    std::filesystem::remove_all(staging,ec);
    return{};
  }

  std::filesystem::path javaExe;
  for(std::filesystem::recursive_directory_iterator it(staging,ec),end;it!=end&&!ec;it.increment(ec)){
    if(it->is_regular_file(ec)&&it->path().filename()=="java.exe"){
      javaExe=it->path();
      break;
    }
  }

  if(javaExe.empty()){
    std::filesystem::remove(archive,ec);
    std::filesystem::remove_all(staging,ec);
    return{};
  }

  const auto extractedRoot=javaExe.parent_path().parent_path();
  std::filesystem::remove_all(installRoot,ec);
  std::filesystem::create_directories(installRoot.parent_path(),ec);

  // Usually the archive contains one top-level Temurin directory.
  // Move that directory to a stable MaineClient runtime path.
  if(extractedRoot!=staging){
    std::filesystem::rename(extractedRoot,installRoot,ec);
  }

  if(ec){
    ec.clear();
    std::filesystem::create_directories(installRoot,ec);
    for(std::filesystem::recursive_directory_iterator it(staging,ec),end;it!=end&&!ec;it.increment(ec)){
      const auto rel=std::filesystem::relative(it->path(),staging,ec);
      if(ec)break;
      const auto dst=installRoot/rel;
      if(it->is_directory(ec))std::filesystem::create_directories(dst,ec);
      else if(it->is_regular_file(ec)){
        std::filesystem::create_directories(dst.parent_path(),ec);
        std::filesystem::copy_file(it->path(),dst,std::filesystem::copy_options::overwrite_existing,ec);
      }
    }
  }

  std::filesystem::remove_all(staging,ec);
  std::filesystem::remove(archive,ec);

  return detect(requiredMajor);
}

}
