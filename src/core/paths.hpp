#pragma once
#include <filesystem>
namespace maine::core {
struct Paths {
  std::filesystem::path root, instances, versions, libraries, assets, runtimes, logs, config;
  static Paths create();
  void ensure() const;
};
}
