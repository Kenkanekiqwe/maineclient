#include "launcher.hpp"
#include "version_downloader.hpp"
#include "download.hpp"
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>
using json=nlohmann::json;
namespace maine::minecraft {
static std::string replaceAll(std::string s,const std::string& a,const std::string& b){size_t p=0;while((p=s.find(a,p))!=std::string::npos){s.replace(p,a.size(),b);p+=b.size();}return s;}
bool MinecraftLauncher::prepare(std::string& e) const { VersionDownloader dl(paths_); return dl.install(config_.selectedVersion,e); }
bool MinecraftLauncher::launch(const LaunchRequest& req,std::string& e) const {
  auto dir=paths_.versions/config_.selectedVersion, meta=dir/(config_.selectedVersion+".json"), jar=dir/(config_.selectedVersion+".jar");
  if(!std::filesystem::exists(meta)||!std::filesystem::exists(jar)){e="Minecraft is not installed. Prepare the selected version first.";return false;}
  std::ifstream in(meta); json j; try{in>>j;}catch(const std::exception& x){e=x.what();return false;}
  int required=j.value("javaVersion",json{}).value("majorVersion",0);
  maine::core::JavaRuntimeManager jm(paths_); auto java=jm.detect(required);
  if(java.executable.empty()) java=jm.detect();
  if(java.executable.empty()){e="Java runtime not found";return false;}
  auto instance=paths_.instances/config_.selectedInstance; std::filesystem::create_directories(instance);
  std::string cp=jar.string();
  for(const auto& x:j.value("libraries",json::array())){
    auto a=x.value("downloads",json{}).value("artifact",json{}); auto p=a.value("path","");
    if(!p.empty()) cp+=";"+(paths_.libraries/p).string();
  }
  auto main=j.value("mainClass","");
  if(main.empty()){e="Missing mainClass";return false;}
  std::string cmd="\""+java.executable.string()+"\" -Xms"+std::to_string(config_.minMemoryMb)+"M -Xmx"+std::to_string(config_.maxMemoryMb)+"M -cp \""+cp+"\" "+main;
  cmd+=" --username "+req.username+" --version "+config_.selectedVersion+" --gameDir \""+instance.string()+"\"";
  cmd+=" --assetsDir \""+paths_.assets.string()+"\" --assetIndex "+j.value("assets","")+" --uuid "+req.uuid+" --accessToken "+req.accessToken;
  STARTUPINFOA si{}; si.cb=sizeof(si); PROCESS_INFORMATION pi{};
  std::vector<char> buf(cmd.begin(),cmd.end()); buf.push_back('\0');
  if(!CreateProcessA(nullptr,buf.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,instance.string().c_str(),&si,&pi)){
    e="CreateProcess failed: "+std::to_string(GetLastError()); return false;
  }
  CloseHandle(pi.hThread); CloseHandle(pi.hProcess); return true;
}
}
