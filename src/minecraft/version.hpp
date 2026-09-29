#pragma once
#include <filesystem>
#include <string>
namespace maine::minecraft {
struct VersionInfo { std::string id,type,mainClass; std::filesystem::path jsonPath; };
class VersionManager {
  std::filesystem::path root_;
public:
  explicit VersionManager(std::filesystem::path root):root_(std::move(root)){}
  bool isInstalled(const std::string&) const;
  VersionInfo readLocal(const std::string&) const;
};
}
