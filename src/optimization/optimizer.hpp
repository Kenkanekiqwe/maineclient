#pragma once
#include "profile.hpp"
namespace maine::optimization {
struct OptimizationReport { Profile profile; std::string summary; };
class Optimizer { public: OptimizationReport analyze(const maine::core::HardwareInfo&) const; };
}
