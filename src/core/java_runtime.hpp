#pragma once
#include "paths.hpp"
#include <filesystem>
#include <string>
namespace maine::core {
struct JavaRuntime { std::filesystem::path executable; int major=0; std::string version; bool bundled=false; };
class JavaRuntimeManager {
  Paths paths_;
public:
  explicit JavaRuntimeManager(Paths p):paths_(std::move(p)){}
  JavaRuntime detect(int requiredMajor=0) const;
  static int parseMajor(const std::string& version);
};
}
