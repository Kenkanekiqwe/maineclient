#include "launcher.hpp"
#include "version_downloader.hpp"
#include "native_extractor.hpp"
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <utility>
#include <functional>
using json=nlohmann::json;
namespace maine::minecraft {
static std::string replaceAll(std::string s,const std::string& a,const std::string& b){size_t p=0;while((p=s.find(a,p))!=std::string::npos){s.replace(p,a.size(),b);p+=b.size();}return s;}
static bool rulesAllow(const json& x){if(!x.contains("rules")||!x["rules"].is_array())return true;bool allowed=false;for(const auto& r:x["rules"]){if(r.value("action","")=="allow")allowed=true;else if(r.value("action","")=="disallow")allowed=false;}return allowed;}
static std::string quoteArg(const std::string& s){std::string r="\"";for(char c:s){if(c=='"')r+="\\\"";else r+=c;}return r+"\"";}
bool MinecraftLauncher::prepare(std::string& e) const {VersionDownloader dl(paths_);return dl.install(config_.selectedVersion,e);}
bool MinecraftLauncher::launch(const LaunchRequest& req,std::string& e) const {
const auto dir=paths_.versions/config_.selectedVersion; const auto meta=dir/(config_.selectedVersion+".json"); const auto jar=dir/(config_.selectedVersion+".jar");
if(!std::filesystem::exists(meta)||!std::filesystem::exists(jar)){e="Minecraft is not installed. Prepare the selected version first.";return false;}
json j;try{std::ifstream in(meta);if(!in){e="Cannot open Minecraft metadata";return false;}in>>j;}catch(const std::exception& x){e=std::string("Invalid Minecraft metadata: ")+x.what();return false;}
const int required=j.value("javaVersion",json{}).value("majorVersion",0); maine::core::JavaRuntimeManager jm(paths_);auto java=jm.ensure(required);
if(java.executable.empty()){
  e="Java "+std::to_string(required)+" could not be installed automatically. Check your internet connection or write logs\\java-runtime.log.";
  return false;
}
const auto instance=paths_.instances/config_.selectedInstance;const auto nativeRoot=instance/"natives";std::filesystem::create_directories(nativeRoot);
std::string cp=jar.string();
for(const auto& lib:j.value("libraries",json::array())){if(!rulesAllow(lib))continue;auto downloads=lib.value("downloads",json{});auto artifact=downloads.value("artifact",json{});auto path=artifact.value("path","");if(!path.empty()){auto file=paths_.libraries/path;if(!std::filesystem::exists(file)){e="Missing library: "+file.string();return false;}cp+=";"+file.string();}auto natives=lib.value("natives",json{});if(natives.is_object()&&natives.contains("windows")){auto classifier=natives["windows"].get<std::string>();auto native=downloads.value("classifiers",json{}).value(classifier,json{});auto nativePath=native.value("path","");if(!nativePath.empty()){auto archive=paths_.libraries/nativePath;if(!std::filesystem::exists(archive)){e="Missing native library: "+archive.string();return false;}std::string ne;if(!NativeExtractor::extractJar(archive,nativeRoot,ne)){e="Native extraction failed: "+ne;return false;}}}}
const auto main=j.value("mainClass","");if(main.empty()){e="Missing mainClass";return false;}
const std::string gameDir=instance.string(),assets=paths_.assets.string(),nativeDir=nativeRoot.string();
auto substitute=[&](std::string s){const std::pair<std::string,std::string> vars[]={
{"${auth_player_name}",req.username},{"${version_name}",config_.selectedVersion},{"${game_directory}",gameDir},{"${assets_root}",assets},{"${assets_index_name}",j.value("assets","")},{"${auth_uuid}",req.uuid},{"${auth_access_token}",req.accessToken},{"${user_type}","msa"},{"${version_type}",j.value("type","release")},{"${natives_directory}",nativeDir},{"${library_directory}",paths_.libraries.string()},{"${launcher_name}","MaineClient"},{"${launcher_version}","0.1.0"},{"${classpath}",cp}};for(const auto& v:vars)s=replaceAll(s,v.first,v.second);return s;};
std::vector<std::string> jvmArgs,gameArgs;
std::function<void(const json&,std::vector<std::string>&)> append; append=[&](const json& v,std::vector<std::string>& out){if(v.is_string()){out.push_back(substitute(v.get<std::string>()));}else if(v.is_array()){for(const auto& x:v)append(x,out);}else if(v.is_object()&&rulesAllow(v)){append(v.value("value",json{}),out);}};
if(j.contains("arguments")&&j["arguments"].is_object()){for(const auto& v:j["arguments"].value("jvm",json::array()))append(v,jvmArgs);for(const auto& v:j["arguments"].value("game",json::array()))append(v,gameArgs);}else{jvmArgs={"-Xms"+std::to_string(config_.minMemoryMb)+"M","-Xmx"+std::to_string(config_.maxMemoryMb)+"M","-Djava.library.path="+nativeDir,"-cp",cp};if(j.contains("minecraftArguments")&&j["minecraftArguments"].is_string())gameArgs.push_back(substitute(j["minecraftArguments"].get<std::string>()));else gameArgs={"--username",req.username,"--version",config_.selectedVersion,"--gameDir",gameDir,"--assetsDir",assets,"--assetIndex",j.value("assets",""),"--uuid",req.uuid,"--accessToken",req.accessToken,"--userType","msa","--versionType",j.value("type","release")};}
if(j.contains("arguments")){jvmArgs.push_back("-Xms"+std::to_string(config_.minMemoryMb)+"M");jvmArgs.push_back("-Xmx"+std::to_string(config_.maxMemoryMb)+"M");}
std::string cmd=quoteArg(java.executable.string());for(const auto& a:jvmArgs)cmd+=" "+quoteArg(a);cmd+=" "+quoteArg(main);for(const auto& a:gameArgs)cmd+=" "+quoteArg(a);
STARTUPINFOA si{};si.cb=sizeof(si);PROCESS_INFORMATION pi{};std::vector<char> buffer(cmd.begin(),cmd.end());buffer.push_back('\0');
if(!CreateProcessA(nullptr,buffer.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,instance.string().c_str(),&si,&pi)){e="CreateProcess failed: "+std::to_string(GetLastError());return false;}CloseHandle(pi.hThread);CloseHandle(pi.hProcess);return true;}
}