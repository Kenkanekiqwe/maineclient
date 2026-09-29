#pragma once
#include <filesystem>
#include <string>
namespace maine::core {
struct Config {
  std::string selectedVersion="1.21.8";
  std::string selectedLoader="vanilla";
  std::string selectedInstance="Default";
  int minMemoryMb=1024, maxMemoryMb=4096;
  bool performanceMode=true;
  static Config load(const std::filesystem::path&);
  void save(const std::filesystem::path&) const;
};
}
