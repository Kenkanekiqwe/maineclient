#pragma once
#include "paths.hpp"
#include "config.hpp"
#include <string>
namespace maine::core {
class Launcher {
  Paths paths_; Config config_;
public:
  Launcher(Paths p,Config c):paths_(std::move(p)),config_(std::move(c)){}
  bool validate(std::string&) const;
  bool launch(std::string&) const;
};
}
