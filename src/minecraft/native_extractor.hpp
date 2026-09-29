#pragma once
#include <filesystem>
#include <string>
namespace maine::minecraft {
class NativeExtractor {
public:
 static bool extractJar(const std::filesystem::path& archive,const std::filesystem::path& destination,std::string& error);
};
}
