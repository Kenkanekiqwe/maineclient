#include "config.hpp"
#include <fstream>
#include <regex>
namespace maine::core {
Config Config::load(const std::filesystem::path& file) {
  Config c; std::ifstream in(file); if(!in) return c;
  std::string s((std::istreambuf_iterator<char>(in)),{});
  auto get=[&](const char* key,std::string fallback){
    std::regex r(std::string("\"")+key+R"(\"\s*:\s*\"([^\"]*)\")");
    std::smatch m; return std::regex_search(s,m,r)?m[1].str():fallback;
  };
  c.selectedVersion=get("version",c.selectedVersion);
  c.selectedLoader=get("loader",c.selectedLoader);
  c.selectedInstance=get("instance",c.selectedInstance);
  c.selectedAccount=get("account",c.selectedAccount);
  return c;
}
void Config::save(const std::filesystem::path& file) const {
  std::filesystem::create_directories(file.parent_path());
  std::ofstream out(file);
  out<<"{\n  \"version\": \""<<selectedVersion<<"\",\n"
     <<"  \"loader\": \""<<selectedLoader<<"\",\n"
     <<"  \"instance\": \""<<selectedInstance<<"\",\n"
     <<"  \"account\": \""<<selectedAccount<<"\"\n}\n";
}
}
