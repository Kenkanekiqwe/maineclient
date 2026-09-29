#include "runtime_optimizer.hpp"
#include <algorithm>
namespace maine::optimization {
RuntimeTuning RuntimeOptimizer::build(int cpu,int ram,bool performance){
  RuntimeTuning t;
  t.workerThreads=std::clamp(performance?cpu-1:cpu/2,1,16);
  t.minMemoryMb=std::clamp(performance?1024:1536,512,ram*1024/2);
  t.maxMemoryMb=std::clamp(performance?std::max(2048,ram*1024/3):std::max(2048,ram*1024/4),2048,ram*1024*3/4);
  t.reuseAllocations=performance;
  t.asyncChunkScheduling=true;
  return t;
}
}
