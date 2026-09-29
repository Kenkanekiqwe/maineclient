#pragma once
#include <filesystem>
#include <string>
#include <vector>
namespace maine::minecraft {
struct Library { std::string name; std::string url; std::string sha1; std::string path; };
struct VersionManifest {
  std::string id, type, mainClass, assets, assetIndexId, assetIndexUrl, assetIndexSha1, clientUrl, clientSha1;
  std::string javaMajor;
  std::vector<Library> libraries;
};
class ManifestManager {
public:
  static bool load(const std::filesystem::path& file, VersionManifest& out, std::string& error);
  static bool save(const std::filesystem::path& file, const VersionManifest& manifest, std::string& error);
};
}
