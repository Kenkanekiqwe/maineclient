#include "manifest.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
using json=nlohmann::json;
namespace maine::minecraft {
bool ManifestManager::load(const std::filesystem::path& file,VersionManifest& o,std::string& e){
  try{
    std::ifstream f(file); if(!f){e="Cannot open version metadata: "+file.string();return false;}
    json j; f>>j; o={};
    o.id=j.value("id",""); o.type=j.value("type",""); o.mainClass=j.value("mainClass","");
    o.assets=j.value("assets",""); o.javaMajor=std::to_string(j.value("javaVersion",json{}).value("majorVersion",0));
    auto ai=j.value("assetIndex",json{}); o.assetIndexId=ai.value("id",""); o.assetIndexUrl=ai.value("url",""); o.assetIndexSha1=ai.value("sha1","");
    auto client=j.value("downloads",json{}).value("client",json{}); o.clientUrl=client.value("url",""); o.clientSha1=client.value("sha1","");
    for(const auto& x:j.value("libraries",json::array())){
      auto d=x.value("downloads",json{});
      auto a=d.value("artifact",json{});
      Library l; l.name=x.value("name",""); l.url=a.value("url",""); l.sha1=a.value("sha1","");
      l.path=a.value("path","");
      for(const auto& n:x.value("natives",json::object())) if(n.is_string()) l.native=true;
      if(!l.name.empty()&&!l.url.empty()&&!l.path.empty()) o.libraries.push_back(std::move(l));
    }
    auto readArgs=[&](const char* key,std::vector<std::string>& out){
      for(const auto& a:j.value(key,json::array())) if(a.is_string()) out.push_back(a.get<std::string>());
    };
    if(j.contains("arguments")){readArgs("arguments",o.gameArguments);}
    if(o.id.empty()){e="Invalid version metadata: missing id";return false;}
    return true;
  }catch(const std::exception& ex){e=std::string("Invalid Minecraft JSON: ")+ex.what();return false;}
}
bool ManifestManager::save(const std::filesystem::path& file,const VersionManifest& m,std::string& e){
  try{json j={{"id",m.id},{"type",m.type},{"mainClass",m.mainClass}}; std::ofstream f(file); if(!f){e="Cannot write metadata";return false;} f<<j.dump(2); return true;}
  catch(const std::exception& ex){e=ex.what();return false;}
}
}
