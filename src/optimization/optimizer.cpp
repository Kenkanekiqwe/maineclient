#include "optimizer.hpp"
#include <sstream>
namespace maine::optimization {
OptimizationReport Optimizer::analyze(const maine::core::HardwareInfo& h) const {
  OptimizationReport r; r.profile=buildProfile(h,Preset::Performance);
  std::ostringstream s; s<<"Maine Performance: "<<h.logicalProcessors<<" threads, "<<h.totalRamMb<<" MB RAM, workers="<<r.profile.workerThreads;
  r.summary=s.str(); return r;
}
}
