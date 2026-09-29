#include "manifest.hpp"
#include <fstream>
#include <regex>
namespace maine::minecraft {
static std::string field(const std::string& s,const std::string& key){
  std::regex r("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
  std::smatch m; return std::regex_search(s,m,r)?m[1].str():"";
}
bool ManifestManager::load(const std::filesystem::path& file, VersionManifest& o, std::string& e){
  std::ifstream f(file,std::ios::binary); if(!f){e="Cannot open version metadata: "+file.string();return false;}
  std::string s((std::istreambuf_iterator<char>(f)),{});
  o.id=field(s,"id"); o.type=field(s,"type"); o.mainClass=field(s,"mainClass");
  o.assets=field(s,"assets"); o.assetIndexId=field(s,"assetIndexId");
  o.assetIndexUrl=field(s,"assetIndexUrl"); o.assetIndexSha1=field(s,"assetIndexSha1");
  o.clientUrl=field(s,"clientUrl"); o.clientSha1=field(s,"clientSha1");
  o.javaMajor=field(s,"majorVersion");
  if(o.id.empty()){e="Invalid version metadata: missing id";return false;} return true;
}
bool ManifestManager::save(const std::filesystem::path& file,const VersionManifest& m,std::string& e){
  std::ofstream f(file,std::ios::binary|std::ios::trunc); if(!f){e="Cannot write metadata";return false;}
  f<<"{\n  \"id\": \""<<m.id<<"\",\n  \"type\": \""<<m.type<<"\",\n  \"mainClass\": \""<<m.mainClass<<"\",\n  \"assets\": \""<<m.assets<<"\",\n  \"assetIndexId\": \""<<m.assetIndexId<<"\",\n  \"assetIndexUrl\": \""<<m.assetIndexUrl<<"\",\n  \"assetIndexSha1\": \""<<m.assetIndexSha1<<"\",\n  \"clientUrl\": \""<<m.clientUrl<<"\",\n  \"clientSha1\": \""<<m.clientSha1<<"\",\n  \"majorVersion\": \""<<m.javaMajor<<"\"\n}\n"; return true;
}
}
