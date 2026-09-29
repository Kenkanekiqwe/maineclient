#pragma once
#include <filesystem>
#include <string>
namespace maine::core {
struct Account { std::string name="Player"; std::string uuid="00000000-0000-0000-0000-000000000000"; };
struct Config {
  std::string selectedVersion="1.21.8";
  std::string selectedLoader="vanilla";
  std::string selectedInstance="Default";
  std::string selectedAccount="Player";
  int minMemoryMb=1024, maxMemoryMb=4096;
  bool performanceMode=true;
  static Config load(const std::filesystem::path&);
  void save(const std::filesystem::path&) const;
};
}
