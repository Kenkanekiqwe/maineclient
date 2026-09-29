#include "launcher.hpp"
#include "version_downloader.hpp"
#include <windows.h>
#include <filesystem>
namespace maine::minecraft {
bool MinecraftLauncher::prepare(std::string& e) const {
  VersionDownloader dl(paths_); return dl.install(config_.selectedVersion,e);
}
bool MinecraftLauncher::launch(const LaunchRequest& req,std::string& e) const {
  auto versionDir=paths_.versions/config_.selectedVersion;
  auto jar=versionDir/(config_.selectedVersion+".jar");
  auto json=versionDir/(config_.selectedVersion+".json");
  if(!std::filesystem::exists(json)){e="Minecraft metadata is not installed. Prepare the version first.";return false;}
  if(!std::filesystem::exists(jar)){e="Minecraft client JAR is not installed yet.";return false;}
  maine::core::JavaRuntimeManager jm(paths_); auto java=jm.detect();
  if(java.executable.empty()){e="Java was not found. Install Java or set JAVA_HOME.";return false;}
  auto instance=paths_.instances/config_.selectedInstance; std::filesystem::create_directories(instance);
  std::string cmd="\""+java.executable.string()+"\" -Xms"+std::to_string(config_.minMemoryMb)+"M -Xmx"+std::to_string(config_.maxMemoryMb)+"M -jar \""+jar.string()+"\"";
  STARTUPINFOA si{}; si.cb=sizeof(si); PROCESS_INFORMATION pi{};
  std::vector<char> buffer(cmd.begin(),cmd.end()); buffer.push_back('\0');
  if(!CreateProcessA(nullptr,buffer.data(),nullptr,nullptr,FALSE,0,nullptr,instance.string().c_str(),&si,&pi)){
    e="CreateProcess failed: "+std::to_string(GetLastError()); return false;
  }
  CloseHandle(pi.hThread); CloseHandle(pi.hProcess); return true;
}
}
