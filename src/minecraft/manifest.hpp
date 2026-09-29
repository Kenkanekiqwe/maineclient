#pragma once
#include <filesystem>
#include <string>
#include <vector>
namespace maine::minecraft {
struct Library {
  std::string name, url, sha1, path;
  bool native=false;
};
struct VersionManifest {
  std::string id, type, mainClass, assets, assetIndexId, assetIndexUrl, assetIndexSha1;
  std::string clientUrl, clientSha1, javaMajor;
  std::vector<Library> libraries;
  std::vector<std::string> jvmArguments, gameArguments;
};
class ManifestManager {
public:
  static bool load(const std::filesystem::path& file, VersionManifest& out, std::string& error);
  static bool save(const std::filesystem::path& file, const VersionManifest& manifest, std::string& error);
};
}
