#include "manifest.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
using json=nlohmann::json;

namespace maine::minecraft {

static json objectOrEmpty(const json& v){
  return v.is_object()?v:json::object();
}

static json arrayOrEmpty(const json& v){
  return v.is_array()?v:json::array();
}

static std::string stringValue(const json& parent,const char* key,const std::string& fallback=""){
  if(!parent.is_object()||!parent.contains(key)||!parent[key].is_string()) return fallback;
  return parent[key].get<std::string>();
}

bool ManifestManager::load(const std::filesystem::path& file,VersionManifest& o,std::string& e){
  try{
    std::ifstream f(file);
    if(!f){e="Cannot open version metadata: "+file.string();return false;}

    json j; f>>j;
    if(!j.is_object()){e="Invalid Minecraft metadata: root is not an object";return false;}

    o={};
    o.id=stringValue(j,"id");
    o.type=stringValue(j,"type");
    o.mainClass=stringValue(j,"mainClass");
    o.assets=stringValue(j,"assets");

    const auto javaVersion=objectOrEmpty(j.value("javaVersion",json{}));
    o.javaMajor=std::to_string(javaVersion.value("majorVersion",0));

    const auto ai=objectOrEmpty(j.value("assetIndex",json{}));
    o.assetIndexId=stringValue(ai,"id");
    o.assetIndexUrl=stringValue(ai,"url");
    o.assetIndexSha1=stringValue(ai,"sha1");

    const auto downloads=objectOrEmpty(j.value("downloads",json{}));
    const auto client=objectOrEmpty(downloads.value("client",json{}));
    o.clientUrl=stringValue(client,"url");
    o.clientSha1=stringValue(client,"sha1");

    for(const auto& x:arrayOrEmpty(j.value("libraries",json::array()))){
      if(!x.is_object()) continue;

      const auto d=objectOrEmpty(x.value("downloads",json{}));
      const auto a=objectOrEmpty(d.value("artifact",json{}));

      Library l;
      l.name=stringValue(x,"name");
      l.url=stringValue(a,"url");
      l.sha1=stringValue(a,"sha1");
      l.path=stringValue(a,"path");

      const auto rules=x.value("rules",json{});
      if(rules.is_array()){
        l.allowed=false;
        for(const auto& rule:rules){
          if(rule.is_object() && stringValue(rule,"action")=="allow") l.allowed=true;
          else if(rule.is_object() && stringValue(rule,"action")=="disallow") l.allowed=false;
        }
      }else{
        l.allowed=true;
      }

      const auto natives=x.value("natives",json{});
      if(natives.is_object() && natives.contains("windows") && natives["windows"].is_string()){
        const std::string classifier=natives["windows"].get<std::string>();
        const auto classifiers=objectOrEmpty(d.value("classifiers",json{}));
        const auto ca=objectOrEmpty(classifiers.value(classifier,json{}));
        l.native=true;
        l.nativeUrl=stringValue(ca,"url");
        l.nativeSha1=stringValue(ca,"sha1");
        l.nativePath=stringValue(ca,"path");
      }

      if(l.native) o.libraries.push_back(l);
      else if(!l.name.empty()&&!l.url.empty()&&!l.path.empty()) o.libraries.push_back(l);
    }

    if(j.contains("minecraftArguments")&&j["minecraftArguments"].is_string())
      o.gameArguments.push_back(j["minecraftArguments"].get<std::string>());

    const auto arguments=objectOrEmpty(j.value("arguments",json{}));
    for(const char* key:{"jvm","game"}){
      const auto values=arrayOrEmpty(arguments.value(key,json::array()));
      auto& out=(std::string(key)=="jvm")?o.jvmArguments:o.gameArguments;
      for(const auto& value:values) if(value.is_string()) out.push_back(value.get<std::string>());
    }

    if(o.id.empty()){e="Invalid version metadata: missing id";return false;}
    return true;
  }catch(const std::exception& ex){
    e=std::string("Invalid Minecraft JSON: ")+ex.what();
    return false;
  }
}

bool ManifestManager::save(const std::filesystem::path& file,const VersionManifest& m,std::string& e){
  try{
    json j={{"id",m.id},{"type",m.type},{"mainClass",m.mainClass}};
    std::ofstream f(file);
    if(!f){e="Cannot write metadata";return false;}
    f<<j.dump(2);
    return true;
  }catch(const std::exception& ex){
    e=ex.what();
    return false;
  }
}

}
