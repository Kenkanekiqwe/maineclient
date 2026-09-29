#pragma once
#include "../core/paths.hpp"
#include "../core/config.hpp"
#include "../core/java_runtime.hpp"
#include "manifest.hpp"
#include <string>
namespace maine::minecraft {
struct LaunchRequest {
  std::string username="Player";
  std::string uuid="00000000-0000-0000-0000-000000000000";
  std::string accessToken="0";
};
class MinecraftLauncher {
  maine::core::Paths paths_;
  maine::core::Config config_;
public:
  MinecraftLauncher(maine::core::Paths p, maine::core::Config c):paths_(std::move(p)),config_(std::move(c)){}
  bool prepare(std::string& error) const;
  bool launch(const LaunchRequest& request,std::string& error) const;
};
}
