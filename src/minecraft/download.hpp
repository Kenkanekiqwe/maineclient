#pragma once
#include <filesystem>
#include <string>
namespace maine::minecraft {
struct DownloadResult { bool ok=false; std::string error; };
class Downloader {
public:
 static DownloadResult file(const std::string& url,const std::filesystem::path& target,const std::string& expectedSha1="");
 static std::string sha1(const std::filesystem::path& file);
};
}
