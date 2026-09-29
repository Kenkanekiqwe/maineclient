#include "java_runtime.hpp"
#include <windows.h>
#include <cstdio>
#include <array>
#include <regex>
namespace maine::core {
static std::string runVersion(const std::filesystem::path& exe){
  std::string cmd="\""+exe.string()+"\" -version 2>&1";
  FILE* p=_popen(cmd.c_str(),"r"); if(!p)return{};
  std::array<char,512> b{}; std::string out; while(fgets(b.data(),(int)b.size(),p))out+=b.data(); _pclose(p); return out;
}
int JavaRuntimeManager::parseMajor(const std::string& v){
  std::regex r(R"(version\s+\"(\d+)(?:\.(\d+))?)"); std::smatch m;
  if(!std::regex_search(v,m,r))return 0; int a=std::stoi(m[1].str()); if(a==1&&m[2].matched)return std::stoi(m[2].str()); return a;
}
JavaRuntime JavaRuntimeManager::detect(int requiredMajor) const{
  std::vector<std::filesystem::path> candidates;
  if(const char* home=std::getenv("JAVA_HOME");home) candidates.push_back(std::filesystem::path(home)/"bin/java.exe");
  candidates.push_back("java.exe");
  for(auto& p:candidates){
    auto text=runVersion(p); int major=parseMajor(text); if(major && (!requiredMajor||major==requiredMajor))
      return {p,major,text,false};
  }
  return {};
}
}
