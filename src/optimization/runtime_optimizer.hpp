#pragma once
#include <string>
#include <thread>
namespace maine::optimization {
struct RuntimeTuning { int workerThreads=1; int minMemoryMb=1024; int maxMemoryMb=4096; bool reuseAllocations=true; bool asyncChunkScheduling=true; };
class RuntimeOptimizer {
public:
 static RuntimeTuning build(int logicalCpu,int totalRamGb,bool performanceMode);
};
}
