#include "version_downloader.hpp"
#include "download.hpp"
#include <fstream>
#include <regex>
namespace maine::minecraft {
static std::string jsonFieldAt(const std::string& s,std::size_t from,const std::string& key){
  std::regex r(R"(\")"+key+R"(\")\s*:\s*\")([^\"]*)\")");
  std::smatch m; auto begin=s.cbegin()+static_cast<std::ptrdiff_t>(from);
  return std::regex_search(begin,s.cend(),m,r)?m[1].str():"";
}
bool VersionDownloader::install(const std::string& version,std::string& e){
  if(version.empty()){e="Version is empty";return false;}
  const std::string manifestUrl="https://piston-meta.mojang.com/mc/game/version_manifest_v2.json";
  auto manifestFile=paths_.versions/"version_manifest_v2.json";
  auto r=Downloader::file(manifestUrl,manifestFile); if(!r.ok){e=r.error;return false;}
  std::ifstream in(manifestFile,std::ios::binary); if(!in){e="Cannot read version manifest";return false;}
  std::string s((std::istreambuf_iterator<char>(in)),{});
  std::regex idRe(R"(\"id\"\s*:\s*\"([^\"]*)\")"); std::smatch m; auto it=s.cbegin();
  std::string url;
  while(std::regex_search(it,s.cend(),m,idRe)){
    if(m[1].str()==version){
      auto absolute=static_cast<std::size_t>(it-s.cbegin())+static_cast<std::size_t>(m.position(0));
      url=jsonFieldAt(s,absolute,"url"); break;
    }
    it=m.suffix().first;
  }
  if(url.empty()){e="Minecraft version not found: "+version;return false;}
  auto dir=paths_.versions/version; std::filesystem::create_directories(dir);
  auto out=dir/(version+".json"); r=Downloader::file(url,out); if(!r.ok){e=r.error;return false;}
  return true;
}
}
