#include "version_downloader.hpp"
#include "download.hpp"
namespace maine::minecraft {
bool VersionDownloader::install(const std::string& version,std::string& e){
  if(version.empty()){e="Version is empty";return false;}
  const std::string manifestUrl="https://piston-meta.mojang.com/mc/game/version_manifest_v2.json";
  auto manifestFile=paths_.versions/"version_manifest_v2.json";
  auto r=Downloader::file(manifestUrl,manifestFile); if(!r.ok){e=r.error;return false;}
  std::ifstream in(manifestFile); std::string s((std::istreambuf_iterator<char>(in)),{});
  std::string needle="\"id\":\""+version+"\""; auto pos=s.find(needle);
  if(pos==std::string::npos){e="Minecraft version not found: "+version;return false;}
  auto u=s.find("\"url\"",pos); if(u==std::string::npos){e="Version URL missing";return false;}
  auto q=s.find('\"',u+5); q=s.find('\"',q+1); auto q2=s.find('\"',q+1); if(q==std::string::npos||q2==std::string::npos){e="Invalid manifest";return false;}
  std::string url=s.substr(q+1,q2-q-1);
  auto dir=paths_.versions/version; std::filesystem::create_directories(dir);
  auto out=dir/(version+".json"); r=Downloader::file(url,out); if(!r.ok){e=r.error;return false;}
  return true;
}
}
