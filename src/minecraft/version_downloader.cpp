#include "version_downloader.hpp"
#include "download.hpp"
#include "manifest.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
using json=nlohmann::json;
namespace maine::minecraft {
bool VersionDownloader::install(const std::string& version,std::string& e){
  if(version.empty()){e="Version is empty";return false;}
  const std::string manifestUrl="https://piston-meta.mojang.com/mc/game/version_manifest_v2.json";
  auto mf=paths_.versions/"version_manifest_v2.json";
  auto r=Downloader::file(manifestUrl,mf); if(!r.ok){e=r.error;return false;}
  try{
    std::ifstream in(mf); json root; in>>root; std::string url,sha;
    for(const auto& v:root.value("versions",json::array())) if(v.value("id","")==version){url=v.value("url","");break;}
    if(url.empty()){e="Minecraft version not found: "+version;return false;}
    auto dir=paths_.versions/version; std::filesystem::create_directories(dir);
    auto meta=dir/(version+".json"); r=Downloader::file(url,meta); if(!r.ok){e=r.error;return false;}
    VersionManifest vm; if(!ManifestManager::load(meta,vm,e)) return false;
    if(vm.clientUrl.empty()){e="Minecraft client download is missing";return false;}
    auto jar=dir/(version+".jar");
    if(!std::filesystem::exists(jar) || (!vm.clientSha1.empty() && Downloader::sha1(jar)!=vm.clientSha1)){
      r=Downloader::file(vm.clientUrl,jar,vm.clientSha1); if(!r.ok){e=r.error;return false;}
    }
    if(!vm.assetIndexUrl.empty()){
      auto ai=paths_.assets/"indexes"/(vm.assetIndexId+".json");
      if(!std::filesystem::exists(ai)||(!vm.assetIndexSha1.empty()&&Downloader::sha1(ai)!=vm.assetIndexSha1)){
        r=Downloader::file(vm.assetIndexUrl,ai,vm.assetIndexSha1); if(!r.ok){e="Asset index: "+r.error;return false;}
      }
      std::ifstream af(ai); json assets; af>>assets;
      for(const auto& [logical,obj] : assets.value("objects",json::object()).items()){
        std::string hash=obj.value("hash","");
        if(hash.size()<2) continue;
        auto target=paths_.assets/"objects"/hash.substr(0,2)/hash;
        if(std::filesystem::exists(target) && Downloader::sha1(target)==hash) continue;
        auto assetUrl="https://resources.download.minecraft.net/"+hash.substr(0,2)+"/"+hash;
        r=Downloader::file(assetUrl,target,hash);
        if(!r.ok){e="Asset "+logical+": "+r.error;return false;}
      }
    }
    for(const auto& lib:vm.libraries) if(lib.allowed){
      if(!lib.url.empty() && !lib.path.empty()){
        auto target=paths_.libraries/lib.path;
        if(!std::filesystem::exists(target) || (!lib.sha1.empty()&&Downloader::sha1(target)!=lib.sha1)){
          r=Downloader::file(lib.url,target,lib.sha1); if(!r.ok){e="Library "+lib.name+": "+r.error;return false;}
        }
      }
      if(lib.native && !lib.nativeUrl.empty() && !lib.nativePath.empty()){
        auto target=paths_.libraries/lib.nativePath;
        if(!std::filesystem::exists(target) || (!lib.nativeSha1.empty()&&Downloader::sha1(target)!=lib.nativeSha1)){
          r=Downloader::file(lib.nativeUrl,target,lib.nativeSha1);
          if(!r.ok){e="Native "+lib.name+": "+r.error;return false;}
        }
      }
    }
    return true;
  }catch(const std::exception& ex){e=std::string("Minecraft metadata error: ")+ex.what();return false;}
}
}
