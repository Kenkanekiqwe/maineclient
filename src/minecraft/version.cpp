#include "version.hpp"
#include <fstream>
#include <regex>
#include <stdexcept>
namespace maine::minecraft {
bool VersionManager::isInstalled(const std::string& id) const { return std::filesystem::exists(root_/id/(id+".json")); }
VersionInfo VersionManager::readLocal(const std::string& id) const {
  VersionInfo v; v.id=id; v.jsonPath=root_/id/(id+".json");
  std::ifstream in(v.jsonPath); if(!in) throw std::runtime_error("Minecraft manifest not found: "+id);
  std::string s((std::istreambuf_iterator<char>(in)),{});
  auto get=[&](const char* key){ std::regex r(std::string("\"")+key+R"(\"\s*:\s*\"([^\"]*)\")"); std::smatch m; return std::regex_search(s,m,r)?m[1].str():std::string{}; };
  v.type=get("type"); v.mainClass=get("mainClass"); return v;
}
}
